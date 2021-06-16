#ifndef BLE_APP_BLE_PERIPHERAL_BASIC_PROFILE_MY_BLE_HRS_H
#define BLE_APP_BLE_PERIPHERAL_BASIC_PROFILE_MY_BLE_HRS_H

#include <stdint.h>
#include <stdbool.h>

#include "ble.h"
#include "ble_srv_common.h"
#include "nrf_sdh_ble.h"
#include "nrf_ble_gatt.h"

#ifdef __cplusplus
extern "C" {
#endif

// 定义心率服务实例，该实例完成2件事
// 1. 定义了static类型心率服务结构体变量，为心率服务结构体分配了内存
// 2. 注册了BLE事件监视者，这使得心率程序模块可以接收BLE协议栈的事件，从而可以在ble_hrs_on_ble_evt()事件回调函数中处理自己感兴趣的事件
#define BLE_HRS_DEF(_name)                                                                          \
static ble_hrs_t _name;                                                                             \
NRF_SDH_BLE_OBSERVER(_name ## _obs,                                                                 \
                     BLE_HRS_BLE_OBSERVER_PRIO,                                                     \
                     ble_hrs_on_ble_evt, &_name)

// 传感器测点位置编码
#define BLE_HRS_BODY_SENSOR_LOCATION_OTHER      0       // 其他位置
#define BLE_HRS_BODY_SENSOR_LOCATION_CHEST      1       // 胸部
#define BLE_HRS_BODY_SENSOR_LOCATION_WRIST      2       // 手腕
#define BLE_HRS_BODY_SENSOR_LOCATION_FINGER     3       // 手指
#define BLE_HRS_BODY_SENSOR_LOCATION_HAND       4       // 手
#define BLE_HRS_BODY_SENSOR_LOCATION_EAR_LOBE   5       // 耳垂
#define BLE_HRS_BODY_SENSOR_LOCATION_FOOT       6       // 脚

// RR Interval 缓存数量
#define BLE_HRS_MAX_BUFFERED_RR_INTERVALS       20


// 心率服务事件类型
typedef enum
{
    BLE_HRS_EVT_NOTIFICATION_ENABLED,       // 心率值通知使能事件
    BLE_HRS_EVT_NOTIFICATION_DISABLED       // 心率值通知关闭事件
} ble_hrs_evt_type_t;


// 心率服务事件结构体
typedef struct
{
    ble_hrs_evt_type_t evt_type;            // 心率服务事件类型
} ble_hrs_evt_t;

// 重新命名心率服务结构体，即将ble_hrs_s命名为ble_hrs_t
typedef struct ble_hrs_s ble_hrs_t;

// 声明函数想指针变量，心率服务结构体中用其定义了函数指针变量预留给用户使用
typedef void (*ble_hrs_evt_handler_t) (ble_hrs_t * p_hrs, ble_hrs_evt_t * p_evt);


// 心论服务初始化结构体，包含初始化心率服务所需的所有选项和数据
typedef struct
{
    ble_hrs_evt_handler_t           evt_handler;                            // 心论服务事件回调函数，预留给用户使用，本例中没有使用，初始化时设置为NULL
    bool                            is_sensor_contact_supported;            // 传感器接触检测支持标志
    uint8_t *                       p_body_sensor_location;                 // 身体测量点位置
    security_req_t                  hrm_cccd_wr_sec;                        // 写CCCD(客记端特征描述符)时的安全需求
    security_req_t                  bsl_rd_sec;                             // 读BSL(传感器位置)特征值时的安全需求
} ble_hrs_init_t;

struct ble_hrs_s
{
    ble_hrs_evt_handler_t           evt_handler;                                        // 心论服务事件回调函数，预留给用户使用
    bool                            is_expended_energy_supported;                       // 能量消耗支持标志，true=支持能量消耗统计
    bool                            is_sensor_contact_supported;                        // 传感器接触检测支持标志，true=支持传感器接触检测
    uint16_t                        service_handle;                                     // 心率服务句柄(由协议栈提供)
    ble_gatts_char_handles_t        hrm_handles;                                        // 心率测量特征句柄
    ble_gatts_char_handles_t        bsl_handles;                                        // 传感器身体测点位置特征句柄
    ble_gatts_char_handles_t        hrcp_handles;                                       // Heart Rate Control Point 特征句柄
    uint16_t                        conn_handle;                                        // 连接句柄(由协议栈提供，如未建立连接，该值为BLE_CONN_HANDLE_INVALID)
    bool                            is_sensor_contact_detected;                         // 传感器接触标志，true=检测到传感器接触
    uint16_t                        rr_interval[BLE_HRS_MAX_BUFFERED_RR_INTERVALS];     // 自上次心率测试传输后的一组rr_interval
    uint16_t                        rr_interval_count;                                  // 自上次心率测试传输后的rr_interval数量
    uint8_t                         max_hrm_len;                                        // 当前一次可发送的最大心率测量数据长度，根据当前ATT MTU值调整
};



/**@brief Function for initializing the Heart Rate Service.
 *
 * @param[out]  p_hrs       Heart Rate Service structure. This structure will have to be supplied by
 *                          the application. It will be initialized by this function, and will later
 *                          be used to identify this particular service instance.
 * @param[in]   p_hrs_init  Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on successful initialization of service, otherwise an error code.
 */
uint32_t ble_hrs_init(ble_hrs_t * p_hrs, ble_hrs_init_t const * p_hrs_init);


/**@brief Function for handling the GATT module's events.
 *
 * @details Handles all events from the GATT module of interest to the Heart Rate Service.
 *
 * @param[in]   p_hrs      Heart Rate Service structure.
 * @param[in]   p_gatt_evt  Event received from the GATT module.
 */
void ble_hrs_on_gatt_evt(ble_hrs_t * p_hrs, nrf_ble_gatt_evt_t const * p_gatt_evt);


/**@brief Function for handling the Application's BLE Stack events.
 *
 * @details Handles all events from the BLE stack of interest to the Heart Rate Service.
 *
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 * @param[in]   p_context   Heart Rate Service structure.
 */
void ble_hrs_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context);


/**@brief Function for sending heart rate measurement if notification has been enabled.
 *
 * @details The application calls this function after having performed a heart rate measurement.
 *          If notification has been enabled, the heart rate measurement data is encoded and sent to
 *          the client.
 *
 * @param[in]   p_hrs                    Heart Rate Service structure.
 * @param[in]   heart_rate               New heart rate measurement.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
uint32_t ble_hrs_heart_rate_measurement_send(ble_hrs_t * p_hrs, uint16_t heart_rate);



/**@brief Function for setting the state of the Sensor Contact Detected bit.
 *
 * @param[in]   p_hrs                        Heart Rate Service structure.
 * @param[in]   is_sensor_contact_detected   TRUE if sensor contact is detected, FALSE otherwise.
 */
void ble_hrs_sensor_contact_detected_update(ble_hrs_t * p_hrs, bool is_sensor_contact_detected);



#ifdef __cplusplus
}
#endif

#endif
