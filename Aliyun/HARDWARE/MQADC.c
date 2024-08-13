#include "MQADC.h"

/*******************************************************************
MQ2  PF5  ADC3_IN15
MQ5  PF4  ADC3_IN14


********************************************************************/
void MQ_Init(void)
{
    ADC_InitTypeDef       ADC_InitStructure;          //ADC��ͨ��
    ADC_CommonInitTypeDef ADC_CommonInitStructure;    //��������
    GPIO_InitTypeDef      GPIO_InitStructure;

    //1.��ADC3������ʱ�� + GPIOF�˿�����ʱ��   
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);  
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC3, ENABLE);
  
    //2.����GPIOF����  PF7  ģ��ģʽ
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_4|GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AN;       //ģ��ģʽ
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL ;
    GPIO_Init(GPIOF, &GPIO_InitStructure);

    //3.����ADC�Ļ�������
    ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;                    //����ģʽ
    ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div4;                 //ʱ�ӷ�Ƶ  21MHZ
    ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;     //��ʹ��DMA
    ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;//�������
    ADC_CommonInit(&ADC_CommonInitStructure);

    //4.����ADC3ת����
    ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;                      //ת������
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;                               //�ر�ɨ��
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;                         //������ת��
    ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None; //��ʹ���ⲿ������,�ڲ��������
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;                      //���ݶ��� �Ҷ���
    ADC_InitStructure.ADC_NbrOfConversion = 1;                                  //ͨ����Ŀ
    ADC_Init(ADC3, &ADC_InitStructure); 

    //����ADCͨ��
    //ADC_RegularChannelConfig(ADC3, ADC_Channel_5,1, ADC_SampleTime_3Cycles);

    //ʹ��ADC
    ADC_Cmd(ADC3, ENABLE);

}


uint16_t AD_GetValue(uint8_t Channel)   //��Ҫָ��ͨ�� PF5 ADC_Channel_15 PF4  ADC_Channel_14
{
	ADC_RegularChannelConfig(ADC3, Channel,1, ADC_SampleTime_3Cycles);	//��ÿ��ת��ǰ�����ݺ����β������Ĺ������ͨ��1
	ADC_SoftwareStartConv(ADC3);       				                    //�������ADת��һ��
	while( ADC_GetFlagStatus(ADC3,ADC_FLAG_EOC) == RESET );             //�ȴ�ת�����
	return ADC_GetConversionValue(ADC3);					            //�����ݼĴ������õ�ADת���Ľ��
}



      