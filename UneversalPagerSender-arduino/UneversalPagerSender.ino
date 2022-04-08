/*
   POCSAG Universal Transmitter for Arduino Nano

   Extended by Hot Pixel 2022

   Based on https://github.com/SinuXVR/arduino-pocsag-transcoder
   Copyright (c) 2021, SinuX. All rights reserved.

   This library is distributed "as is" without any warranty.
   See MIT License for more details.
*/
#include "Rf7021.h"
#include "PocsagEncoder.h"
#include "CmdProc.h"
#include "Cmd.h"
#include "Storage.h"
#include <SoftwareSerial.h>
#include "MemoryFree.h"


// RF7021 Wiring
#define RF_CE_PIN      6
#define RF_SLE_PIN     7
#define RF_SDATA_PIN   8
#define RF_SREAD_PIN   9
#define RF_SCLK_PIN    10
#define RF_TX_CLK_PIN  11
#define RF_TX_DATA_PIN 12


// RF7021 settings
//#define RF_TCXO 12287154UL      // 12.288 MHz TCXO
#define RF_TCXO 12288114UL      // 12.288 MHz TCXO
#define RF_PRINT_ADF_INFO       // Read and print chip revision, temperature and voltage at startup
#define RF_TEST_MODE 4          // Test mode - "010101..." pattern
#define RF_TEST_DURATION 10000  // Test duration 10 seconds

// Pocsag default settings
#define PCSG_DEFAULT_DATA_RATE 1200        // 1200bps
#define PCSG_DEFAULT_FREQ      160000000UL // 160.000 MHz
#define PCSG_DEFAULT_FREQ_DEV  4500        // 4.5 KHz
#define PCSG_DEFAULT_CAP_CODE  1234567     // 7th frame
#define PCSG_DEFAULT_SOURCE    3           // Alphanumeric
#define PCSG_DEFAULT_ENCODING  0           // Latin (Motorola)

static byte fixedIDs[STORAGE_COUNT];

Rf7021 rf = Rf7021();
PocsagEncoder pocsagEncoder = PocsagEncoder();


void resetFix() {
  for (int i = 0; i < STORAGE_COUNT; i++) {
    fixedIDs[i] = 0xFF;
  }
}
void resetDefaults() {
  rf.setDataRate(PCSG_DEFAULT_DATA_RATE);
  rf.setFrequency(PCSG_DEFAULT_FREQ);
  // Apply default encoder params
  pocsagEncoder.setCapCode(PCSG_DEFAULT_CAP_CODE);
  pocsagEncoder.setSource(PCSG_DEFAULT_SOURCE);
  pocsagEncoder.setEncodingId(PCSG_DEFAULT_ENCODING);
}

void setup() {
  
  // Set transmitter control pins
  rf.setCEPin(RF_CE_PIN);
  rf.setSLEPin(RF_SLE_PIN);
  rf.setSDATAPin(RF_SDATA_PIN);
  rf.setSREADPin(RF_SREAD_PIN);
  rf.setSCLKPin(RF_SCLK_PIN);
  rf.setTxRxCLKPin(RF_TX_CLK_PIN);
  rf.setTxRxDataPin(RF_TX_DATA_PIN);


  // Reset fixed IDs
  resetFix();


  // Apply default frequency and deviation
  rf.setXtalFrequency(RF_TCXO);
  rf.setHasExternalInductor(1);

  rf.setFrequencyDeviation(PCSG_DEFAULT_FREQ_DEV);
  rf.setPowerAmplifierEnabled(1);
  resetDefaults();


  cmdInit(9600);

  setWelcome();
  printFree();
}

void printFree() {
  // Serial.print(F("Free RAM = "));
  // Serial.println(freeMemory());
}


void transmit(String& msg, const Pager& pager, byte mode,  byte raw) {
   
  rf.setFrequency(pager.frequency * 1000);
  rf.setDataInvertEnabled(pager.inversion);
  if (pager.rate != 1200 | pager.rate != 500 | pager.rate != 2400)
    rf.setDataRate(1200);
  else
    rf.setDataRate(pager.rate);
  pocsagEncoder.setCapCode(pager.cap);
  pocsagEncoder.setSource(pager.msgSource);
  pocsagEncoder.setEncodingId(pager.enconding);

  PocsagEncoder::PocsagMessage message;
  if (mode == 0)
    message =  pocsagEncoder.encodeAlphanumeric(msg);
  else if (mode == 1)
    message = pocsagEncoder.encodeNumeric(msg);
  else if (mode == 2) {
    Serial.println(F("Dsnt work. Guru meditation..."));
    // message = pocsagEncoder.encodeTone();
  }


  Serial.print(String(F("Sending to ")));
  if (raw)
    Serial.print(pager.cap);
  else
    Serial.print(pager.alias);
  Serial.print(F("... "));

  if (rf.sendMessage(message.dataBytes, message.messageLength))
    printSent();
  else
    printTrError();
}

