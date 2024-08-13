#include "ESP8266.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "CJSON.h"
#include "event_groups.h"

unsigned char esp8266_buf[256];
unsigned short esp8266_cnt = 0 ;
unsigned short esp8266_cntPre = 0;
unsigned char  recv_cnt=0;

volatile bool avalue;    //标志警报的数字
volatile bool bvalue;    //标志卧室灯的数字


void ESP8266_Clear(void)
{
	memset(esp8266_buf, 0, sizeof(esp8266_buf));
	esp8266_cnt = 0;
}

bool ESP8266_WaitRecive(void)
{
	if(esp8266_cnt == 0) 					//如果接收计数为0 则说明没有处于接收数据中，所以直接跳出，结束函数
		return 0;
	if(esp8266_cnt == esp8266_cntPre)		//如果上一次的值和这次相同，则说明接收完毕
	{
        recv_cnt=esp8266_cnt;
		esp8266_cnt = 0;					//清0接收计数	
		return 1;						    //返回接收完成标志
	}	
	esp8266_cntPre = esp8266_cnt;			//置为相同
	return 0;								//返回接收未完成标志
}

bool ESP8266_SendCmd(char *cmd, char *res)
{
	unsigned char timeOut = 200;            //超时处理
    USART2_SendString(cmd);                 //发送AT指令
	while(timeOut--)
	{
		if(ESP8266_WaitRecive() == 1)							    //如果收到数据
		{
			if(strstr((const char *)esp8266_buf, res) != NULL)		//如果检索到关键词
			{
				ESP8266_Clear();									//清空缓存
				
				return 0;
			}
		}
		
		Delay_ms(10);
	}
	
	return 1;

}
//发送一个字节的数据
void USART2_SendByte(uint8_t Byte)
{
	USART_SendData(USART2, Byte);
	while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
}


//发送一个字符串的数据
void USART2_SendString(char *String)
{
	while(*String != '\0')
    {
        USART2_SendByte(*String++);
    }
}
//PA2 -- TX PA3 -- RX
void ESP8266_Init(uint32_t baud)
{
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;

    //打开引脚时钟
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

    //打开串口时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;        //复用模式
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;  //速度
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;      //推挽模式
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;         //上拉模式
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2);

    USART_InitStructure.USART_BaudRate = baud;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b; //八位数据位
    USART_InitStructure.USART_StopBits = USART_StopBits_1;      //1位停止位
    USART_InitStructure.USART_Parity = USART_Parity_No;         //无校验
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; //无硬件流控
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART2, &USART_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);   //启动接收中断
    
    USART_Cmd(USART2, ENABLE);
	
    //复位引脚初始化
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
    
    
    //配置PC4为推挽输出模式
    GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_4;   
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType   = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_NOPULL;
    //调用该函数对GPIOC端口进行初始化
    GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	GPIO_SetBits(GPIOC,GPIO_Pin_4);    
    

}

void ESP8266_Network(void)
{
	char buf[256]={0};
	
	//硬件复位
	GPIO_ResetBits(GPIOC,GPIO_Pin_4);
	Delay_ms(100);
	GPIO_SetBits(GPIOC,GPIO_Pin_4);
	Delay_ms(100);
	
    
	ESP8266_Clear();
    
    printf("1. 软复位8266\r\n");
	while(ESP8266_SendCmd("AT+RST\r\n","OK"))
    {
        
    }
	Delay_ms(100);
    
    printf("2. 设置为透传模式\r\n");
	while(ESP8266_SendCmd("AT+CWMODE=1\r\n","OK"))
    {
        
    }
	Delay_ms(100);
    
    printf("3. 连接阿里云服务器\r\n");
	while(ESP8266_SendCmd("AT+CIPSNTPCFG=1,8,\"ntp1.aliyun.com\"\r\n","OK"))
    {
        
    }
	Delay_ms(100);
    
    printf("4. 连接WIIF热点\r\n");
    memset(buf,0,sizeof(buf));
    sprintf(buf,"AT+CWJAP=\"%s\",\"%s\"\r\n",WIFI,WFIIPASSWD);
	while(ESP8266_SendCmd(buf,"OK"))
    {
        
    }
	Delay_ms(100);
    
    printf("5. 连接阿里云的产品ID等信息\r\n");
    memset(buf,0,sizeof(buf));
    sprintf(buf,"AT+MQTTUSERCFG=0,1,\"NULL\",\"%s\",\"%s\",0,0,\"\"\r\n",USERNAME,PASSWD);
	while(ESP8266_SendCmd(buf,"OK"))
    {
        
    }
    Delay_ms(100);
    
    printf("6. 连接阿里云MQTT等信息\r\n");
    memset(buf,0,sizeof(buf));
    sprintf(buf,"AT+MQTTCLIENTID=0,\"%s\"\r\n",CLIENTID);
	while(ESP8266_SendCmd(buf,"OK"))
    {
        
    }
    Delay_ms(100);
    
    printf("7. 连接阿里云域名端口等\r\n");
    memset(buf,0,sizeof(buf));
    sprintf(buf,"AT+MQTTCONN=0,\"%s\",%s,1\r\n",MQTTHOSTURL,POART);
	while(ESP8266_SendCmd(buf,"OK"))
    {
        
    }
	Delay_ms(100);
    
    printf("8. 订阅阿里云服务器主题\r\n");
    memset(buf,0,sizeof(buf));
    sprintf(buf,"AT+MQTTSUB=0,\"%s\",1\r\n",MQTT_SUBSCRIBE_TOPIC);
	while(ESP8266_SendCmd(buf,"OK"))
    {
        
    }
    Delay_ms(100);
    
    printf("ESP8266初始化完成\r\n");

}


