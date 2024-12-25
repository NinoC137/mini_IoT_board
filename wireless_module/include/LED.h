#ifndef __LED_H__
#define __LED_H__

#include <Arduino.h>
#include "common.h"

extern SemaphoreHandle_t LED_Mutex;

void LED_On();

void LED_Off();

void LED_Toggle();

#endif