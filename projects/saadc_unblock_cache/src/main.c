#include <stdbool.h>
#include "nrf_delay.h"
#include "nrf_drv_saadc.h"
#include "nrfx_saadc.h"
#include "sdk_errors.h"
#include "boards.h"

// 这里定义了缓存的大小，则当缓存全部储存完成之后，会产生一次回调中断
#define SAMPLES_BUFFER_LEN 5

static nrf_saadc_value_t m_buffer_pool[2][SAMPLES_BUFFER_LEN];
// 保存进入事件回调的次数
static uint32_t m_adc_evt_counter;

void saadc_callback(nrf_drv_saadc_evt_t const * p_event)
{
    float val;

    // 表明事件的类型是SAADC已经处理完成
    if (p_event->type == NRFX_SAADC_EVT_DONE) {
        ret_code_t err_code;

        // 设置好缓存，为下一次采样准备
        err_code = nrfx_saadc_buffer_convert(p_event->data.done.p_buffer, SAMPLES_BUFFER_LEN);
        APP_ERROR_CHECK(err_code);

        int i;

        printf("\r\nADC event number: %d\r\n", (int) m_adc_evt_counter);

        for (i = 0; i < SAMPLES_BUFFER_LEN; i++) {
            printf("Samples value: %d    ", p_event->data.done.p_buffer[i]);

            val = p_event->data.done.p_buffer[i] * 3.6 / 1024;
            printf("Voltage = %.3fV\r\n", val);
        }

        m_adc_evt_counter++;
        printf("\r\n");
    }
}

void saadc_init(void)
{
    ret_code_t err_code;

    // 定义ADC通道配置结构体，并使用单端采样配置宏初始化
    // NRF_SAADC_INPUT_AIN2 是使用的模拟输入通道
    nrf_saadc_channel_config_t channel_config = NRFX_SAADC_DEFAULT_CHANNEL_CONFIG_SE(NRF_SAADC_INPUT_AIN2);

    // 初始化SAADC，注册事件回调函数
    err_code = nrf_drv_saadc_init(NULL, saadc_callback);
    APP_ERROR_CHECK(err_code);

    // 初始化SAADC通道0
    err_code = nrfx_saadc_channel_init(0, &channel_config);
    APP_ERROR_CHECK(err_code);


    // 配置缓存1，将缓存1地址赋值给SAADC驱动程序中的控制块m_cb的一级缓存指针
    err_code = nrfx_saadc_buffer_convert(m_buffer_pool[0], SAMPLES_BUFFER_LEN);
    APP_ERROR_CHECK(err_code);

    // 配置缓存2，将缓存2地址赋值给SAADC驱动程序中的控制块m_cb的二级缓存指针
    err_code = nrfx_saadc_buffer_convert(m_buffer_pool[1], SAMPLES_BUFFER_LEN);
    APP_ERROR_CHECK(err_code);

}

int main(void)
{
    bsp_board_init(BSP_INIT_LEDS);

    saadc_init();

    while (true) {
        // 启动一次ADC采样 500ms 采样一次，缓存长度配置的是5，所以是2.5s产生次中断，回调saadc_callback
        nrfx_saadc_sample();

        nrf_gpio_pin_toggle(LED_1);
        nrf_delay_ms(500);
    }
}