/**
   Отправка команды в ручном режиме
*/
void sendCommands(int arg_cnt, char **args) {
  printFree();
  Pager pager;
  for (int i = 0; i < arg_cnt; i++) {
    String arg = String(args[i]);

    // Deviation
    if (arg == "d") {
      if (arg_cnt > i + 1) {
        uint32_t freqDev = atoi(args[i + 1]);
        if (freqDev == 0 | freqDev > 10000) {
          errorIn(F("d"));
          return;
        }
        rf.setFrequencyDeviation(freqDev);
        Serial.print(F("Frequency Deviation: "));
        Serial.print(freqDev / 1000.0, 2);
        Serial.println(F(" KHz"));
      } else {
        errorIn(F("d"));
      }
    }


    // Freq
    if (arg == "f") {
      if (arg_cnt > i + 1) {
        uint32_t freq = atol(args[i + 1]);
        if (freq >= 80000 && freq <= 950000) {
          pager.frequency = freq;
          rf.setFrequency(freq * 1000);
          Serial.print(F("Frequency: "));
          Serial.print(freq / 1000.0, 3);
          Serial.println(F(" MHz"));
        } else {
          Serial.println(freq);
          Serial.println(F("Wrong frequency, should be 80000 to 950000 kHz"));
        }
      } else {
        errorIn(F("f"));
      }
    }

    //Inversion
    if (arg == "i") {
      if (arg_cnt > i + 1) {
        byte inv = atoi(args[i + 1]);
        if (inv > 1) {
          errorIn(F("i"));
          return;
        } else {
          pager.inversion = inv;
          Serial.print(F("Inversion: "));
          Serial.println(inv ? F("enabled") : F("disabled"));
        }
      } else {
        errorIn(F("i"));
      }
    }

    // Baut
    if (arg == "r") {
      if (arg_cnt > i + 1) {
        word rate = atoi(args[i + 1]);
        if (rate == 512 || rate == 1200 || rate == 2400) {
          pager.rate = rate;
          Serial.print(F("Data rate: "));
          Serial.println(rate);
        } else {
          rf.setDataRate(rate);
          Serial.println(F("Wrong data rate, should be 512, 1200 or 2400"));
        }
      } else {
        errorIn(F("r"));
      }
    }

    // Test
    if (arg == "x") {
      Serial.println(F("Testing..."));
      rf.setFrequency(pager.frequency);
      rf.txTest(1, 15000);
      Serial.println(F("Test completed"));
    }

    // Тонкая подстройка частоты кварца.
    // Позволяет компенсировать погрешность кварца и "подтянуть" частоту передачи
    // к эталонной. После этой команды передается тестовый сигнал, который поможет
    // определить степень подстройки. После подстройки необходимо вписать полученное
    // значение в константу RF_TCXO

    // Увеличить частоту, Гц.
    if (arg == "h") {
      if (arg_cnt > i + 1) {
        int argUp = atoi(args[i + 1]);
        rf.setXtalFrequency(rf.getXtalFrequency() + argUp);
        Serial.println(rf.getXtalFrequency());
        rf.txTest(1, 3000);
      } else {
        errorIn(F("u"));
      }
    }

    // Понизить частоту, Гц
    if (arg == "l") {
      if (arg_cnt > i + 1) {
        int argUp = atoi(args[i + 1]);
        rf.setXtalFrequency(rf.getXtalFrequency() - argUp);
        Serial.println(rf.getXtalFrequency());
        rf.txTest(1, 3000);
      } else {
        errorIn(F("j"));
      }
    }

    // CAP
    if (arg == "c") {
      if (arg_cnt > i + 1) {
        uint32_t cap = atol(args[i + 1]);
        if (cap <= 9999999) {
          pager.cap = cap;
          Serial.print(F("CAP code: "));
          Serial.println(cap);
        } else {
          Serial.println(F("Wrong CAP"));

          return;
        }
      } else {
        errorIn(F("c"));
      }
    }

    // Source code
    if (arg == "s") {
      if (arg_cnt > i + 1) {
        byte src = atoi(args[i + 1]);
        if (src <= 3) {
          pager.msgSource = src;
          Serial.print(F("Source code: "));
          Serial.println(src);
        } else {
          Serial.println(F("Wrong source code, should be 0, 1, 2 or 3"));

          return;
        }
      } else {
        errorIn(F("s"));
      }
    }


    // Encoding
    if (arg == "e") {
      if (arg_cnt > i + 1) {
        byte enc = atoi(args[i + 1]);
        if (enc <= 3) {
          pager.enconding = enc;
          Serial.print(F("Encoding: "));
          Serial.print(enc);
          Serial.println(enc == 0 ? F(" (EN Motorola)") : enc == 1 ? F(" (EN/RU Motorola)") : enc == 2 ? F(" (EN/RU Philips)") : enc == 3 ? F(" (RU Motorola)") : F(" (Unknown)"));
        } else {
          Serial.println(F("Wrong encoding, should be 0, 1, 2 or 3"));

          return;
        }
      } else {
        errorIn(F("e"));
      }
    }

    // Send numeric message
    if (arg == "n") {
      if (arg_cnt > i + 1) {
        String msg = args[i + 1];
        transmit(msg, pager, 1, 1);
        delay(30);
      } else {
        errorIn(F("n"));
      }
    }

    // Send text message
    if (arg == "m") {
      printFree();
      if (arg_cnt > i + 1) {

        String msg = "";
        for (int ii = i + 1; ii < arg_cnt; ii++) {
          msg = msg + String(args[ii]) + " ";
        }

        transmit(msg, pager, 0, 1);
        delay(30);
        printFree();
      } else {
        Serial.println(F("Message empty"));
      }
      break;
    }


    // Send tone message
    if (arg == "t") {
      //  PocsagEncoder::PocsagMessage message = pocsagEncoder.encodeTone();
      //  printSending();
      //   rf.sendMessage(message.dataBytes, message.messageLength);

      String msg = "";
      transmit(msg, pager, 2, 1);
      printSent();
    }
  }
}

