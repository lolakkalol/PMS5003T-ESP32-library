#include <Arduino.h>
#include "PMS5003T.h"

PMS5003T* pms5003t;

void setup() {
    Serial.begin(115200);
    while (!Serial){}

    pms5003t = new PMS5003T(13, 15);
}

void loop() {
    PMS5003T_STATUS status = pms5003t->receive_data();
    sleep(2);
}