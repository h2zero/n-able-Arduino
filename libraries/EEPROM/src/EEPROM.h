/*
  EEPROM.h - EEPROM library
  Original Copyright (c) 2006 David A. Mellis.  All right reserved.
  New version by Christopher Andrews 2015.
  This copy has minor modificatons for use with Teensy, by Paul Stoffregen
  Modified by Ryan Powell for use with n-able arduino core.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.
  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef EEPROM_H
#define EEPROM_H

#include <inttypes.h>

#include <nvm_store.h>
#include <FC_Store.h>
#include <type_traits>

class EEPROMStoreCBs : public FCStoreCallbacks {
    uint32_t getPageSize() {
        return flash_get_page_size();
    }
    uint32_t getStoreBeginPage() {
        return flash_get_store_page(flash_get_page_size() * 2);
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

class EEPROMClass {
    EEPROMStoreCBs _eepCBs;
    FCStore _store;

  public:
    EEPROMClass(): _store(&_eepCBs, flash_get_page_size()){}

    void begin() { _store.begin();}

    /**
     * Read an eeprom cell
     * @param id
     * @return value
     */
    uint8_t read(int id) { uint8_t val = _store.read(id, &val, 1); return val; }

    /**
     * Write value to an eeprom cell
     * @param id
     * @param value
     */
    void write(int id, uint8_t val) { _store.write(id, &val, 1); }

    /**
     * Update a eeprom cell
     * @param id
     * @param value
     */
    void update(int id, uint8_t val) { _store.write(id, &val, 1); }

    void format() { _store.format(); }
    void dump() { _store.dump(); }

    //Functionality to 'get' and 'put' objects to and from EEPROM.
    template< typename T > T &get( int id, T &t ){
        static_assert(std::is_trivially_copyable<T>::value,"You can not use this type with EEPROM.get" ); // the code below only makes sense if you can "memcpy" T
        _store.read(id, (uint8_t*)&t, sizeof(T));
        return t;
    }

    template< typename T > const T &put( int id, const T &t ){
        static_assert(std::is_trivially_copyable<T>::value, "You can not use this type with EEPROM.put"); // the code below only makes sense if you can "memcpy" T
        if(!_store.write(id, (uint8_t*)&t, sizeof(T))) {
            _store.defrag();
            _store.write(id, (uint8_t*)&t, sizeof(T));
        }
        return t;
    }
};

// put - Specialization for Arduino Strings -------------------------------
// to put an Arduino String to the EEPROM we copy its internal buffer
// including the trailing \0 to the eprom
template <>
inline const String &EEPROMClass::put(int id, const String &s)
{
    if(!_store.write(id, (uint8_t*)s.c_str(), s.length() + 1)) {
        _store.defrag();
        _store.write(id, (uint8_t*)s.c_str(), s.length() + 1);
    }
    return s;
}

// get - Specialization for Arduino Strings -------------------------------
template <>
inline String &EEPROMClass::get(int id, String &s){
    s = "";
    FCStoreDesc * desc = _store.find(id);
    if (desc != nullptr) {
        s = (const char*)((uint32_t)desc + sizeof(FCStoreDesc));
    }
    return s;
}

static EEPROMClass EEPROM __attribute__ ((unused));


#endif