/* Arduino SPIFlash Library v.1.3.0
 * Copyright (C) 2015 by Prajwal Bhattaram
 * Modified by Prajwal Bhattaram - 29/08/2015
 *
 * This file is part of the Arduino SPIFlash Library. This library is for
 * W25Q80BV serial flash memory. In its current form it enables reading 
 * and writing bytes from and to various locations, reading and writing pages,
 * sector, block and chip erase, powering down for low power operation and
 * continuous read functions.
 *
 * This Library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License v3.0
 * along with the Arduino SPIFlash Library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */
#ifndef SPIFLASH_H
#define SPIFLASH_H

#include <Arduino.h>

class SPIFlash {
public:
  SPIFlash(uint8_t cs = 10, bool overflow = true);
  uint8_t  begin();
	uint32_t getID();
	bool     readByte(uint16_t page_number, uint8_t offset, uint8_t data),
           writeByte(uint16_t page_number, uint8_t offset, uint8_t data, bool errorCheck = true),
           writeBytes(uint16_t page_number, uint8_t offset, uint8_t *data_buffer, bool errorCheck = true),
           writeChar(uint16_t page_number, uint8_t offset, int8_t data, bool errorCheck = true),
           writeShort(uint16_t page_number, uint8_t offset, int16_t data, bool errorCheck = true),
           writeWord(uint16_t page_number, uint8_t offset, uint16_t data, bool errorCheck = true),
           writeLong(uint16_t page_number, uint8_t offset, int32_t data, bool errorCheck = true),
           writeULong(uint16_t page_number, uint8_t offset, uint32_t data, bool errorCheck = true),
           writeFloat(uint16_t page_number, uint8_t offset, float data, bool errorCheck = true),
	         writePage(uint16_t page_number, uint8_t *data_buffer, bool errorCheck = true),
	         eraseSector(uint16_t page_number),
	         eraseBlock32K(uint16_t page_number),
	         eraseBlock64K(uint16_t page_number),
	         eraseChip(void),
	         suspendProg(void),
	         resumeProg(void),
	         powerDown(void),
	         powerUp(void);
	void     readBytes(uint16_t page_number, uint8_t offset, uint8_t *data_buffer),
           readPage(uint16_t page_number, uint8_t *data_buffer),
           printPage(uint16_t page_number, uint8_t outputType),
	         printAllPages(uint8_t outputType);
	int8_t   readChar(uint16_t page_number, uint8_t offset);
  uint8_t  readByte(uint16_t page_number, uint8_t offset);
  int16_t  readShort(uint16_t page_number, uint8_t offset);
  uint16_t readWord(uint16_t page_number, uint8_t offset);
  int32_t  readLong(uint16_t page_number, uint8_t offset);
  uint32_t readULong(uint16_t page_number, uint8_t offset);
  float    readFloat(uint16_t page_number, uint8_t offset);
  template <class T>  bool writeAnything(uint16_t page_number, uint8_t offset, const T& value, bool errorCheck = true);
  template <class T> uint32_t readAnything(uint16_t page_number, uint8_t offset, T& value);


private:
	void     _chipSelect(void),
	         _chipDeselect(void),
	         _cmd(uint8_t c),
           _endProcess(void),
	         _empty(uint8_t *array),
	         _printPageBytes(uint8_t *data_buffer, uint8_t outputType);
	bool     _notBusy(uint32_t timeout = 100L),
           _addressCheck(uint32_t address),
           _beginRead(uint32_t address),
           _beginWrite(uint32_t address),
           _readPage(uint16_t page_number, uint8_t *page_buffer),
           _writeNextByte(uint8_t c),
		       _writeEnable(void),
		       _writeDisable(void),
	         _getJedecId(uint8_t *b1, uint8_t *b2, uint8_t *b3);
  uint8_t  _readNextByte(void);
  uint32_t _getAddress(uint16_t page_number, uint8_t offset = 0),
           _prepRead(uint16_t page_number, uint8_t offset = 0),
           _prepWrite(uint16_t page_number, uint8_t offset = 0);
  template <class T> bool _errorCheck(uint32_t address, const T& value);
  
  volatile uint8_t *cs_port;
  uint8_t           cs_mask;
  uint8_t			chipSelect;
  uint32_t    capacity, maxPage;
  bool			  pageOverflow;
  const uint8_t memType[8]   = {0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18};
  const uint16_t memSize[8]  = {64, 128, 512, 1, 2, 4, 8, 16};
  const uint16_t KB          = 1024L;
  const uint32_t MB          = 1024L * 1024L;
};

template <class T> bool SPIFlash::writeAnything(uint16_t page_number, uint8_t offset, const T& value, bool errorCheck)
{
 uint32_t address = _getAddress(page_number, offset);
  const byte* p = (const byte*)(const void*)&value;
  uint16_t i;
  _beginWrite(address);
  for(i = 0; i < sizeof(value);i++)
  {
    _writeNextByte(*p++);
  }
  _endProcess();

  if (!errorCheck)
    return true;
  else
    return _errorCheck(address, value);
}

template <class T> uint32_t SPIFlash::readAnything(uint16_t page_number, uint8_t offset, T& value)
{
  uint32_t address = _getAddress(page_number, offset);
  byte* p = (byte*)(void*)&value;
  uint16_t i;
  _beginRead(address);
  for(i = 0; i<sizeof(value);i++)
  {
    *p++ = _readNextByte();
  }
  _endProcess();
  return i;
}

template <class T> bool SPIFlash::_errorCheck(uint32_t address, const T& value)
{
  const byte* p = (const byte*)(const void*)&value;
  uint16_t i;
  _beginRead(address);
  for(i = 0; i < (sizeof(value));i++)
  {
    if (*p++ != _readNextByte())
      return false;
  }
  return true;
}


#endif // _SPIFLASH_H_