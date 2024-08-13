#ifndef  __MQADC_H
#define  __MQADC_H

#include "stm32f4xx.h"  //±ØÐë°üº¬
void MQ_Init(void);
uint16_t AD_GetValue(uint8_t Channel);

#endif
