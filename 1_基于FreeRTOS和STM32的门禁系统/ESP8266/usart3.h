#ifndef __USART3_H
#define __USART3_H
#include "stm32f4xx.h"

#define USART3_TXBUFF_SIZE 1024
#define USART3_RXBUFF_SIZE 1024
#define USART_DEBUG USART1

extern char USART3_RxBuff[USART3_RXBUFF_SIZE];         //用于保存USART3接收到的数据
extern uint32_t USART3_RxCounter;                  //记录USART3总共接收了多少字节的数据

void USART3_Init( u32 bound );
void USART3_Printf( char *format, ... );
void USART3_TxData( uint8_t *data );
void Usart_SendString(USART_TypeDef *USARTx, u8 *str, u16 len);
void Usart_Printf(USART_TypeDef *USARTx, char *format, ...);

#endif













