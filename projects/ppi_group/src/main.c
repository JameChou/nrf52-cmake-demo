#include <stdbool.h>
#include <stdio.h>
#include <nrf_drv_ppi.h>
#include "nrf_ppi.h"
#include "nrf_drv_gpiote.h"
#include "boards.h"

nrf_ppi_channel_t my_ppi_channel1;
nrf_ppi_channel_t my_ppi_channel2;
nrf_ppi_channel_group_t my_ppi_group;

void gpiote_init(void)
{
    ret_code_t err_code;

    // 初始化GPIOTE
    err_code = nrf_drv_gpiote_init();
    APP_ERROR_CHECK(err_code);

    nrf_drv_gpiote_out_config_t  out_config = NRFX_GPIOTE_CONFIG_OUT_TASK_TOGGLE(true);
    // 初始化GPIOTE输出引脚，初始化时会分配一个GPIOTE通道
    err_code = nrfx_gpiote_out_init(LED_1, &out_config);
    APP_ERROR_CHECK(err_code);
    // LED_1任务触发
    nrf_drv_gpiote_out_task_enable(LED_1);

    // 初始化LED_2 为GPIOTE输出，作为PPI的次级任务
    err_code = nrfx_gpiote_out_init(LED_2, &out_config);
    APP_ERROR_CHECK(err_code);
    nrf_drv_gpiote_out_task_enable(LED_2);

    // Button1 GPIOTE输入下降沿产生事件
    nrf_drv_gpiote_in_config_t  in_config = NRFX_GPIOTE_CONFIG_IN_SENSE_HITOLO(true);
    in_config.pull = NRF_GPIO_PIN_PULLUP;
    err_code = nrfx_gpiote_in_init(BUTTON_1, &in_config, NULL);
    APP_ERROR_CHECK(err_code);
    nrf_drv_gpiote_in_event_enable(BUTTON_1, true);

    // Button2 GPIOTE输入下降沿产生事件
    err_code = nrfx_gpiote_in_init(BUTTON_2, &in_config, NULL);
    APP_ERROR_CHECK(err_code);
    nrf_drv_gpiote_in_event_enable(BUTTON_2, true);
}

void ppi_config(void)
{
    uint32_t err_code = NRF_SUCCESS;

    // 初始化ppi 模块
    err_code = nrf_drv_ppi_init();
    APP_ERROR_CHECK(err_code);

    // 申请ppi通道，ppi通道的分配是由驱动函数完成，分配的通道号保存到 my_ppi_channel变量中
    err_code = nrfx_ppi_channel_alloc(&my_ppi_channel1);
    APP_ERROR_CHECK(err_code);
    // 设置PPI通道my_ppi_channel的EEP和TEP
    err_code = nrfx_ppi_channel_assign(my_ppi_channel1, nrfx_gpiote_in_event_addr_get(BUTTON_1),
                                       nrfx_gpiote_out_task_addr_get(LED_1));
    APP_ERROR_CHECK(err_code);
    // 使能PPI通道
    err_code = nrfx_ppi_channel_enable(my_ppi_channel1);
    APP_ERROR_CHECK(err_code);


    /*--------------------------channel 2----------------------------------*/
    err_code = nrfx_ppi_channel_alloc(&my_ppi_channel2);
    APP_ERROR_CHECK(err_code);
    err_code = nrfx_ppi_channel_assign(my_ppi_channel2, nrfx_gpiote_in_event_addr_get(BUTTON_2),
                                       nrfx_gpiote_out_task_addr_get(LED_2));
    APP_ERROR_CHECK(err_code);
    err_code = nrfx_ppi_channel_enable(my_ppi_channel2);
    APP_ERROR_CHECK(err_code);


    /*--------------------------PPI Group--------------------------------*/
    // 申请PPI组
    err_code = nrfx_ppi_group_alloc(&my_ppi_group);
    APP_ERROR_CHECK(err_code);

    // 将PPI channel1以及channel2加入到PPI组中
    err_code = nrfx_ppi_channel_include_in_group(my_ppi_channel1, my_ppi_group);
    APP_ERROR_CHECK(err_code);
    err_code = nrfx_ppi_channel_include_in_group(my_ppi_channel2, my_ppi_group);
    APP_ERROR_CHECK(err_code);

    // 使能PPI Group
    err_code = nrfx_ppi_group_enable(my_ppi_group);
    APP_ERROR_CHECK(err_code);
}

int main(void)
{
    gpiote_init();
    ppi_config();

    ret_code_t err_code;

    nrf_gpio_cfg_output(LED_3);
    nrf_gpio_cfg_output(LED_4);

    nrf_gpio_cfg_input(BUTTON_3, NRF_GPIO_PIN_PULLUP);
    nrf_gpio_cfg_input(BUTTON_4, NRF_GPIO_PIN_PULLUP);

    while (true) {
        // 按下BTN3时会将Group使能
        if (nrf_gpio_pin_read(BUTTON_3) == 0) {
            nrf_gpio_pin_clear(LED_3);
            nrf_gpio_pin_set(LED_4);

            while (nrf_gpio_pin_read(BUTTON_3) == 0);
            err_code = nrfx_ppi_group_enable(my_ppi_group);
            APP_ERROR_CHECK(err_code);
        }

        // 按下BTN4时会将Group Disable
        if (nrf_gpio_pin_read(BUTTON_4) == 0) {
            nrf_gpio_pin_set(LED_3);
            nrf_gpio_pin_clear(LED_4);

            while (nrf_gpio_pin_read(BUTTON_4) == 0);
            err_code = nrfx_ppi_group_disable(my_ppi_group);
            APP_ERROR_CHECK(err_code);
        }
    }
}