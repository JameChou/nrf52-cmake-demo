#include "sdk_common.h"
#if NRF_MODULE_ENABLED(BLE_MY_DIS)

#include "my_ble_dis.h"

#include <stdlib.h>
#include <string.h>
#include "app_error.h"
#include "ble_gatts.h"
#include "ble_srv_common.h"


// 定义uint16类型变量，用来保存服务句柄。添加服务成功后，协议栈会分配句柄保存到该变量，之后该句柄即可用来标志设备信息服务
static uint16_t                 service_handle;
static ble_gatts_char_handles_t manufact_name_handles;


/**@brief Function for adding the Characteristic.
 *
 * @param[in]   uuid           UUID of characteristic to be added.
 * @param[in]   p_char_value   Initial value of characteristic to be added.
 * @param[in]   char_len       Length of initial value. This will also be the maximum value.
 * @param[in]   rd_sec         Security requirement for reading characteristic value.
 * @param[out]  p_handles      Handles of new characteristic.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t char_add(uint16_t                         uuid,
                         uint8_t                        * p_char_value,
                         uint16_t                         char_len,
                         security_req_t const             rd_sec,
                         ble_gatts_char_handles_t       * p_handles)
{
    ble_add_char_params_t add_char_params;                  // 定义特征参数结构体变量
    APP_ERROR_CHECK_BOOL(p_char_value != NULL);             // 检查指向制造商名称字符串的指针是否有效
    APP_ERROR_CHECK_BOOL(char_len > 0);                     // 检查制造商名称字符是否大于0

    memset(&add_char_params, 0, sizeof(add_char_params));   // 初始化参数之前，先清零add_char_params

    add_char_params.uuid                    = uuid;             // 制造商名称字符串特征UUID
    add_char_params.max_len                 = char_len;         // 设置制造商名称字符串特征值的最大长度
    add_char_params.init_len                = char_len;         // 设置制造商名称字符串特征值的初始长度
    add_char_params.p_init_value            = p_char_value;     // 设置制造商名称字符串特征值的初始值
    add_char_params.char_props.read         = 1;                // 设置制造商名称字符串特征的性质：支持读取
    add_char_params.read_access             = rd_sec;

    // 向设备信息服务中添加制造商名称字符串特征
    return characteristic_add(service_handle, &add_char_params, p_handles);
}


// 初始化设备信息服务
uint32_t ble_dis_init(ble_dis_init_t const * p_dis_init)
{
    uint32_t    err_code;

    // 定义UUID结构体变量
    ble_uuid_t  ble_uuid;

    // 设备信息服务的UUID
    BLE_UUID_BLE_ASSIGN(ble_uuid, BLE_UUID_DEVICE_INFORMATION_SERVICE);
    // 添加服务到属性列表
    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &ble_uuid, &service_handle);

    if (err_code != NRF_SUCCESS) {
        return err_code;
    }

    if (p_dis_init->manufact_name_str.length > 0) {
        // 添加制造商名称字符串特征
        err_code = char_add(BLE_UUID_MANUFACTURER_NAME_STRING_CHAR,
                            p_dis_init->manufact_name_str.p_str,
                            p_dis_init->manufact_name_str.length,
                            p_dis_init->dis_char_rd_sec,
                            &manufact_name_handles);

        if (err_code != NRF_SUCCESS) {
            return err_code;
        }
    }

    return NRF_SUCCESS;
}
#endif