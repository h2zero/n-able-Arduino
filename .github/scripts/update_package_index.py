#!/usr/bin/env python3
"""
Update the Arduino board package index with a new release version.
"""
import json
import os
import sys


def main():
    version = os.environ.get('VERSION')
    archive_name = os.environ.get('ARCHIVE_NAME')
    checksum = os.environ.get('CHECKSUM')
    size = os.environ.get('SIZE')
    
    if not all([version, checksum, size]):
        print("Error: Missing required environment variables")
        print(f"VERSION={version}, CHECKSUM={checksum}, SIZE={size}")
        sys.exit(1)
    
    # Load existing package index
    with open('package_n-able_boards_index.json', 'r') as f:
        package_data = json.load(f)
    
    # New platform entry
    new_platform = {
        "name": "Arm (Nim)BLE Boards",
        "architecture": "arm-ble",
        "version": version,
        "category": "Contributed",
        "help": {
            "online": "https://github.com/h2zero/n-able-Arduino/issues"
        },
        "url": f"https://github.com/h2zero/n-able-Arduino/archive/refs/tags/{version}.tar.gz",
        "archiveFileName": f"n-able-Arduino-{version}.tar.gz",
        "checksum": f"SHA-256:{checksum}",
        "size": str(size),
        "boards": [
            {"name": "Adafruit CLUE nRF52840"},
            {"name": "Adafruit Circuit Playground Bluefruit"},
            {"name": "Adafruit Feather nRF52832"},
            {"name": "Adafruit Feather nRF52840 Express"},
            {"name": "Adafruit Feather nRF52840 Sense"},
            {"name": "Adafruit ItsyBitsy nRF52840 Express"},
            {"name": "BBC micro:bit"},
            {"name": "BBC micro:bit v2"},
            {"name": "Bluz DK"},
            {"name": "Calliope mini"},
            {"name": "Ebyte E104-BT5032A-TB"},
            {"name": "Ebyte E104-BT5040UA Dongle"},
            {"name": "Electronut labs bluey"},
            {"name": "Electronut labs hackaBLE"},
            {"name": "Electronut labs hackaBLE v2"},
            {"name": "Generic nRF51822"},
            {"name": "Generic nRF52810"},
            {"name": "Generic nRF52832"},
            {"name": "Generic nRF52833"},
            {"name": "Generic nRF52840"},
            {"name": "ng-beacon"},
            {"name": "nRF51 Dongle"},
            {"name": "nRF51822 DK"},
            {"name": "nRF52832 DK"},
            {"name": "nRF52833 DK"},
            {"name": "nRF52840 DK"},
            {"name": "nRF52840 Dongle"},
            {"name": "Nordic Beacon Kit"},
            {"name": "OSHChip"},
            {"name": "RedBear BLE Nano"},
            {"name": "RedBear BLE Nano 2"},
            {"name": "RedBear Blend 2"},
            {"name": "RedBear nRF51822"},
            {"name": "Sino:bit"},
            {"name": "TinyBLE"},
            {"name": "Waveshare BLE400"},
            {"name": "Seeed XIAO nRF52840 Sense"}
        ],
        "toolsDependencies": [
            {
                "packager": "h2zero",
                "name": "gcc-arm-none-eabi",
                "version": "9.3.1-1"
            },
            {
                "packager": "h2zero",
                "name": "openocd",
                "version": "0.11.0-4"
            }
        ]
    }
    
    # Check if version already exists and update or append
    found = False
    for platform in package_data['packages'][0]['platforms']:
        if platform['version'] == version:
            # Update existing version
            platform.update(new_platform)
            found = True
            print(f"Updated existing package index entry for version {version}")
            break
    
    if not found:
        # Append new version (maintaining reverse chronological order)
        package_data['packages'][0]['platforms'].insert(0, new_platform)
        print(f"Added new package index entry for version {version}")
    
    # Write updated package index
    with open('package_n-able_boards_index.json', 'w') as f:
        json.dump(package_data, f, indent=2)
    
    print(f"Successfully updated package index")


if __name__ == '__main__':
    main()
