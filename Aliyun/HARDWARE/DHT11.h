#ifndef  __DHT11_H
#define  __DHT11_H

#include <stdbool.h>
#include "stm32f4xx.h"
#include "Delay.h"
#include "Serial.h"

void DHT11_Init(void);
bool DHT11_ReadData(volatile uint8_t *dhtbuf);

#endif
