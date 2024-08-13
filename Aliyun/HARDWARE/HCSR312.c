#include "HCSR312.h"


//PA5
void HCSR312_Init(void)
{
    GPIO_InitTypeDef   GPIO_InitStructure;
    
    //��GPIO�˿ڵ�����ʱ��  GPIOA
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    
    
    //����PA5Ϊ����ģʽ
    GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_5;   
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_DOWN;   //��������
    //���øú�����GPIOF�˿ڽ��г�ʼ��
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}

bool IsBodySensing(void)  //1��Ӧ��  0û��Ӧ��
{
    return GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_5);
}
