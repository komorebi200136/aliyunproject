#include "delay.h"
#include "FreeRTOS.h"				
#include "task.h"
#include "delay.h"

static u8  fac_us=0;							//us��ʱ������			   
static u16 fac_ms=0;							//ms��ʱ������,��os��,����ÿ�����ĵ�ms��

//��ʵʱ����ϵͳ
void delay_us(uint32_t xus)
{
    SysTick->CTRL = 0;         // �رն�ʱ��
    SysTick->LOAD = xus*21-1;  //��������
    SysTick->VAL = 0;          // �����ǰֵ��counter flag��־λ
    SysTick->CTRL = 1;         //����Ϊ�ⲿʱ��21MHz,�򿪶�ʱ��
    while ((SysTick->CTRL & 0x00010000)==0);// �ȴ�count flag ��1
    SysTick->CTRL = 0;          // �رն�ʱ��
}

void delay_ms(uint32_t xms)
{
	while(xms--)
	{
		delay_us(1000);  //1ms
	}
}
 
/**
  * @brief  �뼶��ʱ
  * @param  xs ��ʱʱ������Χ��0~4294967295
  * @retval ��
  */
void delay_s(uint32_t xs)
{
	while(xs--)
	{
		delay_ms(1000);
	}
} 
	   
//��ʼ���ӳٺ���
//SYSTICK��ʱ�ӹ̶�ΪAHBʱ�ӣ�������������SYSTICKʱ��Ƶ��ΪAHB/8
//����Ϊ�˼���FreeRTOS�����Խ�SYSTICK��ʱ��Ƶ�ʸ�ΪAHB��Ƶ�ʣ�
//SYSCLK:ϵͳʱ��Ƶ��

void Delay_Init(uint8_t SYSCLK)
{
	u32 reload;
 	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);   //Ƶ��168MHZ
	fac_us=SYSCLK;							//�����Ƿ�ʹ��OS,fac_us����Ҫʹ��
	reload=SYSCLK;							//ÿ���ӵļ������� ��λΪM	   
	reload*=1000000/configTICK_RATE_HZ;		//����delay_ostickspersec�趨���ʱ��
                                            //reloadΪ24λ�Ĵ���,���ֵ:16777216,��168M��,Լ��0.0998s����	
	fac_ms=1000/configTICK_RATE_HZ;			//����OS������ʱ�����ٵ�λ	   
	SysTick->CTRL|=SysTick_CTRL_TICKINT_Msk;//����SYSTICK�ж�
	SysTick->LOAD=reload; 					//ÿ1/configTICK_RATE_HZ��һ��	
	SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk; //����SYSTICK     
}								    

//ϵͳ��΢�� 
void Delay_us(uint32_t nus)
{
	uint32_t told,tnow,tcnt=0;
	uint32_t temp=0;
	uint32_t reload=SysTick->LOAD;					//ϵͳ��ʱ��������ֵ
	uint32_t ticks=nus*(SystemCoreClock/1000000);	//�ܹ�Ҫ�ȴ��ĵδ���Ŀ
	told=SysTick->VAL;        						//�ս���ʱ�ļ�����ֵ
	
	//������������[��ѡ]
	//vTaskSuspendAll();
	
	while(1)
	{
		//��ȡ��ǰ����ֵ
		tnow=SysTick->VAL;					
		
		if(tnow!=told)
		{	    
			//SYSTICK��һ���ݼ��ļ�����
			if(tnow<told)
				tcnt+=told-tnow;			
			else 
				tcnt+=reload-tnow+told;
			
			told=tnow;
			
			//ʱ�䳬��/����Ҫ�ӳٵ�ʱ��,���˳�.
			if(tcnt>=ticks)
				break;						
		}  
		
		temp=SysTick->CTRL;
		
		//����ʱ����;�ر��ˣ�����ѭ��
		if((temp & 0x01)==0)			
			break;
	}
	//�ָ���������[��ѡ]
	//xTaskResumeAll();
}

//���鲻Ҫʹ�����ms��ʱ��ʹ��ϵͳ�ṩ��vTaskDelay ms��ʱ
void Delay_ms(u32 nms)
{	
	if(xTaskGetSchedulerState()!=taskSCHEDULER_NOT_STARTED)//ϵͳ�Ѿ�����
	{		
		if(nms>=fac_ms)						//��ʱ��ʱ�����OS������ʱ������ 
		{ 
   			vTaskDelay(nms/fac_ms);	 		//FreeRTOS��ʱ 
		}
		nms%=fac_ms;						//OS�Ѿ��޷��ṩ��ôС����ʱ��,������ͨ��ʽ��ʱ    
	}
	Delay_us((u32)(nms*1000));				//��ͨ��ʽ��ʱ
}

// �Լ�д����ʱ����
void MY_DelayMs(uint32_t ms_time)
{
	uint32_t ms_i = 0;
	while( ms_time-- )
	{
		ms_i = 16000;
		while( ms_i--);
	}
}

