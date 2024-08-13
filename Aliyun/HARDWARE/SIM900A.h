#ifndef  __SIM900A_H
#define  __SIM900A_H

#include "stm32f4xx.h"
#include <string.h>

void SIM_Init(uint32_t baud);
void SIM_SendString(char *String);
int8_t SIM_RX_Jude(void);
void SIM_SendByte(uint8_t Byte);
void Send_EnglishMessages(char message[],char phone[]);
void Send_ChineseMessages(char message[],char phone[]);

#endif
