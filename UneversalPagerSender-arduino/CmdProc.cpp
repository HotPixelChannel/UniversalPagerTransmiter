#include <Arduino.h>
#include "Rf7021.h"
#include "Storage.h"
#include "Cmd.h"
#include <SoftwareSerial.h> 

Rf7021 rfD;

String alignStr(char* strI, int len) {
  String ret = String();
  String str = String(strI);
  int dif = len - str.length();
  if (dif > 0) {
    int pre;

    if (dif % 2 == 0)
      pre = dif / 2;
    else
      pre = dif / 2 - 1 + dif % 2;

    for (int i = 0; i < dif; i++) {
      if (i == pre)
        ret += str;
      ret += (F(" "));
    }

  } else {
    ret = str.substring(0, len);
  }
  return ret;
}

void pstr(String &str) {
  Serial.println(str);
}

void reserBuf() {
  while (Serial.available() > 0) {
    Serial.read();
  }
}

String getCurStr() {
  return String(F(" / cur. "));
}
void printSet(String val) {
  Serial.println(String(F("Set: ")) + val);
}
void printDiv() {
  Serial.println(F("\r--------------------------"));
}




void printWelcome(Rf7021 rf) {
  rfD = rf;
  Serial.println("\r\r\r\r\r");
  Serial.println(F("-----===[ Welcome to Universal POCSAG Transmitter ]===-----"));
  Serial.print(F("Sysinfo: FW ver: 1.0, HW rev. "));
  Serial.print(rf.getSiliconRev(), HEX);
  Serial.print(F(", temp.: "));
  Serial.print(rf.getTemp());
  Serial.print(F(", volt.: "));
  Serial.println(rf.getVoltage());
  Serial.println(F("\rSelect mode:"));
  Serial.println(F("  1 - Free mode"));
  Serial.println(F("  2 - Specified pager(s)"));
  Serial.println(F("  3 - Saved pagers"));
  Serial.flush();
  printDiv();
}


/**
 * Меню базовых команд
 */
void printFreeMode() {
  //  curPage = 1;
  Serial.println("\r\r\r\r\r");
  Serial.println(F("-----===[ Free mode ]===-----"));
  Serial.println(F("Here you can set manual CAP, freq and other options"));
  Serial.println(F("Command \"send\" and base commands:"));
  Serial.println(F(" [f] - frequency, kHz"));
  Serial.println(F(" [c] - set CAP code"));
  Serial.println(F(" [r] - set data rate (512, 1200 or 2400)"));
  Serial.println(F(" [m] - text message, must be at the end of the prompt"));
  Serial.println(F("\r Example: \"send f 160218 c 123456 m Hello world!\""));
  Serial.println(F("\r full - see all commands"));
  Serial.println(F(" 0 - go back"));
  printDiv();
}

/**
 * Меню всех команд
 */
void printFreeModeAdv() {
  Serial.println(F("Advanced commands:"));
  Serial.println(F(" [n] - send numeric message (e.g. \"n U *123*456*789\")"));
  Serial.println(F(" [t] - send tone message"));
  Serial.println(F(" [d] - set frequency deviation in kHz"));
  Serial.println(F(" [i] - set data inversion (0 - disabled, 1 - enabled)"));
  Serial.println(F(" [s] - set message source code (1 to 3)"));
  Serial.println(F(" [e] - set encoding"));
  Serial.println(F("    0 - EN Motorola"));
  Serial.println(F("    1 - EN/RU Motorola"));
  Serial.println(F("    2 - EN/RU Philips"));
  Serial.println(F("    3 - RU Motorola"));
  Serial.println(F(" [x] - run transmiter test"));
  printDiv();
}



/**
 * Вывод информации о пейджере
 */
void printPager(Pager *pager, bool detailed, bool wHeader) {
  // Serial.println(pager->cap);
  if (wHeader) {
    Serial.print(F("\r  ID |    Name   |   CAP  |"));
    if (detailed) {
      Serial.print(F(" Freq  | Rate | Source |  Enc | Inv |"));
    }
    Serial.println();
    for (int i = 0; i < (detailed ? 64 : 27); i++) {
      Serial.print("-");
    }
    Serial.println();
  }

  String det = "";
  char buf[7];
  char spc = ' ';
  char dev = '|';

  ltoa(pager->addr, buf, DEC);
  det += alignStr(buf, 5) + dev;

  det += spc + alignStr(pager->alias, 10) + dev;

  ltoa(pager->cap, buf, DEC);
  det += spc + alignStr(buf, 7) + dev;


  if (detailed) {
    ltoa(pager->frequency, buf, DEC);
    det += spc + alignStr(buf, 6) + dev;

    ltoa(pager->rate, buf, DEC);
    det += spc + alignStr(buf, 5) + dev;

    ltoa(pager->msgSource, buf, DEC);
    det += spc + alignStr(buf, 7) + dev;

    ltoa(pager->enconding, buf, DEC);
    det += spc + alignStr(buf, 5) + dev;

    //ltoa(pager->inversion, buf, DEC);
   // det += spc + alignStr(buf, 4) + dev;
   buf[0] = pager->inversion==1?'y':'n';
    det += spc + alignStr(buf, 4) + dev;
  }

  Serial.println(det);
  det = "";
}

