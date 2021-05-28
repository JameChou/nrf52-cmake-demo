#include "app_error.h"
#include "nrf_pwr_mgmt.h"
#include "tri_app_power.h"

static void power_management_init(void)
{
    ret_code_t err_code;
    err_code = nrf_pwr_mgmt_init();
    APP_ERROR_CHECK(err_code);
}
