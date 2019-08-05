/****************************************************
*comport.c
*This file contains functions to facilitate communications
*	between the collar and the user via the USB/UART bridge
****************************************************/

//Includes
#include "comport.h"
#include "globals.h"
#include "main.h"
#include "flash.h"
#include <string.h>
#include <stdio.h>
#include "xbee.h"
#include "timers.h"
//Software init
	char ComRxBuff[BUFFER_SIZE];	//Ring bufer for raw input
	int ComRxWriteIndex = 0;	//Current write index of buffer
	int ComRxReadTo;
	int ComRxReadFrom;


//---------------------------------------------------
int CC_ComPortPresent()
{
	
#ifndef __PRODUCTION_
		return 0;
#endif	
	
	
	if( (USB_PRSNT_GPIO_Port->IDR & USB_PRSNT_Pin) == USB_PRSNT_Pin ){
		return 1;
	}else{
		return 0;
	}
	
	//return TRUE; //Placeholder function, should poll the usb
							//present line in future
}
//---------------------------------------------------
int CC_SendByte(char byte)
{
	LL_USART_TransmitData8(USART1, byte);
	
	//Empty wait statement, should use sleep in future
		while(!((USART1->ISR & USART_ISR_TC) == USART_ISR_TC));
	return 0; //No err
}
//---------------------------------------------------
int CC_SendData(char * string, int size)
{
	int i;
	
	for(i = 0; i < size; i++)
	{
		CC_SendByte(* string);
		string++;
	}
	return 0; //No err	
}
//---------------------------------------------------
int CC_CheckForHandshake()
{
	int i,j;
	char temp[5];
	
	temp[4] = 0; //Null end char
	
	//Due to ring buffer, this will read until write index is hit
	for(i=ComRxReadFrom; i!=ComRxWriteIndex; i = (i + 1) % BUFFER_SIZE)
	{
		//shifts temp string to try to fit "ACK?"
		for(j = 0; j < 3; j++)
		{
			temp[j] = temp[j+1];
		}
		temp[j] = ComRxBuff[i];
		
		if(ComRxBuff[i] == EOC) //Found end of command
		{
			if(!strcmp(temp, HANDSHAKE))	//Found handshake
			{
				CC_SendData(HANDSHAKE, strlen(HANDSHAKE));
				ComRxReadFrom = i + 1;
				return YES;
			}//Handshake		
			ComRxReadFrom = i + 1;//Update anyway 
		}//if EOC	
		
	}//For loop
	
	return NO;
}
//---------------------------------------------------
int CC_ParseCommand()
{
	int i, currWriteIndex = ComRxWriteIndex; //Need temp index so global won't be 
																					//iterated during function call. makes errors
	char temp[50];
	
	//Check for end of command first in buffer
	//Ideally, this sort of buffering should allow for multiple 
	//	commands in the buffer at once 
	for(i=ComRxReadFrom; i!=currWriteIndex; i = (i+1)%BUFFER_SIZE)
	{
		if(ComRxBuff[i] == EOC)
		{
			ComRxReadTo = i + 1;
			break;
		}
	}	
	if(i == currWriteIndex) return NO; //No full command found
	
	memset(temp, 0, sizeof(temp));//clears string 
	i = 0;
	//If command found, read command to local string
	for(; ComRxReadFrom!=ComRxReadTo; ComRxReadFrom=(ComRxReadFrom + 1)%BUFFER_SIZE)
	{
		temp[i++] = ComRxBuff[ComRxReadFrom];
	}
	//Check command against known strings 	
	// Return case for switch case statement
	if(!strcmp(temp, TIMESET)) return TIMESET_N;
	else if(!strcmp(temp, PROGRAM)) return PROGRAM_N;
	else if(!strcmp(temp, TIMESTAMP)) return TIMESTAMP_N;
	else if(!strcmp(temp, GETPGM)) return GETPGM_N;
	else if(!strcmp(temp, CONFIGXBEE)) return CONFIGXBEE_N;
	else if(!strcmp(temp, GETXBCFG)) return GETXBCFG_N;
	else if(!strcmp(temp, GETSN)) return GETSN_N;
	else if(!strcmp(temp, ERSFLASH)) return ERSFLASH_N;
	else if(!strcmp(temp, DOWNLOAD)) return DOWNLOAD_N;
	
	else return NO;	//Default for unknown command
}
//---------------------------------------------------
int CC_ExecuteCommand(int command)
{
	switch(command)
	{
		//Commands run here
		//-------------------------------------------------------------
		case TIMESET_N:
			return CC_SetRTC();
			break;
		case TIMESTAMP_N:
			return CC_SendTimestamp();
			break;
		case PROGRAM_N:
			return CC_SetProgram();
			break;
		case GETPGM_N:
			return CC_ReadProgram();
			break;
#ifdef __LARGE_COLLAR_		
		case CONFIGXBEE_N:
			return CC_ConfigureXbee();
			break;
		case GETXBCFG_N:
			return XB_GetConfig();
			break;
		case GETSN_N:
			return XB_GetSN();
			break;
#endif		
		case ERSFLASH_N:
			return FLASH_initDataStorage();
		case DOWNLOAD_N:
			return CC_DownloadFixes();
			break;
		//-------------------------------------------------------------
		default:	//Unprogrammed response should be 0 but this is safer
			return -1;//No command executed
			break;
	
	}
}
//---------------------------------------------------
int CC_SetRTC()
{
	//ssmmhhddmmyyd? = 14 
	char time[15];
	int i;
	LL_RTC_TimeTypeDef currtime;
	LL_RTC_DateTypeDef currdate;
	
	//Check for data in buffer
	
	TIM2_initDelay_inline( COMPORT_TIMEOUT	);
	while( ((TIM2->SR & TIM_SR_UIF) == 0) && (ComRxBuff[ComRxWriteIndex-1] != EOC)) //Waiting for end of second transmission
	{
		
	}		//Really dirty way of doing this, @TODO fix this
	ComRxReadTo = ComRxWriteIndex - 1;
	
	for(i = 0; ComRxReadFrom != ComRxReadTo; ComRxReadFrom = (ComRxReadFrom+1)%BUFFER_SIZE)
	{
		time[i++] = ComRxBuff[ComRxReadFrom];
	}	
	time[14] = 0;	
	
	
	currtime.TimeFormat = LL_RTC_HOURFORMAT_24HOUR;
	
	//ascii MSD - 48 to get num then * 10 to left shift the decimal, then add LSD
	currtime.Seconds = ((time[0]-48) * 10) + time[1]-48;
	currtime.Minutes = ((time[2]-48) * 10) + time[3]-48;
	currtime.Hours = ((time[4]-48) * 10) + time[5]-48;
	
	currdate.Day = LL_RTC_DATE_GetDay(RTC);
	
	//currdate.Day = ((time[6]-48) * 10) + time[7]-48;
	currdate.Day = ((time[6]-48) * 10) + time[7]-48;
	currdate.Month = ((time[8]-48) * 10) + time[9]-48;
	currdate.Year = ((time[10]-48) * 10) + time[11]-48;
		
	currdate.WeekDay = (time[12]-48)%7; //Python formatting
	
	//Write to RTC
	
	//Readback from RTC
	
	//LL_RTC_DATE_SetDay(RTC, currdate.Day);
	
	LL_RTC_TIME_Init(RTC, LL_RTC_FORMAT_BIN, &currtime);
	LL_RTC_DATE_Init(RTC, LL_RTC_FORMAT_BIN, &currdate);
	
	currtime.Seconds = LL_RTC_TIME_GetSecond(RTC);
	currtime.Minutes = LL_RTC_TIME_GetMinute(RTC);
	currtime.Hours = LL_RTC_TIME_GetHour(RTC);
	currdate.Day = LL_RTC_DATE_GetDay(RTC);
	currdate.Month = LL_RTC_DATE_GetMonth(RTC);
	currdate.Year = LL_RTC_DATE_GetYear(RTC);
	currdate.WeekDay = LL_RTC_DATE_GetWeekDay(RTC);
	//LL_RTC_DATE_Get
	
	
	
	sprintf(time, "%x%x%x%x%x%x%x", currtime.Seconds,
		currtime.Minutes, currtime.Hours, currdate.Day,
		currdate.Month, currdate.Year, currdate.WeekDay);
	CC_SendData(time, strlen(time));
	//function call check
	//char str[5] = "RESP";
	//CC_SendData(str, 4);
	return 0;
	
}
//---------------------------------------------------
int CC_SendTimestamp()
{
	//ssmmhhddmmyyd? = 14 
	char time[15];
	int i;
	LL_RTC_TimeTypeDef currtime;
	LL_RTC_DateTypeDef currdate;
	
	
	//Populate struct
	currtime.Seconds = LL_RTC_TIME_GetSecond(RTC);
	currtime.Minutes = LL_RTC_TIME_GetMinute(RTC);
	currtime.Hours = LL_RTC_TIME_GetHour(RTC);
	currdate.Day = LL_RTC_DATE_GetDay(RTC);
	currdate.Month = LL_RTC_DATE_GetMonth(RTC);
	currdate.Year = LL_RTC_DATE_GetYear(RTC);
	currdate.WeekDay = LL_RTC_DATE_GetWeekDay(RTC);
	
	
	
	sprintf(time, "%x%x%x%x%x%x%x", currtime.Seconds,
		currtime.Minutes, currtime.Hours, currdate.Day,
		currdate.Month, currdate.Year, currdate.WeekDay);
	CC_SendData(time, strlen(time));
	//function call check
	//char str[5] = "RESP";
	//CC_SendData(str, 4);
	return 0;	
}
//---------------------------------------------------
int CC_SetProgram()
{ 
	unsigned char str[20];
	int i, ret;	
	uint8_t program[MAX_PGM_LEN];
	memset(program, 0, sizeof(program));
	
	//Check for data in buffer
	TIM2_initDelay_inline( COMPORT_TIMEOUT	);
	while( ((TIM2->SR & TIM_SR_UIF) == 0) && (ComRxBuff[ComRxWriteIndex-1] != EOC)) //Waiting for end of second transmission
	{
		
	}		//Really dirty way of doing this, @TODO fix this
	ComRxReadTo = ComRxWriteIndex - 1;
	
	memset(str, 0, sizeof(str));//Clear string 
	//Clear EEPROM region
	FLASH_Unlock();
	ret = EEPROM_WriteData((unsigned char *)str, MAX_PGM_LEN, DATA_EEPROM_BASE);
	FLASH_Lock();
	
	//Get data from buffer
	for(i = 0; ComRxReadFrom != ComRxReadTo; ComRxReadFrom = (ComRxReadFrom+1)%BUFFER_SIZE)
	{
		str[i++] = ComRxBuff[ComRxReadFrom];
	}	
	//Skip EOC
	ComRxReadFrom++;
	ComRxReadTo++;
	
	
	
	//Format program array
	/* day of the week
		 hour vhf start
		 hour vhf end
		 hours between fix
		 min between fix
	*/
	program[0] = (uint8_t)str[8];
	program[1] = ((str[4]-48)<<4)|(str[5]-48);
	program[2] = ((str[6]-48)<<4)|(str[7]-48);
	program[3] = ((str[0]-48)*10)+(str[1]-48);
	program[4] = ((str[2]-48)*10)+(str[3]-48);
		
	//Program flash starting at EEPROM Base addr
	FLASH_Unlock();
	ret = EEPROM_WriteData((unsigned char *)program, sizeof(program), DATA_EEPROM_BASE);
	FLASH_Lock();
	return ret;
}
//---------------------------------------------------
int CC_ReadProgram()
{
	int i;
	unsigned char str[MAX_PGM_LEN];
		
	//Get data from EEPROM
	FLASH_ReadData(str, MAX_PGM_LEN, DATA_EEPROM_BASE);

	CC_SendData((char *) str, strlen((char *)str));
	return 0;
}
//---------------------------------------------------
#ifdef __LARGE_COLLAR_
int CC_ConfigureXbee()
{	
	char str[MAX_PGM_LEN];
	int i, ret;	
	
	//Check for data in buffer
	TIM2_initDelay_inline( COMPORT_TIMEOUT	);
	while( ((TIM2->SR & TIM_SR_UIF) == 0) && (ComRxBuff[ComRxWriteIndex-1] != EOC)) //Waiting for end of second transmission
	{
		
	}		//Really dirty way of doing this, @TODO fix this
	ComRxReadTo = ComRxWriteIndex - 1;
	
	memset(str, 0, sizeof(str));//Clear string 
	
	//Get data from buffer
	for(i = 0; ComRxReadFrom != ComRxReadTo; ComRxReadFrom = (ComRxReadFrom+1)%BUFFER_SIZE)
	{
		str[i++] = ComRxBuff[ComRxReadFrom];
	}
	//Skip EOC
	//ComRxReadFrom++;
	//ComRxReadTo++;	
	
	return XB_ConfigureDevice(str, strlen(str));	
}
#endif
//---------------------------------------------------
int CC_DownloadFixes()
{
	//We need to read all fixes a word at a time 
	//Start loop from the first address
	//Loop until we are at the last write address 
	//Parse out these fixes into a string and then send that over 
	//This should work for both comport and 
	
	char buff[DATA_LENGTH + 1];
	
	while(!FLASH_readFixes((unsigned char *)buff))
	{
		//Send time
		CC_SendData(buff, 2);
		CC_SendByte(',');
		CC_SendData(&buff[2], 2);
		CC_SendByte(',');
		
		//Send Date
		CC_SendData(&buff[4], 2);
		CC_SendByte(',');
		CC_SendData(&buff[6], 2);
		CC_SendByte(',');
		CC_SendData(&buff[8], 2);
		CC_SendByte(',');

		//Send Lat
		CC_SendData(&buff[10], 10);
		CC_SendByte(',');		
		CC_SendData(&buff[20], 1);
		CC_SendByte(',');
		
		//Send Long
		CC_SendData(&buff[21], 10);
		CC_SendByte(',');
		CC_SendData(&buff[31], 1);
		CC_SendByte(',');
		CC_SendByte('\n');
		memset(buff, 0, sizeof(buff));
	}
	CC_SendByte(EOC);
	
	return 0;
}	

int setLED(int state){
	if( state == 1){
		LED0_GPIO_Port -> ODR |= LED0_Pin;
		return 0;
	}else if(state == 2){
		LED0_GPIO_Port -> ODR ^= LED0_Pin;
		return 0;
	}else{
		LED0_GPIO_Port -> ODR &= ~LED0_Pin;
		return 0;
	}
}
