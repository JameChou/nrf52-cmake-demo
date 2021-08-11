#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "nrf_log_default_backends.h"
#include "nrf_log_ctrl.h"
#include "nrf_log.h"
#include "boards.h"

#include "mpu6050.h"

static void log_init(void)
{
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);

    // 输出终端在sdk_config.h里配置的是RTT View
    NRF_LOG_DEFAULT_BACKENDS_INIT();
}

int main(void)
{
    static int16_t acc_value[3], gyro_value[3];

    bsp_board_init(BSP_INIT_LEDS | BSP_INIT_BUTTONS);
    log_init();

    // 初始化TWI
    twi_master_init();

    // 上电服务前延时，否则数据可能会出错
    nrf_delay_ms(2000);
    // 等待mpu6050初始化
    while (!mpu6050_init()) {
        NRF_LOG_INFO("mpu6050 init failed\r\n");
        nrf_delay_ms(800);
    }

    NRF_LOG_INFO("mpu6050 example started");

    bsp_board_leds_on();

    while (mpu_dmp_init()) {
        NRF_LOG_INFO("mpu6050 init error\r\n");
        nrf_delay_ms(1000);
    }

    bsp_board_leds_off();

    while (true) {
        if (MPU6050_ReadAcc(&acc_value[0], &acc_value[1], &acc_value[2])) {
            NRF_LOG_INFO("ACC:  %d  %d  %d ", acc_value[0], acc_value[1], acc_value[2]);
        } else {
            NRF_LOG_ERROR("Read ACC failed.");
        }

        if (MPU6050_ReadGyro(&gyro_value[0], &gyro_value[1], &gyro_value[2])) {
            NRF_LOG_INFO("GYRO: %d  %d  %d ", gyro_value[0], gyro_value[1], gyro_value[2]);
        } else {
            NRF_LOG_ERROR("Read GYRO failed.");
        }

        NRF_LOG_FLUSH();
    }
}
