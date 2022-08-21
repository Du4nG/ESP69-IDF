#include <stdio.h>
#include <esp_log.h>
#include <driver/gpio.h>
#include "input_iot.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"

input_callback_t input_callback = NULL;
timeoutButton_t timeoutButton_callback = NULL;

static uint64_t _start, _stop, _pressTick;
static TimerHandle_t xTimers[1];

static void gpio_input_handler(void *arg)
{
    int gpio_num = (uint32_t)arg;
    uint32_t rtc = xTaskGetTickCountFromISR();

    if (gpio_get_level(gpio_num) == 0)
    {
        _start = rtc;
        xTimerStart(xTimers, 0);
    }
    else
    {
        xTimerStop(xTimers, 0);
        _stop = rtc;
        _pressTick = _stop - _start;
        input_callback(gpio_num, _pressTick);
    }
}

static void vTimerCallback(TimerHandle_t xTimer)
{
    uint32_t ID = (uint32_t)pvTimerGetTimerID(xTimer);

    if (ID == 0)
    {
        timeoutButton_callback(GPIO_NUM_0);
    }
    else
    {
        printf("Timer1\n");
    }
}

void input_io_create(gpio_num_t gpio_num, interrupt_type_edge_t type)
{
    gpio_set_direction(gpio_num, GPIO_MODE_INPUT);
    gpio_set_pull_mode(gpio_num, GPIO_PULLUP_ONLY);
    gpio_set_intr_type(gpio_num, type);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(gpio_num, gpio_input_handler, (void *)gpio_num);

    xTimers[0] = xTimerCreate("Timer", 5000 / portTICK_RATE_MS, pdFALSE, (void *)0, vTimerCallback);
}

void input_io_get_level(gpio_num_t gpio_num)
{
    return gpio_get_level(gpio_num);
}

void input_set_callback(void *cb)
{
    input_callback = cb;
}

void input_set_timeout_callback(void *cb)
{
    timeoutButton_callback = cb;
}