/**
   Фиксация нужных пейджеров (для отпавки одного сообщения на несколько устройств)
*/
void addFixedIDs(int arg_cnt, char **args) {
  if (arg_cnt == 1) {
    Serial.println(F("No IDs.."));
    return;
  }
  resetFix();
  for (byte i = 1; i < arg_cnt; i++) {
    fixedIDs[i - 1] = atoi( args[i]);
  }
  printFixAdded(fixedIDs);
}


/**
   Отправка на зафиксированные пейджеры
*/
void sendToFixed(int arg_cnt, char **args) {

  if (arg_cnt == 1) {
    Serial.println(F("Message not found"));
    return;
  }

  if (fixedIDs[0] == 0xff) {
    Serial.println(F("Set IDs first"));
    return;
  }

  String msg = "";
  for (byte i = 1; i < arg_cnt; i++) {
    for (int ii = i + 1; ii < arg_cnt; ii++) {
      msg += String(args[ii]) + " ";
    }
  }

  for (int i = 0; i < STORAGE_COUNT ; i++) {
    if (fixedIDs[i] == 0xff)
      continue;

    Pager pager;
    getPager(fixedIDs[i], &pager);

    // if detect pager struct
    if (pager.crc[0] == CRC[0] && pager.crc[1] == CRC[1]) {
      transmit(msg,  pager, 0, 0);
    } else {
      Serial.println(String(F("ID not found: ")) + args[i]);
    }
    delay(30);
  }
  printFree();
}

