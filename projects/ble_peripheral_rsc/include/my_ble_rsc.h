/**
 * BLE RSC服务相关头文件定义
 */
#ifndef BLE_RSC_H__
#define BLE_RSC_H__

#include <stdint.h>
#include <stdbool.h>
#include "ble.h"
#include "ble_srv_common.h"
#include "nrf_sdh_ble.h"
#include "nrf_ble_gatt.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BLE_RSC_DEF(_name)                                                              \
static ble_rsc_t _name;                                                                 \
NRF_SDH_BLE_OBSERVER(_name ## _obs,                                                     \
                     BLE_RSCS_BLE_OBSERVER_PRIO,                                        \
                     ble_rsc_on_ble_evt, &_name)


// 定义RR Interval缓存数量
#define BLE_RSC_MAX_BUFFERED_RR_INTERVALS     20

// RSC服务事件结构
typedef enum {
    BLE_RSC_EVT_NOTIFICATION_ENABLED,               // RSC服务通知使能事件
    BLE_RSC_EVT_NOTIFICATION_DISABLED               // RSC服务通知关闭事件
} ble_rsc_evt_type_t;

typedef struct ble_rsc_s ble_rsc_t;

typedef void (*ble_rsc_evt_handler_t) (ble_rsc_t * p_rsc, ble_rsc_evt_type_t * p_evt);


// RSC事件服务结构体
typedef struct {
    ble_rsc_evt_type_t evt_type;
} ble_rsc_evt_t;

typedef struct {
    // RSC服务事件回调函数，预留给用户使用
    ble_rsc_evt_handler_t       evt_handler;
    // 写CCCD(客户端特征描述符)时的安全需求
    security_req_t              rsc_cccd_wr_sec;
} ble_rsc_init_t;

// RSC服务结构体，包含RSC服务所需的所有选项和数据
struct ble_rsc_s {
    // RSC服务事件回调函数，预留给用户使用
    ble_rsc_evt_handler_t       evt_handler;
    // RSC服务句柄
    uint16_t                    service_handle;
    // RSC测量句柄
    ble_gatts_char_handles_t    rsc_handles;
    // RSC Control Point特征句柄
    ble_gatts_char_handles_t    rsccp_handles;
    // 连接句柄(由协议栈提供，如未建立连接，该值为BLE_CONN_HANDLE_INVALID
    uint16_t                    conn_handle;
    // 自上次RSC测试传输数据后的一组rr_interval
    uint16_t                    rr_interval[BLE_RSC_MAX_BUFFERED_RR_INTERVALS];
    // 自上次RSC测试传输后的rr_interval数量
    uint16_t                    rr_interval_count;
    // 当前一次可发送的最大心率测量数据长度，根据当前ATT MTU值调整
    uint8_t                     max_rsc_len;
};

/**
 * 初始化RSC服务
 *
 * @param p_rsc RSC服务的结构体
 * @param p_rsc_init 关于初始化服务的一些信息结构体
 * @return 当无错误时返回NRF_SUCCESS，如果有错误就返回相应的错误代码
 */
uint32_t ble_rsc_init(ble_rsc_t * p_rsc, ble_rsc_init_t const * p_rsc_init);

/**
 * handle gatt 模块事件
 *
 * 处理所有关于RSC的GATT事件
 *
 * @param p_rsc RSC服务结构体
 * @param p_gatt_evt 收到GATT模块的event
 */
void ble_rsc_on_gatt_evt(ble_rsc_t * p_rsc, nrf_ble_gatt_evt_t const * p_gatt_evt);

/**
 * 处理Application中的BLE栈发过来的events
 *
 * 处理所有BLE协议栈关于RSC的events
 *
 * @param p_ble_evt BLE协议栈发过来的events
 * @param p_context RSC服务结构体
 */
void ble_rsc_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context);

/**
 * 当订阅服务打开后，发送speed以及cadence数据
 *
 * @param p_rsc RSC服务结构体
 * @param speed 速度
 * @param cadence 步频
 * @return NRF_SUCCESS on success, otherwise an error code.
 */
uint32_t ble_rsc_measurement_send(ble_rsc_t * p_rsc, uint16_t speed, uint16_t cadence);

#ifdef __cplusplus
}
#endif

#endif // BLE_RSC_H__

/** @} */
