#include "BlueTooth.h"

//PB10  -- USART3_TX
//PB11  -- USART3_RX

char blueteeth_buf[512];
uint16_t blueteeth_cnt;
uint16_t blueteeth_cntPre;
uint16_t blueteeth_recvcnt;

void BlueTooth_Init(uint32_t baud)
{
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef  NVIC_InitStructure;
    GPIO_InitTypeDef  GPIO_InitStructure;

    //使能GPIOB时钟
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

    //使能USART3时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;        //复用模式
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;  //速度
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;      //推挽模式
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;         //上拉模式
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10|GPIO_Pin_11;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_USART3);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, GPIO_AF_USART3);


    USART_InitStructure.USART_BaudRate = baud;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b; //八位数据位
    USART_InitStructure.USART_StopBits = USART_StopBits_1;      //1位停止位
    USART_InitStructure.USART_Parity = USART_Parity_No;         //无校验
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; //无硬件流控
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART3, &USART_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);   //启动接收中断
    
    USART_Cmd(USART3, ENABLE);
    
}

//发送一个字节的数据
void BlueTooth_SendByte(uint8_t Byte)
{
	USART_SendData(USART3, Byte);
	while(USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);
}

//发送一个字符串的数据
void BlueTooth_SendString(char *String)
{
	while(*String != '\0')
    {
        BlueTooth_SendByte(*String++);
    }
}

void Blueteeth_Clear(void)
{
	memset(blueteeth_buf, 0, sizeof(blueteeth_buf));
	blueteeth_cnt = 0;
}

bool Blueteeth_WaitRecive(void)
{
	if(blueteeth_cnt == 0) 						//如果接收计数为0 则说明没有处于接收数据中，所以直接跳出，结束函数
		return 0;	
	if(blueteeth_cnt == blueteeth_cntPre)		//如果上一次的值和这次相同，则说明接收完毕
	{
        blueteeth_recvcnt=blueteeth_cnt;
		blueteeth_cnt = 0;						//清0接收计数	
		return 1;								//返回接收完成标志
	}		
	blueteeth_cntPre = blueteeth_cnt;			//置为相同
	return 0;								    //返回接收未完成标志

}

//切割接收到的字符串
uint16_t slice(char *str) 
{
    char *ptr;
    int value;

    // 使用strtok函数以'='为分隔符分割字符串，得到"tem"和"16"
    ptr = strtok(str, "=");

    // 继续调用strtok函数，得到"16"
    ptr = strtok(NULL, "=");

    // 使用atoi函数将字符串转换为整数
    value = atoi(ptr);

    // 打印提取的整数
    return value;
}

void USART3_IRQHandler(void)
{
    //判断中断是否触发
    if (USART_GetITStatus(USART3, USART_IT_RXNE) == SET)
    {  
	  if(blueteeth_cnt >= sizeof(blueteeth_buf))	blueteeth_cnt = 0; //防止串口被刷爆     
	  blueteeth_buf[blueteeth_cnt++]=USART_ReceiveData(USART3);
      USART_ClearITPendingBit(USART3,USART_IT_RXNE); 
    }
}



