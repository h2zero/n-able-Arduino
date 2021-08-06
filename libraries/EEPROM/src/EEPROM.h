#ifndef EEPROM_H
#define EEPROM_H

#include <inttypes.h>

#include <nvm_store.h>
#include <FC_Store.h>
#include <type_traits>

#ifndef EEPROM_SIZE_BYTES
#define EEPROM_SIZE_BYTES 4096
#endif

class EEPROMStoreCBs : public FCStoreCallbacks {
    uint32_t getPageSize() {
        return flash_get_page_size();
    }
    uint32_t getStoreBeginPage() {
        return flash_get_store_page(EEPROM_SIZE_BYTES * 2);
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
    EEPROMClass(): _store(&_eepCBs, EEPROM_SIZE_BYTES){}

    void begin() { _store.begin();}

    /**
     * Read an eeprom cell
     * @param index
     * @return value
     */
    uint8_t read(int id) { uint8_t val = _store.read(id, &val, 1); return val; }

    /**
     * Write value to an eeprom cell
     * @param index
     * @param value
     */
    void write(int id, uint8_t val) { _store.write(id, &val, 1); }

    /**
     * Update a eeprom cell
     * @param index
     * @param value
     */
    void update(int id, uint8_t val) { _store.write(id, &val, 1); }

    void format() { _store.format(); }
    void dump() { _store.dump(); }

    //Functionality to 'get' and 'put' objects to and from EEPROM.
    template< typename T > T &get( int id, T &t ){
        static_assert(std::is_trivially_copyable<T>::value,"You can not use this type with EEPROM.get" ); // the code below only makes sense if you can "memcpy" T
        _store.read(id, (uint8_t*)t, sizeof(T));
        return t;
    }

    template< typename T > const T &put( int id, const T &t ){
        static_assert(std::is_trivially_copyable<T>::value, "You can not use this type with EEPROM.put"); // the code below only makes sense if you can "memcpy" T
        if(!_store.write(id, (uint8_t*)t, sizeof(T))) {
            _store.defrag();
            _store.write(id, (uint8_t*)t, sizeof(T));
        }
        return t;
    }
};

static EEPROMClass EEPROM __attribute__ ((unused));


#endif