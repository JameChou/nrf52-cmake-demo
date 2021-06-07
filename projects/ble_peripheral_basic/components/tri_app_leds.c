#define USE_BOARD

#include "tri_app_leds.h"

#ifndef USE_BOARD

#include "nrf_gpio.h"

#define LED_1 17
#define LED_2 18
#define LED_3 19
#define LED_4 20

#else

#include "bsp.h"

#endif

void leds_init(void)
{
#if defined(USE_BOARD)
    ret_code_t err_code;
    err_code = bsp_init(BSP_INIT_LEDS, NULL);
    APP_ERROR_CHECK(err_code);
#else
    nrf_gpio_cfg_output(LED_1);
    nrf_gpio_cfg_output(LED_2);
    nrf_gpio_cfg_output(LED_3);
    nrf_gpio_cfg_output(LED_4);
#endif
}
