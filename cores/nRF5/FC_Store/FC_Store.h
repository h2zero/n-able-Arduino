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

#ifndef FC_STORE_H_
#define FC_STORE_H_

#include <inttypes.h>

#define FC_STORE_MAGIC_VAL 0xf1a5c0f5UL
#define FC_STORE_EMPTY_VAL 0xFFFFFFFFUL

#ifndef FC_WORD_BYTES
#define WORD_BYTES 4
#else
#define WORD_BYTES FC_WORD_BYTES
#endif

#ifdef FC_STORE_DEBUG
#define STORE_PRINTF(msg, ...) printf((msg), ##__VA_ARGS__);
#else
#define STORE_PRINTF(msg, ...) void(msg)
#endif

class FCStoreCallbacks {
public:
    virtual ~FCStoreCallbacks(){};
    virtual uint32_t getPageSize() = 0;
    virtual uint32_t getStoreBeginPage() = 0;
    virtual void     flashWriteWord(uint32_t address, uint32_t value) = 0;
    virtual void     flashWriteWords(uint32_t address, const uint32_t * src, uint32_t num_words) = 0;
    virtual void     flashPageErase(uint32_t address) = 0;
};

class FCStoreDesc {
    uint32_t id;
    uint32_t newAddress;
    uint32_t length;

public:
    FCStoreDesc(uint32_t val) { id = val; newAddress = FC_STORE_EMPTY_VAL; length = 0; }
    uint16_t getByteLength() { return (uint16_t)((length >> 16) & (uint32_t)0x0000FFFF); }
    uint16_t getStoreLength() {
        uint16_t Blen = getByteLength();
        return (uint16_t)(Blen + WORD_BYTES - (Blen & (uint16_t)(WORD_BYTES  - 1)));
    }
    void     setByteLength(uint16_t len) { length = (uint32_t)((len << 16) | (uint32_t)0x0000FFFF); }
    void     setId(uint32_t val) { id = val;}
    uint32_t getId() { return id; }
    uint32_t getNewAddress() { return newAddress; }
};

class FCStore {
    FCStoreCallbacks * cbf;
    uint32_t firstPage;
    uint32_t lastPage;
    uint32_t nextWriteAddr;
    uint16_t storeSize;
    bool     initialized;

public:
    FCStore(FCStoreCallbacks * callbacks, uint16_t numBytes = 4096);
    FCStoreDesc * find(uint32_t id);

    bool     begin();
    bool     write(uint32_t id, uint8_t * buf, uint16_t len);
    void     dump();
    void     format();
    void     eraseAll();
    bool     read(uint32_t id, uint8_t * buf, uint16_t maxBytes);
    uint16_t analyze();
    bool     defrag();
};

#endif /* FC_STORE_H_ */
