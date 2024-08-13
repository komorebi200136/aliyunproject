#include "MQADC.h"

/*******************************************************************
MQ2  PF5  ADC3_IN15
MQ5  PF4  ADC3_IN14


********************************************************************/
void MQ_Init(void)
{
    ADC_InitTypeDef       ADC_InitStructure;          //ADC的通道
    ADC_CommonInitTypeDef ADC_CommonInitStructure;    //基本参数
    GPIO_InitTypeDef      GPIO_InitStructure;

    //1.打开ADC3的外设时钟 + GPIOF端口外设时钟   
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);  
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC3, ENABLE);
  
    //2.配置GPIOF引脚  PF7  模拟模式
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_4|GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AN;       //模拟模式
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL ;
    GPIO_Init(GPIOF, &GPIO_InitStructure);

    //3.配置ADC的基本参数
    ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;                    //独立模式
    ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div4;                 //时钟分频  21MHZ
    ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;     //不使用DMA
    ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;//采样间隔
    ADC_CommonInit(&ADC_CommonInitStructure);

    //4.配置ADC3转换器
    ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;                      //转换精度
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;                               //关闭扫描
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;                         //不连续转换
    ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None; //不使用外部触发沿,内部软件触发
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;                      //数据对齐 右对齐
    ADC_InitStructure.ADC_NbrOfConversion = 1;                                  //通道数目
    ADC_Init(ADC3, &ADC_InitStructure); 

    //配置ADC通道
    //ADC_RegularChannelConfig(ADC3, ADC_Channel_5,1, ADC_SampleTime_3Cycles);

    //使能ADC
    ADC_Cmd(ADC3, ENABLE);

}


uint16_t AD_GetValue(uint8_t Channel)   //需要指定通道 PF5 ADC_Channel_15 PF4  ADC_Channel_14
{
	ADC_RegularChannelConfig(ADC3, Channel,1, ADC_SampleTime_3Cycles);	//在每次转换前，根据函数形参灵活更改规则组的通道1
	ADC_SoftwareStartConv(ADC3);       				                    //软件触发AD转换一次
	while( ADC_GetFlagStatus(ADC3,ADC_FLAG_EOC) == RESET );             //等待转换完成
	return ADC_GetConversionValue(ADC3);					            //读数据寄存器，得到AD转换的结果
}



      