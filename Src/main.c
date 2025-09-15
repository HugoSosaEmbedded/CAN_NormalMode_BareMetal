#include "main.h"
#include "stdio.h"
static void MPU_Config(void);

char buffer1[BUFFER_SIZE], buffer2[BUFFER_SIZE];

char* tx_buffer = buffer1;
char* fill_buffer = buffer2;

volatile uint32_t i = 0, count = 0, CODE;
volatile uint8_t count_can = 0, received = 0;
volatile char msg[2];

int main(void)
{

  MPU_Config();
  System_Clock_Config();
  GPIO_CAN1();
  CAN1_Init();
  GPIO_USART3();
  USART3_Init();

  while(1)
  {
	  CODE = (CAN1->TSR >> 24) & 0x3; // In case at least one transmit mailbox is free, the code value is equal to the number of the next transmit mailbox free

	  if(count >= 500)
	  {
		  if(CAN1->TSR & (1 << (26 + CODE)))
		  {
			  count_can++;
			  CAN1->sTxMailBox[CODE].TIR = (0x123<<21); //Standard identifier
			  CAN1->sTxMailBox[CODE].TIR &= ~(CAN_TI0R_IDE | CAN_TI0R_RTR); //Standard identifier, Data frame
			  CAN1->sTxMailBox[CODE].TDTR = 0x1; // 1 Data length code
			  CAN1->sTxMailBox[CODE].TDLR = count_can;
			  CAN1->sTxMailBox[CODE].TIR |= (CAN_TI0R_TXRQ); // Set to request the transmission for the corresponding mailbox

			  if(count_can == 255)
			  {
				  count_can = 0;
			  }
		  }
		  if(received % 2 == 0)
		  {
				//Code
		  }

		  else
		  {
			//Code
		  }

		  sprintf(fill_buffer, "Counter: %s\r\n",msg);

		  count = 0;
		  if(USART3->ISR & (1<<6)) // USART3
		  {
			  Swap_Buffer();
			  USART3->CR1 |= USART_CR1_TXEIE; // Enable interrupt
			  i = 0;
		  }
	  }
  }

  return 0;

}

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

void GPIO_CAN1(void)
{
	//PD0  RX  PD1  TX
	RCC->AHB1ENR |= (1<<3); // Enable clock port D, RCC_AHB1ENR_GPIOBEN
	GPIOD->MODER &= ~((3<<0) | (3<<2));
	GPIOD->MODER |= ((2<<0) | (2<<2)); // PD1 and PD0 as alternate function
	GPIOD->OSPEEDR &= ~((3<<0) | (3<<2));
	GPIOD->OSPEEDR |= ((2<<0) | (2<<2)); // PD0 and PD1 high speed
	GPIOD->AFR[0] &= ~((0xF<<0) | (0xF<<4));
	GPIOD->AFR[0] |= ((9<<0) | (9<<4)); // PD0 y PD1 AF9
}

void CAN1_Init(void)
{
	RCC->APB1ENR |= (1<<25); //Enable clock for CAN1
	/*--------------------------Initialization----------------------------------------------------------------*/
	CAN1->MCR |= CAN_MCR_INRQ; //Initialization request
	CAN1->MCR &= ~CAN_MCR_SLEEP; // Sleep mode request disable
	while(!(CAN1->MSR & CAN_MSR_INAK)); // Wait until hardware indicates CAN is in initialization mode
	while(CAN1->MSR & CAN_MSR_SLAK); // Verify if CAN is not in Sleep mode
	CAN1->MCR |= CAN_MCR_TXFP; // Transmit FIFO priority driven by the identifier of the message
	CAN1->MCR &= ~CAN_MCR_RFLM; // Receive FIFO not locked on overrun. Once a receive FIFO is full the next incoming message overwrites the previous one
	CAN1->MCR &= ~CAN_MCR_NART; // The CAN hardware automatically retransmits the message until it has been successfully transmitted
	CAN1->MCR |= CAN_MCR_AWUM; //The sleep mode is left automatically by hardware on CAN message detection
	CAN1->MCR |= CAN_MCR_ABOM; //The Bus-Off state is left automatically by hardware once 128 occurrences of 11 11 recessive bits have been monitored
	/*-----------------------Bit Timing------------------------------------------------------------------------*/
	CAN1->BTR = (6<<0) // BRP = 6, Baud rate prescaler
			  | (10<<16) // TS1 = 10, Time segment 1
			  | (7<<20) // TS2 = 7, Time segment 2
			  | (1<<24); // SJW = 1, Resynchronization jump width
	/*-----------------------Filtering--------------------------------------------------------------------------*/
	CAN1->FMR |= CAN_FMR_FINIT; // Initialization mode for the filters
	CAN1->FA1R &= ~CAN_FA1R_FACT0; // Filter 0 is not active to configure it
	CAN1->FM1R |= CAN_FM1R_FBM0; // Two 32 bit registers of filter bank 0 are in identifier list mode
	CAN1->FFA1R &= ~CAN_FFA1R_FFA0; // Filter assigned to FIFO0
	CAN1->sFilterRegister[0].FR1 = (0x123<<21) | (0<<2) | (0<<1); // ID = 0x123, Standard identifier, Data frame
	CAN1->sFilterRegister[0].FR2 = 0x00000000;
	CAN1->FA1R |=CAN_FA1R_FACT0; // Filter 0 is active
	CAN1->FMR &= ~CAN_FMR_FINIT; // Active filters mode

	CAN1->MCR &= ~CAN_MCR_INRQ; // Normal mode
	while(CAN1->MSR & CAN_MSR_INAK); // Wait until CAN get into to Normal mode

	CAN1->IER |= CAN_IER_TMEIE; // Transmit mailbox interrupt enable generated when RQCPx bit is set
	NVIC_EnableIRQ(CAN1_TX_IRQn);
	NVIC_SetPriority(CAN1_TX_IRQn,0);

	CAN1->IER |= CAN_IER_FMPIE0; // FIFO message pending interrupt enable
	NVIC_EnableIRQ(CAN1_RX0_IRQn);
	NVIC_SetPriority(CAN1_RX0_IRQn,0);
}


