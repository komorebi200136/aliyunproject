#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/*  

LCD:PG12 PE3 PE5 PC0 PE1 PB7

OLED:PD11(SDA) PD13(SCL)

调试串口：PA9(USART1_TX)、PA10(USART1_RX)
WIFI模块：PA3(USART2_RX)、PA2(USART2_TX)、RST(PC4)
蓝牙模块：PB10(USART3_TX)、PB11(USART3_RX)
GPRS模块：PC10(UART4 TX)、PC11(UART4 RX)
MP3模块： PC12（UART5 TX、PD2(UART5 RX)
备用串口：PG14（USART6 TX），PG9（USART6 RX）

ADC模块
MQ2:AO PF5 DO PF7
MQ5:AO PF4 DO PF3
火焰传感器:AO PF8 DO PF6

人体红外传感器:PA5


MPU6050:PB1(SCL)、PC5(SDA)
蜂鸣器:PB15

DHT11:PD9

LED1:PB4
LED2:PE6

KEY1:PD3
KEY2:PC8

*/

extern volatile bool avalue;          //警报数据
extern volatile bool bvalue;          //卧室灯数据
extern volatile bool avalue_key;      //按键警报数据
extern volatile bool bvalue_key;      //按键卧室灯数据

extern char blueteeth_buf[512];       //蓝牙接受的数据
extern char blueteeth_recvcnt;
extern volatile uint8_t TimeDisplay;  //表示时间唤醒标志

volatile uint8_t DHT11Data[5]={0};    //存储温湿度数据，DHT11Data[2]是温度，DHT11Data[0]是湿度
volatile uint8_t temp,hum;            //存储温湿度数据
volatile uint16_t mq2,mq5;            //储存MQ2和MQ5的ADC转换值，烟雾和可燃气体
volatile bool people;                 //是否有人
volatile bool fire;                   //是否有火
float pitch,roll,yaw; 	              //欧拉角

volatile uint16_t tvalue_tmp,tvalue_hum,tvalue_mq2,tvalue_mq5,tvalue_tangle;  //各类数据的阈值
	


SemaphoreHandle_t LCDMutex;       //用于LCD的互斥锁
SemaphoreHandle_t OLEDMutex;      //用于OLED的互斥锁
SemaphoreHandle_t PubMutex;       //用于订阅消息的互斥锁
SemaphoreHandle_t  EspNum;        // 用于协调联网的二值型信号量
SemaphoreHandle_t SemaphoreNum;   //信号量
SemaphoreHandle_t  g_sem_tmp;     // 二值型信号量

TaskHandle_t EspHandle = NULL; 
TaskHandle_t BedlampControlHandle = NULL; 
TaskHandle_t AlarmControlHandle = NULL; 
TaskHandle_t ScreenDisplayHandle = NULL; 
TaskHandle_t DHT11Handle = NULL; 
TaskHandle_t MQADCHandle = NULL; 
TaskHandle_t FIREHandle = NULL; 
TaskHandle_t BODYREATIONHandle = NULL; 
TaskHandle_t MPU6050Handle = NULL; 
TaskHandle_t BlueTeethHandle = NULL; 
TaskHandle_t SIM900AHandle = NULL;
TaskHandle_t MP3Handle = NULL;

void Esp_DataMana(void *pvParameters);   //阿里云数据控制
void BedlampControl( void *pvParameters);//卧室灯控制
void AlarmControl(void *pvParameters);   //警报控制
void ScreenDisplay(void *pvParameters);  //屏幕显示
void DHT11(void *pvParameters);          //温湿度获取
void MQADC(void *pvParameters);          //烟雾和可燃气体
void Fire(void *pvParameters);           //火焰传感器
void BodyReaction(void *pvParameters);   //人体感应器
void MPU6050(void *pvParameters);        //实现安防系统防盗
void BlueTeeth(void *pvParameters);      //蓝牙模块
void SIM900A(void *pvParameters);        //发短信模块
void MP3(void *pvParameters);            //语音播放模块

