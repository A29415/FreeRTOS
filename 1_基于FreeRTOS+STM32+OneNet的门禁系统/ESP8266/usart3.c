#include "usart3.h"
#include "FreeRTOS.h"
#include "event_groups.h"
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

char USART3_TxBuff[USART3_TXBUFF_SIZE];         //暂存用于发送的数据
char USART3_RxBuff[USART3_RXBUFF_SIZE];         //用于保存USART3接收到的数据
uint32_t USART3_RxCounter = 0;                  //记录USART3总共接收了多少字节的数据

/**
 * @name: USART3_Init
 * @brief: USART3初始化，以及开启更新中断
 * @param {u32} bound   ：波特率
 * @return {*}
 */
void USART3_Init( u32 bound )
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOB, ENABLE );     //使能GPIOB时钟 
    RCC_APB1PeriphClockCmd( RCC_APB1Periph_USART3, ENABLE );    //使能USART3时钟

    GPIO_PinAFConfig( GPIOB, GPIO_PinSource10, GPIO_AF_USART3 );      //PA2复用为USART3
    GPIO_PinAFConfig( GPIOB, GPIO_PinSource11, GPIO_AF_USART3 );      //PA3复用为USART3

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;      //USART3的TX(PB10)、RX(PB11)
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;           //IO速率50M
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;                //复用模式
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;              //推挽输出
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;                //上拉
    GPIO_Init( GPIOB, &GPIO_InitStructure );                    //USART3初始化设置

    USART_InitStructure.USART_BaudRate = bound;                                     //波特率
    USART_InitStructure.USART_Parity = USART_Parity_No;                             //无奇偶校验位
    USART_InitStructure.USART_StopBits = USART_StopBits_1;                          //1个停止位
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;                     //8个数据位
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;                 //收发模式
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; //无硬件数据流控制
    USART_Init( USART3, &USART_InitStructure );

    USART_Cmd( USART3, ENABLE );                                //使能USART3

    USART_ITConfig( USART3, USART_IT_RXNE, ENABLE );            //开启接收中断

    NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;           //设置USART3中断
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;             //中断通道使能
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4;   //抢占优先级
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;          //响应优先级
    NVIC_Init( &NVIC_InitStructure );                           //中断初始化
}

/**
 * @name: USART23_Printf
 * @brief: USART3打印输出
 * @param {char} *format    ：参数列表
 * @return {*}
 */
void USART3_Printf( char *format, ... )
{
    uint32_t i, length;

    /* VA_LIST是C中解决变参问题的一组宏，用于获取不确定个数的参数，所在头文件 #include <stdarg.h>  */
    va_list ap;                                         //定义一个va_list的变量，表示可变参数列表类型
    va_start( ap, format );                             //将可变参数传入列表中
    vsprintf( USART3_TxBuff, format, ap );              //使用可变参数列表格式化输出到指定字符串中
    va_end( ap );                                       //释放ap

    length = strlen( ( const char* )USART3_TxBuff );    //获取USART3_TxBuff的长度
    while( ( USART3->SR & 0x40 ) == 0 );                //等待上一个数据发送完成
    for( i = 0; i < length; i++ )                       
    {
        USART3->DR = USART3_TxBuff[i];                  //依次发送USART3_TxBuff中的每个字节
        while( ( USART3->SR & 0X40 ) == 0 );            //等待发送完成
    }
}

/**
 * @name: USART3_TxData
 * @brief: USART3发送缓冲区中的数据
 * @param {uint8_t} *data   ：数据
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
		while(USART_GetFlagStatus(USARTx, USART_FLAG_TC) == RESET);//等待发送完成
	}
}

/**
  ************************************************************************** 
  ** -------------------------------------------------------------------- **
  ** @函数名        : Usart_Printf
  ** @描述          : 格式化打印
  ** @参数          : USARTx：串口组
**										format：不定长参
  ** @返回值        : None
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