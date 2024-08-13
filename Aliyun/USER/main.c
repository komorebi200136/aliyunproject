#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/*  

LCD:PG12 PE3 PE5 PC0 PE1 PB7

OLED:PD11(SDA) PD13(SCL)

���Դ��ڣ�PA9(USART1_TX)��PA10(USART1_RX)
WIFIģ�飺PA3(USART2_RX)��PA2(USART2_TX)��RST(PC4)
����ģ�飺PB10(USART3_TX)��PB11(USART3_RX)
GPRSģ�飺PC10(UART4 TX)��PC11(UART4 RX)
MP3ģ�飺 PC12��UART5 TX��PD2(UART5 RX)
���ô��ڣ�PG14��USART6 TX����PG9��USART6 RX��

ADCģ��
MQ2:AO PF5 DO PF7
MQ5:AO PF4 DO PF3
���洫����:AO PF8 DO PF6

������⴫����:PA5


MPU6050:PB1(SCL)��PC5(SDA)
������:PB15

DHT11:PD9

LED1:PB4
LED2:PE6

KEY1:PD3
KEY2:PC8

*/

extern volatile bool avalue;          //��������
extern volatile bool bvalue;          //���ҵ�����
extern volatile bool avalue_key;      //������������
extern volatile bool bvalue_key;      //�������ҵ�����

extern char blueteeth_buf[512];       //�������ܵ�����
extern char blueteeth_recvcnt;
extern volatile uint8_t TimeDisplay;  //��ʾʱ�份�ѱ�־

volatile uint8_t DHT11Data[5]={0};    //�洢��ʪ�����ݣ�DHT11Data[2]���¶ȣ�DHT11Data[0]��ʪ��
volatile uint8_t temp,hum;            //�洢��ʪ������
volatile uint16_t mq2,mq5;            //����MQ2��MQ5��ADCת��ֵ������Ϳ�ȼ����
volatile bool people;                 //�Ƿ�����
volatile bool fire;                   //�Ƿ��л�
float pitch,roll,yaw; 	              //ŷ����

volatile uint16_t tvalue_tmp,tvalue_hum,tvalue_mq2,tvalue_mq5,tvalue_tangle;  //�������ݵ���ֵ
	


SemaphoreHandle_t LCDMutex;       //����LCD�Ļ�����
SemaphoreHandle_t OLEDMutex;      //����OLED�Ļ�����
SemaphoreHandle_t PubMutex;       //���ڶ�����Ϣ�Ļ�����
SemaphoreHandle_t  EspNum;        // ����Э�������Ķ�ֵ���ź���
SemaphoreHandle_t SemaphoreNum;   //�ź���
SemaphoreHandle_t  g_sem_tmp;     // ��ֵ���ź���

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

void Esp_DataMana(void *pvParameters);   //���������ݿ���
void BedlampControl( void *pvParameters);//���ҵƿ���
void AlarmControl(void *pvParameters);   //��������
void ScreenDisplay(void *pvParameters);  //��Ļ��ʾ
void DHT11(void *pvParameters);          //��ʪ�Ȼ�ȡ
void MQADC(void *pvParameters);          //����Ϳ�ȼ����
void Fire(void *pvParameters);           //���洫����
void BodyReaction(void *pvParameters);   //�����Ӧ��
void MPU6050(void *pvParameters);        //ʵ�ְ���ϵͳ����
void BlueTeeth(void *pvParameters);      //����ģ��
void SIM900A(void *pvParameters);        //������ģ��
void MP3(void *pvParameters);            //��������ģ��

