#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f7xx_hal.h"

/*----------------------------- Function prototypes ----------------------------------------------------------------------------------- */
void System_Clock_Config(void);
void Error_Handler(void);
void CAN1_Init(void);
void GPIO_CAN1(void);
void CAN1_TX(void);
void USART3_Init(void);
void GPIO_USART3(void);
void Swap_Buffer(void);
void TIM4_Init(void);
void GPIO_LED(void);
void CAN1_Send(uint16_t id, uint8_t* data);
uint8_t CAN1_Received(void);
void CAN1_SetFlag(void);


//-----------------------------------------------------------------------------------------------------------------------------------------
#define BUFFER_SIZE 64

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
