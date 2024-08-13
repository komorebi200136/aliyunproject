#include "LED.h"

//LED1 PB4 LED2 PE6
void LED_Init(void)
{
    GPIO_InitTypeDef   GPIO_InitStructure;
    
    //打开GPIO端口的外设时钟  GPIOB和GPIOE
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB|RCC_AHB1Periph_GPIOE, ENABLE);
    
    
    //配置PB4为推挽输出模式
    GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_4;   
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType   = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_NOPULL;
    //调用该函数对GPIOB端口进行初始化
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    //配置PE6为推挽输出模式
    GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_6;   
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType   = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_NOPULL;
    //调用该函数对GPIOE端口进行初始化
    GPIO_Init(GPIOE, &GPIO_InitStructure);
	GPIO_SetBits(GPIOE,GPIO_Pin_6);
    GPIO_SetBits(GPIOB,GPIO_Pin_4);
}

void RLED_On(void)
{
	
	GPIO_ResetBits(GPIOE,GPIO_Pin_6);
}
void RLED_Off(void)
{
	GPIO_SetBits(GPIOE,GPIO_Pin_6);
}
void WLED_On(void)
{
	GPIO_ResetBits(GPIOB,GPIO_Pin_4);
}
void WLED_Off(void)
{
	GPIO_SetBits(GPIOB,GPIO_Pin_4);
}
