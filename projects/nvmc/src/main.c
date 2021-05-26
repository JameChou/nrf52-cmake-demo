#include <stdbool.h>
#include <stdio.h>
#include "nrf_nvmc.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "boards.h"

int main(void)
{
    uint32_t addr;

    // 定义读取flash pointer
    uint32_t *pdat;

    bsp_board_init(BSP_INIT_LEDS);

    nrf_gpio_cfg_input(BUTTON_1, NRF_GPIO_PIN_PULLUP);

    while (true) {
        if (nrf_gpio_pin_read(BUTTON_1) == 0) {
            nrf_gpio_pin_clear(LED_1);

            while (nrf_gpio_pin_read(BUTTON_1));

            addr = 0x0007F000;

            // 擦除页
            nrf_nvmc_page_erase(addr);
            // 向地址0x0007F000写入一个字节数据 0x12345678
            nrf_nvmc_write_word(addr, 0x12345678);
            pdat = (uint32_t *)addr;

            printf("0x%x was read from flash\r\n", *pdat);
            nrf_gpio_pin_set(LED_1);
        }
    }
}