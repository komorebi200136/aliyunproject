#ifndef  __ESP8266_H
#define  __ESP8266_H


//�ֻ��ȵ��WIFI���������
#define WIFI        "esp"
#define WFIIPASSWD  "20013637"

/*��������Ϣ �ǵ���CLIENTID��,ǰ�����\\*/
#define POART                        "1883"
#define CLIENTID                     "k0gp9PrRZUk.ietterminal|securemode=2\\,signmethod=hmacsha256\\,timestamp=1710999993065|"  
#define USERNAME                     "ietterminal&k0gp9PrRZUk"
#define PASSWD                       "a769464d50754f600f4fec821c1e1d3ed1740a87fb7c749d75d5a8bbeb80d703"
#define MQTTHOSTURL                  "iot-06z00e72pmhfm3r.mqtt.iothub.aliyuncs.com"
#define	MQTT_PUBLISH_TOPIC           "/k0gp9PrRZUk/ietterminal/user/terminal"                      //�ϴ�������Ϣת��
#define	MQTT_PUBLISH_PHYSICSTOPIC    "/sys/k0gp9PrRZUk/ietterminal/thing/event/property/post"      //�ϴ�������ģ��
#define MQTT_SUBSCRIBE_TOPIC         "/k0gp9PrRZUk/ietterminal/user/get"                           //�������⣬С���������ת

#include "stm32f4xx.h"  //�������
#include "Delay.h"
#include "Serial.h"
#include <string.h>
#include <stdio.h>
#include <stdbool.h>


void ESP8266_Network(void);
void ESP8266_Init(uint32_t baud);
bool ESP8266_SendCmd(char *cmd, char *res);
void USART2_SendString(char *String);
void PubTopic(char *device,char *data);
int SubTopic(void);
void PubTopicToMP(int temp,int hum,int smoke ,int combu,bool people,bool fire);
#endif
