#include "sg90.h"

/***************************************************************************
SG90����ź��߽���PF9��
ʹ��TIM14��ʱ�����PWM��

***************************************************************************/

void SG90_Init(u32 arr, u32 psc)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
    TIM_OCInitTypeDef TIM_OCInitStruct;
    
    RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOF, ENABLE );
    RCC_APB1PeriphClockCmd( RCC_APB1Periph_TIM14, ENABLE );
    
    GPIO_PinAFConfig( GPIOF, GPIO_PinSource9, GPIO_AF_TIM14 );  //GPIOF9����Ϊ��ʱ��14
    
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_Init( GPIOF, &GPIO_InitStruct );
    
    TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStruct.TIM_Period = arr;
    TIM_TimeBaseInitStruct.TIM_Prescaler = psc;
    TIM_TimeBaseInit( TIM14, &TIM_TimeBaseInitStruct );
    
    TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1; //ѡ��ʱ��ģʽ:TIM�����ȵ���ģʽ2
    TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable; //�Ƚ����ʹ��
    TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High; //�������:TIM����Ƚϼ��Ե�
    TIM_OC1Init( TIM14, &TIM_OCInitStruct );
    
    TIM_OC1PreloadConfig( TIM14, TIM_OCPreload_Enable );  //ʹ��TIM14��CCR1�ϵ�Ԥװ�ؼĴ���
    TIM_ARRPreloadConfig( TIM14,ENABLE );//ARPEʹ��
    TIM_Cmd( TIM14, ENABLE );  //ʹ��TIM14
    
    SG90_SetAngle( 0 );
}

void SG90_SetAngle( float angle )
{
    int servo_temp;
    
    if ( angle > 180 )
        angle = 180;
    else if ( angle < 0 )
        angle = 0;
    
    servo_temp = angle * 4000 / 180 + 1000;
    TIM_SetCompare1(TIM14,servo_temp);
}
