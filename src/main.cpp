#include <Arduino.h>
#include "PMS5003T.h"

PMS5003T* pms5003t;

size_t count = 0;

void setup() {
    Serial.begin(115200);
    while (!Serial){}
    
    pms5003t = new PMS5003T(13, 15);

    uint8_t data[2] = {0x00, 0x00};
    pms5003t->send_command((uint8_t) PMS5003T_CMD::CHANGE_MODE, data);
}

void loop() {
    uint8_t data[2] = {0x00, 0x00};
    pms5003t->send_command((uint8_t) PMS5003T_CMD::READ_REQUEST_PASSIVE, data);
    pms5003t->receive_data();

    Serial.print("PM1.0 concentration: ");
    Serial.println( pms5003t->get_PM25_std() );
    sleep(2);
}