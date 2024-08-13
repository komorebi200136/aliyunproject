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

    //ʹ��GPIOCʱ��
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

    //ʹ��UART4ʱ��
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);

    //��������
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;        //����ģʽ
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;  //�ٶ�
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;      //����ģʽ
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;         //����ģʽ
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10  | GPIO_Pin_11;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource10, GPIO_AF_UART4);
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource11, GPIO_AF_UART4);


    USART_InitStructure.USART_BaudRate = baud;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b; //��λ����λ
    USART_InitStructure.USART_StopBits = USART_StopBits_1;      //1λֹͣλ
    USART_InitStructure.USART_Parity = USART_Parity_No;         //��У��
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; //��Ӳ������
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(UART4, &USART_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);   //���������ж�
    
    USART_Cmd(UART4, ENABLE);
    
}

//����һ���ֽڵ�����
void SIM_SendByte(uint8_t Byte)
{
	USART_SendData(UART4, Byte);
	while(USART_GetFlagStatus(UART4, USART_FLAG_TXE) == RESET);
}

//����һ���ַ���������
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

	if(SIM_Count == 0) 							//������ռ���Ϊ0 ��˵��û�д��ڽ��������У�����ֱ����������������
		return 0;
		
	if(SIM_Count == SIM_CountPre)				//�����һ�ε�ֵ�������ͬ����˵���������
	{
		SIM_Count = 0;							//��0���ռ���
			
		return 1;								//���ؽ�����ɱ�־
	}
		
	SIM_CountPre = SIM_Count;					//��Ϊ��ͬ
	
	return 0;								    //���ؽ���δ��ɱ�־

}

bool SIM900A_SendCmd(char *cmd, char *ret)
{
	int timeOut = 5000;                       //��ʱ����
    SIM_SendString(cmd);                      //����ATָ��
	while(timeOut--)
	{
		if(SIM900A_WaitRecive() == 1)		  //������ݽ�������
		{
			if(strstr(SIM_Buf, ret) != NULL)  //����������ؼ���
			{
				SIM900A_Clear();			  //��ջ���
				
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
	
	SIM900A_SendCmd("AT+CSCS=\"UCS2\"\r\n", "OK"); //��������
	/* 
    ��UCS2��
     16λͨ��8�ֽڱ��������ַ���(ISO/IEC10646);
	 UCS2 �ַ���ת��Ϊ�� 0000�� FFFF �� 16 ������;
	 ����:"004100620063'��ʾ���� 16 λ�ַ���ʮ���Ƶ�ֵ�ֱ��� 65.98 �� 99��
	*/
	Delay_ms(100);
	SIM900A_SendCmd("AT+CMGF=1\r\n", "OK"); //���ö���ϵͳ�����ı�ģʽ����֮��Ӧ���� PDU ģʽ��
	Delay_ms(100);
	SIM900A_SendCmd("AT+CSCA?\r\n", "OK");  //��ѯ����
	Delay_ms(100);
	SIM900A_SendCmd("AT+CSMP=17,167,0,25\r\n", "OK"); //���ͺ����ֵĶ���
	Delay_ms(100);
	SIM900A_SendCmd(cmdphone, ">");   //�绰Ҫ��unicon��
	Delay_ms(100);
	SIM_SendString(message);        //�������ݣ������Ƕ�Ӧ��unicon ���롣
	SIM_SendByte(0X1A);             //ȷ�Ϸ���  16����1A���ַ�1A������
    Delay_ms(5000);

}
void Send_EnglishMessages(char message[],char phone[])
{
    char cmdphone[100]={0};
    sprintf(cmdphone,"AT+CMGS=\"%s\"\r\n",phone);
	
    SIM900A_SendCmd("AT\r\n", "OK");       //5S��ʱ����
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
    SIM_SendString(message);        //���Ͷ�������
    SIM_SendByte(0X1A);             //ȷ�Ϸ���  16����1A���ַ�1A������
    Delay_ms(5000);

}
void UART4_IRQHandler(void)
{
    //�ж��ж��Ƿ񴥷�
    if (USART_GetITStatus(UART4, USART_IT_RXNE) == SET)
    {   
      if(SIM_Count >= sizeof(SIM_Buf))	SIM_Count = 0; //��ֹ���ڱ�ˢ��    
	  SIM_Buf[SIM_Count++]=USART_ReceiveData(UART4);  
      USART_ClearITPendingBit(UART4,USART_IT_RXNE);
    }

}
