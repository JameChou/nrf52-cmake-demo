#include <stdbool.h>
#include <sdk_errors.h>
#include <nrf_drv_gpiote.h>
#include <nrf_delay.h>

#define LED_1 17

// 如果定义了USE_AS_GPIO就是表明使用传统方式来操作GPIO
#define USE_AS_GPIO

int main(void)
{
    ret_code_t err_code;
    err_code = nrf_drv_gpiote_init();
    APP_ERROR_CHECK(err_code);

    nrf_drv_gpiote_out_config_t  config = GPIOTE_CONFIG_OUT_TASK_TOGGLE(true);

    err_code = nrf_drv_gpiote_out_init(LED_1, &config);

    APP_ERROR_CHECK(err_code);

#if !defined(USE_AS_GPIO)
    nrf_drv_gpiote_out_task_enable(LED_1);
#endif
    while (true) {
#if !defined(USE_AS_GPIO)
        // 任务触发引脚状态翻转
        nrf_drv_gpiote_out_task_trigger(LED_1);
        nrf_delay_ms(150);
#else
        nrfx_gpiote_out_set(LED_1);
        nrf_delay_ms(500);
        nrfx_gpiote_out_clear(LED_1);
        nrf_delay_ms(500);
#endif
    }
}