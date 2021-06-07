#include "nrf_ble_qwr.h"
#include "app_error.h"
#include "tri_app_qwr.h"

void nrf_qwr_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}

void services_init(void)
{
    ret_code_t err_code;

    // 定义排队写入初始化结构体变量
    nrf_ble_qwr_init_t qwr_init = {0};

    // 排队写入事件处理函数
    qwr_init.error_handler = nrf_qwr_error_handler;
    APP_ERROR_CHECK(err_code);
}
