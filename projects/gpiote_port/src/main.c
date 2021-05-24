#include <stdbool.h>
#include <sdk_errors.h>
#include <nrf_drv_gpiote.h>
#include <nrf_delay.h>
#include <boards.h>

#define BUTTON_TOUCH 3

void in_pin_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    switch (pin) {
        case BUTTON_1:
            nrf_gpio_pin_toggle(LED_1);
            break;
        case BUTTON_2:
            nrf_gpio_pin_toggle(LED_2);
            break;
        case BUTTON_3:
            nrf_gpio_pin_toggle(LED_3);
            break;
        case BUTTON_4:
            nrf_gpio_pin_toggle(LED_4);
            break;
        case BUTTON_TOUCH:
            nrf_gpio_pin_toggle(LED_1);
            nrf_gpio_pin_toggle(LED_2);
            nrf_gpio_pin_toggle(LED_3);
            nrf_gpio_pin_toggle(LED_4);
            break;
    }
}

/**
 * nRF52832的GPIOTE共8个通道，可以检测8路输入，如果我们需要检测的输入超过8路，使用GPIOTE是无法实现的，
 * 这时我们可以使用GPIOTE的PORT来完成检测。PORT事件是整个P0端口32个IO共有的事件，所以使用PORT最多可以检测
 * 32路引脚输入变化
 *
 *
 * PORT事件是32个IO共有的事件，所以PORT事件产生后，需要查询是哪一个引脚产生的。
 *
 * PORT的时钟源来自于32.768KHz低频时钟，所以它的功耗更低，但是反应速度没有GPIOTE通道快速。
 *
 * @return
 */
int main(void)
{
    bsp_board_init(BSP_INIT_LEDS);

    ret_code_t err_code;

    // 初始化GPIOTE模块
    err_code = nrf_drv_gpiote_init();

    APP_ERROR_CHECK(err_code);

    // 定义GPIOTE配置结构体，配置为下降沿触发(按键改为低电平有效)，低精度
    // 对比GPIOTE_IN这个项目里的配置，如果使用GPIOTE通道，则这里传参是 "true"
    nrf_drv_gpiote_in_config_t in_config_hitolo = GPIOTE_CONFIG_IN_SENSE_HITOLO(false);

    // 开启引脚的上拉电阻
    in_config_hitolo.pull = NRF_GPIO_PIN_PULLUP;

    err_code = nrf_drv_gpiote_in_init(BUTTON_1, &in_config_hitolo, in_pin_handler);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_gpiote_in_init(BUTTON_2, &in_config_hitolo, in_pin_handler);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_gpiote_in_init(BUTTON_3, &in_config_hitolo, in_pin_handler);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_gpiote_in_init(BUTTON_4, &in_config_hitolo, in_pin_handler);
    APP_ERROR_CHECK(err_code);

    // 配置触摸按键config -> 上升沿时触发
    // 在sdk_config.h里 #define GPIOTE_CONFIG_NUM_OF_LOW_POWER_EVENTS 6
    // <a href="https://devzone.nordicsemi.com/f/nordic-q-a/35883/error-4-no-memory-for-operation-with-gpiote"></a>
    // 如果配置上述最多可以处理几个事件，当GPIOTE事件高于这个配置时这里的error_code就会抛出4
    // error_code为4表示没有多余的内存处理此请求
    nrf_drv_gpiote_in_config_t in_config_lotohi = GPIOTE_CONFIG_IN_SENSE_LOTOHI(false);
    in_config_lotohi.pull = NRF_GPIO_PIN_PULLUP;

    err_code = nrf_drv_gpiote_in_init(BUTTON_TOUCH, &in_config_lotohi, in_pin_handler);
    APP_ERROR_CHECK(err_code);

    // 开启引脚感知功能
    nrf_drv_gpiote_in_event_enable(BUTTON_1, true);
    nrf_drv_gpiote_in_event_enable(BUTTON_2, true);
    nrf_drv_gpiote_in_event_enable(BUTTON_3, true);
    nrf_drv_gpiote_in_event_enable(BUTTON_4, true);
    nrf_drv_gpiote_in_event_enable(BUTTON_TOUCH, true);


    while (true) {}
}