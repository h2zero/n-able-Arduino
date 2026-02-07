#if defined(NRF51)
#include "nrf51_nimconfig.h"
#elif defined(NRF52810_XXAA)
#include "nrf52810_nimconfig.h"
#elif defined(NRF52832_XXAA) || defined(NRF52832_XXAB)
#include "nrf52832_nimconfig.h"
#elif defined(NRF52833_XXAA)
#include "nrf52833_nimconfig.h"
#elif defined(NRF52840_XXAA)
#include "nrf52840_nimconfig.h"
#else
#error No supported mcu config specified
#endif

#ifdef USE_LFRC
#define MYNEWT_VAL_BLE_LL_SCA (500)
#endif

#ifndef MYNEWT_VAL_BLE_LL_CFG_FEAT_LE_2M_PHY
#ifdef NRF51
#define MYNEWT_VAL_BLE_LL_CFG_FEAT_LE_2M_PHY (0)
#else
#define MYNEWT_VAL_BLE_LL_CFG_FEAT_LE_2M_PHY (1)
#endif
#endif

#ifndef MYNEWT_VAL_BLE_LL_CFG_FEAT_LE_CODED_PHY
#define MYNEWT_VAL_BLE_LL_CFG_FEAT_LE_CODED_PHY (MYNEWT_VAL_MCU_TARGET__nRF52833 || \
                                                 MYNEWT_VAL_MCU_TARGET__nRF52840 || \
                                                 MYNEWT_VAL_MCU_TARGET__nRF52820 || \
                                                 MYNEWT_VAL_MCU_TARGET__nRF52811 || \
                                                 MYNEWT_VAL_MCU_TARGET__nRF5340_NET)
#endif

#define MYNEWT_VAL_BLE_CONTROLLER (1)
#define CONFIG_BT_NIMBLE_LEGACY_VHCI_ENABLE 1

#ifndef min
#define min(a, b) ((a)<(b)?(a):(b))
#endif

#ifndef max
#define max(a, b) ((a)>(b)?(a):(b))
#endif
