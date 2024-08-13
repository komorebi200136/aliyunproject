#ifndef  __BLUETOOTH_H
#define  __BLUETOOTH_H

#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "stm32f4xx.h"
#include "serial.h"

void BlueTooth_Init(uint32_t baud);
void BlueTooth_SendString(char *String);
uint16_t slice(char *str);
bool Blueteeth_WaitRecive(void);
void Blueteeth_Clear(void);

#endif
