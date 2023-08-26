#ifndef __USART3_H
#define __USART3_H
#include "stm32f4xx.h"

#define USART3_TXBUFF_SIZE 1024
#define USART3_RXBUFF_SIZE 1024
#define USART_DEBUG USART1

extern char USART3_RxBuff[USART3_RXBUFF_SIZE];         //���ڱ���USART3���յ�������
extern uint32_t USART3_RxCounter;                  //��¼USART3�ܹ������˶����ֽڵ�����

void USART3_Init( u32 bound );
void USART3_Printf( char *format, ... );
void USART3_TxData( uint8_t *data );
void Usart_SendString(USART_TypeDef *USARTx, u8 *str, u16 len);
void Usart_Printf(USART_TypeDef *USARTx, char *format, ...);

#endif













