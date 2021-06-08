//
// Created by James Chow on 2021/5/26.
// BLE操作相关协议栈处理
//
#ifndef BLE_APP_BLE_PERIPHERAL_BASIC_TRI_APP_BLE_H
#define BLE_APP_BLE_PERIPHERAL_BASIC_TRI_APP_BLE_H

#include "ble_advertising.h"
#include "ble_conn_params.h"

/**
 * 初始化BLE协议栈
 */
void ble_stack_init(void);

/**
 * 处理BLE回调函数
 *
 * @param p_ble_evt {@see ble_evt_t}
 * @param p_context
 */
void ble_evt_handler(ble_evt_t const * p_ble_evt, void * p_context);

/**
 * 初始化GATT程序模块
 */
void gatt_init(void);

/**
 * 连接参数协商模块错误处理事件，参数nrf_error包含了错误代码，通过nrf_error可以分析错误信息
 *
 * @param nrf_error
 */
void conn_params_error_handler(uint32_t nrf_error);

/**
 * 连接参数协商模块想事件处理函数
 *
 * @param p_evt
 */
void on_conn_params_evt(ble_conn_params_evt_t * p_evt);

/**
 * 连接参数协商模块初始化
 */
void conn_params_init(void);

/**
 * GAP 参数初始化，该函数配置需要的GAP参数，包括设备名称，外观特征，首选项连接参数
 */
void gap_params_init(void);

/**
 * 广播事件处理函数
 *
 * @param ble_adv_evt 广播事件EVENT
 */
void on_adv_evt(ble_adv_evt_t ble_adv_evt);

/**
 * 广播初始化函数
 */
void advertising_init(void);

/**
 * 广播开始
 */
void advertising_start(void);

void nrf_qwr_error_handler(uint32_t nrf_error);
void services_init(void);


#endif //BLE_APP_BLE_PERIPHERAL_BASIC_TRI_APP_BLE_H
