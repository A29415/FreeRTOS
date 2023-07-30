#include "esp8266.h"
#include "delay.h"
#include "string.h"
#include "usart.h"
#include "usart3.h"

void esp8266_start_trans(void)
{
    esp8266_send_cmd("AT+CWMODE=2","OK",50);
	//Wifiģ������
	esp8266_send_cmd("AT+RST","OK",20);
	delay_ms(1000);         //��ʱ3S�ȴ������ɹ�
	delay_ms(1000);
	delay_ms(1000);	
    //APģʽ
	esp8266_send_cmd("AT+CWSAP=\"WiFi_TCP\",\"123456\",11,3","OK",200);
	esp8266_send_cmd("AT+CIPMUX=1","OK",20);
	esp8266_send_cmd("AT+CIPSERVER=1,8086","OK",200);
	esp8266_send_cmd("AT+CIFSR","OK",200);
	esp8266_send_cmd("AT+CIPSEND=0,8","OK",200);

	
	
}	


u8 esp8266_send_cmd(u8 *cmd,u8 *ack,u16 waittime)
{
	u8 res=0; 
	USART3_RX_STA=0;
	u3_printf("%s\r\n",cmd);	//��������
	if(ack&&waittime)		//��Ҫ�ȴ�Ӧ��
	{
		while(--waittime)	//�ȴ�����ʱ
		{
			delay_ms(10);
			if(USART3_RX_STA&0X8000)//���յ��ڴ���Ӧ����
			{
				if(esp8266_check_cmd(ack))
				{
					printf("ack:%s\r\n",(u8*)ack);
					break;//�õ���Ч���� 
				}
					USART3_RX_STA=0;
			} 
		}
		if(waittime==0)res=1; 
	}
	return res;
}

u8* esp8266_check_cmd(u8 *str)
{
	char *strx=0;
	if(USART3_RX_STA&0X8000)		//���յ�һ��������
	{ 
		USART3_RX_BUF[USART3_RX_STA&0X7FFF]=0;//��ӽ�����
		strx=strstr((const char*)USART3_RX_BUF,(const char*)str);
	} 
	return (u8*)strx;
}





