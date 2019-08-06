
#include "lowpower.h"
#include "globals.h"
#include "main.h"
#include "gps.h"
#include "xbee.h"

int LPM_sleep(void){
		
	SCB->SCR &= ~SCR_DEEPSLEEP; // Enable deep sleep feature

	//PWR->CR  &= ~PWR_CR_PDDS; 	//Only relevant when DEEPSLEEP = 1
	PWR->CSR &= ~PWR_CSR_WUF;
	
	__WFI();
	
	return 0;
}
int LPM_stop(void){

	GPS_GPSDisable();
#ifdef __LARGE_COLLAR_	
	XB_DisableXbee();
#endif
	// Disable SPI:
	SPI1->CR1 &= ~SPI_CR1_SPE; 
	// Disable USART1:
	USART1->CR1 &= ~USART_CR1_UE;
	// Disable USART4:
	USART4->CR1 &= ~USART_CR1_UE;
	
#ifdef __PRODUCTION_
	LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
	LL_APB1_GRP1_DisableClock(LL_APB1_GRP1_PERIPH_USART4);
	//LL_APB2_GRP1_DisableClock(LL_APB2_GRP1_PERIPH_SPI1);
	LL_APB2_GRP1_DisableClock(LL_APB2_GRP1_PERIPH_USART1);
	
	
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_DOWN;
	
	// USART1
	GPIO_InitStruct.Pin = LL_GPIO_PIN_9;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	// USART1
  GPIO_InitStruct.Pin = LL_GPIO_PIN_10;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	// USART4
	GPIO_InitStruct.Pin = LL_GPIO_PIN_10;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	// USART4
  GPIO_InitStruct.Pin = LL_GPIO_PIN_1;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	/*
	// SPI
	GPIO_InitStruct.Pin = LL_GPIO_PIN_5;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	// SPI
  GPIO_InitStruct.Pin = LL_GPIO_PIN_6;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	// SPI
  GPIO_InitStruct.Pin = LL_GPIO_PIN_7;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);*/
	
	//GPIOA->ODR &= ~(LL_GPIO_PIN_0 | LL_GPIO_PIN_1 | LL_GPIO_PIN_5| LL_GPIO_PIN_6| LL_GPIO_PIN_7| LL_GPIO_PIN_9 | LL_GPIO_PIN_10 );
	GPIOA->ODR &= ~(LL_GPIO_PIN_0 | LL_GPIO_PIN_1 | LL_GPIO_PIN_9 | LL_GPIO_PIN_10 );

#endif
 
	/* Clear any pending EXTI */
	EXTI->PR = 0xFFFFFFFF;
	/* Clear any pending peripheral interrupts */
	int i;
	for (i = 0; i < 8; i++) {
			NVIC->ICPR[i] = 0xFFFFFFFF;
	}
	/* Clear the RTC pending flags */
	RTC->ISR &= ~(RTC_ISR_ALRAF | RTC_ISR_ALRBF | RTC_ISR_WUTF | RTC_ISR_TSF | RTC_ISR_TAMP1F);


	SCB->SCR |= SCR_DEEPSLEEP; 	// Enable deep sleep feature

	PWR->CR  &= ~PWR_CR_PDDS;		// Enter Stop mode when Deepsleep is entered
	PWR->CSR &= ~PWR_CSR_WUF;		// Clear wakup flag

	
#ifdef __PRODUCTION_
	__WFI();
#endif	
	
	LL_USART_Enable(USART1);

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
	
	GPIO_InitStruct.Pin = LL_GPIO_PIN_0;
  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	GPIO_InitStruct.Pin = LL_GPIO_PIN_1;
  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

#ifndef __LARGE_COLLAR_
	// Disables XBEE_EN pin on small collar
	GPIO_InitStruct.Pin = XBEE_EN_Pin;
  LL_GPIO_Init(XBEE_EN_GPIO_Port, &GPIO_InitStruct);
	// Disables VHF_EN Pin on small collar
	GPIO_InitStruct.Pin = VHF_EN_Pin;
  LL_GPIO_Init(VHF_EN_GPIO_Port, &GPIO_InitStruct);
	
	GPIOA -> ODR &= ~(XBEE_EN_Pin | VHF_EN_Pin);
#endif
	
	
	GPIOA -> ODR &= ~( LL_GPIO_PIN_2 | LL_GPIO_PIN_4 | LL_GPIO_PIN_11 | LL_GPIO_PIN_12 | LL_GPIO_PIN_15);
	GPIOB -> ODR &= ~(LL_GPIO_PIN_0 | LL_GPIO_PIN_1);

	return 0;	
};