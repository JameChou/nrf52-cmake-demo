#include <stdbool.h>
#include <stdio.h>
#include "nrf_delay.h"
#include "nrf_drv_clock.h"
#include "nrfx_wdt.h"
#include "boards.h"

// 定义，保存向系统申请的喂狗通道
nrfx_wdt_channel_id m_channel_id;

void wdt_event_handler(void)
{
    // WDT事件处理

    /*
     * 点亮4个LED，因为最长只有2个32.768KHz时钟周期时间，所以程序运行后我们会看到4个LED微弱的闪烁一次，也就是4个LED刚点亮之后，系统就会RESET,
     * 当系统RESET之后，LED就会熄灭
     */
    bsp_board_leds_on();
}

void wdt_init(void)
{
    uint32_t err_code = NRF_SUCCESS;

    // 使用默认的结构体
    nrfx_wdt_config_t config = NRFX_WDT_DEAFULT_CONFIG;
    // 初始化WDT
    err_code = nrfx_wdt_init(&config, wdt_event_handler);
    APP_ERROR_CHECK(err_code);

    // 申请喂狗通道
    err_code = nrfx_wdt_channel_alloc(&m_channel_id);
    APP_ERROR_CHECK(err_code);

    // 启动WDT
    nrfx_wdt_enable();
}

/**
 * 根据sdk_config中的配置来说 2000ms WDT会启动一次
 * 如果在这个期间进行喂狗操作之后，则不会触发WDT事件，系统也不会REST
 *
 * @return
 */
int main(void)
{
    uint8_t i;
    uint32_t err_code = NRF_SUCCESS;

    // 配置BTN1 pin
    nrf_gpio_cfg_input(BUTTON_1, NRF_GPIO_PIN_PULLUP);

    // 配置32.768KHz低频时钟，使用外部32.768KHz晶体。如果不配置低频时钟的话，WDT会强制打开内部低频RC时钟
    err_code = nrf_drv_clock_init();
    APP_ERROR_CHECK(err_code);

    nrf_drv_clock_lfclk_request(NULL);

    bsp_board_init(BSP_INIT_LEDS);

    // 闪烁指示灯D1表示系统启动
    for (i = 0; i < 8; i++) {
        bsp_board_led_invert(0);
        nrf_delay_ms(150);
    }

    bsp_board_leds_off();

    wdt_init();

    while (true) {
        if (nrf_gpio_pin_read(BUTTON_1) == 0) {
            nrf_gpio_pin_clear(LED_2);

            // 喂狗操作
            nrfx_wdt_channel_feed(m_channel_id);

            while (nrf_gpio_pin_read(BUTTON_1) == 0);

            nrf_gpio_pin_set(LED_2);
        }
    }
}