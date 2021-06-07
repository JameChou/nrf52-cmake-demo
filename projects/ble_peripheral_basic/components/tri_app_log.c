#include "tri_app_log.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

void app_log_init(void)
{
    // 初始化log程序模块
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);

    // 设置log输出终端伙(根据sdk_config.h中的配置设置输出终端为UART或者RTT)
    NRF_LOG_DEFAULT_BACKENDS_INIT();
}
