/****************************************************
*xbee.c
*This file contains functions to facilitate communications
* with a researchers computer via the xbee device onboard
****************************************************/



//Includes
#include "comport.h"
#include "globals.h"
#include "main.h"
#include "flash.h"
#include <string.h>
#include "xbee.h"
#include "timers.h"
#include "lowpower.h"

//Software init
char XbeeRxBuff[BUFFER_SIZE];	//Ring bufer for raw input
int XbeeRxWriteIndex;	//Current write index of buffer
int XbeeRxReadTo;
int XbeeRxReadFrom;
int XbeeRxBufferActive;	//Flag set to signify the existence of new data in the Xbee buffer
int XbeeDongleConnected;
//---------------------------------------------------
int XB_XbeeSubroutine()
{
	//Enable XBee
	XB_EnableXbee();	
	
	// Enable USART4:
	USART4->CR1 |= USART_CR1_UE;
	
	
	// Start Timeout timer
	// Delay is in ms 
	// Clear the update event flag 
	TIM2->SR = 0;
	
	// Make sure it's reset
	TIM2->CNT = 0;
	
	// Move the delay into the ARR:
	TIM2->ARR = SUBROUTINE_TIMEOUT;
	
	// Start the timer:
	TIM2->CR1 |= TIM_CR1_CEN;
	
	
	//Check for dongle handshake and timeout
	//While not timed out 
	while(!(TIM2->SR & TIM_SR_UIF))
	{
		if(!XB_CheckForDongle())//Handshake complete			
		{	
			//Set flag (Can be cleared by some xbee functions)
			XbeeDongleConnected = YES;
			
			//Set connected timeout
			//Same timeout logic as before
			TIM2->SR = 0;						
			TIM2->CNT = 0;			
			TIM2->ARR = SUBROUTINE_TIMEOUT;
			TIM2->CR1 |= TIM_CR1_CEN;		
			
			
			while(XbeeDongleConnected && !(TIM2->SR & TIM_SR_UIF))
			{
				XB_ExecuteCommand(XB_ParseCommand());			
			}//While connected
		}//Look for connection request
	}//While not connection request timeout
		
	XbeeDongleConnected = NO;
	// Disable USART4:
	USART4->CR1 &= ~USART_CR1_UE;
	XB_DisableXbee();
	return 0;//Maybe do something here for flag checking 
}
//---------------------------------------------------
void XB_EnableXbee()
{
	XBEE_EN_GPIO_Port->ODR &= ~XBEE_EN_Pin;
	TIM2_delay(500);
}
//---------------------------------------------------
void XB_DisableXbee()
{
	XBEE_EN_GPIO_Port->ODR |= XBEE_EN_Pin;
}
//---------------------------------------------------
int XB_SendByte(char byte)
{
	// Enable TX Complete Interrupt
	//USART4->CR1 |= LL_USART_CR1_TCIE;
	
	LL_USART_TransmitData8(USART4, byte);
	while(!((USART4->ISR & LL_USART_ISR_TC) == LL_USART_ISR_TC));
	/*
	if( !((USART4->ISR & LL_USART_ISR_TC) == LL_USART_ISR_TC) ){
		LPM_sleep();
	}
	*/

	return 0;
}
//---------------------------------------------------	
int XB_SendData(char * string, int size)
{
	int i;
	for(i=0; i<size; i++)
	{
		XB_SendByte(* string);
		string++;
	}
	return 0;
}
//---------------------------------------------------
int XB_GetResponse(char * resp, int *len)
{
	int i;
	
	//Check for data in buffer (Wait for resp from device)
	TIM2_initDelay_inline(PROGRAM_TIMEOUT);
	while((XbeeRxBuff[XbeeRxWriteIndex-1] != CRtn) && !(TIM2->SR & TIM_SR_UIF)) //Waiting for end of second transmission
	{
		
	}		//Really dirty way of doing this, @TODO fix this
	
	if(XbeeRxBuff[XbeeRxWriteIndex-1] != CRtn)
	{
		return -1;//Timeout no response
	}
	
	
	
	
	XbeeRxReadTo = XbeeRxWriteIndex - 1;
	
	//Get data from buffer
	for(i = 0; XbeeRxReadFrom != XbeeRxReadTo; XbeeRxReadFrom = (XbeeRxReadFrom+1)%BUFFER_SIZE)
	{
		resp[i++] = XbeeRxBuff[XbeeRxReadFrom];
	}	
	
	XbeeRxBufferActive = 0; //Clear buffer flag for safety
	
	*len = strlen(resp);
	
	//Skip /r to keep String handling simple 
	XbeeRxWriteIndex = (XbeeRxWriteIndex + 1) % BUFFER_SIZE;
	XbeeRxReadFrom = (XbeeRxReadFrom + 2) % BUFFER_SIZE;
	return 0;
}
//---------------------------------------------------
int XB_ConfigureDevice(char * sn, int len)
{
	char str[10];
	char dl[10], dh[10];
	int i, temp;
	
	//Enable Device
	XB_EnableXbee();
	
	memset(dl, 0, sizeof(dl));
	memset(dh, 0, sizeof(dh));
	
	memcpy(dh, sn, DL_LEN);
	memcpy(dl, sn+DL_LEN, len - DL_LEN);
	
	//CC_SendData(dl, strlen(dl));
	//CC_SendData(dh, strlen(dh));
	
		
	
	//Clear tempstring
	memset(str, 0, sizeof(str));
	
	//Enter command mode
	XB_SendData(PROGRAM_MODE_CMD, 3);
	
	XB_GetResponse(str, &temp);	
	CC_SendData(str, temp);	
	
	
	//Setting the parameters
	//Just in case, update timing params
	XB_SendData(SET_CT, strlen(SET_CT));
	XB_SendData(DEF_CT, strlen(DEF_CT));
	XB_SendByte(CRtn);
	
	XB_GetResponse(str, &temp);	
	CC_SendData(str, temp);
	
	XB_SendData(SET_GT, strlen(SET_GT));
	XB_SendData(DEF_GT, strlen(DEF_GT));
	XB_SendByte(CRtn);
	
	XB_GetResponse(str, &temp);	
	CC_SendData(str, temp);
	
	//DL 
	XB_SendData(SET_DL, strlen(SET_DL));
	XB_SendData(dl, strlen(dl));
	XB_SendByte(CRtn);
	
	XB_GetResponse(str, &temp);	
	CC_SendData(str, temp);	
	
	//DH
	XB_SendData(SET_DH, strlen(SET_DL));
	XB_SendData(dh, strlen(dh));
	XB_SendByte(CRtn);
	
	XB_GetResponse(str, &temp);	
	CC_SendData(str, temp);	
	
	//Confirm with AC
	XB_SendData(CONFIRM_SET, strlen(SET_DL));	
	XB_SendByte(CRtn);
	
	XB_GetResponse(str, &temp);	
	CC_SendData(str, temp);	
	
	//Write to NVM
	XB_SendData(SET_NVM, strlen(SET_NVM));
	XB_SendByte(CRtn);
	
	XB_GetResponse(str, &temp);	
	CC_SendData(str, temp);	
	
	//Get Config and ECHO
	XB_SendData(SET_DH, strlen(SET_DH));
	XB_SendByte(CRtn);
	
	XB_GetResponse(str, &temp);	
	CC_SendData(str, temp);	
	
	XB_SendData(SET_DL, strlen(SET_DL));
	XB_SendByte(CRtn);
	
	XB_GetResponse(str, &temp);	
	CC_SendData(str, temp);	
	
	XB_DisableXbee();
	
	return 0;
}
//---------------------------------------------------
int XB_GetConfig()
{
	char str[50];
	int temp;
	memset(str, 0, sizeof(str));
	
	XB_EnableXbee();
	
	//Enter command mode
	XB_SendData(PROGRAM_MODE_CMD, 3);	
	XB_GetResponse(str, &temp);	
	
	CC_SendData(str, temp);	
	
	//Get Config and ECHO
	XB_SendData(SET_DH, strlen(SET_DH));
	XB_SendByte(CRtn);
	
	XB_GetResponse(str, &temp);	
	CC_SendData(str, temp);	
	
	XB_SendData(SET_DL, strlen(SET_DL));
	XB_SendByte(CRtn);
	
	XB_GetResponse(str, &temp);	
	CC_SendData(str, temp);	
	
	XB_DisableXbee();
	
	return 0;
}
//---------------------------------------------------
int XB_GetSN()
{
	char str[50];
	int temp;
	
	memset(str, 0, sizeof(str));
	
	XB_EnableXbee();
	
	//Enter command mode
	XB_SendData(PROGRAM_MODE_CMD, 3);	
	XB_GetResponse(str, &temp);	
	
	CC_SendData(str, temp);	
	
	//Get Config and ECHO
	XB_SendData(GET_SH, strlen(GET_SH));
	XB_SendByte(CRtn);
	
	XB_GetResponse(str, &temp);	
	CC_SendData(str, temp);	
	
	XB_SendData(GET_SL, strlen(GET_SL));
	XB_SendByte(CRtn);
	
	XB_GetResponse(str, &temp);	
	CC_SendData(str, temp);	
	
	XB_DisableXbee();
	
	return 0;	
}
//---------------------------------------------------
int XB_CheckForDongle()
{
	int i;
	if(XbeeRxBufferActive)
	{
		//Check for correct character
		for(i = XbeeRxReadFrom;i != XbeeRxWriteIndex; i = (i + 1) % BUFFER_SIZE)
		{
			//If proper handshake char from dongle read
			if(XbeeRxBuff[i] == EOC)
			{
				XbeeRxReadFrom = (i + 1) % BUFFER_SIZE;
				//Respond with handshake				
				XB_SendByte(EOC);
				XB_SendByte(EOC);
				XB_SendByte(EOC);
				XB_SendByte(EOC);
				XB_SendByte(EOC);
				return 0;				
			}
		}
		return -2; //No correct data on line 
		//for(chars in buffer)
		//if char is EOC 
		//send resp
		//return 
		//end of for
		//return not correct response 
	}
	else
	{
		return -1;//No activity on rx line
	}
}
//---------------------------------------------------
int XB_ParseCommand()
{
	int i, currWriteIndex = XbeeRxWriteIndex; //Need temp index so global won't be 
																					//iterated during function call. makes errors
	char temp[50];
	
	//Check for end of command first in buffer
	//Ideally, this sort of buffering should allow for multiple 
	//	commands in the buffer at once 
	for(i=XbeeRxReadFrom; i!=currWriteIndex; i = (i+1)%BUFFER_SIZE)
	{
		if(XbeeRxBuff[i] == EOC)
		{
			XbeeRxReadTo = i + 1;
			break;
		}
	}	
	if(i == currWriteIndex) return NO; //No full command found
	
	memset(temp, 0, sizeof(temp));//clears string 
	i = 0;
	//If command found, read command to local string
	for(; XbeeRxReadFrom!=XbeeRxReadTo; XbeeRxReadFrom=(XbeeRxReadFrom + 1)%BUFFER_SIZE)
	{
		temp[i++] = XbeeRxBuff[XbeeRxReadFrom];
	}
	//Check command against known strings 	
	// Return case for switch case statement
	if(!strcmp(temp, GETFIXES)) 
		return GETFIXES_N;
	//else if(!strcmp(temp, )) return ;
	
	else return NO;	//Default for unknown command
}
//---------------------------------------------------
int XB_ExecuteCommand(int command)
{
	switch(command)
	{
		//Commands run here
		//-------------------------------------------------------------
		case GETFIXES_N:
			return XB_TransmitFixes();
			break;
		//-------------------------------------------------------------
		default:	//Unprogrammed response should be 0 but this is safer
			return -1;//No command executed
			break;
	
	}
}
//---------------------------------------------------
int XB_TransmitFixes()
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
		XB_SendData(buff, 2);
		XB_SendByte(',');
		XB_SendData(&buff[2], 2);
		XB_SendByte(',');
		
		//Send Date
		XB_SendData(&buff[4], 2);
		XB_SendByte(',');
		XB_SendData(&buff[6], 2);
		XB_SendByte(',');
		XB_SendData(&buff[8], 2);
		XB_SendByte(',');

		//Send Lat
		XB_SendData(&buff[10], 10);
		XB_SendByte(',');		
		XB_SendData(&buff[20], 1);
		XB_SendByte(',');
		
		//Send Long
		XB_SendData(&buff[21], 10);
		XB_SendByte(',');
		XB_SendData(&buff[31], 1);
		XB_SendByte(',');
		XB_SendByte('\n');
		memset(buff, 0, sizeof(buff));
	}
	XB_SendByte(EOC);
	
	
	
	
	
	//Currently a test function
	
	//XB_SendData(test, strlen(test));
	//XB_SendByte(EOC);
	XbeeDongleConnected = NO;
	return 0;//Holder	
}
//---------------------------------------------------
void VHF_EnableVHF()
{
	VHF_EN_GPIO_Port->ODR &= ~VHF_EN_Pin;
	return;
}
//---------------------------------------------------
void VHF_DisableVHF()
{
	VHF_EN_GPIO_Port->ODR |= VHF_EN_Pin;
	return;
}

//---------------------------------------------------
/*--EOF--*/
