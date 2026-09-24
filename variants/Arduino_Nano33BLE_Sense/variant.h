#ifndef _ARDUINO_NANO33BLE_SENSE_H_
#define _ARDUINO_NANO33BLE_SENSE_H_

#define TARGET_ARDUINO_NANO33BLE_SENSE

/** Master clock frequency */
#define VARIANT_MCK       (64000000ul)

#define USE_LFXO      // Board uses 32khz crystal for LF
//#define USE_LFRC    // Board uses RC for LF

/*----------------------------------------------------------------------------
 *        Headers
 *----------------------------------------------------------------------------*/

#include "WVariant.h"

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#define PINS_COUNT              (34)
#define NUM_DIGITAL_PINS        (34)
#define NUM_ANALOG_INPUTS       (8)
#define NUM_ANALOG_OUTPUTS      (0)

// LEDs
#define PIN_LED                 (13u)
#define LED_BUILTIN             PIN_LED

#define LED_RED                 (22u)
#define LED_GREEN               (23u)
#define LED_BLUE                (24u)
#define LED_PWR                 (25u)

#define LED_STATE_ON            (1)     // State when LED is litted

// Buttons
#define PIN_BUTTON1             (PINS_COUNT)

// Digital pins
static const uint8_t D0  = 0;
static const uint8_t D1  = 1;
static const uint8_t D2  = 2;
static const uint8_t D3  = 3;
static const uint8_t D4  = 4;
static const uint8_t D5  = 5;
static const uint8_t D6  = 6;
static const uint8_t D7  = 7;
static const uint8_t D8  = 8;
static const uint8_t D9  = 9;
static const uint8_t D10 = 10;
static const uint8_t D11 = 11;
static const uint8_t D12 = 12;
static const uint8_t D13 = 13;

// Analog pins
#define PIN_A0                  (14u)
#define PIN_A1                  (15u)
#define PIN_A2                  (16u)
#define PIN_A3                  (17u)
#define PIN_A4                  (18u)
#define PIN_A5                  (19u)
#define PIN_A6                  (20u)
#define PIN_A7                  (21u)

static const uint8_t A0  = PIN_A0;
static const uint8_t A1  = PIN_A1;
static const uint8_t A2  = PIN_A2;
static const uint8_t A3  = PIN_A3;
static const uint8_t A4  = PIN_A4;
static const uint8_t A5  = PIN_A5;
static const uint8_t A6  = PIN_A6;
static const uint8_t A7  = PIN_A7;

#define ADC_RESOLUTION          (12)

// Serial interfaces
#ifndef USB_CDC_DEFAULT_SERIAL
  #define USB_CDC_DEFAULT_SERIAL (1)
#endif

#if USB_CDC_DEFAULT_SERIAL
  #define PIN_SERIAL1_TX         (0)
  #define PIN_SERIAL1_RX         (1)
#else
  #define PIN_SERIAL_TX          (0)
  #define PIN_SERIAL_RX          (1)
#endif

// SPI Interfaces
#define SPI_INTERFACES_COUNT    (1)

#define PIN_SPI_MISO            (12)
#define PIN_SPI_MOSI            (11)
#define PIN_SPI_SCK             (13)

static const uint8_t SS   = 10;
static const uint8_t MOSI = PIN_SPI_MOSI;
static const uint8_t MISO = PIN_SPI_MISO;
static const uint8_t SCK  = PIN_SPI_SCK;

// Wire Interfaces
#define WIRE_INTERFACES_COUNT   (2)

// External I2C (A4/A5)
#define PIN_WIRE_SDA            (18u)
#define PIN_WIRE_SCL            (19u)

static const uint8_t SDA = PIN_WIRE_SDA;
static const uint8_t SCL = PIN_WIRE_SCL;

// Internal I2C for onboard sensors
#define PIN_WIRE1_SDA           (30u)
#define PIN_WIRE1_SCL           (31u)

// APDS9960 interrupt pin
#define PIN_INT_APDS            (26u)

// PDM microphone (MP34DT05)
#define PIN_PDM_PWR             (27u)
#define PIN_PDM_CLK             (28u)
#define PIN_PDM_DIN             (29u)

// Internal power control pins
#define PIN_ENABLE_I2C_PULLUP   (32u)
#define PIN_ENABLE_SENSORS_3V3  (33u)

#ifdef __cplusplus
}
#endif

/*----------------------------------------------------------------------------
 *        Arduino objects - C++ only
 *----------------------------------------------------------------------------*/

#endif
