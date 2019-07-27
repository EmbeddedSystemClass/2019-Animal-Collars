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
	
	// Check VHF Active Period
	if( (VHF_activeDay && (0x01 << sDate.WeekDay) ) != 0){
		if( (sTime.Hours >= VHF_startTime) && (sTime.Hours < VHF_endTime) ){
			*XB_VHF_active = 1;
		}else{
			*XB_VHF_active = 0;
		}
	}else{
		*XB_VHF_active = 0;
	};
	
	if( (sTime.Hours == 0x00) && (sTime.Minutes == 0) ){
		*GPS_active = 1;
		
		gps_lastHours = sTime.Hours;
		gps_lastMins  = sTime.Minutes;
		
	}else if( ( (sTime.Hours - GPS_hoursBetween) >= gps_lastHours) &
						(	(sTime.Minutes - GPS_minutesBetween) >= gps_lastMins) ){
							
		*GPS_active = 1;
		gps_lastHours = sTime.Hours;
		gps_lastMins  = sTime.Minutes;
		EEPROM_WriteByte(gps_lastHours, LASTHOUR_ADDR);
		EEPROM_WriteByte(gps_lastMins, LASTMINS_ADDR);
	}
	
	
}
//-------------------------
