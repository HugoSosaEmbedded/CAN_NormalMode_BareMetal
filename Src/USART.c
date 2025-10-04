#include "main.h"

void USART3_Init(void)
{
	// USART3 help us to debug the message we are sending
	RCC->APB1ENR |= (1<<18); //Enable clock for USART3

	USART3->CR1 &= ~(1<<15); // Oversamplig  by 16, USART3_CR1_OVER8
	USART3->CR1 &= ~(1<<12); // 8 data bits as word length, USART3_CR1_M0
	USART3->CR1 &= ~(1<<28); // 1 Start bit, 8 data bits, n stop bits, USART3_CR1_M1
	USART3->CR1 |= (1<<3); //Transmitter is enabled, USART3_CR1_TE

	USART3->CR2 &= ~((1<<12) | (1<<13)); // USART configured with 1 stop bit, USART3_CR2_STOP
	USART3->BRR = (29<<4) | 5; // Baud rate desired 115200

	USART3->CR1 |= (1<<0); //USART enable, USART3_CR1_UE

	NVIC_EnableIRQ(USART3_IRQn);
	NVIC_SetPriority(USART3_IRQn,1);
	USART3->CR1 |= (1<<7); //Enable TXEIE interrupt, USART3_CR1_TXEIE

}


