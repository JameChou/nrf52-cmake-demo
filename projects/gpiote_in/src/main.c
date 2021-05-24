#include <stdbool.h>
#include <sdk_errors.h>
#include <nrf_drv_gpiote.h>
#include <nrf_delay.h>
#include <boards.h>

void in_pin_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    if (pin == BUTTON_1) {
        nrf_gpio_pin_toggle(LED_1);
    }

    if (action == NRF_GPIOTE_POLARITY_HITOLO) nrf_gpio_pin_toggle(LED_2);
    else if (action == NRF_GPIOTE_POLARITY_LOTOHI) nrf_gpio_pin_toggle(LED_3);
    else if (action == NRF_GPIOTE_POLARITY_TOGGLE) nrf_gpio_pin_toggle(LED_4);
}

int main(void)
{
    bsp_board_init(BSP_INIT_LEDS);

    ret_code_t err_code;

    err_code = nrf_drv_gpiote_init();

    APP_ERROR_CHECK(err_code);

    // 以下代码配置P0.13 作为GPIOTE输入，并分别配置引脚上升沿，下降沿和任意电平变化产生事件
    // 从高电平到低电平变化产生事件->下降沿产生事件
    nrf_drv_gpiote_in_config_t in_config = NRFX_GPIOTE_CONFIG_IN_SENSE_HITOLO(true);

    // 低电平到高电平变化产生事件->上升沿产生事件
    // nrf_drv_gpiote_in_config_t in_config = NRFX_GPIOTE_CONFIG_IN_SENSE_LOTOHI(true);

    // 任意电平变化产生事件
    // nrf_drv_gpiote_in_config_t in_config = NRFX_GPIOTE_CONFIG_IN_SENSE_TOGGLE(true);

    // 开启P0.13引脚的上拉电阻
    in_config.pull = NRF_GPIO_PIN_PULLUP;

    err_code = nrf_drv_gpiote_in_init(BUTTON_1, &in_config, in_pin_handler);

    APP_ERROR_CHECK(err_code);

    nrf_drv_gpiote_in_event_enable(BUTTON_1, true);

    while (true) {}
}