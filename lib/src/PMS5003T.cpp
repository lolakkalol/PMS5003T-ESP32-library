#include <Arduino.h>
#include "PMS5003T.h"

PMS5003T::PMS5003T(size_t rx_pin, size_t tx_pin) {
    this->Serial_p = new SoftwareSerial(rx_pin, tx_pin, false);
    this->Serial_p->setTimeout(5);
    this->Serial_p->begin(9600);
    this->PM10_std  = 0;
    this->PM25_std  = 0;
    this->PM100_std = 0;
    this->PM10_atm  = 0;
    this->PM25_atm  = 0;
    this->cns_atm   = 0;
    this->part_03   = 0;
    this->part_05   = 0;
    this->part_10   = 0;
    this->part_25   = 0;
    this->part_50   = 0;
    this->part_100  = 0;
}

PMS5003T::~PMS5003T() {
    free(this->Serial_p);
}

#if SERIAL_DEBUG
PMS5003T_STATUS PMS5003T::receive_data() {
    Serial.println("-------------------------------------------------------------");
    PMS5003T_STATUS status = this->receive_data_hook();
    Serial.print("STATUS: ");
    Serial.println((uint16_t) status);
    Serial.print("PM1.0 concentration unit μ g/m3 (CF=1. standard particle): ");
    Serial.println(this->get_PM10_std());
    Serial.print("PM2.5 concentration unit μ g/m3 (CF=1. standard particle) : ");
    Serial.println(this->get_PM25_std());
    Serial.print("PM10  concentration unit μ g/m3 (CF=1. standard particle) : ");
    Serial.println(this->get_PM100_std());
    Serial.print("PM1.0 concentration unit μ g/m3 (under atmospheric environment): ");
    Serial.println(this->get_PM10_atm());
    Serial.print("PM2.5 concentration unit μ g/m3 (under atmospheric environment): ");
    Serial.println(this->get_PM25_atm());
    Serial.print("concentration unit (under atmospheric environment) μ g/m3 (Might be PM10?): ");
    Serial.println(this->get_cns_atm());
    Serial.print("the number of particles with diameter beyond 0.3 um in 0.1 L of air: ");
    Serial.println(this->get_part_03());
    Serial.print("the number of particles with diameter beyond 0.5 um in 0.1 L of air: ");
    Serial.println(this->get_part_05());
    Serial.print("the number of particles with diameter beyond 1.0 um in 0.1 L of air: ");
    Serial.println(this->get_part_10());
    Serial.print("the number of particles with diameter beyond 2.5 um in 0.1 L of air: ");
    Serial.println(this->get_part_25());
    Serial.print("the number of particles with diameter beyond 5.0 um in 0.1 L of air: ");
    Serial.println(this->get_part_50());
    Serial.print("the number of particles with diameter beyond 10 um in 0.1 L of air: ");
    Serial.println(this->get_part_100());

    return status;
}

PMS5003T_STATUS PMS5003T::receive_data_hook() {
#else
PMS5003T_STATUS PMS5003T::receive_data() {
#endif

    // receive data
    char packet[32];

    while(this->Serial_p->available() < 32) {
        sleep(1);
    }
    this->Serial_p->readBytes(packet, 32);

    #ifdef SERIAL_DEBUG
    Serial.print("Message: ");
    for (size_t i = 0; i < 32; i++) {
        if (packet[i] < 0x10) {
            Serial.print("0");
        }

        Serial.print(String(packet[i], HEX));
        Serial.print(" ");
    }

    Serial.println();
    #endif

    // Checking data integrety
    PMS5003T_STATUS status = this->check_packet_validity(packet);
    if (status != PMS5003T_STATUS::OK) {
        return status;
    }

    // Deconstruct packet and save in it self
    this->unpack_into_struct(packet);

    return PMS5003T_STATUS::OK;
}

void PMS5003T::unpack_into_struct(char* packet) {
    this->PM10_std  = packet[4] << 8 | packet[5];
    this->PM25_std  = packet[6] << 8 | packet[7];
    this->PM100_std = packet[8] << 8 | packet[9];
    this->PM10_atm  = packet[10] << 8 | packet[11];
    this->PM10_atm  = packet[12] << 8 | packet[13];
    this->cns_atm   = packet[14] << 8 | packet[15];
    this->part_03   = packet[16] << 8 | packet[17];
    this->part_05   = packet[18] << 8 | packet[19];
    this->part_10   = packet[20] << 8 | packet[21];
    this->part_25   = packet[22] << 8 | packet[23];
    this->part_50   = packet[24] << 8 | packet[25];
    this->part_100  = packet[26] << 8 | packet[27];
}

PMS5003T_STATUS PMS5003T::check_packet_validity(char* packet, size_t n /*= 32*/) {
    uint16_t check_bytes = packet[30] << 8 | packet[31];

    // Check start bytes
    if (packet[0] != 0x42 || packet[1] != 0x4d) {
        return PMS5003T_STATUS::STARTBIT_ERROR;
    }

    // Check check sum
    #if SERIAL_DEBUG 
    Serial.print("Read checsum: ");
    Serial.print(String(packet[30], HEX));
    Serial.println(String(packet[31], HEX));
    #endif
    if (this->calculate_check(packet, 30) != check_bytes) {
        return PMS5003T_STATUS::CHECKBIT_ERROR;
    }

    return PMS5003T_STATUS::OK;
}

/**
 * @brief Calculates the 2 byte checksum as per the PMS5003T's datasheet (Summation of all bytes).
*/
uint16_t PMS5003T::calculate_check(char* packet, size_t n) {
    uint16_t sum = 0;

    for (int i = 0; i < n; i++) {
        sum += packet[i];
    }

    #if SERIAL_DEBUG
    Serial.print("Calculated checksum: ");
    Serial.println(String(sum, HEX));
    #endif

    return sum;
}

/* Getters */
uint16_t PMS5003T::get_PM10_std() {
    return this->PM10_std;
}
uint16_t PMS5003T::get_PM25_std() {
    return this->PM25_std;
}
uint16_t PMS5003T::get_PM100_std() {
    return this->PM100_std;
}
uint16_t PMS5003T::get_PM10_atm() {
    return this->PM10_atm;
}
uint16_t PMS5003T::get_PM25_atm() {
    return this->PM25_atm;
}
uint16_t PMS5003T::get_cns_atm() {
    return this->cns_atm;
}
uint16_t PMS5003T::get_part_03() {
    return this->part_03;
}
uint16_t PMS5003T::get_part_05() {
    return this->part_05;
}
uint16_t PMS5003T::get_part_10() {
    return this->part_10;
}
uint16_t PMS5003T::get_part_25() {
    return this->part_25;
}
uint16_t PMS5003T::get_part_50() {
    return this->part_50;
}
uint16_t PMS5003T::get_part_100() {
    return this->part_100;
}