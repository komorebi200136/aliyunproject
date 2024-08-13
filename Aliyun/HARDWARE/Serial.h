#ifndef  __SERIAL_H
#define  __SERIAL_H

#include "stm32f4xx.h"  //±ØÐë°üº¬
#include <stdio.h>

#define USART1_TX  GPIO_Pin_9
#define USART1_RX  GPIO_Pin_10


void Serial_Init(uint32_t baud);
void Serial_SendByte(uint8_t Byte);
void Serial_SendString(char *String);

#endif
