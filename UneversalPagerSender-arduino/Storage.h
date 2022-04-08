#ifndef _STORAGE_H
#define _STORAGE_H

#include <EEPROM.h>
#include "Arduino.h"

#define PAGER_SIZE  161
#define STORAGE_COUNT  20

const byte CRC[2] =  {0x45, 0x99};

typedef struct Pager {
  byte addr: 8;             // Address in EEPROM
  char alias[10];           // Readable name
  uint32_t cap : 24;        // CAP code
  uint32_t frequency : 20;  // Frequency
  byte msgSource: 2;        // Message source (0-3)
  byte enconding: 2;        // Text enconding
  byte inversion : 1;       // Bit inversion
  word rate: 12;            // Bautrate (512, 1200, 2400)
  byte crc[2];              // Index bytes for detecting
};


void clearEE();
bool addPager(Pager *pager, bool toDel);
void getPager(byte address, struct Pager *pager) ;
Pager*  getPagers();

#endif
