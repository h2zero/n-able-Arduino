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

#ifndef NVM_STORE_H
#define NVM_STORE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

void     flash_page_erase(uint32_t address);
void     flash_write_word(uint32_t address, uint32_t value);
void     flash_write_byte(uint32_t address, uint8_t value);
void     flash_write_bytes(uint32_t address, const uint8_t * src, uint32_t num_bytes);
void     flash_write_words(uint32_t address, const uint32_t * src, uint32_t num_words);
uint32_t flash_read_unaligned_32(uint32_t address);
uint32_t flash_get_store_page(uint32_t bytes);
uint32_t flash_get_page_size(void);
uint32_t flash_get_bond_store_page(void);

#ifdef __cplusplus
}
#endif

#endif