int main()
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);  //中断优先级分组
    Serial_Init(115200);        //调试串口初始化
    ESP8266_Init(115200);       //WIFI模块初始化
	BlueTooth_Init(9600);       //蓝牙初始化
	SIM_Init(9600);             //GPRS模块初始化
	MP3_Init(9600);             //语音播放模块初始化
	MQ_Init();                  //烟雾和可燃气体
	Key_Init();                 //按键初始化
	RTC_Config();               //RTC初始化
    LED_Init();                 //LED初始化
	HCSR312_Init();             //人体感应传感器
	Fire_Init();                //火焰感应器
	Beep_Init();                //蜂鸣器初始化
	DHT11_Init();               //温湿度初始化
    Delay_Init(168);
	
    LCDMutex=xSemaphoreCreateMutex();           //创建互斥锁
	OLEDMutex=xSemaphoreCreateMutex();          //创建互斥锁
	PubMutex=xSemaphoreCreateMutex();           //创建互斥锁
	EspNum=xSemaphoreCreateBinary();            //二值信号量
	SemaphoreNum=xSemaphoreCreateCounting(2,0); //最大值为2,初始值为0，
	//发短信任务           
    xTaskCreate(   SIM900A,   
                   "sim900atask",     
                   256,            
                   NULL,           
                   5,               
                   &SIM900AHandle     
           );  
    //ESP任务           
    xTaskCreate(   Esp_DataMana,   
                   "esptask",     
                   256,            
                   NULL,           
                   4,               
                   &EspHandle     
           );  
	//警报任务           
    xTaskCreate(   AlarmControl,   
                   "alarmtask",     
                   256,            
                   NULL,           
                   4,               
                   &AlarmControlHandle  
           );  
	//卧室灯任务           
    xTaskCreate(   BedlampControl,   
                   "bedlamptask",     
                   256,            
                   NULL,           
                   4,               
                   &BedlampControlHandle  
           );
	//蓝牙设置任务           
    xTaskCreate(   BlueTeeth,   
                   "blueteethtask",     
                   256,            
                   NULL,           
                   4,               
                   &BlueTeethHandle     
           );  
   
	//屏幕显示任务           
    xTaskCreate(   ScreenDisplay,   
                   "screendisplaytask",     
                   256,            
                   NULL,           
                   3,               
                   &ScreenDisplayHandle  
           );

    //温湿度获取任务           
    xTaskCreate(   DHT11,   
                   "dht11task",     
                   256,            
                   NULL,           
                   3,               
                   &DHT11Handle      //优先级至少是3，不然无法获取温湿度
           );
	//MQ2和MQ5ADC值的获取任务           
    xTaskCreate(   MQADC,   
                   "mqadctask",     
                   256,            
                   NULL,           
                   2,               
                   &MQADCHandle  
           );
	//火焰传感器任务           
    xTaskCreate(   Fire,   
                   "firetask",     
                   256,            
                   NULL,           
                   2,               
                   &FIREHandle
           );
    //人体感应任务           
    xTaskCreate(   BodyReaction,   
                   "bodyreactiontask",     
                   256,            
                   NULL,           
                   2,               
                   &BODYREATIONHandle
           );
	//实现安防系统防盗任务           
    xTaskCreate(   MPU6050,   
                   "mpu6050task",     
                   256,            
                   NULL,           
                   2,               
                   &MPU6050Handle
           );
	//实现语音播放任务           
    xTaskCreate(   MP3,   
                   "mp3task",     
                   256,            
                   NULL,           
                   2,               
                   &MP3Handle
           );
    //启动调度器  调度器一旦启动，就进入无限循环
    vTaskStartScheduler();
}


//阿里云数据上传和订阅处理
void Esp_DataMana(void *pvParameters)
{  
	int i=0;
	uint8_t temp=0; 
	uint8_t hum=0;
	uint16_t tmq2,tmq5;
	bool tpeople;     
	bool tfire;       
    ESP8266_Network();  //连接阿里云
    while(1)
    {
		SubTopic();  //订阅数据
		if(i>500)    //5秒上传一次数据
		{		
			i=0;
			temp=DHT11Data[2];
			hum=DHT11Data[0];
			tmq2=mq2;
			tmq5=mq5;
			tpeople=people;
			tfire=fire;
			PubTopicToMP(temp,hum,tmq2 ,tmq5,tpeople,tfire);
		}
		i++;
		vTaskDelay(10);
    }
}


//订阅数据,卧室灯
void BedlampControl(void *pvParameters)
{  
    while(1)
    {
		if(bvalue || bvalue_key)   //按键或者远程控制
			WLED_On();
		else
			WLED_Off();
		vTaskDelay(100);
    }
}
//警报灯，蜂鸣器
void AlarmControl(void *pvParameters)
{  
    while(1)
    {
		if(avalue || avalue_key) //按键或者远程控制
	    {
			GPIO_ToggleBits(GPIOE, GPIO_Pin_6);
			GPIO_ToggleBits(GPIOB, GPIO_Pin_15);
		}
		else
		{
			RLED_Off();   //关闭警报灯
			Beep_Off();   //关闭警报
		}
		vTaskDelay(100);
    }
}

