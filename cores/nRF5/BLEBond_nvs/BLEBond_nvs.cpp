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

#include "ble_bond_nvs.h"
#include "nvm_store.h"
#include "FC_Store.h"
#include <string.h>

#define BLE_HS_ESTORE_CAP  0x1b
#define BLE_HS_ESTORE_FAIL 0x1c

class bondStoreCBs : public FCStoreCallbacks {
    uint32_t getPageSize() {
        return flash_get_page_size();
    }
    uint32_t getStoreBeginPage() {
        return flash_get_bond_store_page();
    }
    void flashWriteWord(uint32_t address, uint32_t value) {
        flash_write_word(address, value);
    }
    void flashWriteWords(uint32_t address, const uint32_t * src, uint32_t num_words) {
        flash_write_words(address, src, num_words);
    }
    void flashPageErase(uint32_t address) {
        flash_page_erase(address);
    }
};

static bondStoreCBs _callbacks;
static FCStore bondStore(&_callbacks , flash_get_page_size());

int ble_bond_nvs_save_entry(uint32_t id, char *value) {
    if (!bondStore.write(id, (uint8_t*)value, strlen(value) + 1)) {
        bondStore.defrag();
        if (!bondStore.write(id, (uint8_t*)value, strlen(value) + 1)) {
            return BLE_HS_ESTORE_FAIL;
        }
    }

    return 0;
}

int ble_bond_nvs_get_entry(uint32_t id, uint32_t *val_addr) {
    bondStore.begin();
    FCStoreDesc* desc = bondStore.find(id);

    if (desc != nullptr) {
        *val_addr = (uint32_t)desc + sizeof(FCStoreDesc);
        return 0;
    }

    return BLE_HS_ESTORE_FAIL;
}

void ble_bond_nvs_erase_all() {
    bondStore.eraseAll();
}

void ble_bond_nvs_dump() {
    bondStore.dump();
}
