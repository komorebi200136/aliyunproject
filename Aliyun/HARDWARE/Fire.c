#include "HCSR312.h"


//PF6
void Fire_Init(void)
{
    GPIO_InitTypeDef   GPIO_InitStructure;
    
    //��GPIO�˿ڵ�����ʱ��  GPIOF
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);
    
    
    //����PF6Ϊ����ģʽ
    GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_6;   
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_UP;   //��������
    //���øú�����GPIOF�˿ڽ��г�ʼ��
    GPIO_Init(GPIOF, &GPIO_InitStructure);
}

bool IsFireSensing(void)  //1û��Ӧ����  0��Ӧ����
{
    return GPIO_ReadInputDataBit(GPIOF,GPIO_Pin_6);
}



