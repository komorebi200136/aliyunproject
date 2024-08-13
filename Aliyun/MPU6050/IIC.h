#ifndef __IIC_H
#define __IIC_H

#include "stm32f4xx.h"

//MPU6050:PB1(SCL)¡¢PC5(SDA)

#define MPU_SCL(x)		GPIO_WriteBit(GPIOB, GPIO_Pin_1, (BitAction)(x))  
#define MPU_SDA(x)		GPIO_WriteBit(GPIOC, GPIO_Pin_5, (BitAction)(x))

#define  MPU_SDA_READ GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_5)

#define  MPU_SCL_PORT GPIOB
#define  MPU_SDA_PORT GPIOC
    
#define  MPU_SCL_PIN GPIO_Pin_1
#define  MPU_SDA_PIN GPIO_Pin_5 

#define  MPU_SCL_CLOCK RCC_AHB1Periph_GPIOB
#define  MPU_SDA_CLOCK RCC_AHB1Periph_GPIOC   

void MPU_Pin_Init(void);
void MPU_Start(void);
void MPU_SendByte(uint8_t Data);
uint8_t MPU_IsSlaveACK(void);
uint8_t MPU_ReadByte(void);
void MPU_SendACK(uint8_t ack);
void MPU_Stop(void);

#endif

