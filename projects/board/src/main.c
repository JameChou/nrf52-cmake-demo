#include <stdbool.h>
#include <stdio.h>
#include "boards.h"

int main(void)
{

    // init the board led configuration
    // the board def is in ../cmake/nRF5x.cmake
    /*
     *  add_definitions(-DNRF52 -DNRF52832 -DNRF52832_XXAA -DNRF52_PAN_74 -DNRF52_PAN_64 -DNRF52_PAN_12 -DNRF52_PAN_58 -DNRF52_PAN_54 -DNRF52_PAN_31 -DNRF52_PAN_51 -DNRF52_PAN_36 -DNRF52_PAN_15 -DNRF52_PAN_20 -DNRF52_PAN_55 -DBOARD_PCA10040)
     */
    bsp_board_init(BSP_INIT_LEDS);
    bsp_board_init(BSP_INIT_BUTTONS);

    printf("init the board buttons and leds. \r\n");

    while (true) {
        if (bsp_board_button_state_get(BSP_BOARD_BUTTON_0)) {
            printf("turn on the led 0. \r\n");
            bsp_board_led_on(BSP_BOARD_LED_0);

            while (bsp_board_button_state_get(BSP_BOARD_BUTTON_0));
            printf("turn off the led 0. \r\n");
            bsp_board_led_off(BSP_BOARD_LED_0);
        }
    }

}
