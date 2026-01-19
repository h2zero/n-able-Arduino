# Changelog

All notable changes to this project will be documented in this file.

## [0.3.1] - 2026-01-18

### Fixed
- Use correct linker script when adafruit bootloader is selected

## [0.3.0] - 2025-09-05

### Fixed
 - Arduino build with nRF52810 boards.
 - potential endless loop in Wire Library.
 - PCA20006 pinmap.
 - Prevent possible endless loop in Uart::write/Serial::print.
 - Increase HCI buffer size when extended advertising is enabled.

### Changed
 - Exposed hw_clock_hfxo_request and int hw_clock_hfxo_release for application use.
 - Disable DLE if not using connections.
 - Increase msys_1 blocks when privacy is enabled.
 - Print::printf implementation now uses write to allow output redirection.
 - Make PHY options configurable without extended advertising.

### Added
 - Config option for BLE secure connections.
 - Support for Seeed xiao sense board.

## [0.2.0] 2024-06-28

### Fixed
 - min/max macros changed to fix compilation issues in some cases.
 - ADC value is now volatile to prevent the compiler from optimizing it and always returning 0.
 - min definition added to fix error when compiling with extended advertising enabled.
 - PHY can now be correctly set when using extended advertising.
 - System will no longer hang when printing to UART when it hasn't been started yet.
 - Building on Windows when no DFU bin is being built will no longer cause an error.

### Changed
 - Use PRIx macros to remove compiler warnings.
 - Watchdog timer is now disabled by default, users should enable manually.
 - pulse assembly code replace with C code to resolve linker errors on Micro:Bit V2.

### Added
 - `pins_arduino.h` file to be more compatible with some libraries.
 - Virtual methods `flush`, `availableForWrite` added to `Print` for compatibility.
 - New constructor for `String` that takes a char* and length parameter.
 - Support for nRF52810.
 - Coded and/or 2M PHY for nRF52832, nRF52833, nRF52810 devices.
 - Support for NimBLE core 1.5.0

## [0.1.1] - 2022-08-26

### Fixed
 - Platformio build instructions.
 - Fixed building Adafruit nRF52840 base boards.

### Added
 - GitHub actions builds

## [0.1.0] - 2022-08-06

### INITIAL RELEASE
