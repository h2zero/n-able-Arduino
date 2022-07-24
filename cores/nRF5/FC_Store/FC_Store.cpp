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

#include "FC_Store.h"

#include <stdio.h>
#include <string.h>
#include <algorithm>


FCStore::FCStore(FCStoreCallbacks * callbacks, uint16_t numBytes) {
    initialized = false;
    cbf = callbacks;
    storeSize = (numBytes < cbf->getPageSize()) ? cbf->getPageSize() : numBytes;
}

bool FCStore::begin () {
    if (initialized == true) {
        return true;
    }

    firstPage = cbf->getStoreBeginPage();
    lastPage = firstPage + storeSize;

    STORE_PRINTF("first page = %lu, last page = %lu\n", firstPage, lastPage);

    // if first page does not contain the storage check the swap
    if (*(uint32_t*)firstPage != FC_STORE_MAGIC_VAL) {
        if (*(uint32_t*)lastPage != FC_STORE_MAGIC_VAL) {
            // no storage found; format new
            format();
            return true;
        } else {
            // found storage at the swap page; adjust addresses accordingly
            firstPage = lastPage;
            lastPage = firstPage + storeSize;
        }
    }

    // We should have a valid storage location now; find the next writable address
    uint32_t offset = WORD_BYTES;
    FCStoreDesc * desc = (FCStoreDesc*)(firstPage + offset);

    while (desc->getId() != FC_STORE_EMPTY_VAL) {
        offset += sizeof(FCStoreDesc) + desc->getStoreLength();

        if ((firstPage + offset) >= lastPage) {
            STORE_PRINTF("error firstpage: %lu offset %lu vallen%u\n", firstPage, offset, desc->getStoreLength());
            return false;
        }

        if (offset & (uint32_t)0x03) {
            STORE_PRINTF("Data error - invalid offset %lu\n", offset);
            return false;
        }

        desc = (FCStoreDesc*)(firstPage + offset);
    }

    nextWriteAddr = firstPage + offset;
    initialized = true;

    return true;
}

bool FCStore::write (uint32_t id, uint8_t* buf, uint16_t len) {
    if (initialized == false) {
        return false;
    }

    if (id == FC_STORE_EMPTY_VAL) {
        STORE_PRINTF("Error: invalid ID\n");
        return false;
    }

    // save the address of the previous data so we can write the new address
    FCStoreDesc *oldDesc = find(id);

    // writing null value buffer with no additional data is a deletion
    // set the new address to 0 as a flag indicating deleted.
    if(oldDesc != nullptr && *buf == 0 && len < 2) {
        cbf->flashWriteWord((uint32_t)oldDesc + WORD_BYTES, 0);
        return true;
    }

    FCStoreDesc storeDesc(id);
    // store the original data length in the upper 16 bits
    storeDesc.setByteLength(len);

    uint8_t numWords = len / WORD_BYTES;
    uint8_t numBytes = len & (uint16_t)0x03;

    // Make sure we check with +1 to the word count if there are stray bytes.
    if ((nextWriteAddr + sizeof(FCStoreDesc) + (numBytes ? len + 1 : len)) > lastPage) {
        STORE_PRINTF("Error not enough storage\n");
        return false;
    }

    uint32_t newAddr = nextWriteAddr;
    cbf->flashWriteWords(nextWriteAddr, (uint32_t*)&storeDesc, sizeof(storeDesc) / WORD_BYTES);
    nextWriteAddr += sizeof(storeDesc);
    cbf->flashWriteWords(nextWriteAddr, (uint32_t*)buf, numWords);
    nextWriteAddr += numWords * WORD_BYTES;

    if (numBytes) {
        // convert the leftover bytes to a word to maintain alignment
        uint32_t val32 = 0;
        memcpy(&val32, buf + (numWords * WORD_BYTES), numBytes);
        cbf->flashWriteWord(nextWriteAddr, val32);
        nextWriteAddr += WORD_BYTES;
    }

    if (oldDesc != nullptr) {
       cbf->flashWriteWord((uint32_t)oldDesc + WORD_BYTES, newAddr);
    }

    // Verify the data
    FCStoreDesc * desc = find(id);

    if (desc == nullptr) {
        STORE_PRINTF("Data verify error - id not found\n");
        return false;
    }

    size_t datalen = desc->getByteLength();
    uint32_t dataAddr = (uint32_t)desc + sizeof(FCStoreDesc);
    if (datalen == len && memcmp(buf, (uint8_t*)dataAddr, datalen) == 0) {
        STORE_PRINTF("Data stored successfully\n");
        return true;
    }

    STORE_PRINTF("Data verify error - corrupted\n");
    return false;
}

