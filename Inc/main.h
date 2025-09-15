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

//-----------------------------------------------------------------------------------------------------------------------------------------
#define BUFFER_SIZE 64

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
