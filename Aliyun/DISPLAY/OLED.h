#ifndef __OLED_H
#define __OLED_H

#include "stm32f4xx.h"                  // Device header
#include "Delay.h"

//PD13(SCL) PD11(SDA) 

#define OLED_SCL(x)		GPIO_WriteBit(GPIOD, GPIO_Pin_13, (BitAction)(x))  
#define OLED_SDA(x)		GPIO_WriteBit(GPIOD, GPIO_Pin_11, (BitAction)(x))

#define  IIC_SDA_READ GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_11)

#define  IIC_SCL_PORT GPIOD
#define  IIC_SDA_PORT GPIOD
    
#define  IIC_SCL_PIN GPIO_Pin_13
#define  IIC_SDA_PIN GPIO_Pin_11 

#define  IIC_SCL_CLOCK RCC_AHB1Periph_GPIOD
#define  IIC_SDA_CLOCK RCC_AHB1Periph_GPIOD               

void OLED_Init(void);
void OLED_Clear(void);
void OLED_ShowChar(uint8_t Line, uint8_t Column, char Char);
void OLED_ShowString(uint8_t Line, uint8_t Column, char *String);
void OLED_ShowNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);
void OLED_ShowSignedNum(uint8_t Line, uint8_t Column, int32_t Number, uint8_t Length);
void OLED_ShowHexNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);
void OLED_ShowBinNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);
void OLED_ShowChinse(uint8_t Line, uint8_t Column, int Char);
void OLED_ShowPicture(void);

#endif
