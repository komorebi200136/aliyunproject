#ifndef  __HCSR312_H
#define  __HCSR312_H

#include "stm32f4xx.h"
#include <stdbool.h>

void HCSR312_Init(void);
bool IsBodySensing(void);

#endif
