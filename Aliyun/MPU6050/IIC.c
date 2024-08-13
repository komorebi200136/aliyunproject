#include "stm32f4xx.h"
#include "sys.h"
#include "IIC.h"
#include "Delay.h"


/*引脚初始化*/
void MPU_Pin_Init(void)
{
    GPIO_InitTypeDef      GPIO_InitStructure;
    
    RCC_AHB1PeriphClockCmd(MPU_SCL_CLOCK, ENABLE);           //打开SCL的时钟
    RCC_AHB1PeriphClockCmd(MPU_SDA_CLOCK, ENABLE);           //打开SDA的时钟
    
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_OUT;         //输出模式
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType   = GPIO_OType_PP;         //推挽输出
    GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Pin     = MPU_SCL_PIN;
    GPIO_Init(MPU_SCL_PORT, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_OUT;         //输出模式
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType   = GPIO_OType_PP;         //推挽输出
    GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Pin     = MPU_SDA_PIN;
    GPIO_Init(MPU_SDA_PORT, &GPIO_InitStructure);

	MPU_SCL(1);
	MPU_SDA(1);
}

//设置SDA引脚为输出模式
void MPU_SetSDAOutput(void)
{
    GPIO_InitTypeDef      GPIO_InitStructure;
    
    RCC_AHB1PeriphClockCmd(MPU_SDA_CLOCK, ENABLE);
    
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_OUT;        //输出模式
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType   = GPIO_OType_PP;        //推挽输出
    GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Pin     = MPU_SDA_PIN;
    GPIO_Init(MPU_SDA_PORT, &GPIO_InitStructure);
}

//设置SDA引脚为输入模式
void MPU_SetSDAInput(void)
{
    GPIO_InitTypeDef      GPIO_InitStructure;
    
    RCC_AHB1PeriphClockCmd(MPU_SDA_CLOCK, ENABLE);
    
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_IN;        //输入模式
    GPIO_InitStructure.GPIO_Pin     = MPU_SDA_PIN;
    GPIO_Init(MPU_SDA_PORT, &GPIO_InitStructure);
}


//开始信号
void MPU_Start(void)
{
    //1.设置SDA引脚为输出模式
    MPU_SetSDAOutput();
    
    //2.确保SDA引脚和SCL引脚设置为空闲状态
    MPU_SCL(1);
    MPU_SDA(1);
    Delay_us(5);
    
    //2.在SCL引脚处于高电平期间把SDA引脚电平拉低并保持稳定
    MPU_SDA(0);
    Delay_us(5);
    
    //3.SDA引脚拉低并保持稳定后，把SCL引脚电平拉低并保持稳定
    MPU_SCL(0);
    Delay_us(5);
}


//数据发送  MSB 高位先出
void MPU_SendByte(uint8_t Data)
{
    uint8_t i = 0;
    
    //1.设置SDA引脚为输出模式
    MPU_SetSDAOutput();
    
    //2.把SCL时钟线拉低，因为在SCL引脚低电平期间能修改数据
    MPU_SCL(0);
    Delay_us(5);
    
    //3.循环发送字节，遵循MSB高位先出
    for(i=0;i<8;i++)
    {
        //3.由于是MSB，所以需要先计算出数据Data的最高位是0还是1
        if(Data & 0x80)
             MPU_SDA(1);
        else
             MPU_SDA(0);
        
        Data <<= 1;
        Delay_us(5);
        
        //4.把SCL时钟线拉高，因为在SCL引脚高电平期间才允许读写
        MPU_SCL(1);
        Delay_us(5);
       
        //5.把SCL时钟线拉低，因为在SCL引脚低电平期间能修改数据
        MPU_SCL(0);
        Delay_us(5);
    }
}

//判断从机是否应答 该函数必须在发送一个字节之后进行调用  ack = 0 表示从机已响应  ack = 1 表示从机未响应
uint8_t MPU_IsSlaveACK(void)
{
    uint8_t ack = 0;
    
    //1.设置SDA数据线为输入模式
    MPU_SetSDAInput();
    
    //2.把SCL时钟线拉低，因为从机在SCL引脚低电平期间能修改数据
    MPU_SCL(0);
    Delay_us(5);
    
    //3.把SCL时钟线拉高，因为主机在SCL引脚高电平期间才允许读写
    MPU_SCL(1);
    Delay_us(5);
    
    //4.主机读取SDA数据线的引脚电平状态，就可以知道从机是否应答
    if(MPU_SDA_READ)
        ack = 1; 
    else
        ack = 0;
    
    //5.把SCL时钟线拉低，因为在SCL引脚低电平期间能修改数据
    MPU_SCL(0);
    Delay_us(5);
    
    return ack;
}

//主机读取从机的字节数据
uint8_t MPU_ReadByte(void)
{
    uint8_t i = 0;
    uint8_t data = 0; //接收读取的字节
    
    //1.设置SDA数据线为输入模式
    MPU_SetSDAInput();
    
    //2.循环接收从机发送的数据字节，注意从机遵循MSB高位先出
    for(i=0;i<8;i++)
    {
        //3.把SCL时钟线拉低，因为从机在SCL引脚低电平期间能修改数据
        MPU_SCL(0);
        Delay_us(5);
        
        //4.把SCL时钟线拉高，因为主机在SCL引脚高电平期间才允许读写
        MPU_SCL(1);
        Delay_us(5);
        
        //5.主机读取SDA数据线的电平状态并存储在变量data中
        data <<= 1;
        data |= MPU_SDA_READ;
    }
    
    //6.把SCL时钟线拉低，因为SCL引脚低电平期间能修改数据
    MPU_SCL(0);
    Delay_us(5);
    
    return data;
}

//主机发送应答信号，该函数应该在主机读取一个字节之后进行调用 
void MPU_SendACK(uint8_t ack)
{
    //1.设置SDA数据线为输出模式
    MPU_SetSDAOutput();
    
    //2.把SCL时钟线拉低，因为主机在SCL引脚低电平期间能修改数据
    MPU_SCL(0);
    Delay_us(5);
    
    //3.主机把应答信号在SCL时钟线低电平期间设置在SDA数据线上面
    MPU_SDA(ack);
    Delay_us(5);
    
    //4.把SCL时钟线拉高，因为从机在SCL引脚高电平期间才允许读写
    MPU_SCL(1);
    Delay_us(5);
    
    //5.把SCL时钟线拉低，因为在SCL引脚低电平期间能修改数据
    MPU_SCL(0);
    Delay_us(5);
}

//主机发送结束信号
void MPU_Stop(void)
{
    //1.设置SDA数据线为输出模式
    MPU_SetSDAOutput();
    
    //2.把SCL时钟线和SDA数据线都拉低，并保持稳定
    MPU_SCL(0);
    MPU_SDA(0);
    Delay_us(5);
    
    //3.把SCL时钟线拉高并保持稳定
    MPU_SCL(1);
    Delay_us(5);
    
    //4.把SDA数据线拉高并保持稳定
    MPU_SDA(1);
    Delay_us(5);
}
