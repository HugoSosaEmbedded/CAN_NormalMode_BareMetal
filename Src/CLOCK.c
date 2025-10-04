#include "main.h"

void System_Clock_Config(void)
{
	RCC->CR |= (1<<18); // HSE oscillator bypassed RCC_CR_HSEBYP
	RCC->CR |= (1<<16); //HSE oscillator, ON RCC_CR_HSEON
	while(!(RCC->CR & (1<<17))); //Wait until HSE oscillator is stable, RCC_CR_HSERDY
	FLASH->ACR &= ~(0xF<<0);
	FLASH->ACR |= (1<<0); // One wait state HCLK 54 MHz, FLASH_ACR_LATENCY
	RCC->PLLCFGR = (4<<0)  | // M = 4, PLL = 8 MHz / 4 = 2, MHz RCC_PLLCFGR_PLLM
	               (54<<6) | // N = 54,  PLL = 2 MHz * 54, RCC_PLLCFGR_PLLN
	               (0<<16) | // P = 2, PLL = 108 MHz / 2 = 54 MHz, RCC_PLLCFGR_PLLP
				   (1<<22) | // HSE oscillator clock selected as PLL, RCC_PLLCFGR_PLLSRC
				   (2<<24);  // Q = 2, RCC_PLLCFGR_PLLQ
	RCC->CR |= (1<<24); // PLL ON, RCC_CR_PLLON
	while(!(RCC->CR & (1<<25))); // Wait until PLL is ready, RCC_CR_PLLRDY

	//RCC->CFGR |= (4<<10) | (0<<13);
	RCC->CFGR |= (2<<0); // PLL used as the system clock, RCC_CFGR_SW_PLL
	while((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);
}


