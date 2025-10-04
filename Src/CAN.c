#include "main.h"

static uint8_t CAN_GetNextMailbox(void);
static volatile uint8_t flag_received;

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

void CAN1_Send(uint16_t id, uint8_t* data)
{
	uint8_t CODE = CAN_GetNextMailbox();

	if(CAN1->TSR & (1 << (26 + CODE)))
	{
		CAN1->sTxMailBox[CODE].TIR = (id<<21); //Standard identifier
		CAN1->sTxMailBox[CODE].TIR &= ~(CAN_TI0R_IDE | CAN_TI0R_RTR); //Standard identifier, Data frame
		CAN1->sTxMailBox[CODE].TDTR = 0x1; // 1 Data length code
		CAN1->sTxMailBox[CODE].TDLR = *data;
		CAN1->sTxMailBox[CODE].TIR |= (CAN_TI0R_TXRQ); // Set to request the transmission for the corresponding mailbox
		data++;
	}
}

void CAN1_SetFlag(void) { flag_received = 1; }

uint8_t CAN1_Received(void)
{
	uint8_t received = 0;

	if(flag_received)
	{
		received = CAN1->sFIFOMailBox[0].RDLR; // Save message received into received variable
		//msg[0] = received & 0xFF;
		//msg[1] = '\0';
		CAN1->RF0R |= CAN_RF0R_RFOM0; //Release
		flag_received = 0;

	}

	return received;
}

static uint8_t CAN_GetNextMailbox(void)
{
	return (CAN1->TSR >> 24) & 0x3; // In case at least one transmit mailbox is free, the code value is equal to the number of the next transmit mailbox free
}

