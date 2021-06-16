#ifndef BLE_APP_BLE_PERIPHERAL_BASIC_PROFILE_MY_BLE_DIS_H
#define BLE_APP_BLE_PERIPHERAL_BASIC_PROFILE_MY_BLE_DIS_H

#include <stdint.h>
#include "ble_srv_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BLE_DIS_VENDOR_ID_SRC_BLUETOOTH_SIG     1               // vendor ID assigned by Bluetooth SIG.
#define BLE_DIS_VENDOR_ID_SRC_USB_IMPL_FORUM    2               // Vendor Id assigned by USB Implementer's Forum.

// System ID parameters
typedef struct
{
    uint64_t manufacturer_id;                                   // Manufacturer ID. Only 5 LSOs shall be used.
    uint32_t organizationally_unique_id;                        // Organizationally unique ID. Only 3 LSOs shall be used.
} ble_dis_sys_id_t;

// IEEE 11073-20601 Regulatory Certification Data List Structure
typedef struct
{
    uint8_t *  p_list;                                          // Pointer the byte array containing the encoded opaque structure based on IEEE 11073-20601 specification.
    uint8_t    list_len;                                        // Length of the byte array.
} ble_dis_reg_cert_data_list_t;

// PnP ID parameters
typedef struct
{
    uint8_t  vendor_id_source;                                  // Vendor ID Source.
    uint16_t vendor_id;                                         // Vendor ID
    uint16_t product_id;                                        // Product ID
    uint16_t product_version;                                   // Product Version
} ble_dis_pnp_id_t;

// 设备信息服务初始化结构体，包含初始化设备信息服务所需的所有可能的特征
typedef struct
{
    ble_srv_utf8_str_t              manufact_name_str;           // 制造商名称字符串
    ble_srv_utf8_str_t              model_num_str;               // 型号字符串
    ble_srv_utf8_str_t              serial_num_str;              // 序列号字符串
    ble_srv_utf8_str_t              hw_rev_str;                  // 硬件版本字符串
    ble_srv_utf8_str_t              fw_rev_str;                  // 固件版本字符串
    ble_srv_utf8_str_t              sw_rev_str;                  // 软件版本字符串
    ble_dis_sys_id_t *              p_sys_id;                    // System ID
    ble_dis_reg_cert_data_list_t *  p_reg_cert_data_list;        // IEEE 11073-20601监管认证列表
    ble_dis_pnp_id_t *              p_pnp_id;                    // PnP ID
    security_req_t                  dis_char_rd_sec;             // 读取DIS特征值时的安全需求
} ble_dis_init_t;

/**@brief Function for initializing the Device Information Service.
 *
 * @details This call allows the application to initialize the device information service.
 *          It adds the DIS service and DIS characteristics to the database, using the initial
 *          values supplied through the p_dis_init parameter. Characteristics which are not to be
 *          added, shall be set to NULL in p_dis_init.
 *
 * @param[in]   p_dis_init   The structure containing the values of characteristics needed by the
 *                           service.
 *
 * @return      NRF_SUCCESS on successful initialization of service.
 */
uint32_t ble_dis_init(ble_dis_init_t const * p_dis_init);

#ifdef __cplusplus
}
#endif

#endif
