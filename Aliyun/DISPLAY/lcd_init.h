#ifndef __LCD_INIT_H
#define __LCD_INIT_H

#include "sys.h"

#define USE_HORIZONTAL 0  //设置横屏或者竖屏显示 0或1为竖屏 2或3为横屏


#if USE_HORIZONTAL==0||USE_HORIZONTAL==1
#define LCD_W 240
#define LCD_H 320

#else
#define LCD_W 320
#define LCD_H 240
#endif



//-----------------LCD端口定义---------------- 

#define LCD_SCLK_Clr() GPIO_ResetBits(GPIOG,GPIO_Pin_12)//SCL=SCLK
#define LCD_SCLK_Set() GPIO_SetBits(GPIOG,GPIO_Pin_12)

#define LCD_MOSI_Clr() GPIO_ResetBits(GPIOE,GPIO_Pin_3)//SDA=MOSI
#define LCD_MOSI_Set() GPIO_SetBits(GPIOE,GPIO_Pin_3)

#define LCD_RES_Clr()  GPIO_ResetBits(GPIOE,GPIO_Pin_5)//RES
#define LCD_RES_Set()  GPIO_SetBits(GPIOE,GPIO_Pin_5)

#define LCD_DC_Clr()   GPIO_ResetBits(GPIOC,GPIO_Pin_0)//DC
#define LCD_DC_Set()   GPIO_SetBits(GPIOC,GPIO_Pin_0)
 		     
#define LCD_CS_Clr()   GPIO_ResetBits(GPIOE,GPIO_Pin_1)//CS
#define LCD_CS_Set()   GPIO_SetBits(GPIOE,GPIO_Pin_1)

#define LCD_BLK_Clr()  GPIO_ResetBits(GPIOB,GPIO_Pin_7)//BLK
#define LCD_BLK_Set()  GPIO_SetBits(GPIOB,GPIO_Pin_7)




void LCD_GPIO_Init(void);//初始化GPIO
void LCD_Writ_Bus(uint8_t dat);//模拟SPI时序
void LCD_WR_DATA8(uint8_t dat);//写入一个字节
void LCD_WR_DATA(uint16_t dat);//写入两个字节
void LCD_WR_REG(uint8_t dat);//写入一个指令
void LCD_Address_Set(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2);//设置坐标函数
void LCD_Init(void);//LCD初始化

#endif




