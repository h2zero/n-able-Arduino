/*
 * n-able EEPROM demo
*/
#include <Arduino.h>
#include <EEPROM.h>

void setup() {
    Serial.begin(115200);
    EEPROM.begin();

    uint32_t item = 0;
    uint32_t count = 0;

    Serial.println("EEPROM Demo started, press a key to see the bootcount.");

    // wait for user input
    while(!Serial.available()) {
      delay(10);
    }

    EEPROM.get(item, count);
    Serial.printf("Bootcount = %u\n", count);

    count++;
    EEPROM.put(item, count);

    systemRestart();
}

void loop() {
}
