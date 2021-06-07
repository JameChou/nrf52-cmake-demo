#include <stdbool.h>
#include "nrf_pwr_mgmt.h"
#include "nrf_log_ctrl.h"
#include "nrf_log.h"
#include "tri_app_log.h"
#include "tri_app_ble.h"
#include "tri_app_leds.h"
#include "tri_app_power.h"
#include "tri_app_timer.h"

static void idle_state_handle();

int main(void)
{
    app_log_init();                                 // 初始化log模块
    timers_init();                                  // 初始化APP定时器
    leds_init();                                    // 初始化按键和指示灯
    power_management_init();                        // 初始化电源管理
    ble_stack_init();                               // 初始化协议栈
    gap_params_init();                              // 初始化GAP参数
    gatt_init();                                    // 初始化GATT
    advertising_init();                             // 初始化广播
    services_init();                                // 初始化QWR服务
    conn_params_init();                             // 连接参数初始化

    NRF_LOG_INFO("BLE template example started.");
    // 启动广播
    advertising_start();

    while (true) {
        idle_state_handle();
    }
}

static void idle_state_handle()
{
    if (NRF_LOG_PROCESS() == false) {
        nrf_pwr_mgmt_run();
    }
}
