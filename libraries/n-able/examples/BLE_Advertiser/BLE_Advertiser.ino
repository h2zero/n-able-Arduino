/**
 * Example of advertising service data.
 *
 * This example will start advertising service data for a custom UUID, the data value of which
 * will increment by one every 5 seconds.
 *
 * Note the configuration options in the build_opt.h file.
 * More BLE examples can be found in the NimBLE-Arduino examples: https://github.com/h2zero/NimBLE-Arduino.
 *
 * For further BLE documentation see: https://h2zero.github.io/NimBLE-Arduino
 */

#include <Arduino.h>
#include <NimBLEDevice.h>

#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"

static NimBLEUUID dataUuid(SERVICE_UUID);
static NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
static uint32_t count = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("Starting Advertising!");

  // Initialize the stack
  NimBLEDevice::init("NimBLE");
}

void loop() {
  if (!pAdvertising->isAdvertising()) {
    // Update the advertised data
    pAdvertising->setServiceData(dataUuid, std::string((char*)&count, sizeof(count)));

    // Start advertising the data
    pAdvertising->start(5);
    Serial.printf("Advertising count = %d\n", count);
    count++;
  }

  // Short delay to allow the stack to reset states.
  delay(100);
}