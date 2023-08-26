#include "usart3.h"
#include "FreeRTOS.h"
#include "event_groups.h"
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

char USART3_TxBuff[USART3_TXBUFF_SIZE];         //�ݴ����ڷ��͵�����
char USART3_RxBuff[USART3_RXBUFF_SIZE];         //���ڱ���USART3���յ�������
uint32_t USART3_RxCounter = 0;                  //��¼USART3�ܹ������˶����ֽڵ�����

/**
 * @name: USART3_Init
 * @brief: USART3��ʼ�����Լ����������ж�
 * @param {u32} bound   ��������
 * @return {*}
 */
void USART3_Init( u32 bound )
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOB, ENABLE );     //ʹ��GPIOBʱ�� 
    RCC_APB1PeriphClockCmd( RCC_APB1Periph_USART3, ENABLE );    //ʹ��USART3ʱ��

    GPIO_PinAFConfig( GPIOB, GPIO_PinSource10, GPIO_AF_USART3 );      //PA2����ΪUSART3
    GPIO_PinAFConfig( GPIOB, GPIO_PinSource11, GPIO_AF_USART3 );      //PA3����ΪUSART3

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;      //USART3��TX(PB10)��RX(PB11)
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;           //IO����50M
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;                //����ģʽ
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;              //�������
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;                //����
    GPIO_Init( GPIOB, &GPIO_InitStructure );                    //USART3��ʼ������

    USART_InitStructure.USART_BaudRate = bound;                                     //������
    USART_InitStructure.USART_Parity = USART_Parity_No;                             //����żУ��λ
    USART_InitStructure.USART_StopBits = USART_StopBits_1;                          //1��ֹͣλ
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;                     //8������λ
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;                 //�շ�ģʽ
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; //��Ӳ������������
    USART_Init( USART3, &USART_InitStructure );

    USART_Cmd( USART3, ENABLE );                                //ʹ��USART3

    USART_ITConfig( USART3, USART_IT_RXNE, ENABLE );            //���������ж�

    NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;           //����USART3�ж�
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;             //�ж�ͨ��ʹ��
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4;   //��ռ���ȼ�
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;          //��Ӧ���ȼ�
    NVIC_Init( &NVIC_InitStructure );                           //�жϳ�ʼ��
}

/**
 * @name: USART23_Printf
 * @brief: USART3��ӡ���
 * @param {char} *format    �������б�
 * @return {*}
 */
void USART3_Printf( char *format, ... )
{
    uint32_t i, length;

    /* VA_LIST��C�н����������һ��꣬���ڻ�ȡ��ȷ�������Ĳ���������ͷ�ļ� #include <stdarg.h>  */
    va_list ap;                                         //����һ��va_list�ı�������ʾ�ɱ�����б�����
    va_start( ap, format );                             //���ɱ���������б���
    vsprintf( USART3_TxBuff, format, ap );              //ʹ�ÿɱ�����б��ʽ�������ָ���ַ�����
    va_end( ap );                                       //�ͷ�ap

    length = strlen( ( const char* )USART3_TxBuff );    //��ȡUSART3_TxBuff�ĳ���
    while( ( USART3->SR & 0x40 ) == 0 );                //�ȴ���һ�����ݷ������
    for( i = 0; i < length; i++ )                       
    {
        USART3->DR = USART3_TxBuff[i];                  //���η���USART3_TxBuff�е�ÿ���ֽ�
        while( ( USART3->SR & 0X40 ) == 0 );            //�ȴ��������
    }
}

/**
 * @name: USART3_TxData
 * @brief: USART3���ͻ������е�����
 * @param {uint8_t} *data   ������
 * @return {*}
 */
void USART3_TxData( uint8_t *data )
{
    int i;

    while( ( USART3->SR & 0x40 ) == 0 );
    for( i = 1; i < ( data[0] * 256 + data[1] ); i++ )
    {
        USART3->DR = data[i+1];
        while( ( USART3->SR & 0x40 ) == 0 );
    }
}

void Usart_SendString(USART_TypeDef *USARTx, u8 *str, u16 len)
{
	u16 count = 0;
	for( ; count < len; count++)
	{
		USART_SendData(USARTx, *str++);
		while(USART_GetFlagStatus(USARTx, USART_FLAG_TC) == RESET);//�ȴ��������
	}
}

/**
  ************************************************************************** 
  ** -------------------------------------------------------------------- **
  ** @������        : Usart_Printf
  ** @����          : ��ʽ����ӡ
  ** @����          : USARTx��������
**										format����������
  ** @����ֵ        : None
  ** -------------------------------------------------------------------- **
  ************************************************************************** 
**/
void Usart_Printf(USART_TypeDef *USARTx, char *format, ...)
{
	u8 str[200];
	u8 *pstr = str;
	
	va_list arg;
	va_start(arg, format);
	vsnprintf((char *)str, sizeof(str), format, arg);
	va_end(arg);
	
	while(*pstr != 0)
	{
		USART_SendData(USARTx, *pstr++);
		while(USART_GetFlagStatus(USARTx, USART_FLAG_TC) == RESET);
	}
}