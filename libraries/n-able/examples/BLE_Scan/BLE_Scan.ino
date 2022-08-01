/**
 * Example of continuous scanning for BLE advertisements.
 * This example will scan for scanTime (seconds) and report all advertisements on the serial monitor.
 *
 * Note the configuration options in the build_opt.h file.
 * More BLE examples can be found in the NimBLE-Arduino examples: https://github.com/h2zero/NimBLE-Arduino.
 *
 * For further BLE documentation see: https://h2zero.github.io/NimBLE-Arduino
 */

#include <Arduino.h>
#include "NimBLEDevice.h"

NimBLEScan* pBLEScan;
uint32_t scanTime = 30; // Scan duration in seconds (0 = forever)

// Callback class for received advertisements
class MyAdvertisedDeviceCallbacks: public NimBLEAdvertisedDeviceCallbacks {
    void onResult(NimBLEAdvertisedDevice* advertisedDevice) {
      Serial.printf("Advertised Device: %s \n", advertisedDevice->toString().c_str());
    }
};

void setup() {
  Serial.begin(115200);
  Serial.println("Scanning...");

  // Initialize the BLE stack
  NimBLEDevice::init("");

  // Create new scan instance
  pBLEScan = NimBLEDevice::getScan();

  // Set the callback for when devices are discovered, no duplicates.
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks(), false);

  // Set active scanning, this will get scan response data from the advertiser.
  pBLEScan->setActiveScan(true);

  // Set how often the scan occurs/switches channels; in milliseconds,
  pBLEScan->setInterval(97);

  // How long to scan during the interval; in milliseconds.
  pBLEScan->setWindow(37);

  // Do not store the scan results, use callback only.
  pBLEScan->setMaxResults(0);
}

void loop() {
  // When the scan stops, restart it. This will cause duplicate devices to be reported again.
  if(pBLEScan->isScanning() == false) {
      // Start scan with: duration = scanTime (seconds), no scan ended callback, not a continuation of a previous scan.
      pBLEScan->start(scanTime, nullptr, false);
  }

  // Short delay to allow the stack to reset states.
  delay(100);
}