#include "HCSR312.h"


//PA5
void HCSR312_Init(void)
{
    GPIO_InitTypeDef   GPIO_InitStructure;
    
    //打开GPIO端口的外设时钟  GPIOA
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    
    
    //配置PA5为输入模式
    GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_5;   
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_DOWN;   //下拉输入
    //调用该函数对GPIOF端口进行初始化
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}

bool IsBodySensing(void)  //1感应到  0没感应到
{
    return GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_5);
}