int main()
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);  //�ж����ȼ�����
    Serial_Init(115200);        //���Դ��ڳ�ʼ��
    ESP8266_Init(115200);       //WIFIģ���ʼ��
	BlueTooth_Init(9600);       //������ʼ��
	SIM_Init(9600);             //GPRSģ���ʼ��
	MP3_Init(9600);             //��������ģ���ʼ��
	MQ_Init();                  //����Ϳ�ȼ����
	Key_Init();                 //������ʼ��
	RTC_Config();               //RTC��ʼ��
    LED_Init();                 //LED��ʼ��
	HCSR312_Init();             //�����Ӧ������
	Fire_Init();                //�����Ӧ��
	Beep_Init();                //��������ʼ��
	DHT11_Init();               //��ʪ�ȳ�ʼ��
    Delay_Init(168);
	
    LCDMutex=xSemaphoreCreateMutex();           //����������
	OLEDMutex=xSemaphoreCreateMutex();          //����������
	PubMutex=xSemaphoreCreateMutex();           //����������
	EspNum=xSemaphoreCreateBinary();            //��ֵ�ź���
	SemaphoreNum=xSemaphoreCreateCounting(2,0); //���ֵΪ2,��ʼֵΪ0��
	//����������           
    xTaskCreate(   SIM900A,   
                   "sim900atask",     
                   256,            
                   NULL,           
                   5,               
                   &SIM900AHandle     
           );  
    //ESP����           
    xTaskCreate(   Esp_DataMana,   
                   "esptask",     
                   256,            
                   NULL,           
                   4,               
                   &EspHandle     
           );  
	//��������           
    xTaskCreate(   AlarmControl,   
                   "alarmtask",     
                   256,            
                   NULL,           
                   4,               
                   &AlarmControlHandle  
           );  
	//���ҵ�����           
    xTaskCreate(   BedlampControl,   
                   "bedlamptask",     
                   256,            
                   NULL,           
                   4,               
                   &BedlampControlHandle  
           );
	//������������           
    xTaskCreate(   BlueTeeth,   
                   "blueteethtask",     
                   256,            
                   NULL,           
                   4,               
                   &BlueTeethHandle     
           );  
   
	//��Ļ��ʾ����           
    xTaskCreate(   ScreenDisplay,   
                   "screendisplaytask",     
                   256,            
                   NULL,           
                   3,               
                   &ScreenDisplayHandle  
           );

    //��ʪ�Ȼ�ȡ����           
    xTaskCreate(   DHT11,   
                   "dht11task",     
                   256,            
                   NULL,           
                   3,               
                   &DHT11Handle      //���ȼ�������3����Ȼ�޷���ȡ��ʪ��
           );
	//MQ2��MQ5ADCֵ�Ļ�ȡ����           
    xTaskCreate(   MQADC,   
                   "mqadctask",     
                   256,            
                   NULL,           
                   2,               
                   &MQADCHandle  
           );
	//���洫��������           
    xTaskCreate(   Fire,   
                   "firetask",     
                   256,            
                   NULL,           
                   2,               
                   &FIREHandle
           );
    //�����Ӧ����           
    xTaskCreate(   BodyReaction,   
                   "bodyreactiontask",     
                   256,            
                   NULL,           
                   2,               
                   &BODYREATIONHandle
           );
	//ʵ�ְ���ϵͳ��������           
    xTaskCreate(   MPU6050,   
                   "mpu6050task",     
                   256,            
                   NULL,           
                   2,               
                   &MPU6050Handle
           );
	//ʵ��������������           
    xTaskCreate(   MP3,   
                   "mp3task",     
                   256,            
                   NULL,           
                   2,               
                   &MP3Handle
           );
    //����������  ������һ���������ͽ�������ѭ��
    vTaskStartScheduler();
}


