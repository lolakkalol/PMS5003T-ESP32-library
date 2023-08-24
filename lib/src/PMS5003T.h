#ifndef PMS5003T_H
#define PMS5003T_H

#include "SoftwareSerial.h"

#define SERIAL_DEBUG (1) // Prints a bunch of debug info to usb serial. Requires that Serial.begin(baud) is run before!

enum class PMS5003T_STATUS {
    OK,
    GENERIC_ERROR,
    STARTBIT_ERROR,
    CHECKBIT_ERROR
};

enum class PMS5003T_CMD {
    READ_REQUEST_PASSIVE = 0xe2,
    CHANGE_MODE          = 0xe1,
    SLEEP_SET            = 0xe4,
};

enum class CMD_ARG {
    SLEEP   = 0x00,
    WAKEUP  = 0x01,
    PASSIVE = 0x00,
    ACTIVE  = 0x01,
};

class PMS5003T
{
private:
    SoftwareSerial* Serial_p;
    uint16_t PM10_std;   // PM1.0 concentration unit μ g/m3 (CF=1，standard particle)
    uint16_t PM25_std;   // PM2.5 concentration unit μ g/m3 (CF=1，standard particle) 
    uint16_t PM100_std;  // PM10  concentration unit μ g/m3 (CF=1，standard particle) 
    uint16_t PM10_atm;   // PM1.0 concentration unit μ g/m3 (under atmospheric environment)
    uint16_t PM25_atm;   // PM2.5 concentration unit μ g/m3 (under atmospheric environment)
    uint16_t cns_atm;    // concentration unit (under atmospheric environment) μ g/m3 (Might be PM10?)
    uint16_t part_03;    // the number of particles with diameter beyond 0.3 um in 0.1 L of air
    uint16_t part_05;    // the number of particles with diameter beyond 0.5 um in 0.1 L of air
    uint16_t part_10;    // the number of particles with diameter beyond 1.0 um in 0.1 L of air
    uint16_t part_25;    // the number of particles with diameter beyond 2.5 um in 0.1 L of air
    uint16_t part_50;    // the number of particles with diameter beyond 5.0 um in 0.1 L of air
    uint16_t part_100;   // the number of particles with diameter beyond 10 um in 0.1 L of air

    #if SERIAL_DEBUG
    PMS5003T_STATUS receive_data_hook();
    #endif

    uint16_t calculate_check(char* packet, size_t n);
    PMS5003T_STATUS check_packet_validity(char* packet, size_t n = 30);
    void unpack_into_struct(char* packet);

public:
    PMS5003T(size_t rx_pin, size_t tx_pin);
    ~PMS5003T();
    PMS5003T_STATUS receive_data();
    PMS5003T_STATUS send_command(uint8_t cmd, uint8_t* data);

    // Getters
    uint16_t get_PM10_std();
    uint16_t get_PM25_std();
    uint16_t get_PM100_std();
    uint16_t get_PM10_atm();
    uint16_t get_PM25_atm();
    uint16_t get_cns_atm();
    uint16_t get_part_03();
    uint16_t get_part_05();
    uint16_t get_part_10();
    uint16_t get_part_25();
    uint16_t get_part_50();
    uint16_t get_part_100();
};


#endif