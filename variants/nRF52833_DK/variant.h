#ifndef _VARIANT_NRF52833_DK_
#define _VARIANT_NRF52833_DK_

// Master clock frequency
#define VARIANT_MCK (64000000ul)

#include "WVariant.h"

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

// Number of pins defined in PinDescription array

#define PINS_COUNT			(42)
#define NUM_DIGITAL_PINS	(42)
#define NUM_ANALOG_INPUTS	(6)
#define NUM_ANALOG_OUTPUTS	(0)

// LEDs
#define PIN_LED1            (13)
#define PIN_LED2            (14)
#define PIN_LED3            (15)
#define PIN_LED4            (16)
#define LED_BUILTIN         PIN_LED1
#define LED_STATE_ON        (0)


// Buttons
#define PIN_BUTTON1         (11)
#define PIN_BUTTON2         (12)
#define PIN_BUTTON3         (24)
#define PIN_BUTTON4         (25)

// Analog pins
#define PIN_AREF            (2)
#define PIN_A0				(3)
#define PIN_A1				(4)
#define PIN_A2				(28)
#define PIN_A3				(29)
#define PIN_A4				(30)
#define PIN_A5				(31)
#define PIN_A6              (0xff)
#define PIN_A7              (0xff)
#define ADC_RESOLUTION 14

static const uint8_t A0 = PIN_A0;
static const uint8_t A1 = PIN_A1;
static const uint8_t A2 = PIN_A2;
static const uint8_t A3 = PIN_A3;
static const uint8_t A4 = PIN_A4;
static const uint8_t A5 = PIN_A5;

// Serial
#define PIN_SERIAL_RTS      (5)
#define PIN_SERIAL_TX		(6)
#define PIN_SERIAL_CTS      (7)
#define PIN_SERIAL_RX		(8)

// SPI
#define SPI_INTERFACES_COUNT 1

#define PIN_SPI_MISO		(22)
#define PIN_SPI_MOSI		(21)
#define PIN_SPI_SCK			(33)

// Wire Interfaces (external and internal)
#define WIRE_INTERFACES_COUNT 1

#define PIN_WIRE_SDA		(30)
#define PIN_WIRE_SCL		(31)

// Other
#define PIN_NFC1            (9)
#define PIN_NFC2            (10)
#define RESET_PIN           (18)

#ifdef __cplusplus
}
#endif // __cplusplus

#endif
