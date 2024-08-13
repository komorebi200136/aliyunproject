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

    //ʹ��GPIOBʱ��
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

    //ʹ��USART3ʱ��
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;        //����ģʽ
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;  //�ٶ�
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;      //����ģʽ
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;         //����ģʽ
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10|GPIO_Pin_11;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_USART3);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, GPIO_AF_USART3);


    USART_InitStructure.USART_BaudRate = baud;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b; //��λ����λ
    USART_InitStructure.USART_StopBits = USART_StopBits_1;      //1λֹͣλ
    USART_InitStructure.USART_Parity = USART_Parity_No;         //��У��
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; //��Ӳ������
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART3, &USART_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);   //���������ж�
    
    USART_Cmd(USART3, ENABLE);
    
}

//����һ���ֽڵ�����
void BlueTooth_SendByte(uint8_t Byte)
{
	USART_SendData(USART3, Byte);
	while(USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);
}

//����һ���ַ���������
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
	if(blueteeth_cnt == 0) 						//������ռ���Ϊ0 ��˵��û�д��ڽ��������У�����ֱ����������������
		return 0;	
	if(blueteeth_cnt == blueteeth_cntPre)		//�����һ�ε�ֵ�������ͬ����˵���������
	{
        blueteeth_recvcnt=blueteeth_cnt;
		blueteeth_cnt = 0;						//��0���ռ���	
		return 1;								//���ؽ�����ɱ�־
	}		
	blueteeth_cntPre = blueteeth_cnt;			//��Ϊ��ͬ
	return 0;								    //���ؽ���δ��ɱ�־

}

//�и���յ����ַ���
uint16_t slice(char *str) 
{
    char *ptr;
    int value;

    // ʹ��strtok������'='Ϊ�ָ����ָ��ַ������õ�"tem"��"16"
    ptr = strtok(str, "=");

    // ��������strtok�������õ�"16"
    ptr = strtok(NULL, "=");

    // ʹ��atoi�������ַ���ת��Ϊ����
    value = atoi(ptr);

    // ��ӡ��ȡ������
    return value;
}

void USART3_IRQHandler(void)
{
    //�ж��ж��Ƿ񴥷�
    if (USART_GetITStatus(USART3, USART_IT_RXNE) == SET)
    {  
	  if(blueteeth_cnt >= sizeof(blueteeth_buf))	blueteeth_cnt = 0; //��ֹ���ڱ�ˢ��     
	  blueteeth_buf[blueteeth_cnt++]=USART_ReceiveData(USART3);
      USART_ClearITPendingBit(USART3,USART_IT_RXNE); 
    }
}



