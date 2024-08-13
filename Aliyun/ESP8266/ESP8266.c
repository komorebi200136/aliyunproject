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

volatile bool avalue;    //��־����������
volatile bool bvalue;    //��־���ҵƵ�����


void ESP8266_Clear(void)
{
	memset(esp8266_buf, 0, sizeof(esp8266_buf));
	esp8266_cnt = 0;
}

bool ESP8266_WaitRecive(void)
{
	if(esp8266_cnt == 0) 					//������ռ���Ϊ0 ��˵��û�д��ڽ��������У�����ֱ����������������
		return 0;
	if(esp8266_cnt == esp8266_cntPre)		//�����һ�ε�ֵ�������ͬ����˵���������
	{
        recv_cnt=esp8266_cnt;
		esp8266_cnt = 0;					//��0���ռ���	
		return 1;						    //���ؽ�����ɱ�־
	}	
	esp8266_cntPre = esp8266_cnt;			//��Ϊ��ͬ
	return 0;								//���ؽ���δ��ɱ�־
}

bool ESP8266_SendCmd(char *cmd, char *res)
{
	unsigned char timeOut = 200;            //��ʱ����
    USART2_SendString(cmd);                 //����ATָ��
	while(timeOut--)
	{
		if(ESP8266_WaitRecive() == 1)							    //����յ�����
		{
			if(strstr((const char *)esp8266_buf, res) != NULL)		//����������ؼ���
			{
				ESP8266_Clear();									//��ջ���
				
				return 0;
			}
		}
		
		Delay_ms(10);
	}
	
	return 1;

}
//����һ���ֽڵ�����
void USART2_SendByte(uint8_t Byte)
{
	USART_SendData(USART2, Byte);
	while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
}


//����һ���ַ���������
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

    //������ʱ��
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

    //�򿪴���ʱ��
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;        //����ģʽ
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;  //�ٶ�
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;      //����ģʽ
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;         //����ģʽ
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2);

    USART_InitStructure.USART_BaudRate = baud;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b; //��λ����λ
    USART_InitStructure.USART_StopBits = USART_StopBits_1;      //1λֹͣλ
    USART_InitStructure.USART_Parity = USART_Parity_No;         //��У��
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; //��Ӳ������
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART2, &USART_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);   //���������ж�
    
    USART_Cmd(USART2, ENABLE);
	
    //��λ���ų�ʼ��
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
    
    
    //����PC4Ϊ�������ģʽ
    GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_4;   
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType   = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_NOPULL;
    //���øú�����GPIOC�˿ڽ��г�ʼ��
    GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	GPIO_SetBits(GPIOC,GPIO_Pin_4);    
    

}

void ESP8266_Network(void)
{
	char buf[256]={0};
	
	//Ӳ����λ
	GPIO_ResetBits(GPIOC,GPIO_Pin_4);
	Delay_ms(100);
	GPIO_SetBits(GPIOC,GPIO_Pin_4);
	Delay_ms(100);
	
    
	ESP8266_Clear();
    
    printf("1. ��λ8266\r\n");
	while(ESP8266_SendCmd("AT+RST\r\n","OK"))
    {
        
    }
	Delay_ms(100);
    
    printf("2. ����Ϊ͸��ģʽ\r\n");
	while(ESP8266_SendCmd("AT+CWMODE=1\r\n","OK"))
    {
        
    }
	Delay_ms(100);
    
    printf("3. ���Ӱ����Ʒ�����\r\n");
	while(ESP8266_SendCmd("AT+CIPSNTPCFG=1,8,\"ntp1.aliyun.com\"\r\n","OK"))
    {
        
    }
	Delay_ms(100);
    
    printf("4. ����WIIF�ȵ�\r\n");
    memset(buf,0,sizeof(buf));
    sprintf(buf,"AT+CWJAP=\"%s\",\"%s\"\r\n",WIFI,WFIIPASSWD);
	while(ESP8266_SendCmd(buf,"OK"))
    {
        
    }
	Delay_ms(100);
    
    printf("5. ���Ӱ����ƵĲ�ƷID����Ϣ\r\n");
    memset(buf,0,sizeof(buf));
    sprintf(buf,"AT+MQTTUSERCFG=0,1,\"NULL\",\"%s\",\"%s\",0,0,\"\"\r\n",USERNAME,PASSWD);
	while(ESP8266_SendCmd(buf,"OK"))
    {
        
    }
    Delay_ms(100);
    
    printf("6. ���Ӱ�����MQTT����Ϣ\r\n");
    memset(buf,0,sizeof(buf));
    sprintf(buf,"AT+MQTTCLIENTID=0,\"%s\"\r\n",CLIENTID);
	while(ESP8266_SendCmd(buf,"OK"))
    {
        
    }
    Delay_ms(100);
    
    printf("7. ���Ӱ����������˿ڵ�\r\n");
    memset(buf,0,sizeof(buf));
    sprintf(buf,"AT+MQTTCONN=0,\"%s\",%s,1\r\n",MQTTHOSTURL,POART);
	while(ESP8266_SendCmd(buf,"OK"))
    {
        
    }
	Delay_ms(100);
    
    printf("8. ���İ����Ʒ���������\r\n");
    memset(buf,0,sizeof(buf));
    sprintf(buf,"AT+MQTTSUB=0,\"%s\",1\r\n",MQTT_SUBSCRIBE_TOPIC);
	while(ESP8266_SendCmd(buf,"OK"))
    {
        
    }
    Delay_ms(100);
    
    printf("ESP8266��ʼ�����\r\n");

}


