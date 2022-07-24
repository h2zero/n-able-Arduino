# Flash Config Storage
This library was originally designed to support storing bond info from NimBLE and has been redesigned to allow use as an EEPROM emulation for storing config data.

## Design
A **minimum** of 2 flash pages are required. Pages used for FC storage are identified with the value 0xf1a5c0f5UL as the first word in the page.

Wear leveling is accomplished by sequentially writing data across a number of flash pages. When data is updated the old data is marked with the location of the new data. When the end of the page is reached the original page is defragmented and valid data copied to the swap page which becomes the new data page and the old page is erased to be used for as the swap for the next defragmentation.

### Data format
The data is stored with a header in the following format:

|  Data ID  | New Address | Length  | Reserved | Data       |
|-----------|-------------|---------|----------|------------|
|  32 bits  | 32 bits     | 16 bits | 16 bits  | data bytes |

- The data ID can be any value from 0x00 to 0xFFFFFFFE (0xFFFFFFFFF is reserved as empty).
- The New Address specifies the location of any updated data (0xFFFFFFFF if unchanged).
- The length is the length of the data in Bytes.
- Data is the stored data.

When a data value is written FC storage will write the header and data in the next available space. If there is not enough space for the new data a defragmentation will be triggered in an attempt to provide space.

When a data value is updated the New Address field of the previous data is updated to reflect the address of the new data location to read the updated data from. This continues for each time the value is updated.

When reading a value FC Storage will find the first instance of the ID and check the New Address field, if a new address has been set it will check the New Address field of that location, move to the next if set, and so on until the latest data is found.

### Defragmentation
The defragmentation process consolidates all the current valid data to the swap page(s) and updates the page pointer to the beginning of the data. This sets all New Address header fields to empty and frees up space needed for new data. The old data page(s) will then be erased and become the swap page for the next defragmentation.




