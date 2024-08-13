#include "Beep.h"

void Beep_Init(void)
{
    GPIO_InitTypeDef   GPIO_InitStructure;
    
    //打开GPIO端口的外设时钟  GPIOB
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    
    
    //配置PF9为推挽输出模式
    GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_15;   
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType   = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_NOPULL;
    //调用该函数对GPIOF端口进行初始化
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    GPIO_SetBits(GPIOB,GPIO_Pin_15);
}

void Beep_On(void)
{
    GPIO_ResetBits(GPIOB,GPIO_Pin_15);
}

void Beep_Off(void)
{
    GPIO_SetBits(GPIOB,GPIO_Pin_15);
}