//蓝牙设置，接收指定格式的数据tem=15;hum=34;mq2=1200;mq5=1600;tangle=50;设置MP3的声音大小
void BlueTeeth(void *pvParameters)
{  
	while(1)
	{
		if(Blueteeth_WaitRecive())  //如果收到数据
		{
			if(strstr(blueteeth_buf,"time"))
			{
				printf("%s\r\n",blueteeth_buf);
				SetTimeDate(blueteeth_buf,blueteeth_recvcnt);  //time=15:24:23
				Blueteeth_Clear();
			}
			else if(strstr(blueteeth_buf,"volume+"))    //加大声音
			{
				printf("%s\r\n",blueteeth_buf);
				VolumeUp();
				Blueteeth_Clear();
			}
			else if(strstr(blueteeth_buf,"volume-"))   //减小声音
			{
				printf("%s\r\n",blueteeth_buf);
				VolumeDown();
				Blueteeth_Clear();
			}
			else if(strstr(blueteeth_buf,"tmp"))       //设置温度阈值
		    {
				printf("%s\r\n",blueteeth_buf);
				tvalue_tmp = slice(blueteeth_buf);
				Blueteeth_Clear();
			}
			else if(strstr(blueteeth_buf,"hum"))       //设置湿度阈值
		    {
				printf("%s\r\n",blueteeth_buf);
				tvalue_hum = slice(blueteeth_buf);
				Blueteeth_Clear();
			}
			else if(strstr(blueteeth_buf,"mq2"))       //设置烟雾阈值
		    {
				printf("%s\r\n",blueteeth_buf);
				tvalue_mq2 = slice(blueteeth_buf);
				Blueteeth_Clear();
			}
			else if(strstr(blueteeth_buf,"mq5"))       //设置可燃气体等阈值
		    {
				printf("%s\r\n",blueteeth_buf);
				tvalue_mq5 = slice(blueteeth_buf);
				Blueteeth_Clear();
			}
			else if(strstr(blueteeth_buf,"tangle"))    //设置角度阈值
		    {
				printf("%s\r\n",blueteeth_buf);
				tvalue_tangle = slice(blueteeth_buf);
				Blueteeth_Clear();
			}
		}
		vTaskDelay(100);
	}
}

//GPRS模块设置,发送中文短信
void SIM900A(void *pvParameters)
{  
	//在30秒内只能发送一次短信，防止短信重复发送
	uint8_t i=0,a=0,b=0,c=0,d=0,e=0,f=0,g=0;
	tvalue_tmp=30; tvalue_hum=80;tvalue_mq2=1000; tvalue_mq5=1000;tvalue_tangle=50;
	vTaskDelay(20000);
	while(1)
	{
		if(temp>tvalue_tmp && a==0)
		{
			a=1;
			Send_ChineseMessages("5BA451856E295EA68FC79AD8","00310038003400370036003900370039003400320038");  
			//室内温度过高
		}
		if(hum>tvalue_hum && b==0)
		{
			b=1;
			Send_ChineseMessages("5BA451856E7F5EA68FC79AD8","00310038003400370036003900370039003400320038");  
			//室内湿度过高
		}
		if(mq2>tvalue_mq2 && c==0)
		{
			c=1;
			Send_ChineseMessages("5BA4518570DF96FE6D535EA68FC79AD8","00310038003400370036003900370039003400320038");  
			//室内烟雾浓度过高
		}
		if(mq5>tvalue_mq5 && d==0)
		{
			d=1;
			Send_ChineseMessages("5BA4518570DF96FE6D535EA68FC79AD8","00310038003400370036003900370039003400320038");  
			//室内可燃气体浓度过高
		}
		if(fire==1 && e==0)
		{
			e=1;
			Send_ChineseMessages("5BA4518553D173B0706B7130","00310038003400370036003900370039003400320038");  
			//室内发现火焰
		}
		if(people==1 && f==0)
		{
			f=1;
			Send_ChineseMessages("67094EBA54588FDB51655BA45185","00310038003400370036003900370039003400320038"); 
			//有人员进入室内
		}
		if((abs((int)pitch)>tvalue_tangle || abs((int)roll)>tvalue_tangle || abs((int)yaw)>tvalue_tangle) && g==0)
		{
			g=1;
			Send_ChineseMessages("667A80FD5B8996327EC87AEF4F4D7F6E653953D8","00310038003400370036003900370039003400320038"); 
			//智能安防终端位置改变
		}
		vTaskDelay(1000);
		i++;
		if(i>30)
		{
			i=0;
			a=0;
			b=0;
			c=0;
			d=0;
			e=0;
			f=0;
			g=0;
		}
	}
}

