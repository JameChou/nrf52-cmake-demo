#include <stdbool.h>
#include "nrf_delay.h"
#include "nrf_gpio.h"

#define LED1 17
#define LED2 18

int main(void)
{

    nrf_gpio_cfg_output(LED1);
    nrf_gpio_cfg_output(LED2);

    while (true)
    {
        nrf_gpio_pin_set(LED1);
        nrf_gpio_pin_clear(LED2);
        nrf_delay_ms(500);

        nrf_gpio_pin_clear(LED1);
        nrf_gpio_pin_set(LED2);

        nrf_delay_ms(500);
    }
}