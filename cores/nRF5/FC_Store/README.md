# Flash Config Store

This library was originally designed to support storing bond info from NimBLE and has been redesigned to allow use as an EEPROM emulation for storing config data.

## Design

A **minimum** of 2 flash pages are required. 

Wear leveling is accomplished by sequentially writing data accross a page of flash. When data is updated the old data is marked with the location of the new data. When the end of the page is reached the original page is defragmented and valid data copied to the swap page which becomes the new main page and the old page is erased.