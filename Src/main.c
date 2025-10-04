#include "main.h"
#include "stdio.h"

char buffer1[BUFFER_SIZE], buffer2[BUFFER_SIZE];
char* tx_buffer = buffer1;
char* fill_buffer = buffer2;

volatile uint32_t i = 0, count = 0;
volatile char msg[2];

extern void setup(void);
extern void CAN1_Send(uint16_t id, uint8_t* data);
extern uint8_t CAN1_Received(void);

int main(void)
{
	uint8_t count_can = 0;

	setup();

	while(1)
	{
		if(count >= 500)
		{
			CAN1_Send(0x123, &count_can);

			if(count_can == 255)
			{
				count_can = 0; // if count_can is equal to 0xFF, restart count_can = 0
			}
			if(CAN1_Received() % 2 == 0)
			{
				GPIOB->ODR |= GPIO_ODR_OD7; // If the received is even, turn on led
			}

			else
			{
				GPIOB->ODR &= ~GPIO_ODR_OD7; // If the received is odd, turn off led
			}

			sprintf(fill_buffer, "CAN PROTCOL\r\n");
			if(USART3->ISR & USART_ISR_TC) // If transmission is complete
			{
				Swap_Buffer(); // Change buffer
				USART3->CR1 |= USART_CR1_TXEIE; // Enable interrupt
				i = 0;
			}

			count = 0; // If count = 500, restart it
		}
	}

  return 0;

}

void Swap_Buffer(void)
{
	//This function is created to change buffer when we have been received a message on USART in order to not saturate the buffer
	char* tmp = tx_buffer;
	tx_buffer = fill_buffer;
	fill_buffer = tmp;
}
