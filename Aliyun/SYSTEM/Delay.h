#ifndef  __DELAY_H
#define  __DELAY_H

#include "stm32f4xx.h"

//�жϷ�ʽ�ӳ�
void Delay_Init(uint8_t SYSCLK);
void Delay_us(uint32_t xus);
void Delay_ms(uint32_t xms);
void MY_DelayMs(uint32_t ms_time);

//���жϷ�ʽ�ӳ�
void delay_us(uint32_t xus);
void delay_ms(uint32_t xms);
void delay_s(uint32_t xs);


#endif