//�ϴ�����
void PubTopic(char *device,char *data)
{
    char command[256]={0};
    sprintf(command,"AT+MQTTPUB=0,\"%s\",\"{\\\"method\\\":\\\"thing.service.property.set\\\"\\,\\\"id\\\":\\\"20013636\\\"\\,\\\"params\\\":{\\\"%s\\\":%s}\\,\\\"version\\\":\\\"1.0.0\\\"}\",1,0\r\n",MQTT_PUBLISH_PHYSICSTOPIC,device,data);
    
	while(ESP8266_SendCmd(command,"OK"));
    //printf("�����ϴ��ɹ�\r\n");
}
//AT+MQTTPUB=0,"/a1KXCyQfMKC/ESP8266DUAN/user/ESP8266DUAN","{\"LightSwitch\":80}",1,0		//ESP8266���ƶ˷�������
///k0gp9PrRZUk/ietterminal/user/terminal
//�ϴ����ݵ�С���� MQTT_PUBLISH_TOPIC

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
	 // �������ַ�������ʼλ�ã������ţ�
    start= strstr(json_string, "{");
    if (start == NULL) {
        printf("Error: Left brace not found\n");
        return 1;
    }
    // �������ַ����Ľ���λ�ã��һ����ţ�
    end = strstr(start, "}");
    if (end == NULL) {
        printf("Error: Right brace not found\n");
        return 1;
    }
    // �������ַ����ĳ���
    length = end - start + 1;
    // ��ȡ���ַ���
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
        cJSON_Delete(root); // �ͷ���Դ
    }
}
//�����������·�������
//+MQTTSUBRECV:0,"/k0gp9PrRZUk/ietterminal/user/get",28,{"value":0,"target":"Alarm"}
int SubTopic(void)
{
	char mysubstring[100]={0};
    if(ESP8266_WaitRecive())   //���յ�����,
    {
		if(strstr((char *)esp8266_buf,"+MQTTSUBRECV")!=NULL) //�������·�������
		{
			printf("���յ�������:%s",esp8266_buf);
			get_json((char *)esp8266_buf,mysubstring);
			//printf("json��ʽ����:%s\r\n",mysubstring);
			cjson_slice(mysubstring);
			ESP8266_Clear();
			return 1;
		}   
    }
    return 0;
}    

void USART2_IRQHandler(void)
{
    //�ж��ж��Ƿ񴥷�
    if (USART_GetITStatus(USART2, USART_IT_RXNE) == SET)
    {   
      if(esp8266_cnt >= sizeof(esp8266_buf))	esp8266_cnt = 0; //��ֹ���ڱ�ˢ��     
	  esp8266_buf[esp8266_cnt++]=USART_ReceiveData(USART2);
      USART_ClearITPendingBit(USART2,USART_IT_RXNE); 
    }
}


