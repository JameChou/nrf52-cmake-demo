#include <stdbool.h>
#include "nrfx_timer.h"
#include "nrf_gpio.h"
#include "boards.h"

const nrfx_timer_t TIMER_LED = NRFX_TIMER_INSTANCE(0);

/**
 * 中断发生时，回调函数
 *
 * @param event_type
 * @param p_context
 */
void timer_led_event_handler(nrf_timer_event_t event_type, void* p_context)
{
    switch (event_type) {
        case NRF_TIMER_EVENT_COMPARE0:
            nrf_gpio_pin_toggle(LED_1);
            break;

        default:
            break;
    }
}

/**
 * 初始化timer -> instance 0
 */
void timer_init(void)
{
    uint32_t err_code = NRF_SUCCESS;
    uint32_t time_ms = 500; // 定义时间200ms会触发一次中断
    uint32_t time_ticks;

    // 定义定时器配置结构体，并使用默认配置参数初始化结构体
    nrfx_timer_config_t timer_cfg = NRFX_TIMER_DEFAULT_CONFIG;

    // 初始化定时器，初始化时会注册 timer_led_event_handler 事件回调函数
    err_code = nrfx_timer_init(&TIMER_LED, &timer_cfg, timer_led_event_handler);

    APP_ERROR_CHECK(err_code);

    // 定时时间(单位ms)转换为ticks
    time_ticks = nrf_timer_ms_to_ticks(time_ms, TIMER_DEFAULT_CONFIG_FREQUENCY);

    // 设置定时器捕获/比较通道及该通道的比较值，使能通道的比较中断
    nrfx_timer_extended_compare(&TIMER_LED, NRF_TIMER_CC_CHANNEL0, time_ticks, NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK, true);

}

int main(void)
{
    nrf_gpio_cfg_output(LED_1);
    nrf_gpio_pin_set(LED_1);

    timer_init();
    nrfx_timer_enable(&TIMER_LED);

    while (true) {}
}