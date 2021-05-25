#include <stdbool.h>
#include <stdio.h>
#include <nrf_delay.h>
#include "nrfx_timer.h"
#include "nrf_gpio.h"
#include "boards.h"

const nrfx_timer_t TIMER_COUNTER = NRFX_TIMER_INSTANCE(0);

/**
 * 中断发生时，回调函数
 *
 * @param event_type
 * @param p_context
 */
void timer_event_handler(nrf_timer_event_t event_type, void* p_context)
{
}

/**
 * 初始化timer -> instance 0
 */
void timer_init(void)
{
    uint32_t err_code = NRF_SUCCESS;

    // 定义定时器配置结构体，并使用默认配置参数初始化结构体
    nrfx_timer_config_t timer_cfg = NRFX_TIMER_DEFAULT_CONFIG;
    // timer0 配置为计数模式
    timer_cfg.mode = NRF_TIMER_MODE_COUNTER;

    // 初始化定时器，定时器工作于计数模式时，没有事件，所以无需回调函数
    // 这里提供了一个空的函数进行传参，当然可以使用可以使用NULL
    err_code = nrfx_timer_init(&TIMER_COUNTER, &timer_cfg, timer_event_handler);
    // err_code = nrfx_timer_init(&TIMER_COUNTER, &timer_cfg, NULL);

    APP_ERROR_CHECK(err_code);
}

int main(void)
{
    int val;
    nrf_gpio_cfg_output(LED_1);
    nrf_gpio_pin_set(LED_1);
    timer_init();

    nrfx_timer_enable(&TIMER_COUNTER);
    while (true) {
        nrfx_timer_increment(&TIMER_COUNTER);

        val = nrfx_timer_capture(&TIMER_COUNTER, NRF_TIMER_CC_CHANNEL0);

        printf("counter value: %d\r\n", val);

        nrf_delay_ms(1000);
        nrf_gpio_pin_toggle(LED_1);
    }
}