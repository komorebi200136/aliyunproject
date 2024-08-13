#include "stm32f4xx.h"
#include "sys.h"
#include "IIC.h"
#include "Delay.h"


/*���ų�ʼ��*/
void MPU_Pin_Init(void)
{
    GPIO_InitTypeDef      GPIO_InitStructure;
    
    RCC_AHB1PeriphClockCmd(MPU_SCL_CLOCK, ENABLE);           //��SCL��ʱ��
    RCC_AHB1PeriphClockCmd(MPU_SDA_CLOCK, ENABLE);           //��SDA��ʱ��
    
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_OUT;         //���ģʽ
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType   = GPIO_OType_PP;         //�������
    GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Pin     = MPU_SCL_PIN;
    GPIO_Init(MPU_SCL_PORT, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_OUT;         //���ģʽ
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType   = GPIO_OType_PP;         //�������
    GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Pin     = MPU_SDA_PIN;
    GPIO_Init(MPU_SDA_PORT, &GPIO_InitStructure);

	MPU_SCL(1);
	MPU_SDA(1);
}

//����SDA����Ϊ���ģʽ
void MPU_SetSDAOutput(void)
{
    GPIO_InitTypeDef      GPIO_InitStructure;
    
    RCC_AHB1PeriphClockCmd(MPU_SDA_CLOCK, ENABLE);
    
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_OUT;        //���ģʽ
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType   = GPIO_OType_PP;        //�������
    GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Pin     = MPU_SDA_PIN;
    GPIO_Init(MPU_SDA_PORT, &GPIO_InitStructure);
}

//����SDA����Ϊ����ģʽ
void MPU_SetSDAInput(void)
{
    GPIO_InitTypeDef      GPIO_InitStructure;
    
    RCC_AHB1PeriphClockCmd(MPU_SDA_CLOCK, ENABLE);
    
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_IN;        //����ģʽ
    GPIO_InitStructure.GPIO_Pin     = MPU_SDA_PIN;
    GPIO_Init(MPU_SDA_PORT, &GPIO_InitStructure);
}


//��ʼ�ź�
void MPU_Start(void)
{
    //1.����SDA����Ϊ���ģʽ
    MPU_SetSDAOutput();
    
    //2.ȷ��SDA���ź�SCL��������Ϊ����״̬
    MPU_SCL(1);
    MPU_SDA(1);
    Delay_us(5);
    
    //2.��SCL���Ŵ��ڸߵ�ƽ�ڼ��SDA���ŵ�ƽ���Ͳ������ȶ�
    MPU_SDA(0);
    Delay_us(5);
    
    //3.SDA�������Ͳ������ȶ��󣬰�SCL���ŵ�ƽ���Ͳ������ȶ�
    MPU_SCL(0);
    Delay_us(5);
}


//���ݷ���  MSB ��λ�ȳ�
void MPU_SendByte(uint8_t Data)
{
    uint8_t i = 0;
    
    //1.����SDA����Ϊ���ģʽ
    MPU_SetSDAOutput();
    
    //2.��SCLʱ�������ͣ���Ϊ��SCL���ŵ͵�ƽ�ڼ����޸�����
    MPU_SCL(0);
    Delay_us(5);
    
    //3.ѭ�������ֽڣ���ѭMSB��λ�ȳ�
    for(i=0;i<8;i++)
    {
        //3.������MSB��������Ҫ�ȼ��������Data�����λ��0����1
        if(Data & 0x80)
             MPU_SDA(1);
        else
             MPU_SDA(0);
        
        Data <<= 1;
        Delay_us(5);
        
        //4.��SCLʱ�������ߣ���Ϊ��SCL���Ÿߵ�ƽ�ڼ�������д
        MPU_SCL(1);
        Delay_us(5);
       
        //5.��SCLʱ�������ͣ���Ϊ��SCL���ŵ͵�ƽ�ڼ����޸�����
        MPU_SCL(0);
        Delay_us(5);
    }
}

//�жϴӻ��Ƿ�Ӧ�� �ú��������ڷ���һ���ֽ�֮����е���  ack = 0 ��ʾ�ӻ�����Ӧ  ack = 1 ��ʾ�ӻ�δ��Ӧ
uint8_t MPU_IsSlaveACK(void)
{
    uint8_t ack = 0;
    
    //1.����SDA������Ϊ����ģʽ
    MPU_SetSDAInput();
    
    //2.��SCLʱ�������ͣ���Ϊ�ӻ���SCL���ŵ͵�ƽ�ڼ����޸�����
    MPU_SCL(0);
    Delay_us(5);
    
    //3.��SCLʱ�������ߣ���Ϊ������SCL���Ÿߵ�ƽ�ڼ�������д
    MPU_SCL(1);
    Delay_us(5);
    
    //4.������ȡSDA�����ߵ����ŵ�ƽ״̬���Ϳ���֪���ӻ��Ƿ�Ӧ��
    if(MPU_SDA_READ)
        ack = 1; 
    else
        ack = 0;
    
    //5.��SCLʱ�������ͣ���Ϊ��SCL���ŵ͵�ƽ�ڼ����޸�����
    MPU_SCL(0);
    Delay_us(5);
    
    return ack;
}

//������ȡ�ӻ����ֽ�����
uint8_t MPU_ReadByte(void)
{
    uint8_t i = 0;
    uint8_t data = 0; //���ն�ȡ���ֽ�
    
    //1.����SDA������Ϊ����ģʽ
    MPU_SetSDAInput();
    
    //2.ѭ�����մӻ����͵������ֽڣ�ע��ӻ���ѭMSB��λ�ȳ�
    for(i=0;i<8;i++)
    {
        //3.��SCLʱ�������ͣ���Ϊ�ӻ���SCL���ŵ͵�ƽ�ڼ����޸�����
        MPU_SCL(0);
        Delay_us(5);
        
        //4.��SCLʱ�������ߣ���Ϊ������SCL���Ÿߵ�ƽ�ڼ�������д
        MPU_SCL(1);
        Delay_us(5);
        
        //5.������ȡSDA�����ߵĵ�ƽ״̬���洢�ڱ���data��
        data <<= 1;
        data |= MPU_SDA_READ;
    }
    
    //6.��SCLʱ�������ͣ���ΪSCL���ŵ͵�ƽ�ڼ����޸�����
    MPU_SCL(0);
    Delay_us(5);
    
    return data;
}

//��������Ӧ���źţ��ú���Ӧ����������ȡһ���ֽ�֮����е��� 
void MPU_SendACK(uint8_t ack)
{
    //1.����SDA������Ϊ���ģʽ
    MPU_SetSDAOutput();
    
    //2.��SCLʱ�������ͣ���Ϊ������SCL���ŵ͵�ƽ�ڼ����޸�����
    MPU_SCL(0);
    Delay_us(5);
    
    //3.������Ӧ���ź���SCLʱ���ߵ͵�ƽ�ڼ�������SDA����������
    MPU_SDA(ack);
    Delay_us(5);
    
    //4.��SCLʱ�������ߣ���Ϊ�ӻ���SCL���Ÿߵ�ƽ�ڼ�������д
    MPU_SCL(1);
    Delay_us(5);
    
    //5.��SCLʱ�������ͣ���Ϊ��SCL���ŵ͵�ƽ�ڼ����޸�����
    MPU_SCL(0);
    Delay_us(5);
}

//�������ͽ����ź�
void MPU_Stop(void)
{
    //1.����SDA������Ϊ���ģʽ
    MPU_SetSDAOutput();
    
    //2.��SCLʱ���ߺ�SDA�����߶����ͣ��������ȶ�
    MPU_SCL(0);
    MPU_SDA(0);
    Delay_us(5);
    
    //3.��SCLʱ�������߲������ȶ�
    MPU_SCL(1);
    Delay_us(5);
    
    //4.��SDA���������߲������ȶ�
    MPU_SDA(1);
    Delay_us(5);
}
