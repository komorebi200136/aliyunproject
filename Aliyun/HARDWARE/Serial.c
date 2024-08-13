#include "Serial.h"

volatile uint8_t RxBuffer [50];        //���ջ�����
volatile uint8_t ubRxIndex = 0x00;     //���ջ��������±�

void Serial_Init(uint32_t baud)
{
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;        //����ģʽ
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;  //�ٶ�
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;      //����ģʽ
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;         //����ģʽ
    GPIO_InitStructure.GPIO_Pin = USART1_TX | USART1_RX;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);


    USART_InitStructure.USART_BaudRate = baud;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b; //��λ����λ
    USART_InitStructure.USART_StopBits = USART_StopBits_1;      //1λֹͣλ
    USART_InitStructure.USART_Parity = USART_Parity_No;         //��У��
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; //��Ӳ������
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1, &USART_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);   //���������ж�
    
    USART_Cmd(USART1, ENABLE);
    
}

//����һ���ֽڵ�����
void Serial_SendByte(uint8_t Byte)
{
	USART_SendData(USART1, Byte);
	while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
}

//����һ���ַ���������
void Serial_SendString(char *String)
{
	while(*String != '\0')
    {
        Serial_SendByte(*String++);
    }
}

void USART1_SendStr(void)
{
    USART_SendData(USART1,RxBuffer[ubRxIndex-1]); //�ѽ��յ����ֽ�ͨ�����ڷ��ͳ�ȥ
    while(USART_GetFlagStatus(USART1, USART_FLAG_TXE)==RESET);
}

//���Զ�fputc���������ض����ض��򵽴��� ��Ϊprintf()���ӵ��øú���,����ʹ��ST΢�Ϳ�����׼C��
int fputc(int ch, FILE *f) 
{
    USART_SendData(USART1,ch); //����һ���ַ�
    while( USART_GetFlagStatus(USART1,USART_FLAG_TXE) == RESET ); //�ȴ��ַ��������
    return ch;
}

void USART1_IRQHandler(void)
{
    //�ж��ж��Ƿ񴥷�
    if (USART_GetITStatus(USART1, USART_IT_RXNE) == SET)
    {   
      //��USART1���յ������ݴ洢�ڽ��ջ�����
      RxBuffer[ubRxIndex++] = USART_ReceiveData(USART1);
      USART_ClearITPendingBit(USART1,USART_IT_RXNE);
    }
}