//�����������ϴ��Ͷ��Ĵ���
void Esp_DataMana(void *pvParameters)
{  
	int i=0;
	uint8_t temp=0; 
	uint8_t hum=0;
	uint16_t tmq2,tmq5;
	bool tpeople;     
	bool tfire;       
    ESP8266_Network();  //���Ӱ�����
    while(1)
    {
		SubTopic();  //��������
		if(i>500)    //5���ϴ�һ������
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


//��������,���ҵ�
void BedlampControl(void *pvParameters)
{  
    while(1)
    {
		if(bvalue || bvalue_key)   //��������Զ�̿���
			WLED_On();
		else
			WLED_Off();
		vTaskDelay(100);
    }
}
//�����ƣ�������
void AlarmControl(void *pvParameters)
{  
    while(1)
    {
		if(avalue || avalue_key) //��������Զ�̿���
	    {
			GPIO_ToggleBits(GPIOE, GPIO_Pin_6);
			GPIO_ToggleBits(GPIOB, GPIO_Pin_15);
		}
		else
		{
			RLED_Off();   //�رվ�����
			Beep_Off();   //�رվ���
		}
		vTaskDelay(100);
    }
}

//�������ã�����ָ����ʽ������tem=15;hum=34;mq2=1200;mq5=1600;tangle=50;����MP3��������С
void BlueTeeth(void *pvParameters)
{  
	while(1)
	{
		if(Blueteeth_WaitRecive())  //����յ�����
		{
			if(strstr(blueteeth_buf,"time"))
			{
				printf("%s\r\n",blueteeth_buf);
				SetTimeDate(blueteeth_buf,blueteeth_recvcnt);  //time=15:24:23
				Blueteeth_Clear();
			}
			else if(strstr(blueteeth_buf,"volume+"))    //�Ӵ�����
			{
				printf("%s\r\n",blueteeth_buf);
				VolumeUp();
				Blueteeth_Clear();
			}
			else if(strstr(blueteeth_buf,"volume-"))   //��С����
			{
				printf("%s\r\n",blueteeth_buf);
				VolumeDown();
				Blueteeth_Clear();
			}
			else if(strstr(blueteeth_buf,"tmp"))       //�����¶���ֵ
		    {
				printf("%s\r\n",blueteeth_buf);
				tvalue_tmp = slice(blueteeth_buf);
				Blueteeth_Clear();
			}
			else if(strstr(blueteeth_buf,"hum"))       //����ʪ����ֵ
		    {
				printf("%s\r\n",blueteeth_buf);
				tvalue_hum = slice(blueteeth_buf);
				Blueteeth_Clear();
			}
			else if(strstr(blueteeth_buf,"mq2"))       //����������ֵ
		    {
				printf("%s\r\n",blueteeth_buf);
				tvalue_mq2 = slice(blueteeth_buf);
				Blueteeth_Clear();
			}
			else if(strstr(blueteeth_buf,"mq5"))       //���ÿ�ȼ�������ֵ
		    {
				printf("%s\r\n",blueteeth_buf);
				tvalue_mq5 = slice(blueteeth_buf);
				Blueteeth_Clear();
			}
			else if(strstr(blueteeth_buf,"tangle"))    //���ýǶ���ֵ
		    {
				printf("%s\r\n",blueteeth_buf);
				tvalue_tangle = slice(blueteeth_buf);
				Blueteeth_Clear();
			}
		}
		vTaskDelay(100);
	}
}

//GPRSģ������,�������Ķ���
void SIM900A(void *pvParameters)
{  
	//��30����ֻ�ܷ���һ�ζ��ţ���ֹ�����ظ�����
	uint8_t i=0,a=0,b=0,c=0,d=0,e=0,f=0,g=0;
	tvalue_tmp=30; tvalue_hum=80;tvalue_mq2=1000; tvalue_mq5=1000;tvalue_tangle=50;
	vTaskDelay(20000);
	while(1)
	{
		if(temp>tvalue_tmp && a==0)
		{
			a=1;
			Send_ChineseMessages("5BA451856E295EA68FC79AD8","00310038003400370036003900370039003400320038");  
			//�����¶ȹ���
		}
		if(hum>tvalue_hum && b==0)
		{
			b=1;
			Send_ChineseMessages("5BA451856E7F5EA68FC79AD8","00310038003400370036003900370039003400320038");  
			//����ʪ�ȹ���
		}
		if(mq2>tvalue_mq2 && c==0)
		{
			c=1;
			Send_ChineseMessages("5BA4518570DF96FE6D535EA68FC79AD8","00310038003400370036003900370039003400320038");  
			//��������Ũ�ȹ���
		}
		if(mq5>tvalue_mq5 && d==0)
		{
			d=1;
			Send_ChineseMessages("5BA4518570DF96FE6D535EA68FC79AD8","00310038003400370036003900370039003400320038");  
			//���ڿ�ȼ����Ũ�ȹ���
		}
		if(fire==1 && e==0)
		{
			e=1;
			Send_ChineseMessages("5BA4518553D173B0706B7130","00310038003400370036003900370039003400320038");  
			//���ڷ��ֻ���
		}
		if(people==1 && f==0)
		{
			f=1;
			Send_ChineseMessages("67094EBA54588FDB51655BA45185","00310038003400370036003900370039003400320038"); 
			//����Ա��������
		}
		if((abs((int)pitch)>tvalue_tangle || abs((int)roll)>tvalue_tangle || abs((int)yaw)>tvalue_tangle) && g==0)
		{
			g=1;
			Send_ChineseMessages("667A80FD5B8996327EC87AEF4F4D7F6E653953D8","00310038003400370036003900370039003400320038"); 
			//���ܰ����ն�λ�øı�
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

//LCD��OLED��Ļ��ʾ
void ScreenDisplay(void *pvParameters)
{  
	RTC_TimeTypeDef   RTC_TimeStructure;
	OLED_Init();			            //��ʼ��OLED 
	OLED_ShowString(1,1,"time:");
	OLED_ShowChar(1, 8, ':');
	OLED_ShowChar(1, 11, ':');
	xSemaphoreTake(LCDMutex,portMAX_DELAY);
	LCD_Init();                         //LCD��ʼ��
    LCD_Fill(0,0,LCD_W,LCD_H,WHITE);
	LCD_ShowString(30,0,(u8*)"aliyunsystem",RED,WHITE,32,0);
	LCD_ShowString(0,30,(u8*)"tem:",BLUE,WHITE,32,0);     //�¶�
	LCD_ShowString(0,70,(u8*)"hum:",BLUE,WHITE,32,0);     //ʪ��
	LCD_ShowString(110,30,(u8*)"C",BLUE,WHITE,32,0);      //�¶ȵ�λ
	LCD_ShowString(110,70,(u8*)"%RH",BLUE,WHITE,32,0);    //ʪ�ȵ�λ
	LCD_ShowString(0,110,(u8*)"mq2:",BLUE,WHITE,32,0);    //����Ũ��
	LCD_ShowString(0,150,(u8*)"mq5:",BLUE,WHITE,32,0);    //��ȼ����Ũ��
	LCD_ShowString(0,190,(u8*)"fire:",BLUE,WHITE,32,0);   //����
	LCD_ShowString(0,230,(u8*)"body:",BLUE,WHITE,32,0);   //�����Ӧ
	LCD_ShowString(0,270,(u8*)"place:",BLUE,WHITE,32,0);  //λ�÷������
	xSemaphoreGive(LCDMutex);
	while(1)
	{
		//��OLED��ʾʱ��
		if(TimeDisplay == 1)
        {
            TimeDisplay = 0;
            //��ȡ��ǰʱ��
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

//DHT11�¶Ȼ�ȡ 
void DHT11(void *pvParameters)
{  
	while(1)
	{
		if(DHT11_ReadData(DHT11Data))
		{
			temp=DHT11Data[2];
			hum=DHT11Data[0];
			xSemaphoreTake(LCDMutex,portMAX_DELAY);
			LCD_ShowIntNum(75,30,DHT11Data[2],2,GREEN,WHITE,32);//��ʾ��������
			LCD_ShowIntNum(75,70,DHT11Data[0],2,GREEN,WHITE,32);//��ʾ��������
			xSemaphoreGive(LCDMutex);
		}
		vTaskDelay(2000);   //��ʱ2��
	} 
}

//MQ2�����MQ5��ȼ����
void MQADC(void *pvParameters)
{  
	while(1)
	{
		mq2=AD_GetValue(ADC_Channel_15);            //PF5 MQ2 PF4 MQ5
        mq5=AD_GetValue(ADC_Channel_14);
		xSemaphoreTake(LCDMutex,portMAX_DELAY);
		LCD_ShowIntNum(60,110,mq2,4,GREEN,WHITE,32);//��ʾ��������
		LCD_ShowIntNum(60,150,mq5,4,GREEN,WHITE,32);//��ʾ��������
		xSemaphoreGive(LCDMutex);
		vTaskDelay(1000);
	}
}

//���洫����
void Fire(void *pvParameters)
{  
	while(1)
	{
		if(IsFireSensing()==0)
		{
			xSemaphoreTake(LCDMutex,portMAX_DELAY);
			fire=1;
			LCD_ShowString(80,190,(u8*)"have",GREEN,WHITE,32,0);  //���Ĳ���0��ʾ�����ַ���1��ʾ������
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

//�����Ӧ
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

//������
/*
PB5 PB6������
//roll:�����.��λ0.01�ȡ� -18000 -> 18000 ��Ӧ -180.00  ->  180.00��
//pitch:������.��λ 0.01�ȡ�-9000 - 9000 ��Ӧ -90.00 -> 90.00 ��
//yaw:�����.��λΪ0.1�� 0 -> 3600  ��Ӧ 0 -> 360.0��
*/
void MPU6050(void *pvParameters)
{ 
	uint8_t res=0;	
	MPU_Init(); //��ʼ��MPU6050	
	while(mpu_dmp_init())
	{
		printf("MPU6050 ERROR \r\n");
		Delay_ms(500);
	}
	xSemaphoreTake(OLEDMutex,portMAX_DELAY);
	OLED_ShowString(2, 1, "pitch:");    //��ʾ��̬�ַ���
	OLED_ShowString(3, 1, "roll:");		//��ʾ��̬�ַ���
    OLED_ShowString(4, 1, "yaw:");		//��ʾ��̬�ַ���
	xSemaphoreGive(OLEDMutex);   
	while(1)
	{
		res=mpu_dmp_get_data(&pitch,&roll,&yaw);
		if(res==0)
		{ 
			if(abs((int)pitch)>30 || abs((int)roll)>30 || abs((int)yaw)>30)
			{
				xSemaphoreTake(LCDMutex,portMAX_DELAY);
				LCD_ShowString(100,270,(u8*)"change  ",GREEN,WHITE,32,0);   //λ�÷������
				xSemaphoreGive(LCDMutex);	
			}
			else
			{
				xSemaphoreTake(LCDMutex,portMAX_DELAY);
				LCD_ShowString(100,270,(u8*)"nochange",GREEN,WHITE,32,0);   //λ�÷������
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

//MP3ģ������,�¶ȡ�ʪ�ȹ��߲��ţ�λ�ñ���
void MP3(void *pvParameters)
{  
	vTaskDelay(20000);   //�ȴ�ϵͳ���ݲɼ���������
	while(1)
	{
		if(temp>tvalue_tmp)
		{
			Start_ZhiDing(0X1);  //�����¶ȹ���
			vTaskDelay(2000);
		}
		if(hum>tvalue_hum)
		{
			Start_ZhiDing(0X2);  //����ʪ�ȹ���
			vTaskDelay(2000);
		}
		if(mq2>tvalue_mq2)
		{
			Start_ZhiDing(0X3);  //��������Ũ�ȹ���
			vTaskDelay(2000);
		}
		if(mq5>tvalue_mq5)
		{
			Start_ZhiDing(0X4);  //���ڿ�ȼ����Ũ�ȹ���
			vTaskDelay(2000);
		}
		if(fire==1)
		{
			Start_ZhiDing(0X5);  //���ڷ��ֻ���
			vTaskDelay(2000);
		}
		if(people==1)
		{
			Start_ZhiDing(0X6);  //����Ա��������
			vTaskDelay(2000);
		}
		if(abs((int)pitch)>tvalue_tangle || abs((int)roll)>tvalue_tangle || abs((int)yaw)>tvalue_tangle)
		{
			Start_ZhiDing(0X7); //���ܰ����ն�λ�øı�
			vTaskDelay(3000);
		}
		vTaskDelay(100);
	}
}