void sendSpecCmdTo(int arg_cnt, char **args) {
  String msg = "";
  int indexMsg = -1;
  byte mode = 0; // 0 - text, 1 - numeric, 2 - tone

  // Ищем тело сообщения
  for (byte i = 0; i < arg_cnt; i++) {
    if ((strcmp(args[i], "send") == 0 | strcmp(args[i], "numeric") == 0) && arg_cnt > i + 1) {
      indexMsg = i;
      if (strcmp(args[i], "send") == 0)
        mode = 0;
      else
        mode = 1;
      for (int ii = i + 1; ii < arg_cnt; ii++) {
        msg = msg + String(args[ii]) + " ";
      }
      break;
    }
    // if Tone mode
    if (strcmp(args[i], "tone") == 0) {
      indexMsg = i;
      mode = 2;
    }
  }

  if (mode != 2 && indexMsg < 0) {
    errorIn(F("Message not found"));
    return;
  }

  // Ищем IDы
  if (indexMsg < 2) {
    errorIn(F("IDs not found"));
    return;
  }
  printFree();

  for (int i = 1; i < indexMsg; i++) {
    Pager pager;
    getPager(atoi(args[i]), &pager);


    if (pager.crc[0] == CRC[0] && pager.crc[1] == CRC[1]) {
      transmit(msg, pager, mode, 0);
      delay(30);
    } else {
      Serial.println(String(F("ID not found: ")) + args[1]);
    }
    printFree();
  }
}


/**
   Режим приветствия
*/
void setWelcome() {
  printFree();
  printWelcome(rf);
  clearCmd();
  cmdAdd("1", setFreeMode);
  cmdAdd("2", setSpecMode);
  cmdAdd("3", setSavedPagers);
  cmdAdd("0", setWelcome);
  cmdAdd("send", sendCommands);
  cmdAdd("testfreq", startTestFreq);
  cmdAdd("testrate", startRateTest);
  printFree();
}

/**
   Режим отправки на сохраненные устройства
*/
void setSpecMode() {
  printSpecMode();
  clearCmd();
  cmdAdd("to", sendSpecCmdTo);
  cmdAdd("fix", addFixedIDs);
  cmdAdd("send", sendToFixed);
  cmdAdd("0", setWelcome);
  printFree();
}


void addDevice() {
  struct Pager pager;
  pager.addr = -1;
  printAddDevice(0, &pager);
}

void removeDevice(int arg_cnt, char **args) {
  if (arg_cnt == 1) {
    return;
  }
  struct Pager pager;
  getPager(atoi(args[1]), &pager);
  addPager(&pager,  true);
  setSavedPagers() ;
}


void editDevice(int arg_cnt, char **args) {
  if (arg_cnt == 0) {
    return;
  }
  pauseCmd(1);
  struct Pager pager ;
  getPager(String(args[1]).toInt(), &pager);
  printAddDevice(0, &pager);
}

void setSavedPagers() {
  printListMode();
  clearCmd();
  cmdAdd("add", addDevice);
  cmdAdd("del", removeDevice);
  cmdAdd("edit", editDevice);
  cmdAdd("0", setWelcome );
  printFree();
}


void setFreeMode() {
  printFreeMode();
  clearCmd();
  cmdAdd("full", printFreeModeAdv);
  cmdAdd("0", setWelcome);
  cmdAdd("send", sendCommands);
  printFree();
}

/**
   Тест для поиска точной частоты приемника
   Базовой установленной частоте дается гистерезис 20 кГц
    и производится цикличная отправка с текстом текущей частоты
*/
void startTestFreq() {
  long fr = rf.getFrequency() - 20000;
  for (int i = 0; i < 40; i = i + 1) {
    fr += 1000;
    rf.setFrequency(fr);
    PocsagEncoder::PocsagMessage message = pocsagEncoder.encodeAlphanumeric(String(fr / 1000));
    Serial.println(String("sending ") + (fr / 1000));
    rf.sendMessage(message.dataBytes, message.messageLength);
    delay(20);
  }
  Serial.println(String("Complete."));
}

/**
   Тест для поиска рабочей скорости передачи.
   Если используется кварц без термостабилизации (не TCXO),
    возможен фактический уход таймингов от установленных
*/
void startRateTest() {
  word rate = 1190;
  for (int i = 0; i < 20; i = i + 1) {
    rate = rate + 1;
    rf.setDataRate(rate);
    PocsagEncoder::PocsagMessage message = pocsagEncoder.encodeAlphanumeric(String(rate));
    Serial.println(String("sending ") + (rate));
    rf.sendMessage(message.dataBytes, message.messageLength);
    delay(130);
  }
  Serial.println(String("sending "));
}


void unrecognized()
{
  Serial.println(F("Sorry?"));
}

void errorIn(const & msg) {
  Serial.print(F("Error in ") );
  Serial.println(msg);
}


void loop() {
  cmdPoll();
}