//LCD和OLED屏幕显示
void ScreenDisplay(void *pvParameters)
{  
	RTC_TimeTypeDef   RTC_TimeStructure;
	OLED_Init();			            //初始化OLED 
	OLED_ShowString(1,1,"time:");
	OLED_ShowChar(1, 8, ':');
	OLED_ShowChar(1, 11, ':');
	xSemaphoreTake(LCDMutex,portMAX_DELAY);
	LCD_Init();                         //LCD初始化
    LCD_Fill(0,0,LCD_W,LCD_H,WHITE);
	LCD_ShowString(30,0,(u8*)"aliyunsystem",RED,WHITE,32,0);
	LCD_ShowString(0,30,(u8*)"tem:",BLUE,WHITE,32,0);     //温度
	LCD_ShowString(0,70,(u8*)"hum:",BLUE,WHITE,32,0);     //湿度
	LCD_ShowString(110,30,(u8*)"C",BLUE,WHITE,32,0);      //温度单位
	LCD_ShowString(110,70,(u8*)"%RH",BLUE,WHITE,32,0);    //湿度单位
	LCD_ShowString(0,110,(u8*)"mq2:",BLUE,WHITE,32,0);    //烟雾浓度
	LCD_ShowString(0,150,(u8*)"mq5:",BLUE,WHITE,32,0);    //可燃气体浓度
	LCD_ShowString(0,190,(u8*)"fire:",BLUE,WHITE,32,0);   //火焰
	LCD_ShowString(0,230,(u8*)"body:",BLUE,WHITE,32,0);   //人体感应
	LCD_ShowString(0,270,(u8*)"place:",BLUE,WHITE,32,0);  //位置防盗设计
	xSemaphoreGive(LCDMutex);
	while(1)
	{
		//在OLED显示时间
		if(TimeDisplay == 1)
        {
            TimeDisplay = 0;
            //获取当前时间
            RTC_GetTime(RTC_Format_BIN, &RTC_TimeStructure);
			xSemaphoreTake(OLEDMutex,portMAX_DELAY);
			OLED_ShowNum(1, 6, RTC_TimeStructure.RTC_Hours, 2);
			OLED_ShowNum(1, 9, RTC_TimeStructure.RTC_Minutes, 2);
			OLED_ShowNum(1, 12, RTC_TimeStructure.RTC_Seconds, 2);
			xSemaphoreGive(OLEDMutex);	
        }
		vTaskDelay(1000);
	}
}

//DHT11温度获取 
void DHT11(void *pvParameters)
{  
	while(1)
	{
		if(DHT11_ReadData(DHT11Data))
		{
			temp=DHT11Data[2];
			hum=DHT11Data[0];
			xSemaphoreTake(LCDMutex,portMAX_DELAY);
			LCD_ShowIntNum(75,30,DHT11Data[2],2,GREEN,WHITE,32);//显示整数变量
			LCD_ShowIntNum(75,70,DHT11Data[0],2,GREEN,WHITE,32);//显示整数变量
			xSemaphoreGive(LCDMutex);
		}
		vTaskDelay(2000);   //延时2秒
	} 
}

//MQ2烟雾和MQ5可燃气体
void MQADC(void *pvParameters)
{  
	while(1)
	{
		mq2=AD_GetValue(ADC_Channel_15);            //PF5 MQ2 PF4 MQ5
        mq5=AD_GetValue(ADC_Channel_14);
		xSemaphoreTake(LCDMutex,portMAX_DELAY);
		LCD_ShowIntNum(60,110,mq2,4,GREEN,WHITE,32);//显示整数变量
		LCD_ShowIntNum(60,150,mq5,4,GREEN,WHITE,32);//显示整数变量
		xSemaphoreGive(LCDMutex);
		vTaskDelay(1000);
	}
}

//火焰传感器
void Fire(void *pvParameters)
{  
	while(1)
	{
		if(IsFireSensing()==0)
		{
			xSemaphoreTake(LCDMutex,portMAX_DELAY);
			fire=1;
			LCD_ShowString(80,190,(u8*)"have",GREEN,WHITE,32,0);  //最后的参数0表示覆盖字符，1表示不覆盖
			xSemaphoreGive(LCDMutex);
		}
		else
		{
			
			xSemaphoreTake(LCDMutex,portMAX_DELAY);
			fire=0;
			LCD_ShowString(80,190,(u8*)"no  ",GREEN,WHITE,32,0);
			xSemaphoreGive(LCDMutex);
		}
		vTaskDelay(10);
	}
}