void CAN1_TX_IRQHandler(void)
{
	if(CAN1->TSR & CAN_TSR_RQCP0) CAN1->TSR |= CAN_TSR_RQCP0;
	if(CAN1->TSR & CAN_TSR_RQCP1) CAN1->TSR |= CAN_TSR_RQCP1;
	if(CAN1->TSR & CAN_TSR_RQCP2) CAN1->TSR |= CAN_TSR_RQCP2;
}

void CAN1_RX0_IRQHandler(void)
{
	received = CAN1->sFIFOMailBox[0].RDLR;
	msg[0] = received & 0xFF;
	msg[1] = '\0';

	CAN1->RF0R |= CAN_RF0R_RFOM0;

}

void USART3_IRQHandler()
{
	if(USART3->ISR & USART_ISR_TXE)
	{
		if(tx_buffer[i] != '\0')
		{
			USART3->TDR = tx_buffer[i++];
		}

		else
		{
			USART3->CR1 &= USART_CR1_TXEIE; //Disable interrupt
		}
	}
}

void USART3_Init(void)
{

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

void GPIO_USART3(void)
{
	//PD8 TX, PD9 RX
	RCC->AHB1ENR |= (1<<3); // Enable clock port D, RCC_AHB1ENR_GPIODEN
	GPIOD->MODER &= ~((3<<16) | (3<<18));
	GPIOD->MODER |= ((2<<16) | (2<<18)); // PD8 and PD9 as alternate function
	GPIOD->OSPEEDR &= ~((3<<16) | (3<<18));
	GPIOD->OSPEEDR |= ((2<<16) | (2<<18)); // PD8 and PD9 high speed
	GPIOD->AFR[1] &= ~((0xF<<0) | (0xF<<4));
	GPIOD->AFR[1] |= ((7<<0) | (7<<4)); // PD8 y PD9 AF7
}

void Swap_Buffer(void)
{
	char* tmp = tx_buffer;
	tx_buffer = fill_buffer;
	fill_buffer = tmp;
}

void TIM4_Init(void)
{
	RCC->APB1ENR |= (1<<2); //TIM4 clock enabled
	TIM4->PSC = 53;
	TIM4->ARR = 999;
	TIM4->CR1 &= ~TIM_CR1_UDIS; // UEV enabled. The update event is generated by Counter overflow/underflow

	TIM4->DIER |= TIM_DIER_UIE; // Update interrupt enable
	NVIC_EnableIRQ(TIM4_IRQn);
	NVIC_SetPriority(TIM4_IRQn, 0);

	TIM4->CR1 |= TIM_CR1_CEN; // Counter enabled
}

void TIM4_IRQHandler(void)
{
	if(TIM4->SR & TIM_SR_UIF)
	{
		count++;
		TIM4->SR &= ~TIM_SR_UIF; // Clean Update interrupt flag to set no update occurred
	}
}

void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct = {0};

  /* Disables the MPU */
  HAL_MPU_Disable();

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.BaseAddress = 0x0;
  MPU_InitStruct.Size = MPU_REGION_SIZE_4GB;
  MPU_InitStruct.SubRegionDisable = 0x87;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  /* Enables the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);

}
