// contains the scheduler 

#include "scheduler.h"
#include "flash.h"


void scheduler(int* GPS_active, int* XB_VHF_active){
	
	// Initalize Variables
	uint8_t VHF_activeDay;
	uint8_t VHF_startTime;
	uint8_t VHF_endTime;
	uint8_t GPS_hoursBetween;
	uint8_t GPS_minutesBetween;
	
	int minLastTime = 0;		// Last time in minutes
	int minCurTime  = 0;		// Current time in minutes
	int minTimeBetw = 0;		// Time between in minutes
	int difference  = 0;
	int minVHFStartTime = 0;
	int minVHFEndTime   = 0;
	
	LL_RTC_TimeTypeDef sTime;
	LL_RTC_DateTypeDef sDate;
	
	uint8_t gps_lastHours = 0;
	uint8_t gps_lastMins  = 0;
	FLASH_ReadData(&gps_lastHours, 1, LASTHOUR_ADDR);
	FLASH_ReadData(&gps_lastMins, 1, LASTMINS_ADDR);
	
	
	// Read data from EEPROM
	VHF_activeDay = *(uint8_t *)EEPROM_START;
	VHF_startTime = *(uint8_t *)(EEPROM_START + 1);
	VHF_endTime	  = *(uint8_t *)(EEPROM_START + 2);
	GPS_hoursBetween  = *(uint8_t *)(EEPROM_START + 3);
	GPS_minutesBetween  = *(uint8_t *)(EEPROM_START + 4);
	
		// Update time & date
	RTC_getTimeDate( &sTime, &sDate);
	
	minCurTime  = (sTime.Hours * 60) + sTime.Minutes;							// Get the current time in minutes from 0-1439
	minLastTime = (gps_lastHours * 60) + gps_lastMins;						// Get the last time the GPS was activated in minutes
	minTimeBetw = (GPS_hoursBetween * 60) + GPS_minutesBetween;		// Get time between GPS fixes in minutes
	difference = minCurTime - minLastTime;												// Get difference between the two
	minVHFStartTime = VHF_startTime * 60;
	minVHFEndTime = VHF_endTime * 60;
	
	
	// Check VHF Active Period
	if( (VHF_activeDay & (0x01 << (sDate.WeekDay - 1) ) ) != 0){
		if( (minCurTime >= minVHFStartTime) && (minCurTime <= minVHFEndTime) ){
			*XB_VHF_active = 1;
		}else{
			*XB_VHF_active = 0;
		}
	}else{
		*XB_VHF_active = 0;
	};
	


	
	
	if( (difference >= minTimeBetw ) || (difference < 0)  || ( (sTime.Hours == 0x00) && (sTime.Minutes == 0) ) ){
							
		*GPS_active = 1;
		gps_lastHours = sTime.Hours;
		gps_lastMins  = sTime.Minutes;
		FLASH_Unlock();
		EEPROM_WriteByte(gps_lastHours, LASTHOUR_ADDR);
		EEPROM_WriteByte(gps_lastMins, LASTMINS_ADDR);
		FLASH_Lock();
	}
	
	
}
//-------------------------
