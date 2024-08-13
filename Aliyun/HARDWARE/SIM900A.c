#include "SIM900A.h"
#include "serial.h"
#include "Delay.h"
#include <stdbool.h>

//UART4 PC10(UART4_TX) PC11(UART4_RX)

uint16_t SIM_Count;
uint16_t SIM_CountPre;
char SIM_Buf[512];

void SIM_Init(uint32_t baud)
{
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef  NVIC_InitStructure;
    GPIO_InitTypeDef  GPIO_InitStructure;

    //使能GPIOC时钟
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

    //使能UART4时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);

    //配置引脚
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;        //复用模式
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;  //速度
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;      //推挽模式
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;         //上拉模式
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10  | GPIO_Pin_11;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource10, GPIO_AF_UART4);
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource11, GPIO_AF_UART4);


    USART_InitStructure.USART_BaudRate = baud;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b; //八位数据位
    USART_InitStructure.USART_StopBits = USART_StopBits_1;      //1位停止位
    USART_InitStructure.USART_Parity = USART_Parity_No;         //无校验
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; //无硬件流控
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(UART4, &USART_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);   //启动接收中断
    
    USART_Cmd(UART4, ENABLE);
    
}

//发送一个字节的数据
void SIM_SendByte(uint8_t Byte)
{
	USART_SendData(UART4, Byte);
	while(USART_GetFlagStatus(UART4, USART_FLAG_TXE) == RESET);
}

//发送一个字符串的数据
void SIM_SendString(char *String)
{
	while(*String != '\0')
    {
        SIM_SendByte(*String++);
    }
}


void SIM900A_Clear(void)
{
	memset(SIM_Buf, 0, sizeof(SIM_Buf));
	SIM_Count = 0;
}

bool SIM900A_WaitRecive(void)
{

	if(SIM_Count == 0) 							//如果接收计数为0 则说明没有处于接收数据中，所以直接跳出，结束函数
		return 0;
		
	if(SIM_Count == SIM_CountPre)				//如果上一次的值和这次相同，则说明接收完毕
	{
		SIM_Count = 0;							//清0接收计数
			
		return 1;								//返回接收完成标志
	}
		
	SIM_CountPre = SIM_Count;					//置为相同
	
	return 0;								    //返回接收未完成标志

}

bool SIM900A_SendCmd(char *cmd, char *ret)
{
	int timeOut = 5000;                       //超时处理
    SIM_SendString(cmd);                      //发送AT指令
	while(timeOut--)
	{
		if(SIM900A_WaitRecive() == 1)		  //如果数据接收完整
		{
			if(strstr(SIM_Buf, ret) != NULL)  //如果检索到关键词
			{
				SIM900A_Clear();			  //清空缓存
				
				return 0;
			}
		}
		
		Delay_ms(10);
	}
	
	return 1;

}

void Send_ChineseMessages(char message[],char phone[])
{
	char cmdphone[100]={0};
	sprintf(cmdphone,"AT+CMGS=\"%s\"\r\n",phone);
	
	SIM900A_SendCmd("AT+CSCS=\"UCS2\"\r\n", "OK"); //设置命令
	/* 
    “UCS2”
     16位通用8字节倍数编码字符集(ISO/IEC10646);
	 UCS2 字符串转换为从 0000到 FFFF 的 16 进制数;
	 例如:"004100620063'表示三个 16 位字符，十进制的值分别是 65.98 和 99。
	*/
	Delay_ms(100);
	SIM900A_SendCmd("AT+CMGF=1\r\n", "OK"); //设置短信系统进入文本模式，与之对应的是 PDU 模式。
	Delay_ms(100);
	SIM900A_SendCmd("AT+CSCA?\r\n", "OK");  //查询命令
	Delay_ms(100);
	SIM900A_SendCmd("AT+CSMP=17,167,0,25\r\n", "OK"); //发送含汉字的短信
	Delay_ms(100);
	SIM900A_SendCmd(cmdphone, ">");   //电话要是unicon码
	Delay_ms(100);
	SIM_SendString(message);        //短信内容，但都是对应的unicon 编码。
	SIM_SendByte(0X1A);             //确认发送  16进制1A和字符1A的区别
    Delay_ms(5000);

}
void Send_EnglishMessages(char message[],char phone[])
{
    char cmdphone[100]={0};
    sprintf(cmdphone,"AT+CMGS=\"%s\"\r\n",phone);
	
    SIM900A_SendCmd("AT\r\n", "OK");       //5S超时处理
	Delay_ms(1000);
    SIM900A_SendCmd("AT+CSQ\r\n", "OK");   
	Delay_ms(1000);
    SIM900A_SendCmd("AT+CPIN?\r\n", "OK");
	Delay_ms(1000);
    SIM900A_SendCmd("AT+COPS?\r\n", "OK");   
	Delay_ms(1000);
    SIM900A_SendCmd("AT+CSCS=\"GSM\"\r\n", "OK");  
	Delay_ms(1000);
    SIM900A_SendCmd("AT+CMGF=1\r\n", "OK");   
	Delay_ms(1000);
    SIM900A_SendCmd(cmdphone, ">");   
    SIM_SendString(message);        //发送短信内容
    SIM_SendByte(0X1A);             //确认发送  16进制1A和字符1A的区别
    Delay_ms(5000);

}
void UART4_IRQHandler(void)
{
    //判断中断是否触发
    if (USART_GetITStatus(UART4, USART_IT_RXNE) == SET)
    {   
      if(SIM_Count >= sizeof(SIM_Buf))	SIM_Count = 0; //防止串口被刷爆    
	  SIM_Buf[SIM_Count++]=USART_ReceiveData(UART4);  
      USART_ClearITPendingBit(UART4,USART_IT_RXNE);
    }

}
