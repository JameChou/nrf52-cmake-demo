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
    app_log_init();
    timers_init();
    leds_init();
    power_management_init();
    ble_stack_init();
    gap_params_init();
    gatt_init();
    advertising_init();
    services_init();
    conn_params_init();

    NRF_LOG_INFO("BLE template example started.");
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
