#include "HCSR312.h"


//PF6
void Fire_Init(void)
{
    GPIO_InitTypeDef   GPIO_InitStructure;
    
    //打开GPIO端口的外设时钟  GPIOF
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);
    
    
    //配置PF6为输入模式
    GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_6;   
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_UP;   //上拉输入
    //调用该函数对GPIOF端口进行初始化
    GPIO_Init(GPIOF, &GPIO_InitStructure);
}

bool IsFireSensing(void)  //1没感应到火  0感应到火
{
    return GPIO_ReadInputDataBit(GPIOF,GPIO_Pin_6);
}



