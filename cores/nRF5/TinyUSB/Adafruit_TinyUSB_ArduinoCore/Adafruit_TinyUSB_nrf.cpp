/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019, hathach for Adafruit
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "tusb_option.h"

#if CFG_TUD_ENABLED

#include "nrfx.h"
#include "nrfx_power.h"

#include "Arduino.h"
#include "Adafruit_USBD_Device.h"

//--------------------------------------------------------------------+
// MACRO TYPEDEF CONSTANT ENUM DECLARATION
//--------------------------------------------------------------------+

#define USBD_STACK_SZ (200)

//--------------------------------------------------------------------+
// Forward USB interrupt events to TinyUSB IRQ Handler
//--------------------------------------------------------------------+
extern "C" void USBD_IRQHandler(void) {
#if CFG_SYSVIEW
  SEGGER_SYSVIEW_RecordEnterISR();
#endif

  tud_int_handler(0);

#if CFG_SYSVIEW
  SEGGER_SYSVIEW_RecordExitISR();
#endif
}

//--------------------------------------------------------------------+
// Porting API
//--------------------------------------------------------------------+

static void usb_hardware_init(void);

// USB Device Driver task
// This top level thread process all usb events and invoke callbacks
static void usb_device_task(void *param) {
  (void)param;

  // Priorities 0, 1, 4 (nRF52) are reserved for SoftDevice
  // 2 is highest for application
  NVIC_SetPriority(USBD_IRQn, 2);

  // init device on rhport0
  tud_init(0);

  usb_hardware_init();

  // RTOS forever loop
  while (1) {
    tud_task();
    TinyUSB_Device_FlushCDC();
  }
}

void TinyUSB_Port_InitDevice(uint8_t rhport) {
  (void)rhport;

  // Create a task for tinyusb device stack
  xTaskCreate(usb_device_task, "usbd", USBD_STACK_SZ, NULL, configMAX_PRIORITIES - 1, NULL);
}

void TinyUSB_Port_EnterDFU(void) {
  // Reset to Bootloader
  enterSerialDfu();
}

uint8_t TinyUSB_Port_GetSerialNumber(uint8_t serial_id[16]) {
  uint32_t *serial_32 = (uint32_t *)serial_id;

  serial_32[0] = __builtin_bswap32(NRF_FICR->DEVICEID[1]);
  serial_32[1] = __builtin_bswap32(NRF_FICR->DEVICEID[0]);

  return 8;
}

//--------------------------------------------------------------------+
// Helper
//--------------------------------------------------------------------+

// tinyusb function that handles power event (detected, ready, removed)
// We must call it within SD's SOC event handler, or set it as power event
// handler if SD is not enabled.
extern "C" void tusb_hal_nrf_power_event(uint32_t event);

static void power_event_handler(nrfx_power_usb_evt_t event) {
  tusb_hal_nrf_power_event((uint32_t)event);
}

// Init usb hardware when starting up. Softdevice is not enabled yet
static void usb_hardware_init(void) {
  // USB power may already be ready at this time -> no event generated
  // We need to invoke the handler based on the status initially
  uint32_t usb_reg = NRF_POWER->USBREGSTATUS;

  // Power module init
  const nrfx_power_config_t pwr_cfg = {};
  nrfx_power_init(&pwr_cfg);

  // Register tusb function as USB power handler
  const nrfx_power_usbevt_config_t config = {.handler = power_event_handler};

  nrfx_power_usbevt_init(&config);
  nrfx_power_usbevt_enable();

  if (usb_reg & POWER_USBREGSTATUS_VBUSDETECT_Msk) {
    tusb_hal_nrf_power_event(NRFX_POWER_USB_EVT_DETECTED);
  }
}

//--------------------------------------------------------------------+
// Dummy callbacks to allow linking when certain class is not used
//--------------------------------------------------------------------+
__attribute__((weak)) uint8_t const * tud_hid_descriptor_report_cb(uint8_t instance) {
  (void) instance;
  return NULL;
}

__attribute__((weak)) uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen) {
  (void) instance;
  (void) report_id;
  (void) report_type;
  (void) buffer;
  (void) reqlen;
  return 0;
}

__attribute__((weak)) void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize) {
  (void) instance;
  (void) report_id;
  (void) report_type;
  (void) buffer;
  (void) bufsize;
}

__attribute__((weak)) void tud_hid_set_protocol_cb(uint8_t instance, uint8_t protocol) {
  (void) instance;
  (void) protocol;
}

__attribute__((weak)) bool tud_hid_set_idle_cb(uint8_t instance, uint8_t idle_rate) {
  (void) instance;
  (void) idle_rate;
  return false;
}

__attribute__((weak)) void tud_hid_report_complete_cb(uint8_t instance, uint8_t const* report, uint16_t len) {
  (void) instance;
  (void) report;
  (void) len;
}

__attribute__((weak)) void tud_hid_report_failed_cb(uint8_t instance, hid_report_type_t report_type, uint8_t const* report, uint16_t xferred_bytes) {
  (void) instance;
  (void) report_type;
  (void) report;
  (void) xferred_bytes;
}

__attribute__((weak)) int32_t tud_msc_read10_cb(uint8_t lun, uint32_t lba, uint32_t offset, void* buffer, uint32_t bufsize) {
  (void) lun;
  (void) lba;
  (void) offset;
  (void) buffer;
  (void) bufsize;
  return -1;
}

__attribute__((weak)) int32_t tud_msc_write10_cb(uint8_t lun, uint32_t lba, uint32_t offset, uint8_t* buffer, uint32_t bufsize) {
  (void) lun;
  (void) lba;
  (void) offset;
  (void) buffer;
  (void) bufsize;
  return -1;
}

__attribute__((weak)) bool tud_msc_test_unit_ready_cb(uint8_t lun) {
  (void) lun;
  return false;
}

__attribute__((weak)) void tud_msc_capacity_cb(uint8_t lun, uint32_t* block_count, uint16_t* block_size) {
  (void) lun;
  *block_count = 0;
  *block_size = 0;
}

__attribute__((weak)) int32_t tud_msc_scsi_cb(uint8_t lun, uint8_t const scsi_cmd[16], void* buffer, uint16_t bufsize) {
  (void) lun;
  (void) scsi_cmd;
  (void) buffer;
  (void) bufsize;
  return -1;
}

// MAX3421 Host Controller API
__attribute__((weak)) void tuh_max3421_spi_cs_api(uint8_t rhport, bool active) {
  (void) rhport;
  (void) active;
}

__attribute__((weak)) bool tuh_max3421_spi_xfer_api(uint8_t rhport, uint8_t const* tx_buf, uint8_t* rx_buf, size_t xfer_bytes) {
  (void) rhport;
  (void) tx_buf;
  (void) rx_buf;
  (void) xfer_bytes;
  return false;
}

__attribute__((weak)) void tuh_max3421_int_api(uint8_t rhport, bool enabled) {
  (void) rhport;
  (void) enabled;
}

#endif // USE_TINYUSB
