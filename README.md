# n-able Arduino core

An Arduino Core for ARM based BLE boards supported by the NimBLE stack.

**This core is currently still in development and not yet available through the Arduino board manager. Manual installation, described below, is required to use this core.**

If you are interested in assisting the development of this project please submit issues and PR's. Or become a collaborator!

## Features

* Fully supports the [NimBLE-Arduino](https://github.com/h2zero/NimBLE-Arduino) library on Arm based BLE devices (only Nordic mcu's for now)
* No softdevice, NimBLE provides a complete BLE stack.
* Contains a subset of the EEPROM library to support basic config storage in flash.
* Supports `printf` and `Serial.printf`
* Uses FreeRTOS.
* Designed to maximize battery life using tickless idle. This allows the MCU to sleep any time `delay` is called.
* Custom configuration of FreeRTOS and NimBLE compile options supported by providing a `build_opt.h` file in your sketch.

## Why do this?

I wanted to have a consistent BLE API on all of the devices I work with. NimBLE is the best choice for this as it is the most feature complete and fully open source library available for Arduino.

## BLE

This Arduino Core does **not** contain any BLE functionality. It has been designed to support using the [NimBLE-Arduino](https://github.com/h2zero/NimBLE-Arduino) library for BLE operation.
**Note:** Currently only the master branch of NimBLE-Arduino supports Arm devices and no releases have been made yet with Arm support.

## Installation

 1. Go to your Arduino folder:
  * OS X: ```~/Documents/Arduino```
  * Linux: ```~/Arduino```
  * Windows: ```~/Documents/Arduino```
 2. Create a folder named ```hardware```, if it does not exist, and change directories to it
 3. Clone this repo: ```git clone https://github.com/h2zero/n-able.git n-able/arm-ble```
 4. Download the tools: (these need to be manually downloaded/installed for now)
  * GCC:
    * [OS X](https://developer.arm.com/-/media/Files/downloads/gnu-rm/9-2020q2/gcc-arm-none-eabi-9-2020-q2-update-mac.tar.bz2?revision=d0d318de-b746-489f-98b0-9d89648ce910&hash=601DBEFBB7540F434D54B7BFF7890875)
    * [Linux](https://developer.arm.com/-/media/Files/downloads/gnu-rm/9-2020q2/gcc-arm-none-eabi-9-2020-q2-update-x86_64-linux.tar.bz2?revision=05382cca-1721-44e1-ae19-1e7c3dc96118&hash=AE874AE7513FAE5077350E4E23B1AC08)
    * [Windows](https://developer.arm.com/-/media/Files/downloads/gnu-rm/9-2020q2/gcc-arm-none-eabi-9-2020-q2-update-win32.zip?revision=95631fd0-0c29-41f4-8d0c-3702650bdd74&hash=1223A90FC1ACF69AD82680A87E0AEDD0)
  * OpenOCD:
    * [OS X](https://github.com/xpack-dev-tools/openocd-xpack/releases/download/v0.11.0-1/xpack-openocd-0.11.0-1-darwin-x64.tar.gz)
    * [Linux](https://github.com/xpack-dev-tools/openocd-xpack/releases/download/v0.11.0-1/xpack-openocd-0.11.0-1-linux-x64.tar.gz)
    * [Windows](https://github.com/xpack-dev-tools/openocd-xpack/releases/download/v0.11.0-1/xpack-openocd-0.11.0-1-win32-x64.zip)
 5. Extract OpenOCD and copy the contents to ```<ARDUINO_FOLDER>/hardware/n-able/arm-ble/tools/openocd```
 6. Extract GCC and copy the contents to ```<ARDUINO_FOLDER>/hardware/n-able/arm-ble/tools/gcc-arm-none-eabi/9_0_2020q2```
 7. Restart the Arduino IDE

### Driver Setup for Segger J-Link

 1. Download [Zadig](http://zadig.akeo.ie)
 2. Plugin Segger J-Link or DK board
 3. Start ```Zadig```
 4. Select ```Options -> List All Devices```
 5. Plug and unplug your device to find what changes, and select the ```Interface 2``` from the device dropdown
 6. Click ```Replace Driver```

__NOTE__: To roll back to the original driver go to: Device Manager -> Right click on device -> Check box for "Delete the driver software for this device" and click Uninstall

### Flashing your device

1. Select your board from the Tools -> Board menu
2. Select any options you want
3. Select a programmer (J-Link, ST-Link V2, CMSIS-DAP, or Black Magic Probe) from the Tools -> "Programmer: " menu
4. Connect your programmer to the device and click Upload

**Note:** Currently no bootloader is provided or required.
Instead you will need to erase any bootloader currently installed on your device. To do this, select your board from the board menu and click "Burn Bootloader", this will completely erase the device flash memory.

## Credits

This core is based on [Arduino-nRF5](https://github.com/sandeepmistry/arduino-nRF5) by Sandeep Mistry,
which in turn is based on the [Arduino SAMD Core](https://github.com/arduino/ArduinoCore-samd).

The following libraries are used:

- [FreeRTOS](https://www.freertos.org/) as operating system
- [nrfx](https://github.com/NordicSemiconductor/nrfx) for peripherals driver
- [TinyUSB](https://github.com/hathach/tinyusb) as usb stack
