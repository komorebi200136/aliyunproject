#ifndef  __MP3_H
#define  __MP3_H

#include "stm32f4xx.h"

void MP3_Init(uint32_t baud);
void MP3_SendString(char *String);
void MP3_SendByte(uint8_t Byte);
void Start_ZhiDing(uint8_t num);
void VolumeUp(void);
void VolumeDown(void);

#endif
