#ifndef __ESP8266_H__
#define __ESP8266_H__
#include "sys.h" 

void esp8266_start_trans(void);
u8 esp8266_send_cmd(u8 *cmd,u8 *ack,u16 waittime);
u8* esp8266_check_cmd(u8 *str);



#endif

