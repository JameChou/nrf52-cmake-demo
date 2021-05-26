#include <stdbool.h>
#include "nrf_delay.h"
#include "nrf_drv_saadc.h"
#include "nrfx_saadc.h"
#include "sdk_errors.h"
#include "boards.h"

void saadc_callback(nrf_drv_saadc_evt_t const * p_event)
{}

void saadc_init(void)
{
    ret_code_t err_code;

    /*
     * 定义ADC通道配置结构体，并使用差分采样配置宏初始化
     *
     * 因为要采样的是VDD和电位器抽头电压(模拟输入NRF_SAADC_INPUT_AIN2)的差值
     * 所以差分采样配置宏的通道正极为: NRF_SAADC_INPUT_VDD，通道负极为:
     * NRF_SAADC_INPUT_AIN2
     */
    nrf_saadc_channel_config_t channel_config = NRFX_SAADC_DEFAULT_CHANNEL_CONFIG_DIFFERENTIAL(NRF_SAADC_INPUT_VDD,
                                                                                               NRF_SAADC_INPUT_AIN2);

    // 初始化SAADC，注册回调函数
    err_code = nrf_drv_saadc_init(NULL, saadc_callback);
    APP_ERROR_CHECK(err_code);

    err_code = nrfx_saadc_channel_init(0, &channel_config);
    APP_ERROR_CHECK(err_code);
}

int main(void)
{
    nrf_saadc_value_t saadc_val;

    bsp_board_init(BSP_INIT_LEDS);

    saadc_init();

    while (true) {
        // 启动一次ADC采样(阻塞模式)
        nrfx_saadc_sample_convert(0, &saadc_val);
        printf("Sample value = %d\r\n", saadc_val);

        // 输出采样值
        printf("Voltage = %.3fV\r\n", saadc_val * 3.6 / 512);

        nrf_delay_ms(500);
        nrf_gpio_pin_toggle(LED_1);
    }
}