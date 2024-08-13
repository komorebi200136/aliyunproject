#include "DHT11.h"

//DHT11��ʪ�ȴ�������ʼ��  
void DHT11_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    //��GPIO����ʱ��  
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
    
    //����GPIO���ŵĲ��������г�ʼ��  ��Ҫע�⣺����ģʽ���ø���ģʽ
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_OUT;        //���ģʽ
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType   = GPIO_OType_PP;        //�������
    GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_9;
    GPIO_Init(GPIOD, &GPIO_InitStructure);
    
    //���ſ���״̬����ߵ�ƽ
    GPIO_SetBits(GPIOD,GPIO_Pin_9);
}


void DHT11_SetOutputMode()
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    //��GPIO����ʱ��  PG9
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
    
    //����GPIO���ŵĲ��������г�ʼ��  ��Ҫע�⣺����ģʽ���ø���ģʽ
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_OUT;        //���ģʽ
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType   = GPIO_OType_PP;        //�������
    GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_9;
    GPIO_Init(GPIOD, &GPIO_InitStructure);
    
     //���ſ���״̬����ߵ�ƽ
    GPIO_SetBits(GPIOD,GPIO_Pin_9);
}

void DHT11_SetInputMode(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    //��GPIO����ʱ��
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
    
    //����GPIO���ŵĲ��������г�ʼ��  ��Ҫע�⣺����ģʽ���ø���ģʽ
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_IN;        //����ģʽ
    GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_9;
    GPIO_Init(GPIOD, &GPIO_InitStructure);
}

//DHT11���Ϳ�ʼ�ź�
void DHT11_SendStart(void)
{
    //��������Ϊ���ģʽ
    DHT11_SetOutputMode();
    
    //�����ŵ�ƽ���Ͳ���������18ms ����20ms
    GPIO_ResetBits(GPIOD,GPIO_Pin_9);
    Delay_ms(20);
    
    //�����ŵ�ƽ���߲�����20~40us  ����30us
    GPIO_SetBits(GPIOD,GPIO_Pin_9);
    Delay_us(30);
}

//�ж�DHT11�Ƿ���Ӧ  ����ֵ false ��ʾδ��Ӧ  true ��ʾ����Ӧ
bool DHT11_IsAck(void) 
{
    uint8_t cnt = 0; //��Ϊ������
    
    //����PG9����Ϊ����ģʽ
    DHT11_SetInputMode();
    
    //�ȴ����ű�����  Ϊ�˱��⴫�������쳣�ģ����Խ�����ӳ�ʱ���� ���糬ʱʱ������Ϊ100ms
    while( GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_9) == SET && cnt < 100)
    {
        Delay_us(1);
        cnt++;
    }
    
    if(cnt >= 100)
        return false; //˵��δ��Ӧ ԭ���ǳ�ʱ
    
    cnt = 0;
    
    //�ȴ����ű�����  Ϊ�˱��⴫�������쳣�ģ����Խ�����ӳ�ʱ���� ���糬ʱʱ������Ϊ100ms
    while( GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_9) == RESET && cnt < 100)
    {
        Delay_us(1);
        cnt++;
    }
    
     if(cnt >= 100)
        return false; //˵��δ��Ӧ ԭ���ǳ�ʱ
     else
        return true;  //˵������Ӧ
}    

//DHT11��ȡ1bit����  ���� 0  ˵������0  0x00 0000 0000  ����1 ˵������1 0x01 0000 0001  ���յ���bitλ�洢���ֽڵ����λ
uint8_t DHT11_ReadBit()
{
    uint8_t cnt = 0; //��Ϊ������
    //�ȴ����ű�DHT11����������
    while( GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_9) == SET && cnt < 100)
    {
         Delay_us(1);
         cnt++;
    }
    cnt = 0;
    //�ȴ����ű�DHT11����������
     while( GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_9) == RESET && cnt < 100)
    {
         Delay_us(1);
         cnt++;
    }
    //��������0������1�ĸߵ�ƽ����ʱ���������� ����0 26~28us   ����1 70us
    Delay_us(40);
    
    //��ȡPG9���ŵ�ƽ��״̬ ������
    if(GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_9) == SET)
        return 1;
    else
        return 0;
}
   
//��ȡһ���ֽ�
uint8_t DHT11_ReadByte(void)  
{
    uint8_t i=0;
    uint8_t data = 0; //Ϊ�˴洢��ȡ���ֽ�  0000 0000    
    
    for(i=0;i<8;i++)
    {
        data <<= 1; // ������    
        data |= DHT11_ReadBit(); 
    }
    
    return data;
}

//��ȡDHT11��ʪ�����ݲ��洢������Ļ�������  ����0 ��ʾ�ɹ�  ����1 ��ʾʧ��
bool DHT11_ReadData(volatile uint8_t *dhtbuf)
{
    uint8_t i = 0;
    
    //���Ϳ�ʼ�ź�
    DHT11_SendStart();
    
    //�ж�DHT11�Ƿ���Ӧ
    if(DHT11_IsAck() == true)
    {
       //ѭ������5���ֽ�
       for(i=0;i<5;i++)
       {
           dhtbuf[i] = DHT11_ReadByte();
       } 

       //����У��  ǰ4���ֽ���ӽ����ĩ8λ�͵�5���ֽڱȽ�
       if(dhtbuf[0]+dhtbuf[1]+dhtbuf[2]+dhtbuf[3] != dhtbuf[4])
       {
            return false; //У��ʧ��
       }
           
    }
    else
    {
        return false;    //û����Ӧ
    }
    
    return true;
}
