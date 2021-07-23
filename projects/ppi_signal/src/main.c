#include <stdbool.h>
#include <stdio.h>
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrf_log.h"
#include "nrfx_ppi.h"
#include "nrf_drv_gpiote.h"
#include "boards.h"
#include "nrf_drv_timer.h"

// 第一组ppi，处理第一次信号下降沿事件
nrf_ppi_channel_t ppi_channel_1;
// 第二组ppi，处理第二个传感器下降沿事件
nrf_ppi_channel_t ppi_channel_2;

#define SENSOR_PORT_1 05
#define SENSOR_PORT_2 03

bool is_complete = false;

const nrfx_timer_t RSC_TIMER = NRFX_TIMER_INSTANCE(1);

void sensor_1_in_gpiote_interrupt_handler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action);
void sensor_2_in_gpiote_interrupt_handler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action);
void rsc_timer_interrupts(nrf_timer_event_t event_type, void * p_context);

/**
 * ppi初始化数据
 */
void ppi_config(void)
{
    ret_code_t err_code = NRF_SUCCESS;

    uint32_t   timer_stop_task_addr;
    uint32_t   gpiote_sensor_1_in_event_addr;
    uint32_t   gpiote_sensor_2_in_event_addr;

    if (!nrfx_gpiote_is_init()) {
        err_code = nrfx_gpiote_init();
        APP_ERROR_CHECK(err_code);
    }

    // 下降沿触发
    nrfx_gpiote_in_config_t sensor_in_config = NRFX_GPIOTE_CONFIG_IN_SENSE_HITOLO(true);
    sensor_in_config.pull = NRF_GPIO_PIN_NOPULL;

    err_code = nrfx_gpiote_in_init(SENSOR_PORT_1, &sensor_in_config, sensor_1_in_gpiote_interrupt_handler);
    APP_ERROR_CHECK(err_code);

    // 使能sensor 1下降沿感知功能
    nrfx_gpiote_in_event_enable(SENSOR_PORT_1, true);

    err_code = nrfx_gpiote_in_init(SENSOR_PORT_2, &sensor_in_config, sensor_2_in_gpiote_interrupt_handler);
    nrfx_gpiote_in_event_enable(SENSOR_PORT_2, true);

    // 设置初始化timer1 使得timer per 1us a tick
    nrfx_timer_config_t timer_cfg = NRFX_TIMER_DEFAULT_CONFIG;
    // 1MHz
    timer_cfg.frequency = 4;
    err_code = nrfx_timer_init(&RSC_TIMER, &timer_cfg, rsc_timer_interrupts);
    APP_ERROR_CHECK(err_code);

    uint32_t duty_cycle_time_ticks;
    duty_cycle_time_ticks = nrfx_timer_ms_to_ticks(&RSC_TIMER, 1500);
    nrfx_timer_extended_compare(&RSC_TIMER, NRF_TIMER_CC_CHANNEL0, duty_cycle_time_ticks, NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK, true);

    timer_stop_task_addr = nrfx_timer_task_address_get(&RSC_TIMER, NRF_TIMER_TASK_STOP);
    gpiote_sensor_1_in_event_addr = nrfx_gpiote_in_event_addr_get(SENSOR_PORT_1);
    gpiote_sensor_2_in_event_addr = nrfx_gpiote_in_event_addr_get(SENSOR_PORT_2);

    err_code = nrfx_ppi_channel_alloc(&ppi_channel_1);
    APP_ERROR_CHECK(err_code);

    err_code = nrfx_ppi_channel_alloc(&ppi_channel_2);
    APP_ERROR_CHECK(err_code);

    err_code = nrfx_ppi_channel_assign(ppi_channel_1, gpiote_sensor_1_in_event_addr, timer_stop_task_addr);
    APP_ERROR_CHECK(err_code);
    err_code = nrfx_ppi_channel_assign(ppi_channel_2, gpiote_sensor_2_in_event_addr, timer_stop_task_addr);
    APP_ERROR_CHECK(err_code);

    // 使能timer
    nrfx_timer_enable(&RSC_TIMER);
    nrfx_timer_pause(&RSC_TIMER);
    nrfx_timer_clear(&RSC_TIMER);
}

void sensor_1_in_gpiote_interrupt_handler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    // sensor1 开始处理
    if (is_complete) {
        is_complete = false;

        nrfx_timer_clear(&RSC_TIMER);
        nrfx_timer_resume(&RSC_TIMER);
    }
}

void sensor_2_in_gpiote_interrupt_handler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    is_complete = true;
    NRF_TIMER1->TASKS_CAPTURE[1] = 1;
    uint32_t pulse_width_us = NRF_TIMER1->CC[1];
    nrfx_timer_pause(&RSC_TIMER);
    NRF_LOG_INFO("width us is %u", pulse_width_us);
}

void rsc_timer_interrupts(nrf_timer_event_t event_type, void * p_context)
{
}

int main(void)
{
    //初始化log程序模块
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);
    //设置log输出终端（根据sdk_config.h中的配置设置输出终端为UART或者RTT）
    NRF_LOG_DEFAULT_BACKENDS_INIT();

    NRF_LOG_INFO("start application");
    ppi_config();
    while (true) {
    }
}