//上传数据
void PubTopic(char *device,char *data)
{
    char command[256]={0};
    sprintf(command,"AT+MQTTPUB=0,\"%s\",\"{\\\"method\\\":\\\"thing.service.property.set\\\"\\,\\\"id\\\":\\\"20013636\\\"\\,\\\"params\\\":{\\\"%s\\\":%s}\\,\\\"version\\\":\\\"1.0.0\\\"}\",1,0\r\n",MQTT_PUBLISH_PHYSICSTOPIC,device,data);
    
	while(ESP8266_SendCmd(command,"OK"));
    //printf("数据上传成功\r\n");
}
//AT+MQTTPUB=0,"/a1KXCyQfMKC/ESP8266DUAN/user/ESP8266DUAN","{\"LightSwitch\":80}",1,0		//ESP8266向云端发送数据
///k0gp9PrRZUk/ietterminal/user/terminal
//上传数据到小程序 MQTT_PUBLISH_TOPIC

//void ESP8266_Send(char *property,int Data)
//{
//	printf("AT+MQTTPUB=0,\"%s\",\"{\\\"%s\\\":%d}\",1,0\r\n",post,property,Data);
//}

void PubTopicToMP(int temp,int hum,int smoke ,int combu,bool people,bool fire)
{
    char command[256]={0};
    sprintf(command,
	"AT+MQTTPUB=0,\"%s\",\"{\\\"temperature\\\":%d\\,\\\"humidity\\\":%d\\,\
	\\\"smoke\\\":%d\\,\\\"combu\\\":%d\\,\\\"people\\\":%d\\,\\\"fire\\\":%d}\",1,0\r\n"
	,MQTT_PUBLISH_TOPIC,temp,hum,smoke,combu,people,fire);
	while(ESP8266_SendCmd(command,"OK"));                                      
}


//+MQTTSUBRECV:0,"/k0gp9PrRZUk/ietterminal/user/get",28,{"value":1,"target":"Alarm"}
int get_json(const char *json_string,char *substring)
{
	const char *start=NULL;
	const char *end=NULL;
	int length;
	 // 查找子字符串的起始位置（左花括号）
    start= strstr(json_string, "{");
    if (start == NULL) {
        printf("Error: Left brace not found\n");
        return 1;
    }
    // 查找子字符串的结束位置（右花括号）
    end = strstr(start, "}");
    if (end == NULL) {
        printf("Error: Right brace not found\n");
        return 1;
    }
    // 计算子字符串的长度
    length = end - start + 1;
    // 提取子字符串
    strncpy(substring, start, length);
    return 0;
}

void cjson_slice(char *substring)
{
	cJSON *value = NULL;
    cJSON *target = NULL;
    cJSON *root = cJSON_Parse(substring);
    if (!root) 
	{
        printf("Error before: [%s]\r\n", cJSON_GetErrorPtr());
    } 
	else 
	{
        value = cJSON_GetObjectItemCaseSensitive(root, "value");
        target = cJSON_GetObjectItemCaseSensitive(root, "target");
        if (cJSON_IsNumber(value) && cJSON_IsString(target)) 
		{
            printf("Value: %d\r\n", value->valueint);
            printf("Target: %s\r\n", target->valuestring);
            
            if (strcmp("Alarm", target->valuestring) == 0) 
			{
				
                avalue = value->valueint;
                printf("avalue: %d\r\n", avalue);
            } 
			else if (strcmp("BedLamp", target->valuestring) == 0) 
			{
                bvalue = value->valueint;
                printf("bvalue: %d\r\n", bvalue);
            }
        }
        cJSON_Delete(root); // 释放资源
    }
}
//解析服务器下发的数据
//+MQTTSUBRECV:0,"/k0gp9PrRZUk/ietterminal/user/get",28,{"value":0,"target":"Alarm"}
int SubTopic(void)
{
	char mysubstring[100]={0};
    if(ESP8266_WaitRecive())   //接收到数据,
    {
		if(strstr((char *)esp8266_buf,"+MQTTSUBRECV")!=NULL) //并且是下发的数据
		{
			printf("接收到的数据:%s",esp8266_buf);
			get_json((char *)esp8266_buf,mysubstring);
			//printf("json格式数据:%s\r\n",mysubstring);
			cjson_slice(mysubstring);
			ESP8266_Clear();
			return 1;
		}   
    }
    return 0;
}    

void USART2_IRQHandler(void)
{
    //判断中断是否触发
    if (USART_GetITStatus(USART2, USART_IT_RXNE) == SET)
    {   
      if(esp8266_cnt >= sizeof(esp8266_buf))	esp8266_cnt = 0; //防止串口被刷爆     
	  esp8266_buf[esp8266_cnt++]=USART_ReceiveData(USART2);
      USART_ClearITPendingBit(USART2,USART_IT_RXNE); 
    }
}


