#include "main.h"

void setup(void)
{
	  System_Clock_Config();
	  GPIO_LED();
	  TIM4_Init();
	  GPIO_CAN1();
	  CAN1_Init();
	  GPIO_USART3();
	  USART3_Init();


}
