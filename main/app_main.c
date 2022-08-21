/* Blink Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "driver/gpio.h"
#include "esp_log.h"
//#include "led_strip.h"
#include "sdkconfig.h"

#include <input_iot.h>
#include <output_iot.h>

#define BLINK_GPIO 2

#define BIT_PRESS_SHORT (1 << 0)
#define BIT_PRESS_NORMAL (1 << 1)
#define BIT_PRESS_LONG (1 << 2)

static EventGroupHandle_t xCreateEventGroup;

void input_event_call_back(int pin, uint64_t tick)
{
    if (pin == GPIO_NUM_0)
    {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        int press_ms = tick * 1000 / configTICK_RATE_HZ;
        if (press_ms < 1000)
        {
            xEventGroupSetBitsFromISR(xCreateEventGroup, BIT_PRESS_SHORT, &xHigherPriorityTaskWoken);
        }
        else if (press_ms < 3000)
        {
            xEventGroupSetBitsFromISR(xCreateEventGroup, BIT_PRESS_NORMAL, &xHigherPriorityTaskWoken);
        }
        else if (press_ms > 3000)
        {
            xEventGroupSetBitsFromISR(xCreateEventGroup, BIT_PRESS_LONG, &xHigherPriorityTaskWoken);
        }
    }
}

void button_timeout_callback(int pin)
{
    if (pin == GPIO_NUM_0)
    {
        printf("time out\n");
    }
}

void vTaskCode1(void *pvParameters)
{
    for (;;)
    {
        EventBits_t uxBits = xEventGroupWaitBits(
            xCreateEventGroup,                                   /* The event group being tested. */
            BIT_PRESS_SHORT | BIT_PRESS_NORMAL | BIT_PRESS_LONG, /* The bits within the event group to wait for. */
            pdTRUE,                                              /* BIT_0 & BIT_4 should be cleared before returning. */
            pdFALSE,                                             /* Don't wait for both bits, either bit will do. */
            portMAX_DELAY);                                      /* Wait a maximum of 100ms for either bit to be set. */

        if (uxBits & BIT_PRESS_SHORT)
        {
            printf("Short\n");
        }
        else if (uxBits & BIT_PRESS_NORMAL)
        {
            printf("Normal\n");
        }
        else if (uxBits & BIT_PRESS_LONG)
        {
            printf("Long\n");
        }
    }
}

void app_main(void)
{
    xCreateEventGroup = xEventGroupCreate();

    output_io_create(BLINK_GPIO);
    input_io_create(GPIO_NUM_0, HI_TO_LO);
    input_set_callback(input_event_call_back);
    input_set_timeout_callback(button_timeout_callback);

    xTaskCreate(
        vTaskCode1,   /* Function that implements the task. */
        "vTaskCode1", /* Text name for the task. */
        1024 * 2,     /* Stack size in words, not bytes. */
        NULL,         /* Parameter passed into the task. */
        4,            /* Priority at which the task is created. */
        NULL);
}