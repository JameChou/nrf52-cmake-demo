#include <stdbool.h>
#include "nrfx_timer.h"
#include "nrf_gpio.h"
#include "boards.h"

const nrfx_timer_t RSC_TIMER = NRFX_TIMER_INSTANCE(1);

/**
 * 中断发生时，回调函数
 *
 * @param event_type
 * @param p_context
 */
void rsc_timer_interrupts(nrf_timer_event_t event_type, void * p_context)
{
    switch (event_type) {
        case NRF_TIMER_EVENT_COMPARE0:
            nrf_gpio_pin_toggle(LED_3);
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
    ret_code_t  err_code;
    // 设置初始化timer1 使得timer per 1us a tick
    nrfx_timer_config_t timer_cfg = NRFX_TIMER_DEFAULT_CONFIG;
    // 1MHz
    timer_cfg.frequency = 4;
    err_code = nrfx_timer_init(&RSC_TIMER, &timer_cfg, rsc_timer_interrupts);
    APP_ERROR_CHECK(err_code);

    uint32_t duty_cycle_time_ticks;
    duty_cycle_time_ticks = nrfx_timer_ms_to_ticks(&RSC_TIMER, 1500);
    nrfx_timer_extended_compare(&RSC_TIMER, NRF_TIMER_CC_CHANNEL0, duty_cycle_time_ticks, NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK, true);

    nrfx_timer_enable(&RSC_TIMER);
    nrf_gpio_cfg_output(LED_3);
    nrf_gpio_pin_set(LED_3);
}

int main(void)
{


    timer_init();

    while (true) {}
}