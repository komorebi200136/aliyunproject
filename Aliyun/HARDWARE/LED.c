#include "LED.h"

//LED1 PB4 LED2 PE6
void LED_Init(void)
{
    GPIO_InitTypeDef   GPIO_InitStructure;
    
    //��GPIO�˿ڵ�����ʱ��  GPIOB��GPIOE
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB|RCC_AHB1Periph_GPIOE, ENABLE);
    
    
    //����PB4Ϊ�������ģʽ
    GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_4;   
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType   = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_NOPULL;
    //���øú�����GPIOB�˿ڽ��г�ʼ��
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    //����PE6Ϊ�������ģʽ
    GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_6;   
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType   = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_NOPULL;
    //���øú�����GPIOE�˿ڽ��г�ʼ��
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
