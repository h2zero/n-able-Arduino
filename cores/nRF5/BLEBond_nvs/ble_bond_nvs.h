/*
  Copyright (c) 2021 Ryan Powell.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef BLE_BOND_NVS_H
#define BLE_BOND_NVS_H

#include <inttypes.h>

static const uint32_t cccd     = 0x64636363; /* cccd */
static const uint32_t peer_sec = 0x63657370; /* psec */
static const uint32_t our_sec  = 0x6365736f; /* osec */

#ifdef __cplusplus
extern "C" {
#endif

int       ble_bond_nvs_get_entry(uint32_t id, uint32_t *val_addr);
int       ble_bond_nvs_save_entry(uint32_t id, char *value);
void      ble_bond_nvs_erase_all(void);
void      ble_bond_nvs_dump(void);

#ifdef __cplusplus
}
#endif

#endif /* BLE_BOND_NVS_H */