/****************************
*Header file for gps function calls 
* and macros
****************************/

#ifndef GPS_H_
#define GPS_H_

#include <stdint.h>
#include "stm32l0xx_ll_rtc.h"

//Macros
//----------------------------------------------
#define GOOD_WORD 0
#define NO_DATA -2
#define ERROR	-1

#define IDLE_LINE (char)0xFF
#define LINE_FEED (char)10

#define GPS_TIMEOUT 30000
#define GPS_ACC_REQ 50
#define PUBX_POS_NUM_FIELDS	23
#define GPS_RX_BUFF_SIZE	125
#define ACC_TEMP_SIZE	4
#define GPS_FIRST_FIX_CYCLES	4		// nNumber of times to try for time/date on first fix -- in multiples of 30s; 10 = 5 minutes of attempts


#define SYNC_CHAR_1 0xB5
#define SYNC_CHAR_2 0x62
#define BFR_CLR_TIMEOUT 40

//Data Word Struct
//----------------------------------------------
/*struct gpsword
{
	char id[2];
	char format[3];
	char data[50];	
};*/


struct UBXmessage{
	uint8_t msgClass;
	uint8_t msgID;
	uint16_t length;
	char payloadLoc[50];
	uint8_t checksumA;
	uint8_t checksumB;
	uint8_t status;
};

struct UBXTXInfo{
	uint8_t status;
	uint8_t checksumA;
	uint8_t checksumB;
};

struct GPS_POS{
	char 		time[4];
	char		date[6];
	char 		lat[10];		// Latitude
	char 		NS;					// North/South Indicator
	char 		longt[10];  // Longitude
	char 		EW;					// East/West Indicator
	float		acc;				// Horizontal Accuracy
};

//Prototypes
//----------------------------------------------
void GPS_GPSEnable(void);
void GPS_GPSDisable(void);
void GPS_GPSCSLow(void);
void GPS_GPSCSHigh(void);
void GPS_SendByte(char byte);
void GPS_SendData(char * data, int size);
void GPS_ClockInByte(char * byte);
int GPS_GetDataWord(char * data, int len);
//int GPS_SendDataWord(struct gpsword

struct UBXTXInfo GPS_UBX_sendMSG(struct UBXmessage msg, char payload[50]);
int GPS_UBX_requestMSG(struct UBXmessage msg);

void clearGPSBuffer(void);

int GPS_UBX_enablePUBX_Position(void);

struct GPS_POS GPS_getNMEA(struct GPS_POS position);

int GPS_subroutine(void);


#endif //GPS_H_