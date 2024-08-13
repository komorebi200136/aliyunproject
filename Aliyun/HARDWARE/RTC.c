#include "RTC.h"

volatile uint8_t TimeDisplay; //��ʾ���ѱ�־

//RTC�ĳ�ʼ��
void RTC_Config(void)
{
    RTC_InitTypeDef   RTC_InitStructure;
    RTC_TimeTypeDef   RTC_TimeStructure;
    RTC_DateTypeDef   RTC_DateStructure;
    NVIC_InitTypeDef  NVIC_InitStructure;
    EXTI_InitTypeDef  EXTI_InitStructure;
    
    //1.��PWR��Դ����������ʱ��
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
    
    //2.������ʱ������൱���������RTC�Ĵ���
    PWR_BackupAccessCmd(ENABLE);
    
    //3.����RTC�����жϵĲ���
    NVIC_InitStructure.NVIC_IRQChannel = RTC_WKUP_IRQn;         //�����ж�ͨ�����
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    //4.����EXTI22�ⲿ�ж��ߵĲ���
    EXTI_ClearITPendingBit(EXTI_Line22);                        //����ж��߱�־   
    EXTI_InitStructure.EXTI_Line = EXTI_Line22;                 //ѡ���ж��߱��
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;         //����Ϊ�ж�ģʽ
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;      //ѡ�������ش���
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);
    
    //5.ʹ�ܵ����ⲿʱ��  LSE ʱ��Ƶ����32.768KHZ  ���Եõ�׼ȷ��1HZ
    RCC_LSEConfig(RCC_LSE_ON);
    
    //6.�ȴ�LSEʱ��Դ׼�����
    while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET);
    
    //7.ѡ���LSE��ΪRTC�����ʱ��Դ
    RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
    
    //8.��RTC�����������  ck_spre(1Hz) = RTCCLK(LSE) /(uwAsynchPrediv + 1)*(uwSynchPrediv + 1)
    RTC_InitStructure.RTC_AsynchPrediv = 128-1;
    RTC_InitStructure.RTC_SynchPrediv =  256-1;
    RTC_InitStructure.RTC_HourFormat = RTC_HourFormat_24; //����24Сʱ��
    RTC_Init(&RTC_InitStructure);
    
    //9.����RTCʱ��   ����BCD��  BCD�룺 0x09 + 1 = 0x10      HEX��: 0x09 + 1 = 0x0A 
    RTC_TimeStructure.RTC_Hours   = 0x17;  // BCD�� Сʱ
    RTC_TimeStructure.RTC_Minutes = 0x00;  // BCD�� ����
    RTC_TimeStructure.RTC_Seconds = 0x00;  // BCD�� ����
    RTC_SetTime(RTC_Format_BCD, &RTC_TimeStructure);  
    
    //10.����RTC����  ����BCD��  BCD�룺 0x09 + 1 = 0x10      HEX��: 0x09 + 1 = 0x0A 
    RTC_DateStructure.RTC_Month = 0x11; // BCD�� �·�
    RTC_DateStructure.RTC_Date  = 0x05; // BCD�� ���� 
    RTC_DateStructure.RTC_Year  = 0x23; // BCD�� ��� 
    RTC_DateStructure.RTC_WeekDay = RTC_Weekday_Sunday;  // BCD�� ���� 
    RTC_SetDate(RTC_Format_BCD, &RTC_DateStructure); 
    
    //11.ʹ��RTCʱ��
    RCC_RTCCLKCmd(ENABLE);

    //12.�ȴ�RTC�Ĵ���ͬ��
    RTC_WaitForSynchro();
    
    //13.ѡ��RTC����ʱ��Դ  ѡ�� �ڲ�ʱ��  ������Ƶ֮��õ���1HZ
    RTC_WakeUpClockConfig(RTC_WakeUpClock_CK_SPRE_16bits);
     
    //14.�����Զ�����ֵ  �ݼ�����  Ϊ�˵õ�����1s����Ҫ����1��
    RTC_SetWakeUpCounter(0);
    
    //15.ѡ��RTC���ж�Դ ѡ�����ж�
    RTC_ITConfig(RTC_IT_WUT, ENABLE);
    
    //16.ʹ��RTC������ ��ʼ�ݼ�����
    RTC_WakeUpCmd(ENABLE); 
    
    //17.���RTC�����жϱ�־
    RTC_ClearITPendingBit(RTC_IT_WUT);
}

void SetTimeDate(char *RxBuffer,char ubRxIndex)
{
    char *token;
    uint8_t num=0;
    RTC_TimeTypeDef   RTC_TimeStructure;
    RTC_DateTypeDef   RTC_DateStructure;
    if(strstr((char*)RxBuffer,"time")!=NULL)  
    {
        strtok((char*)RxBuffer,"=");          //time=15:24:23
        token = strtok(NULL,"=");            //token="15:24:23"
                 
        num=atoi(strtok(token,":"));      //dest=15
        //Serial_SendByte(num);
        RTC_TimeStructure.RTC_Hours = num/10*16+num%10;  // BCD�� Сʱ
                  
        num=atoi(strtok(NULL,":"));   //dest=24
        //Serial_SendByte(num);
        RTC_TimeStructure.RTC_Minutes = num/10*16+num%10;  // BCD�� ����
        
        num=atoi(strtok(NULL,":"));   //dest=23
        //Serial_SendByte(num);
        RTC_TimeStructure.RTC_Seconds = num/10*16+num%10;  // BCD�� ����
        
        RTC_SetTime(RTC_Format_BCD, &RTC_TimeStructure); 

    }
    if(strstr((char*)RxBuffer,"date")!=NULL)  
    {  
        strtok((char*)RxBuffer,"=");         //date=23-11-05
        token = strtok(NULL,"=");            //token=23-11-05
                 
        num=atoi(strtok(token,"-"));      //dest=23
        //Serial_SendByte(num);
        RTC_DateStructure.RTC_Year = num/10*16+num%10;  // BCD�� ��
                  
        num=atoi(strtok(NULL,"-"));   //dest=11
        //Serial_SendByte(num);
        RTC_DateStructure.RTC_Month = num/10*16+num%10;  // BCD�� ��
        
        num=atoi(strtok(NULL,"-"));   //dest=05
        //Serial_SendByte(num);
        RTC_DateStructure.RTC_Date = num/10*16+num%10;  // BCD�� ��
        
        RTC_SetDate(RTC_Format_BCD, &RTC_DateStructure); 
    }
}

//RTC�����жϷ����� ÿ��1����
void RTC_WKUP_IRQHandler(void)
{
  //�жϻ����ж��Ƿ񴥷�
  if(RTC_GetITStatus(RTC_IT_WUT) != RESET)
  {
      TimeDisplay = 1; //˵������ 
    
      RTC_ClearITPendingBit(RTC_IT_WUT);
      EXTI_ClearITPendingBit(EXTI_Line22);
  }
}
