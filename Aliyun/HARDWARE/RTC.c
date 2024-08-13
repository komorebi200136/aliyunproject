#include "RTC.h"

volatile uint8_t TimeDisplay; //表示唤醒标志

//RTC的初始化
void RTC_Config(void)
{
    RTC_InitTypeDef   RTC_InitStructure;
    RTC_TimeTypeDef   RTC_TimeStructure;
    RTC_DateTypeDef   RTC_DateStructure;
    NVIC_InitTypeDef  NVIC_InitStructure;
    EXTI_InitTypeDef  EXTI_InitStructure;
    
    //1.打开PWR电源控制器外设时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
    
    //2.允许访问备份域，相当于允许访问RTC寄存器
    PWR_BackupAccessCmd(ENABLE);
    
    //3.配置RTC唤醒中断的参数
    NVIC_InitStructure.NVIC_IRQChannel = RTC_WKUP_IRQn;         //唤醒中断通道编号
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    //4.配置EXTI22外部中断线的参数
    EXTI_ClearITPendingBit(EXTI_Line22);                        //清除中断线标志   
    EXTI_InitStructure.EXTI_Line = EXTI_Line22;                 //选择中断线编号
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;         //设置为中断模式
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;      //选择上升沿触发
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);
    
    //5.使能低速外部时钟  LSE 时钟频率是32.768KHZ  可以得到准确的1HZ
    RCC_LSEConfig(RCC_LSE_ON);
    
    //6.等待LSE时钟源准备完成
    while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET);
    
    //7.选择把LSE作为RTC外设的时钟源
    RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
    
    //8.对RTC外设进行配置  ck_spre(1Hz) = RTCCLK(LSE) /(uwAsynchPrediv + 1)*(uwSynchPrediv + 1)
    RTC_InitStructure.RTC_AsynchPrediv = 128-1;
    RTC_InitStructure.RTC_SynchPrediv =  256-1;
    RTC_InitStructure.RTC_HourFormat = RTC_HourFormat_24; //采用24小时制
    RTC_Init(&RTC_InitStructure);
    
    //9.设置RTC时间   采用BCD码  BCD码： 0x09 + 1 = 0x10      HEX制: 0x09 + 1 = 0x0A 
    RTC_TimeStructure.RTC_Hours   = 0x17;  // BCD码 小时
    RTC_TimeStructure.RTC_Minutes = 0x00;  // BCD码 分钟
    RTC_TimeStructure.RTC_Seconds = 0x00;  // BCD码 秒钟
    RTC_SetTime(RTC_Format_BCD, &RTC_TimeStructure);  
    
    //10.设置RTC日期  采用BCD码  BCD码： 0x09 + 1 = 0x10      HEX制: 0x09 + 1 = 0x0A 
    RTC_DateStructure.RTC_Month = 0x11; // BCD码 月份
    RTC_DateStructure.RTC_Date  = 0x05; // BCD码 日期 
    RTC_DateStructure.RTC_Year  = 0x23; // BCD码 年份 
    RTC_DateStructure.RTC_WeekDay = RTC_Weekday_Sunday;  // BCD码 星期 
    RTC_SetDate(RTC_Format_BCD, &RTC_DateStructure); 
    
    //11.使能RTC时钟
    RCC_RTCCLKCmd(ENABLE);

    //12.等待RTC寄存器同步
    RTC_WaitForSynchro();
    
    //13.选择RTC唤醒时钟源  选择 内部时钟  经过分频之后得到的1HZ
    RTC_WakeUpClockConfig(RTC_WakeUpClock_CK_SPRE_16bits);
     
    //14.设置自动重载值  递减计数  为了得到周期1s，需要计数1次
    RTC_SetWakeUpCounter(0);
    
    //15.选择RTC的中断源 选择唤醒中断
    RTC_ITConfig(RTC_IT_WUT, ENABLE);
    
    //16.使能RTC计数器 则开始递减计数
    RTC_WakeUpCmd(ENABLE); 
    
    //17.清除RTC唤醒中断标志
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
        RTC_TimeStructure.RTC_Hours = num/10*16+num%10;  // BCD码 小时
                  
        num=atoi(strtok(NULL,":"));   //dest=24
        //Serial_SendByte(num);
        RTC_TimeStructure.RTC_Minutes = num/10*16+num%10;  // BCD码 分钟
        
        num=atoi(strtok(NULL,":"));   //dest=23
        //Serial_SendByte(num);
        RTC_TimeStructure.RTC_Seconds = num/10*16+num%10;  // BCD码 秒钟
        
        RTC_SetTime(RTC_Format_BCD, &RTC_TimeStructure); 

    }
    if(strstr((char*)RxBuffer,"date")!=NULL)  
    {  
        strtok((char*)RxBuffer,"=");         //date=23-11-05
        token = strtok(NULL,"=");            //token=23-11-05
                 
        num=atoi(strtok(token,"-"));      //dest=23
        //Serial_SendByte(num);
        RTC_DateStructure.RTC_Year = num/10*16+num%10;  // BCD码 年
                  
        num=atoi(strtok(NULL,"-"));   //dest=11
        //Serial_SendByte(num);
        RTC_DateStructure.RTC_Month = num/10*16+num%10;  // BCD码 月
        
        num=atoi(strtok(NULL,"-"));   //dest=05
        //Serial_SendByte(num);
        RTC_DateStructure.RTC_Date = num/10*16+num%10;  // BCD码 日
        
        RTC_SetDate(RTC_Format_BCD, &RTC_DateStructure); 
    }
}

//RTC唤醒中断服务函数 每隔1秒钟
void RTC_WKUP_IRQHandler(void)
{
  //判断唤醒中断是否触发
  if(RTC_GetITStatus(RTC_IT_WUT) != RESET)
  {
      TimeDisplay = 1; //说明唤醒 
    
      RTC_ClearITPendingBit(RTC_IT_WUT);
      EXTI_ClearITPendingBit(EXTI_Line22);
  }
}
