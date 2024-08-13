#include "MP3.h"

void MP3_Init(uint32_t baud)
{
    USART_InitTypeDef USART_InitStructure;
    GPIO_InitTypeDef  GPIO_InitStructure;

    //ʹ��GPIOBʱ��
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

    //ʹ��UART5ʱ��
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5, ENABLE);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;        //����ģʽ
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;  //�ٶ�
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;      //����ģʽ
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;         //����ģʽ
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_Init(GPIOD, &GPIO_InitStructure);
    
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource12, GPIO_AF_UART5);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource2, GPIO_AF_UART5);


    USART_InitStructure.USART_BaudRate = baud;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b; //��λ����λ
    USART_InitStructure.USART_StopBits = USART_StopBits_1;      //1λֹͣλ
    USART_InitStructure.USART_Parity = USART_Parity_No;         //��У��
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; //��Ӳ������
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(UART5, &USART_InitStructure);

    USART_Cmd(UART5, ENABLE);
    
}

//����һ���ֽڵ�����
void MP3_SendByte(uint8_t Byte)
{
	USART_SendData(UART5, Byte);
	while(USART_GetFlagStatus(UART5, USART_FLAG_TXE) == RESET);
}

void Start(void)
{
    int i=0;
    uint8_t data[4]={0x7E,0x02,0x01,0xEF};
    for(i=0;i<4;i++)
    {
        MP3_SendByte(data[i]);
    }
}

void Start_ZhiDing(uint8_t num)
{
    int i=0;
    uint8_t data[6]={0x7E,0x04,0x41,0x00,0x02,0xEF};
    data[4]=num;
    for(i=0;i<6;i++)
    {
        MP3_SendByte(data[i]);
    }
}

//�������� 7E 02 05 EF
void VolumeUp(void)
{
    int i=0;
    uint8_t data[4]={0x7E,0x02,0x05,0xEF};
    for(i=0;i<4;i++)
    {
        MP3_SendByte(data[i]);
    }
}
//�������� 7E 02 06 EF
void VolumeDown(void)
{
    int i=0;
    uint8_t data[4]={0x7E,0x02,0x06,0xEF};
    for(i=0;i<4;i++)
    {
        MP3_SendByte(data[i]);
    }
}

//����һ���ַ���������
void MP3_SendString(char *String)
{
	while(*String != '\0')
    {
        MP3_SendByte(*String++);
    }
}
