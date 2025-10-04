/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f7xx_it.h"
/* Private includes ----------------------------------------------------------*/

extern volatile uint32_t count, i;
extern char* tx_buffer;
extern void CAN1_SetFlag(void);

void TIM4_IRQHandler(void)
{
	//This interruption help us to implement a counter, it increments each 1ms
	if(TIM4->SR & TIM_SR_UIF)
	{
		count++;
		TIM4->SR &= ~TIM_SR_UIF; // Clean Update interrupt flag to set no update occurred
	}
}

void CAN1_TX_IRQHandler(void)
{
	if(CAN1->TSR & CAN_TSR_RQCP0) CAN1->TSR |= CAN_TSR_RQCP0; //Clean writing 1 in RQCP0 to not active interruptions
	if(CAN1->TSR & CAN_TSR_RQCP1) CAN1->TSR |= CAN_TSR_RQCP1; //Clean writing 1 in RQCP1 to not active interruptions
	if(CAN1->TSR & CAN_TSR_RQCP2) CAN1->TSR |= CAN_TSR_RQCP2; //Clean writing 1 in RQCP2 to not active interruptions
}

void CAN1_RX0_IRQHandler(void)
{
	//This interruption is activated when there is a message received, the interruption is diable when the message is released
	CAN1_SetFlag();
}

void USART3_IRQHandler()
{
	//This interruption is to get the message that usart will be transmitting
	if(USART3->ISR & USART_ISR_TXE) // If the content has been transferred into the shift register
	{
		if(tx_buffer[i] != '\0')
		{
			USART3->TDR = tx_buffer[i++]; //Save value into USART3_TDR
		}

		else
		{
			USART3->CR1 &= ~USART_CR1_TXEIE; //Disable interrupt
		}
	}
}
