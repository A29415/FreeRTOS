#ifndef _ONENET_H_
#define _ONENET_H_





_Bool OneNet_DevLink(void);
void OneNet_SendData(void);
void OneNet_Subscribe(const char *topics[], unsigned char topic_cnt);
_Bool OneNet_Publish(const char *topic, const char *msg);
void OneNet_RevPro(unsigned char *cmd);


void OneNet_HandleData(char *req_payload, int num);

#endif
