/**
 * @file PMS5003T.cpp
 * @author Alexander Stenlund (alexander.stenlund@telia.com)
 * @brief All implementations of methods for the class PMS5003T 
 * is located here. These methods are used to communicate with 
 * a PMS5003T sensor using software serial.
 * @version 0.1
 * @date 2023-08-24
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#include <Arduino.h>
#include "PMS5003T.h"

/**
 * @brief A constructor for the creation of a object that handles
 *  the serial communication with a PMS5003T sensor. This object 
 * uses software serial to communicate so the pins only need to 
 * be avaliable and digital.
 * @param rx_pin A GPIO pin connected to the sensors TX pin.
 * @param tx_pin A GPIO pin connected to the sensors RX pin.
*/
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

/**
 * @brief Sends a command to the sensor.
 * @param cmd The command to be sent to the sensor, see sensor datasheet
 *  for codes or PMS5003T_CMD in pms5003t.h.
*/
PMS5003T_STATUS PMS5003T::send_command(uint8_t cmd, uint8_t* data) {
    char packet[7] = {0x42, 0x4d, cmd, data[0], data[1], 0, 0};

    uint16_t check_bytes = this->calculate_check(packet, 7);

    packet[5] = (char) (check_bytes >> 8);
    packet[6] = (char) (check_bytes & 0x00FF);

    #if SERIAL_DEBUG
    Serial.print("Packet to send: ");
    for (int i = 0; i < 7; i++) {
        Serial.print(String(packet[i], HEX));
        Serial.print(" ");
    }
    Serial.println();
    #endif

    while(!this->Serial_p->availableForWrite()){
        sleep(0.1);
    }

    this->Serial_p->write(packet, 7);
    this->Serial_p->flush();

    return PMS5003T_STATUS::OK;
}

#if SERIAL_DEBUG
/**
 * @brief Only used when SERIAL_DEBUG is set to 1. Re defines the receive_data() 
 * method as to be able to print debug messages, like the status output of the method.
*/
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
/**
 * @brief Waits for data from the sensor to be put on the bus and converts it
 *  into usable values which are store in the caller object.
*/
PMS5003T_STATUS PMS5003T::receive_data() {
#endif

    // receive data
    char packet[32];

    while(this->Serial_p->available() < 32) {
        sleep(1);
    }
    this->Serial_p->readBytes(packet, 32);

    #if SERIAL_DEBUG
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

/**
 * @brief Unpackes the `packet` and saves into itself.
 * @param packet The packet received from the sensor unmodified.
*/
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

/**
 * @brief Sends a request to the sensor to send back data. Only used
 * when sensor is in passive mode.
 * @returns Returns a error code acording to PMS5003T.h.
*/
PMS5003T_STATUS PMS5003T::request_data() {
    uint8_t data[2] = {0x00, 0x00};
    this->send_command((uint8_t) PMS5003T_CMD::READ_REQUEST_PASSIVE, data);
    return PMS5003T_STATUS::OK;
}

/**
 * @brief Sets the sensor to passive or active mode.
 * @param enable if true sets the mode to passive, if false sets the mode to active.
 * @returns Returns a error code acording to PMS5003T.h.
*/
PMS5003T_STATUS PMS5003T::passive_mode(bool enable) {
    uint8_t data[2] = {0x00, enable ? 0x00 : 0x01};
    this->send_command((uint8_t) PMS5003T_CMD::CHANGE_MODE, data);
    return PMS5003T_STATUS::OK;
}

/**
 * @brief Commands the sensor to go into sleep or wakes it up.
 * @param enable if true sets the sensor to sleep, if false wakes the sensor up.
 * @returns Returns a error code acording to PMS5003T.h.
*/
PMS5003T_STATUS PMS5003T::sleep(bool enable) {
    uint8_t data[2] = {0x00, enable ? 0x00 : 0x01};
    this->send_command((uint8_t) PMS5003T_CMD::SLEEP_SET, data);
    return PMS5003T_STATUS::OK;
}

/**
 * @brief Instead of having to call `request_data()` and then `receive_data()` this 
 * method, sends a request for data, receives the data and saves it in the 
 * object it was called from. Only use if sensor is in passive mode, otherwise use 
 * `receive_data()`.
 * @returns Returns a error code acording to PMS5003T.h.
*/
PMS5003T_STATUS PMS5003T::update_data() {
    PMS5003T_STATUS status;
    
    status = this->request_data();
    if (status != PMS5003T_STATUS::OK) {
        return status;
    }
    
    status = this->receive_data();
    if (status != PMS5003T_STATUS::OK) {
        return status;
    }

    return PMS5003T_STATUS::OK;
}

/**
 * @brief Check that the start and check bytes are acording to PMS5003T's datasheet.
 * @param packet The packet to check, including start and check bits.
 * @param n The number of bytes in `packet`, defaults to 32.
*/
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