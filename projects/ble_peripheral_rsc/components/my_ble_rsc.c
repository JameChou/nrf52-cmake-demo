#include "sdk_common.h"
#if NRF_MODULE_ENABLED(BLE_MY_RSC)
#include "my_ble_rsc.h"
#include <string.h>
#include <nrf_log.h>
#include "ble_srv_common.h"

//操作码长度：1个字节
#define OPCODE_LENGTH 1
//句柄长度：2个字节
#define HANDLE_LENGTH 2
//BLE一次能传输的RSC量的最大字节数，根据BLE内核协议，MTU（最大传输单元）包含：1字节Opcode + 2字节属性handle + 有效载荷
#define MAX_RSC_LEN      (NRF_SDH_BLE_GATT_MAX_MTU_SIZE - OPCODE_LENGTH - HANDLE_LENGTH)
//RSC测量中的速度初始值
#define INITIAL_VALUE_SPEED                      0
// RSC测量中的步频初始值
#define INITIAL_VALUE_CADENCE                    0

// 是否支持瞬间最大步幅数据测量
#define RSC_FLAG_INSTANTANEOUS_STRIDE_LENGTH     (0x01 << 0)
// 是否支持总距离数量测量
#define RSC_FLAG_TOTAL_DISTANCE                  (0x01 << 1)
// 现在运动的状态 0 -> 走路 1 -> 跑步
#define RSC_FLAG_IS_RUNNING                      (0x01 << 2)


/**
 * handling th connect event
 *
 * @param p_rsc RSC Service structure
 * @param p_ble_evt Event received from the BLE stack.
 */
static void on_connect(ble_rsc_t * p_rsc, ble_evt_t const * p_ble_evt)
{
    p_rsc->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
}

/**
 * Handling the disconnect event.
 *
 * @param p_rsc RSC service structure.
 * @param p_ble_evt Event received from the BLE stack.
 */
static void on_disconnect(ble_rsc_t * p_rsc, ble_evt_t const * p_ble_evt)
{
    //UNUSED_PARAMETER(ble_evt_t);
    // 当断开连接之后这里应该将处理这个事件的句柄设置成为BLE_CONN_HANDLE_INVALID
    p_rsc->conn_handle = BLE_CONN_HANDLE_INVALID;
}

/**
 * Function for handling write events to the RSC measurement characteristic.
 *
 * @param p_rsc RSC service structure.
 * @param p_evt_write Write event received from the BLE stack.
 */
static void on_rsc_cccd_write(ble_rsc_t * p_rsc, ble_gatts_evt_write_t const * p_evt_write)
{
    if (p_evt_write->len == 2) {
        if (p_rsc->evt_handler != NULL) {
            ble_rsc_evt_t evt;

            if (ble_srv_is_notification_enabled(p_evt_write->data)) {
                evt.evt_type = BLE_RSC_EVT_NOTIFICATION_ENABLED;
            } else {
                evt.evt_type = BLE_RSC_EVT_NOTIFICATION_DISABLED;
            }

            p_rsc->evt_handler(p_rsc, &evt);
        }
    }
}

/**
 * Function for handling the write event.
 *
 * @param p_rsc
 * @param p_ble_evt
 */
static void on_write(ble_rsc_t * p_rsc, ble_evt_t const * p_ble_evt)
{
    ble_gatts_evt_write_t  const * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;

    if (p_evt_write->handle == p_rsc->rsc_handles.cccd_handle) {
        on_rsc_cccd_write(p_rsc, p_evt_write);
    }
}

void ble_rsc_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context)
{
    ble_rsc_t * p_rsc = (ble_rsc_t *) p_context;

    switch (p_ble_evt->header.evt_id) {
        case BLE_GAP_EVT_CONNECTED:
            on_connect(p_rsc, p_ble_evt);
            break;
        case BLE_GAP_EVT_DISCONNECTED:
            on_disconnect(p_rsc, p_ble_evt);
            break;
        case BLE_GATTS_EVT_WRITE:
            on_write(p_rsc, p_ble_evt);
            break;
        default:
            break;
    }
}

uint32_t ble_rsc_init(ble_rsc_t * p_rsc, const ble_rsc_init_t * p_rsc_init)
{
    uint32_t              err_code;
    ble_uuid_t            ble_uuid;
    // 定义特征参数结构体变量
    ble_add_char_params_t add_char_params;
    uint8_t               initial_rsc[9];

    p_rsc->evt_handler                  = p_rsc_init->evt_handler;
    p_rsc->conn_handle                  = BLE_CONN_HANDLE_INVALID;
    p_rsc->rr_interval_count            = 0;
    p_rsc->max_rsc_len                  = MAX_RSC_LEN;

    // RSC服务的UUID
    BLE_UUID_BLE_ASSIGN(ble_uuid, BLE_UUID_RUNNING_SPEED_AND_CADENCE);

    // 添加服务到属性列表
    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
                                        &ble_uuid,
                                        &p_rsc->service_handle);

    if (err_code != NRF_SUCCESS) {
        return err_code;
    }

    // 加入RSC测量特征
    memset(&add_char_params, 0, sizeof(add_char_params));
    add_char_params.uuid               = BLE_UUID_RSC_MEASUREMENT_CHAR;
    add_char_params.max_len            = MAX_RSC_LEN;
    add_char_params.init_len           = 0;
    add_char_params.p_init_value       = initial_rsc;
    add_char_params.is_var_len         = true;

    // 设置传感器位置特征性质:支持通知
    add_char_params.char_props.notify  = 1;
    add_char_params.cccd_write_access  = p_rsc_init->rsc_cccd_wr_sec;

    err_code = characteristic_add(p_rsc->service_handle, &add_char_params, &(p_rsc->rsc_handles));

    if (err_code != NRF_SUCCESS) {
        return err_code;
    }

    return NRF_SUCCESS;
}

// 发送速度数据以及步频数据
uint32_t ble_rsc_measurement_send(ble_rsc_t * p_rsc, double speed, uint16_t cadence)
{
    uint32_t err_code;
    uint8_t flags = 0x00;

    if (p_rsc->conn_handle != BLE_CONN_HANDLE_INVALID) {
        uint8_t                 rsc_data[1+2+1];
        uint16_t                hvx_len;
        ble_gatts_hvx_params_t  hvx_params;
        // 当大于三公里每小时时
        if (speed > 3) {
            flags |= RSC_FLAG_IS_RUNNING;
        }

        uint16_t speed_m_s_256 = (speed * 1000 / 3600) * 256;

        NRF_LOG_INFO("The speed value is: %f", speed);

        rsc_data[0] = flags;
        rsc_data[1] = speed_m_s_256;
        rsc_data[2] = speed_m_s_256 >> 8;
        rsc_data[3] = cadence;


        hvx_len = 1 + 2 + 1;
        memset(&hvx_params, 0, sizeof(hvx_params));
        // RSC测量特征句柄
        hvx_params.handle = p_rsc->rsc_handles.value_handle;
        hvx_params.type = BLE_GATT_HVX_NOTIFICATION;
        hvx_params.offset = 0;
        hvx_params.p_len = &hvx_len;
        hvx_params.p_data = rsc_data;

        err_code = sd_ble_gatts_hvx(p_rsc->conn_handle, & hvx_params);

    } else {
        err_code = NRF_ERROR_INVALID_STATE;
    }

    return err_code;

}

#endif // NRF_MODULE_ENABLED(BLE_MY_RSC)
