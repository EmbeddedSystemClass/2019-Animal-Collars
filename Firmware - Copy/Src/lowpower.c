
#include "globals.h"
#include "main.h"
#include "gps.h"
#include "xbee.h"


int LPM_stopMode(void){
	
	GPS_GPSDisable();
	VHF_DisableVHF();
	XB_DisableXbee();
	
		
	
	
	SCB->SCR |= 1<<2; // Enable deep sleep feature

	PWR->CR  &= ~PWR_CR_PDDS;
	PWR->CSR &= ~PWR_CSR_WUF;
	
	return 0;
}
int LPM_gpioInit(void){
	
	LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
	
	// Unused pins: PA2, PA4 ,PA11, PA12, PA15, PB0, PB1
	
	// Same for all pins:
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_DOWN;
	
	GPIO_InitStruct.Pin = LL_GPIO_PIN_2;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	GPIO_InitStruct.Pin = LL_GPIO_PIN_4;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	GPIO_InitStruct.Pin = LL_GPIO_PIN_11;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	GPIO_InitStruct.Pin = LL_GPIO_PIN_12;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	GPIO_InitStruct.Pin = LL_GPIO_PIN_15;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	GPIO_InitStruct.Pin = LL_GPIO_PIN_0;
  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	GPIO_InitStruct.Pin = LL_GPIO_PIN_1;
  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	GPIOA -> ODR &= ~( LL_GPIO_PIN_2 | LL_GPIO_PIN_4 | LL_GPIO_PIN_11 | LL_GPIO_PIN_12 | LL_GPIO_PIN_15);

	return 0;	
};