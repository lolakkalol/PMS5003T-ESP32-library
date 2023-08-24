# Interface library for the PMS5003T particle sensor
This library attempts to abstract away the communication of between the MCU and the sensor and make it easy to get values from it.

## Description
This library is written using the Arduino framework and a ESP32 Devkit C but should work on any ESP32 as long as it is configured correctly.
Using software define serial communication the ESP32 communicates to the PMS5003T.

## Getting Started
Connect the pins of the PMS5003T and the ESP as follows:

| ESP32 | PMS5003T |
| ------------- | ------------- |
| 5V | VCC |
| GND | GND |
| NC | SET |
| TX (digital GPIO)<sup>[1](#footnote1)</sup> | RX |
| RX (digital GPIO)<sup>[1](#footnote1)</sup> | TX |
| NC | RESET |
| NC | NC |

<a name="footnote1">1</a>: Do not use the actual TX/RX pins used for the USB port, select two digital GPIO pins that are free for example 13 and 15 and connect them to RX/TX on the PMS5003T. These GPIO pins are later used when configuring the library.

Start a new projectIO project for your board and import the EspSoftwareSerial libray by adding `lib_deps = plerup/EspSoftwareSerial@^8.1.0` to your `platformio.ini`, then copy the `lib` folder from github into your projectIO project.
The library is now ready to be used just instatiate a `PMS5003T` object and with the GPIO pins you decided to use as TX/RX, and call the objects methods to interract with the sensor!

You can alternatevly clone the github repository, open it in platformIO and change `platformio.ini` to fit your use case.

### Small example
```c++
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
```

### Dependencies

* ESP-IDF
* Arduino framework
* [ESPSoftwareSerial](https://github.com/plerup/espsoftwareserial)

## Help
