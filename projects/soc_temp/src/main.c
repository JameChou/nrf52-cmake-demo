#include <stdbool.h>
#include "nrf_temp.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "boards.h"

int main(void)
{
    int32_t volatile temp;

    bsp_board_init(BSP_INIT_LEDS);

    // 初始化温度传感器
    nrf_temp_init();

    while (true) {
        // 每次测量都需要触发一次START任务，启动温度测量
        NRF_TEMP->TASKS_START = 1;

        // 等待温度测量完成
        while (NRF_TEMP->EVENTS_DATARDY == 0);

        // 清零
        NRF_TEMP->EVENTS_DATARDY = 0;
        // 读取温度值
        temp = (nrf_temp_read() / 4);
        // 温度传感器模块的模拟电路在检测完成后不会自动关闭，需要手动关闭以节省功耗
        NRF_TEMP->TASKS_STOP = 1;

        printf("Temperature: %d\r\n", (int) temp);
        nrf_delay_ms(500);
        nrf_gpio_pin_toggle(LED_1);
    }
}