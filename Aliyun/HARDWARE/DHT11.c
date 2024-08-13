#include "DHT11.h"

//DHT11温湿度传感器初始化  
void DHT11_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    //打开GPIO外设时钟  
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
    
    //配置GPIO引脚的参数并进行初始化  需要注意：引脚模式设置复用模式
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_OUT;        //输出模式
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType   = GPIO_OType_PP;        //推挽输出
    GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_9;
    GPIO_Init(GPIOD, &GPIO_InitStructure);
    
    //引脚空闲状态输出高电平
    GPIO_SetBits(GPIOD,GPIO_Pin_9);
}


void DHT11_SetOutputMode()
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    //打开GPIO外设时钟  PG9
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
    
    //配置GPIO引脚的参数并进行初始化  需要注意：引脚模式设置复用模式
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_OUT;        //输出模式
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType   = GPIO_OType_PP;        //推挽输出
    GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_9;
    GPIO_Init(GPIOD, &GPIO_InitStructure);
    
     //引脚空闲状态输出高电平
    GPIO_SetBits(GPIOD,GPIO_Pin_9);
}

void DHT11_SetInputMode(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    //打开GPIO外设时钟
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
    
    //配置GPIO引脚的参数并进行初始化  需要注意：引脚模式设置复用模式
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_IN;        //输入模式
    GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_9;
    GPIO_Init(GPIOD, &GPIO_InitStructure);
}

//DHT11发送开始信号
void DHT11_SendStart(void)
{
    //设置引脚为输出模式
    DHT11_SetOutputMode();
    
    //把引脚电平拉低并持续至少18ms 建议20ms
    GPIO_ResetBits(GPIOD,GPIO_Pin_9);
    Delay_ms(20);
    
    //把引脚电平拉高并持续20~40us  建议30us
    GPIO_SetBits(GPIOD,GPIO_Pin_9);
    Delay_us(30);
}

//判断DHT11是否响应  返回值 false 表示未响应  true 表示已响应
bool DHT11_IsAck(void) 
{
    uint8_t cnt = 0; //作为计数器
    
    //设置PG9引脚为输入模式
    DHT11_SetInputMode();
    
    //等待引脚被拉低  为了避免传感器是异常的，所以建议添加超时机制 比如超时时间设置为100ms
    while( GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_9) == SET && cnt < 100)
    {
        Delay_us(1);
        cnt++;
    }
    
    if(cnt >= 100)
        return false; //说明未响应 原因是超时
    
    cnt = 0;
    
    //等待引脚被拉高  为了避免传感器是异常的，所以建议添加超时机制 比如超时时间设置为100ms
    while( GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_9) == RESET && cnt < 100)
    {
        Delay_us(1);
        cnt++;
    }
    
     if(cnt >= 100)
        return false; //说明未响应 原因是超时
     else
        return true;  //说明已响应
}    

//DHT11读取1bit数据  返回 0  说明数字0  0x00 0000 0000  返回1 说明数字1 0x01 0000 0001  接收到的bit位存储在字节的最低位
uint8_t DHT11_ReadBit()
{
    uint8_t cnt = 0; //作为计数器
    //等待引脚被DHT11传感器拉低
    while( GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_9) == SET && cnt < 100)
    {
         Delay_us(1);
         cnt++;
    }
    cnt = 0;
    //等待引脚被DHT11传感器拉高
     while( GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_9) == RESET && cnt < 100)
    {
         Delay_us(1);
         cnt++;
    }
    //由于数字0和数字1的高电平持续时间区别明显 数字0 26~28us   数字1 70us
    Delay_us(40);
    
    //读取PG9引脚电平的状态 并返回
    if(GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_9) == SET)
        return 1;
    else
        return 0;
}
   
//读取一个字节
uint8_t DHT11_ReadByte(void)  
{
    uint8_t i=0;
    uint8_t data = 0; //为了存储读取的字节  0000 0000    
    
    for(i=0;i<8;i++)
    {
        data <<= 1; // 先左移    
        data |= DHT11_ReadBit(); 
    }
    
    return data;
}

//读取DHT11温湿度数据并存储到传入的缓冲区中  返回0 表示成功  返回1 表示失败
bool DHT11_ReadData(volatile uint8_t *dhtbuf)
{
    uint8_t i = 0;
    
    //发送开始信号
    DHT11_SendStart();
    
    //判断DHT11是否响应
    if(DHT11_IsAck() == true)
    {
       //循环接收5个字节
       for(i=0;i<5;i++)
       {
           dhtbuf[i] = DHT11_ReadByte();
       } 

       //进行校验  前4个字节相加结果的末8位和第5个字节比较
       if(dhtbuf[0]+dhtbuf[1]+dhtbuf[2]+dhtbuf[3] != dhtbuf[4])
       {
            return false; //校验失败
       }
           
    }
    else
    {
        return false;    //没有响应
    }
    
    return true;
}
