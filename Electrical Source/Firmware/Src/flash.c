/****************************************************
*flash.c
*This file contains functions to facilitate storing and
* reading stored data to and from the onboard flash memory
****************************************************/

//Includes
#include "comport.h"
#include "globals.h"
#include "main.h"
#include "flash.h"
#include <string.h>
//Software init



//---------------------------------------------------
int FLASH_Unlock()
{
	/* Unlocking FLASH_PECR register access */
	if((FLASH->PECR & FLASH_PECR_PELOCK) != RESET)
	{  
		//FLASH->PEKEYR = FLASH_PEKEYR_PEKEYR_Msk;
		FLASH->PEKEYR = UNLOCK_KEY_1;
		FLASH->PEKEYR = UNLOCK_KEY_2;
		return 0; //No err
	}
	return -1; //Not reset	 
}
//---------------------------------------------------
int FLASH_Lock()
{
	while ((FLASH->SR & FLASH_SR_BSY) != 0) //Wait for not busy
	{
	 /* For robust implementation, add here time-out management */
	}
	FLASH->PECR |= FLASH_PECR_PELOCK; //Lock flash write
	
	return 0;
}
//---------------------------------------------------
int EEPROM_WriteByte(unsigned char byte, unsigned int addr)
{
	/*
	if((FLASH->PECR & FLASH_PECR_PELOCK) == RESET)
	{
		return -1; //Write protected
	}*/
	
	unsigned char * ptr = (unsigned char *) addr;
	*ptr = byte;
	return 0; //No err	
}
//---------------------------------------------------
int EEPROM_WriteData(unsigned char * str, int len, unsigned int addr)
{
	int i;
	for(i=0; i<len; i++)
	{
		if(EEPROM_WriteByte(*(str), addr))
		{
			return -1; //Error
		}
		str++;
		addr++;
	}
	return 0; //No err
}
//---------------------------------------------------
int FLASH_ReadData(unsigned char * str, int len, unsigned int addr)
{
	int i;
	unsigned char * ptr = (unsigned char *) addr;
	
	for(i=0; i<len; i++)
	{
		*str =  *ptr;
		str++;
		ptr++;
	}
	return 0;
}
//---------------------------------------------------
int FLASH_unlockPrgm(void){
	/* Unlocking FLASH_PECR register access */
	if((FLASH->PECR & FLASH_PECR_PRGLOCK) != RESET)
	{ 
		
		FLASH->PRGKEYR = PRGKEY1;
		FLASH->PRGKEYR = PRGKET2;
		FLASH->PECR &= ~FLASH_PECR_PRGLOCK;
		return 0; //No err
	}
	return -1; //Not reset
}
//---------------------------------------------------
int FLASH_writeWord( uint32_t data, unsigned int addr){
	__disable_irq();
	while ((FLASH->SR & FLASH_SR_BSY) != 0);
	*(__IO uint32_t*)(addr) = data;
	while ((FLASH->SR & FLASH_SR_BSY) != 0)
	{
		/* For robust implementation, add here time-out management */
	}
	__enable_irq();
	if ((FLASH->SR & FLASH_SR_EOP) != 0)
	{
		FLASH->SR = FLASH_SR_EOP;
		return 0;
	}else{
		// Error
		return -1;
	}
}
//---------------------------------------------------
int FLASH_erasePage(unsigned int addr){
	
	FLASH->PECR |= FLASH_PECR_ERASE | FLASH_PECR_PROG; 
	*(__IO uint32_t *)addr = (uint32_t)0;
	while ((FLASH->SR & FLASH_SR_BSY) != 0);
	if ((FLASH->SR & FLASH_SR_EOP) != 0)
	{
		FLASH->SR = FLASH_SR_EOP;
	}else{
		return -1;
	}
	FLASH->PECR &= ~(FLASH_PECR_ERASE | FLASH_PECR_PROG);
	return 0;
}
//---------------------------------------------------
int FLASH_initDataStorage(void){
	uint32_t addr = NEXTSTARTADDR_ADDR;
	uint32_t data = DATA_START_ADDR;
	uint8_t state = 0;
	
	FLASH_Unlock();
	EEPROM_WriteData((unsigned char *)&data, 4, addr);
	
	FLASH_unlockPrgm();
	FLASH_erasePage( DATA_START_ADDR );
	FLASH_Lock();
	
	return state;	
}
//---------------------------------------------------
int FLASH_saveFix( struct GPS_POS position){
	
	uint32_t startAddr;
	uint32_t nextStartAddr;
	uint32_t *data;
	
	char str1[4], str2[4], str3[4];

	
	str1[0] = position.date[4];  str1[1] = position.date[5];
	str1[2] = position.lat[0];   str1[3] = position.lat[1];
	str2[0] = position.NS; 			 str2[1] = position.longt[0];
	str2[2] = position.longt[1]; str2[3] = position.longt[2];
	str3[0] = position.longt[7]; str3[1] = position.longt[8];
	str3[2] = position.longt[9]; str3[3] = position.EW;
	
	
	FLASH_ReadData( &startAddr, 4, NEXTSTARTADDR_ADDR);
	if( startAddr < 0x08005900 ) startAddr = 0x08005900;
	
	nextStartAddr = startAddr + DATA_LENGTH;
	
	if( startAddr >= END_OF_MEMORY) return -1;
	
	FLASH_Unlock();
	//EEPROM_WriteByte( nextStartAddr, NEXTSTARTADDR_ADDR);
	//EEPROM_WriteData( nextStartAddr, 
	EEPROM_WriteData((unsigned char *) &nextStartAddr, 4, NEXTSTARTADDR_ADDR);
	FLASH_unlockPrgm();
	if( (startAddr % 128) == 0){
			FLASH_erasePage( startAddr );
	}
	
	// Written in order: 
	// Time | Date | Lat | NS | Longt | EW
	data = &position.time[0];	// Time: 4/4
	FLASH_writeWord( *data, startAddr);
	data = &position.date[0]; // Date: 4/6
	FLASH_writeWord( *data, startAddr+4);
	data = &str1[0]; 					// Date: 6/6 | Lat 2/10
	FLASH_writeWord( *data, startAddr+8);
	data = &position.lat[2];	// Lat: 6/10
	FLASH_writeWord( *data, startAddr+12);
	data = &position.lat[6];	// Lat: 10/10
	FLASH_writeWord( *data, startAddr+16);
	data = &str2[0];					// NS: 1/1 | Longt: 3/10
	FLASH_writeWord( *data, startAddr+20);
	data = &position.longt[3];// Longt: 7/10
	FLASH_writeWord( *data, startAddr+24);
	data = &str3[0];
	FLASH_writeWord( *data, startAddr+28);
	
	FLASH_Lock();
	
	return 0;
}
//---------------------------------------------------
int FLASH_readFixes(unsigned char *rtn){
	static uint32_t currAddr = DATA_START_ADDR;
	uint32_t lastSavedAddr = 0;
	FLASH_ReadData( &lastSavedAddr, 4, NEXTSTARTADDR_ADDR);
	
	FLASH_ReadData(rtn, DATA_LENGTH, currAddr);
	currAddr = currAddr + DATA_LENGTH;
	
	if(currAddr >= END_OF_MEMORY)
	{
		currAddr = DATA_START_ADDR;
		return -1;
	}
	else if(currAddr > lastSavedAddr)
	{
		currAddr = DATA_START_ADDR; //Reset the read proccess 
		return -1;
	}
	else
	{
		return 0;
	}
}
//---------------------------------------------------