void printFixAdded(byte *arr) {
  Serial.print(F("Fixed IDs: "));

  for (byte i = 0; i < STORAGE_COUNT; i++) {
    if (arr[i] < 0xff) {
      Serial.print(arr[i]);
      Serial.print(F(" "));
    }
  }

  Serial.println(F(" "));
}

/**
 * Меню отправки на сохраненные устройства
 */
void printSpecMode() {
  Serial.println(F("\r\r-----===[ Sending to saved devices ]===-----\r"));
  boolean header = true;
  struct Pager pager;
  for (byte i = 0; i < STORAGE_COUNT; i++) {
    getPager(i, &pager);

    if (pager.crc[0] == CRC[0] && pager.crc[1] == CRC[1]) {
      printPager(&pager, true, header);
      header = false;
    }
  }
  memset(&pager, 0, sizeof(Pager));
  Serial.println(F("\r  to [ID ID] [send [text], numeric [0-9,-,*,\" \",(,)], tone] - Send message"));
  Serial.println(F("             to selected IDs"));
  Serial.println(F("  fix [ID ID] - select IDs (then just \"send [text]\")"));
  Serial.println(F("  0 - go back"));

  Serial.println(String(F("\r  Example: \"to 0 4 12 7 send Hello! Get out of here!\"")));
  Serial.println(String(F("           \"to 4 12 numeric 8-800-2000-600\"")));
  Serial.println(String(F("           \"to 3 5 tone\"")));
  printDiv();
}

/**
 * Меню операций с хранилищем
 */
void printListMode() {
  Serial.println(F("\r\r\r-----===[ Stored list mode ]===-----\r"));

  boolean header = true;
  struct Pager pager;
  for (byte i = 0; i < STORAGE_COUNT; i++) {
    getPager(i, &pager);
    if (pager.crc[0] == CRC[0] && pager.crc[1] == CRC[1]) {
      printPager(&pager, true, header);
      header = false;
    }
  }

  Serial.println(F("\r  add - add device"));
  Serial.println(F("  edit [ID] - edit device"));
  Serial.println(F("  del [ID]  - remove device"));
  Serial.println(F("  0 - go back"));

  printDiv();
}

/**
 * Функция добавления/редактирования пейджера
 */
