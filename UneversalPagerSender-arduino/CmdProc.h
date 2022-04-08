#ifndef _CMDPROC_H_
#define _CMDPROC_H_
#include "Storage.h"



void printWelcome(Rf7021 rf);
void processCommand();
void printWelcome();
void printFreeMode();
void printFreeModeAdv();

void printSending();
void printSent();
void printTrError();
void pstr(String &str);
void reserBuf();

void printListMode();
void printPager(Pager *pager, bool detailed, bool wHeader); // Print pager info
void printAddDevice(byte stepC, Pager *pager) ;
void printSpecMode();
void printFixAdded(byte *fixed);

#endif 
