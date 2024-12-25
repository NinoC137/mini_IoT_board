#include "LED.h"

SemaphoreHandle_t LED_Mutex;

static uint8_t LED_Status = 0;

void LED_On()
{
    if (xSemaphoreTake(LED_Mutex, (TickType_t)100) == pdTRUE)
    {
        digitalWrite(SYSTEM_LED, 0);
        LED_Status = 1;
        xSemaphoreGive(LED_Mutex);
    }
    else
    {
        /* We could not obtain the semaphore and can therefore not access
            the shared resource safely. */
    }
}

void LED_Off()
{
    if (xSemaphoreTake(LED_Mutex, (TickType_t)100) == pdTRUE)
    {
        digitalWrite(SYSTEM_LED, 1);
        LED_Status = 0;
        xSemaphoreGive(LED_Mutex);
    }
    else
    {
        /* We could not obtain the semaphore and can therefore not access
            the shared resource safely. */
    }
}

void LED_Toggle()
{
    if (LED_Status == 0)
    {
        LED_On();
    }
    else
    {
        LED_Off();
    }
}

void LED_Blink()
{
    LED_Toggle();
    vTaskDelay(30);
    LED_Toggle();
}