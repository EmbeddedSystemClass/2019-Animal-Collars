/****************************
*Header file for flash function calls 
* and macros
****************************/

#ifndef FLASH_H_
#define FLASH_H_

#include "gps.h"
#include "main.h"

//Macros
//----------------------------------------------
#define UNLOCK_KEY_1 0x89ABCDEF
#define UNLOCK_KEY_2 0x02030405
#define PRGKEY1 0x8C9DAEBF
#define PRGKET2	0x13141516

#define PAGE_SZE 128 // in bytes
#define DATA_LENGTH 32

#define DATA_START_ADDR 	0x08005900  // Page 178

#define END_OF_MEMORY			0x08017FFF

#define NEXTSTARTADDR_ADDR  0x08080014


// We can fit 4 fixes per page
// @ 1 word per fix

//	EEPROM MAP
//	EEPROM START 				0x08080000	
//	VHF_activeDay 			0x08080000
//	VHF_startTime				0x08080001
//	VHF_endTime					0x08080002
//	GPS_hoursBetween		0x08080003
// 	GPS_minutesBetween	0x08080004
// 	GPS_req_acc					  
//			----
//  NEXTSTARTADDR_ADDR	0x08080014
// 			----
//	GPS_lastHour				0x08080030
//  GPS_lastMinute			0x08080031

// End of memory is 0x0800 3FFF

//Prototypes
//----------------------------------------------
int FLASH_Unlock();
int FLASH_Lock();
int EEPROM_WriteByte(unsigned char byte, unsigned int addr);
int EEPROM_WriteData(unsigned char * str, int len, unsigned int addr);

int FLASH_unlockPrgm(void);
int FLASH_ReadData(unsigned char * str, int len, unsigned int addr);
int FLASH_erasePage(unsigned int addr);


int FLASH_writeWord(uint32_t data, unsigned int addr);

int FLASH_initDataStorage(void);
int FLASH_saveFix(struct GPS_POS position);

int FLASH_readFixes(unsigned char *rtn);

#endif //FLASH_H_