FCStoreDesc* FCStore::find (uint32_t id) {
    uint32_t findIdAddr = firstPage + WORD_BYTES;
    FCStoreDesc* desc = nullptr;

    while (findIdAddr < lastPage) {
        desc = (FCStoreDesc*)findIdAddr;

        if (desc->getId() == FC_STORE_EMPTY_VAL) {
            break;
        } else if (desc->getId() == id) {
            if (desc->getNewAddress() == 0) { // skip deleted
                findIdAddr += sizeof(FCStoreDesc) + desc->getStoreLength();
            } else if (desc->getNewAddress() != FC_STORE_EMPTY_VAL) { // found old entry
                findIdAddr = desc->getNewAddress();
            } else { // found
                return desc;
            }
        } else { // skip to the next item
            findIdAddr += sizeof(FCStoreDesc) + desc->getStoreLength();
        }
    }

    STORE_PRINTF("ID not found\n");
    return nullptr;
}

void FCStore::format() {
    eraseAll();
    cbf->flashWriteWord(firstPage, FC_STORE_MAGIC_VAL);
    nextWriteAddr = firstPage + WORD_BYTES;
    initialized = true;
}

bool FCStore::defrag() {
    uint32_t destAddr = cbf->getStoreBeginPage();
    uint32_t srcAddr = firstPage + WORD_BYTES;

    if (firstPage == destAddr) {
        destAddr = lastPage;
    }

    cbf->flashWriteWord(destAddr, FC_STORE_MAGIC_VAL);
    destAddr += WORD_BYTES;

    uint16_t entryLen = 0;
    FCStoreDesc * desc = (FCStoreDesc*)srcAddr;

    while(desc->getId() != FC_STORE_EMPTY_VAL && srcAddr < lastPage) {
        entryLen = sizeof(FCStoreDesc) + desc->getStoreLength();

        if (desc->getNewAddress() == FC_STORE_EMPTY_VAL) {
            cbf->flashWriteWords(destAddr, (uint32_t*)srcAddr, entryLen / WORD_BYTES);
            destAddr += entryLen;
        }

        srcAddr += entryLen;
        desc = (FCStoreDesc*)srcAddr;
    }

    nextWriteAddr = destAddr;
    srcAddr = firstPage;

    while (srcAddr < lastPage) {
        cbf->flashPageErase(srcAddr);
        srcAddr += cbf->getPageSize();
    }

    if (destAddr > lastPage) {
        lastPage += storeSize;
    } else {
        lastPage = firstPage;
    }

    firstPage = lastPage - storeSize;
    return true;
}

void FCStore::eraseAll() {
    STORE_PRINTF("\nErasing store\n");
    firstPage   = cbf->getStoreBeginPage();
    lastPage    = firstPage + storeSize;
    uint32_t erasePage = firstPage;

    while (erasePage < (firstPage + storeSize * 2)) {
        cbf->flashPageErase(erasePage);
        erasePage += cbf->getPageSize();
    }
}

bool FCStore::read(uint32_t id, uint8_t* buf, uint16_t maxBytes) {
    FCStoreDesc * desc = find(id);

    if (desc != nullptr) {
        uint16_t datalen = desc->getByteLength();
        uint32_t dataAddr = (uint32_t)desc + sizeof(FCStoreDesc);
        memcpy(buf, (uint8_t*)dataAddr, std::min(datalen, maxBytes));
        return true;
    }

    return false;
}

uint16_t FCStore::analyze() {
    uint32_t offset = WORD_BYTES;
    uint16_t recoverable = 0;
    FCStoreDesc * desc = (FCStoreDesc*)(firstPage + offset);

    while (desc->getId() != FC_STORE_EMPTY_VAL) {

        uint16_t len = sizeof(FCStoreDesc) + desc->getStoreLength();

        if (desc->getNewAddress() != FC_STORE_EMPTY_VAL) {
            recoverable += len;
        }

        offset += len;

        if ((firstPage + offset) > lastPage) {
            STORE_PRINTF("error firstpage: %lu offset %lu len%u\n", firstPage, offset, desc->getStoreLength());
            return 0;
        }

        if (offset & (uint32_t)0x03) {
            STORE_PRINTF("Data error - invalid offset %lu\n", offset);
            return 0;
        }

        desc = (FCStoreDesc*)(firstPage + offset);
    }

    return recoverable;
}

void FCStore::dump() {
    STORE_PRINTF("\n");
    for(size_t i=0; i<storeSize; i+=64) {
      for(size_t j=i; j<(i+64);j++) {
          STORE_PRINTF("0x%02x ",*(uint8_t*)(firstPage + j));
      }
      STORE_PRINTF("\n");
    }
}
