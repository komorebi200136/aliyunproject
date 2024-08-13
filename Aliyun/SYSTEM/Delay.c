#include "delay.h"
#include "FreeRTOS.h"				
#include "task.h"
#include "delay.h"

static u8  fac_us=0;							//us延时倍乘数			   
static u16 fac_ms=0;							//ms延时倍乘数,在os下,代表每个节拍的ms数

//非实时操作系统
void delay_us(uint32_t xus)
{
    SysTick->CTRL = 0;         // 关闭定时器
    SysTick->LOAD = xus*21-1;  //计数次数
    SysTick->VAL = 0;          // 清除当前值和counter flag标志位
    SysTick->CTRL = 1;         //设置为外部时钟21MHz,打开定时器
    while ((SysTick->CTRL & 0x00010000)==0);// 等待count flag 置1
    SysTick->CTRL = 0;          // 关闭定时器
}

void delay_ms(uint32_t xms)
{
	while(xms--)
	{
		delay_us(1000);  //1ms
	}
}
 
/**
  * @brief  秒级延时
  * @param  xs 延时时长，范围：0~4294967295
  * @retval 无
  */
void delay_s(uint32_t xs)
{
	while(xs--)
	{
		delay_ms(1000);
	}
} 
	   
//初始化延迟函数
//SYSTICK的时钟固定为AHB时钟，基础例程里面SYSTICK时钟频率为AHB/8
//这里为了兼容FreeRTOS，所以将SYSTICK的时钟频率改为AHB的频率！
//SYSCLK:系统时钟频率

void Delay_Init(uint8_t SYSCLK)
{
	u32 reload;
 	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);   //频率168MHZ
	fac_us=SYSCLK;							//不论是否使用OS,fac_us都需要使用
	reload=SYSCLK;							//每秒钟的计数次数 单位为M	   
	reload*=1000000/configTICK_RATE_HZ;		//根据delay_ostickspersec设定溢出时间
                                            //reload为24位寄存器,最大值:16777216,在168M下,约合0.0998s左右	
	fac_ms=1000/configTICK_RATE_HZ;			//代表OS可以延时的最少单位	   
	SysTick->CTRL|=SysTick_CTRL_TICKINT_Msk;//开启SYSTICK中断
	SysTick->LOAD=reload; 					//每1/configTICK_RATE_HZ断一次	
	SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk; //开启SYSTICK     
}								    

//系统的微秒 
void Delay_us(uint32_t nus)
{
	uint32_t told,tnow,tcnt=0;
	uint32_t temp=0;
	uint32_t reload=SysTick->LOAD;					//系统定时器的重载值
	uint32_t ticks=nus*(SystemCoreClock/1000000);	//总共要等待的滴答数目
	told=SysTick->VAL;        						//刚进入时的计数器值
	
	//挂起所有任务[可选]
	//vTaskSuspendAll();
	
	while(1)
	{
		//获取当前计数值
		tnow=SysTick->VAL;					
		
		if(tnow!=told)
		{	    
			//SYSTICK是一个递减的计数器
			if(tnow<told)
				tcnt+=told-tnow;			
			else 
				tcnt+=reload-tnow+told;
			
			told=tnow;
			
			//时间超过/等于要延迟的时间,则退出.
			if(tcnt>=ticks)
				break;						
		}  
		
		temp=SysTick->CTRL;
		
		//若定时器中途关闭了，跳出循环
		if((temp & 0x01)==0)			
			break;
	}
	//恢复所有任务[可选]
	//xTaskResumeAll();
}

//建议不要使用这个ms延时，使用系统提供的vTaskDelay ms延时
void Delay_ms(u32 nms)
{	
	if(xTaskGetSchedulerState()!=taskSCHEDULER_NOT_STARTED)//系统已经运行
	{		
		if(nms>=fac_ms)						//延时的时间大于OS的最少时间周期 
		{ 
   			vTaskDelay(nms/fac_ms);	 		//FreeRTOS延时 
		}
		nms%=fac_ms;						//OS已经无法提供这么小的延时了,采用普通方式延时    
	}
	Delay_us((u32)(nms*1000));				//普通方式延时
}

// 自己写的延时函数
void MY_DelayMs(uint32_t ms_time)
{
	uint32_t ms_i = 0;
	while( ms_time-- )
	{
		ms_i = 16000;
		while( ms_i--);
	}
}