void printAddDevice(byte stepC, Pager* pager) { //stepC - for enter parametr
  if (pager->addr == 0xff) {
    // Create default params

    pager->cap = 0;
    pager->frequency = 0;
    pager->msgSource = 0;
    pager->enconding = 4;
    pager->inversion = 0;
    pager->rate = 1200;
  }
  int srs;
  long srsL;
  String arg = "";
  switch (stepC) {
    case 0: {
        reserBuf();
        if (pager->addr != 0xff) {
          arg = getCurStr() + pager->addr;
        }
        Serial.print(F("Enter ID (0-20)"));
        Serial.println(arg);

        while (Serial.available() == 0) {
        }

        if (pager->addr != 0xff && Serial.peek() == '\r') {
          printSet(String(pager->addr));
          printAddDevice(1, pager);
          return;
        }

        srs = Serial.parseInt();

        if (srs > 20 | srs < 0) {
          Serial.println(F("Incorrect data"));
          printAddDevice(0, pager);
        } else {
          pager->addr = srs;
          printSet(String(srs));
          printAddDevice(1, pager);
        }

        return;
      }
    case 1: {
        reserBuf();
        if (pager->cap > 0) {
          arg = getCurStr() + pager->cap;
        }
        Serial.print(F("Enter CAP"));
        Serial.println(arg);

        while (Serial.available() == 0) {
        }


        if (pager->cap > 0 && Serial.peek() == '\r') {
          printSet(String(pager->cap));
          printAddDevice(2, pager);
          return;
        }

        srsL = Serial.parseInt();

        if (srsL < 100 | srsL > 9999999) {
          Serial.println(F("Incorrect data"));
          printAddDevice(1, pager);
        } else {
          pager->cap = srsL;
        }

        printSet(String(pager->cap));
        printAddDevice(2, pager);

        return;
      }

    case 2: {
        reserBuf();
        Serial.print(F("Enter name (max 10 symbols)"));
        if (pager->alias[0] > 0) {
          arg = getCurStr() + pager->alias;
        }
        Serial.println(arg);

        while (Serial.available() == 0) {
        }

        if (pager->alias[0] > 0 && Serial.peek() == '\r') {
          printSet(String(pager->alias));
          printAddDevice(3, pager);
          return;
        }

        String alias = Serial.readString();
        alias.replace("\n", "");
        alias.replace("\r", "");
        alias.trim();
        alias.toCharArray(pager->alias, 10);
        printAddDevice(3, pager);
        return;
      }

    case 3: {
        reserBuf();
        if (pager->frequency > 0) {
          arg = getCurStr() + pager->frequency;
        }

        Serial.print(F("Enter frequency, kHz"));
        Serial.println(arg);

        while (Serial.available() == 0) {}

        if (pager->frequency > 0 && Serial.peek() == '\r') {
          printSet(String(pager->frequency));
          printAddDevice(4, pager);
          return;
        }

        srsL = Serial.readString().toInt();

        if (srsL < 100000 | srsL > 500000) {
          Serial.println(F("Incorrect data"));
          printAddDevice(3, pager);
        } else {
          pager->frequency = srsL;
          printSet(String(srsL ));
          printAddDevice(4, pager);
        }

        return;
      }

    case 4: {
        reserBuf();

        arg = getCurStr() + pager->msgSource;
        Serial.print(F("Enter source (0-3) or empty by default"));
        Serial.println(arg);

        while (Serial.available() == 0) {
        }

        if (Serial.peek() == '\r') {
          printSet(String(pager->msgSource));
          printAddDevice(5, pager);
          return;
        }

        srs = Serial.parseInt();

        if (srs >= 0  & srs <= 3) {
          pager->msgSource = srs;
        }

        printSet(String( pager->msgSource));
        printAddDevice(5, pager);

        return;
      }

    case 5: {
        reserBuf();
        arg = getCurStr() + pager->enconding;
        Serial.print(F("Enter enconding (0-EN Moto 1-EN/RU Moto 2-EN/RU Philips 3-RU Motorola)\r or empty by default"));
        Serial.println(arg);


        while (Serial.available() == 0) {
        }

        if (Serial.peek() == '\r') {
          printSet(String(pager->enconding));
          printAddDevice(6, pager);
          return;
        }

        srs = Serial.parseInt();
        if (srs > 0  & srs <= 3) {
          pager->enconding = srs;
        }
        printSet(String( pager->enconding));
        printAddDevice(6, pager);

        return;
      }

    case 6: {
        reserBuf();

        Serial.println(F("Enable inversion? Enter y/n or empty by default (n)"));

        while (Serial.available() == 0) {
        }

        if (Serial.peek() == '\r') {
          printSet(String(pager->inversion));
          printAddDevice(7, pager);
          return;
        }


        char s = Serial.readString().charAt(0);
        if (s == 'y') {
          pager->inversion = 1;
        } else {
          pager->inversion = 0;
        }
        printSet(String( pager->inversion == 1 ? "Yes" : "No"));
        printAddDevice(7, pager);

        return;
      }

    case 7: {
        reserBuf();
        arg = getCurStr() + pager->rate;
        Serial.print(F("Enter rate (512, 1200, 2400) or empty by default"));
        Serial.println(arg);

        while (Serial.available() == 0) {
        }

        if (Serial.peek() == '\r') {
          printSet(String(pager->rate));
          printAddDevice(8, pager);
          return;
        }

        srs = Serial.parseInt();

        if (srs == 512 | srs == 1200 | srs == 2400) {
          pager->rate  = srs;
        }

        printSet(String(pager->rate));
        printAddDevice(8, pager);
        return;
      }

    case 8: {
        reserBuf();
        printPager(pager, true, true);
        Serial.println(F("\n\n   [1] - Save, [2] - Cancel"));
        while (Serial.available() == 0) {
        }
        srs = Serial.parseInt();
        switch (srs) {
          case 1: {
              addPager(pager,  false);
              //   memset(&pager, 0, sizeof(Pager));
              Serial.println(F("Saved!\r\r\r"));
              printListMode();
              reserBuf();
              pauseCmd(0);
              break;
            }
          case 2: {
              reserBuf();
              pauseCmd(0);
              printListMode();
              break;
            }
          default: {
              reserBuf();
              Serial.println(F("Sorry?"));
              printAddDevice(8, pager);
              break;
            }
        }

      }
  }
}





void printSending() {
  Serial.println(F("Sending..."));
}

void printSent() {
  Serial.println(F("Message sent"));
}

void printTrError() {
  Serial.println(F("Transmit error =/"));
}