//人体感应
void BodyReaction(void *pvParameters)
{  
    while(1)
	{
		if(IsBodySensing())
		{
			
			xSemaphoreTake(LCDMutex,portMAX_DELAY);
			people=1;
			LCD_ShowString(80,230,(u8*)"have",GREEN,WHITE,32,0);
			xSemaphoreGive(LCDMutex);
		}
		else
		{
			
			xSemaphoreTake(LCDMutex,portMAX_DELAY);
			people=0;
			LCD_ShowString(80,230,(u8*)"no  ",GREEN,WHITE,32,0);
			xSemaphoreGive(LCDMutex);
		}
		vTaskDelay(10);
	}
}

//俯仰角
/*
PB5 PB6不能用
//roll:横滚角.单位0.01度。 -18000 -> 18000 对应 -180.00  ->  180.00度
//pitch:俯仰角.单位 0.01度。-9000 - 9000 对应 -90.00 -> 90.00 度
//yaw:航向角.单位为0.1度 0 -> 3600  对应 0 -> 360.0度
*/
void MPU6050(void *pvParameters)
{ 
	uint8_t res=0;	
	MPU_Init(); //初始化MPU6050	
	while(mpu_dmp_init())
	{
		printf("MPU6050 ERROR \r\n");
		Delay_ms(500);
	}
	xSemaphoreTake(OLEDMutex,portMAX_DELAY);
	OLED_ShowString(2, 1, "pitch:");    //显示静态字符串
	OLED_ShowString(3, 1, "roll:");		//显示静态字符串
    OLED_ShowString(4, 1, "yaw:");		//显示静态字符串
	xSemaphoreGive(OLEDMutex);   
	while(1)
	{
		res=mpu_dmp_get_data(&pitch,&roll,&yaw);
		if(res==0)
		{ 
			if(abs((int)pitch)>30 || abs((int)roll)>30 || abs((int)yaw)>30)
			{
				xSemaphoreTake(LCDMutex,portMAX_DELAY);
				LCD_ShowString(100,270,(u8*)"change  ",GREEN,WHITE,32,0);   //位置防盗设计
				xSemaphoreGive(LCDMutex);	
			}
			else
			{
				xSemaphoreTake(LCDMutex,portMAX_DELAY);
				LCD_ShowString(100,270,(u8*)"nochange",GREEN,WHITE,32,0);   //位置防盗设计
				xSemaphoreGive(LCDMutex);
			}
			xSemaphoreTake(OLEDMutex,portMAX_DELAY);
			OLED_ShowSignedNum(2, 7, pitch, 5);
            OLED_ShowSignedNum(3, 7, roll, 5);
            OLED_ShowSignedNum(4, 7, yaw, 5);
			xSemaphoreGive(OLEDMutex);   
			Delay_ms(200);
        }
	}
}

//MP3模块设置,温度、湿度过高播放，位置变了
void MP3(void *pvParameters)
{  
	vTaskDelay(20000);   //等待系统数据采集正常运行
	while(1)
	{
		if(temp>tvalue_tmp)
		{
			Start_ZhiDing(0X1);  //室内温度过高
			vTaskDelay(2000);
		}
		if(hum>tvalue_hum)
		{
			Start_ZhiDing(0X2);  //室内湿度过高
			vTaskDelay(2000);
		}
		if(mq2>tvalue_mq2)
		{
			Start_ZhiDing(0X3);  //室内烟雾浓度过高
			vTaskDelay(2000);
		}
		if(mq5>tvalue_mq5)
		{
			Start_ZhiDing(0X4);  //室内可燃气体浓度过高
			vTaskDelay(2000);
		}
		if(fire==1)
		{
			Start_ZhiDing(0X5);  //室内发现火焰
			vTaskDelay(2000);
		}
		if(people==1)
		{
			Start_ZhiDing(0X6);  //有人员进入室内
			vTaskDelay(2000);
		}
		if(abs((int)pitch)>tvalue_tangle || abs((int)roll)>tvalue_tangle || abs((int)yaw)>tvalue_tangle)
		{
			Start_ZhiDing(0X7); //智能安防终端位置改变
			vTaskDelay(3000);
		}
		vTaskDelay(100);
	}
}

