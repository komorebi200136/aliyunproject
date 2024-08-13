#ifndef  __RTC_H
#define  __RTC_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "stm32f4xx.h"  
#include "Serial.h"

void RTC_Config(void);
void SetTimeDate(char *RxBuffer,char ubRxIndex);

#endif
