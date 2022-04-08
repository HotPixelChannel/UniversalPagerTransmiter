#include "Storage.h"
#include <Arduino.h>

void clearEE() {
  for (int i = 0; i < EEPROM.length(); i++) EEPROM.update(i, 255);
}
// Add pager to EEPROM (addr 0-20)
bool addPager(Pager *pager, bool toDel) {
  Pager lPager = *pager;
  if (lPager.addr > STORAGE_COUNT)
    return false;

  if (!toDel) {
    lPager.crc[0] =  CRC[0];
    lPager.crc[1] =  CRC[1];
  }else{
    lPager.crc[0] =  0;
    lPager.crc[1] =  0;
  }
  EEPROM.put(lPager.addr * sizeof(Pager), lPager);
  return true;
}

void getPager(byte address, struct Pager *pager) {
  EEPROM.get(address*sizeof(Pager), *pager);
}
