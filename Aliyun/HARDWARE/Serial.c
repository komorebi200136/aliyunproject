#include "Serial.h"

volatile uint8_t RxBuffer [50];        //接收缓冲区
volatile uint8_t ubRxIndex = 0x00;     //接收缓冲区的下标

void Serial_Init(uint32_t baud)
{
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;        //复用模式
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;  //速度
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;      //推挽模式
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;         //上拉模式
    GPIO_InitStructure.GPIO_Pin = USART1_TX | USART1_RX;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);


    USART_InitStructure.USART_BaudRate = baud;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b; //八位数据位
    USART_InitStructure.USART_StopBits = USART_StopBits_1;      //1位停止位
    USART_InitStructure.USART_Parity = USART_Parity_No;         //无校验
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; //无硬件流控
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1, &USART_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);   //启动接收中断
    
    USART_Cmd(USART1, ENABLE);
    
}

//发送一个字节的数据
void Serial_SendByte(uint8_t Byte)
{
	USART_SendData(USART1, Byte);
	while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
}

//发送一个字符串的数据
void Serial_SendString(char *String)
{
	while(*String != '\0')
    {
        Serial_SendByte(*String++);
    }
}

void USART1_SendStr(void)
{
    USART_SendData(USART1,RxBuffer[ubRxIndex-1]); //把接收到的字节通过串口发送出去
    while(USART_GetFlagStatus(USART1, USART_FLAG_TXE)==RESET);
}

//可以对fputc函数进行重定向，重定向到串口 因为printf()会间接调用该函数,可以使用ST微型库代替标准C库
int fputc(int ch, FILE *f) 
{
    USART_SendData(USART1,ch); //发送一个字符
    while( USART_GetFlagStatus(USART1,USART_FLAG_TXE) == RESET ); //等待字符发送完成
    return ch;
}

void USART1_IRQHandler(void)
{
    //判断中断是否触发
    if (USART_GetITStatus(USART1, USART_IT_RXNE) == SET)
    {   
      //把USART1接收到的数据存储在接收缓冲区
      RxBuffer[ubRxIndex++] = USART_ReceiveData(USART1);
      USART_ClearITPendingBit(USART1,USART_IT_RXNE);
    }
}
