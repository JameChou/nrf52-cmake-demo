#include "ble_conn_params.h"
#include "ble_advertising.h"
#include "nrf_ble_qwr.h"
#include "nrf_ble_gatt.h"
#include "nrf_log.h"
#include "nrf_sdh_ble.h"
#include "app_error.h"
#include "nrf_sdh.h"
#include "bsp_btn_ble.h"
#include "tri_app_ble.h"
#include "app_timer.h"

#define DEVICE_NAME                     "James' BLE"                        // 设备名称
#define FIRST_CONN_PARAMS_UPDATE_DELAY  APP_TIMER_TICKS(5000)               // 定义首次调用sd_ble_gap_conn_param_update()函数更新连接参数延迟时间（5秒)
#define NEXT_CONN_PARAMS_UPDATE_DELAY   APP_TIMER_TICKS(30000)              // 定义每次调用sd_ble_gap_conn_param_update()函数更新连接参数的间隔时间（30秒)
#define MAX_CONN_PARAMS_UPDATE_COUNT    3                                   // 定义放弃连接参数协商前尝试连接参数协商的最大次数（3次）
#define MIN_CONN_INTERVAL               MSEC_TO_UNITS(100, UNIT_1_25_MS)    // 最小连接间隔(0.1s)
#define MAX_CONN_INTERVAL               MSEC_TO_UNITS(200, UNIT_1_25_MS)    // 最大连接间隔(0.2s)
#define SLAVE_LATENCY                   0                                   // 从机延迟
#define CONN_SUP_TIMEOUT                MSEC_TO_UNITS(4000, UNIT_10_MS)     // 监督超时(4s)

// BLE设置相关
#define APP_BLE_CONN_CFG_TAG            1                                   // SoftDevice BLE配置标志
#define APP_BLE_OBSERVER_PRIO           3                                   // 应用程序BLE事件监视者优先级，应用程序不能修改该数值

#define APP_ADV_INTERVAL                300                                 // 广播间隔(187.5ms)，单位0.625ms
#define APP_ADV_DURATION                0                                   // 广播持续时间，单位:10ms。设置为0表示不超时

uint16_t m_conn_handle = BLE_CONN_HANDLE_INVALID;

NRF_BLE_GATT_DEF(m_gatt);
NRF_BLE_QWR_DEF(m_qwr);
BLE_ADVERTISING_DEF(m_advertising);

static void ble_stack_init(void)
{
    ret_code_t err_code;
    // 请求使能SoftDevice，该函数会根据sdk_config.h文件中低频时钟的设置来配置低频时钟
    err_code = nrf_sdh_enable_request();
    APP_ERROR_CHECK(err_code);

    // 定义保存应用程序RAM起始地址的变量
    uint32_t  ram_start = 0;

    err_code = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start);
    APP_ERROR_CHECK(err_code);

    // 注册BLE事件回调函数
    NRF_SDH_BLE_OBSERVER(m_observer, APP_BLE_OBSERVER_PRIO, ble_evt_handler, NULL);
}

static void ble_evt_handler(ble_evt_t const * p_ble_evt, void * p_context)
{
    ret_code_t err_code = NRF_SUCCESS;

    // 判断BLE事件类型，根据事件类型执行相应的操作
    switch (p_ble_evt->header.evt_id) {

        // 断开事件
        case BLE_GAP_EVT_DISCONNECTED:
            NRF_LOG_INFO("Disconnected.");
            break;

        // 连接事件
        case BLE_GAP_EVT_CONNECTED:
            NRF_LOG_INFO("Connected.")
            // 设置指示灯状态为连接状态->指示灯D1常亮
            err_code = bsp_indication_set(BSP_INDICATE_CONNECTED);
            APP_ERROR_CHECK(err_code);
            // 保存连接句柄
            m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
            // 将连接句柄分配给队写入实例，分配后写入实倒和该连接关联，这样，当有多个连接的时候，通过关联不同的排队写入实例，很方便单独处理各个连接
            err_code = nrf_ble_qwr_conn_handle_assign(&m_qwr, m_conn_handle);
            APP_ERROR_CHECK(err_code);
            break;

        // PHY更新事件
        case BLE_GAP_EVT_PHY_UPDATE_REQUEST:
        {
            NRF_LOG_DEBUG("PHY update request.");
            ble_gap_phys_t  const phys = {
                    .rx_phys = BLE_GAP_PHY_AUTO,
                    .tx_phys = BLE_GAP_PHY_AUTO,
            };

            err_code = sd_ble_gap_phy_update(p_ble_evt->evt.gap_evt.conn_handle, &phys);
            APP_ERROR_CHECK(err_code);
        } break;

        // GATT客户端超时事件
        case BLE_GATTC_EVT_TIMEOUT:
            NRF_LOG_DEBUG("GATT Client Timeout.");
            // 断开当前连接
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;

        // GATT 服务器超时事件
        case BLE_GATTS_EVT_TIMEOUT:
            NRF_LOG_DEBUG("GATT Server Timeout.");
            // 断开当前连接
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;

        default:
            break;

    }
}

static void gatt_init(void)
{
    // 初始化GATT程序模块
    ret_code_t err_code = nrf_ble_gatt_init(&m_gatt, NULL);
    APP_ERROR_CHECK(err_code);
}

static void conn_params_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}

static void on_conn_params_evt(ble_conn_params_evt_t * p_evt)
{
    ret_code_t err_code;
    // 判断事件类型，根据事件类型执行动作
    if (p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED) {
        err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
        APP_ERROR_CHECK(err_code);
    }

    // 连接参数协商成功
    if (p_evt->evt_type == BLE_CONN_PARAMS_EVT_SUCCEEDED) {
        // TODO: 功能代码
    }
}

static void conn_params_init(void)
{
    ret_code_t err_code;
    ble_conn_params_init_t cp_init;
    memset(&cp_init, 0, sizeof(cp_init));

    cp_init.p_conn_params                   = NULL;                             // 设置为NULL，从主机获取连接参数
    cp_init.first_conn_params_update_delay  = FIRST_CONN_PARAMS_UPDATE_DELAY;   // 连接或启动通知到首次发起连接参数更新请求之间的时间设置为5秒
    cp_init.next_conn_params_update_delay   = NEXT_CONN_PARAMS_UPDATE_DELAY;    // 每次调用sd_ble_gap_conn_param_update()函数发起连接参数更新请求之间的间隔设置为30秒
    cp_init.max_conn_params_update_count    = MAX_CONN_PARAMS_UPDATE_COUNT;     // 放弃连接参数协商前尝试连接协商的最大次数设置
    cp_init.start_on_notify_cccd_handle     = BLE_GATT_HANDLE_INVALID;          // 连接参数更新从连接事件开始计时
    cp_init.disconnect_on_fail              = false;                            // 连接参数更新失败不断开连接
    cp_init.evt_handler                     = on_conn_params_evt;               // 注册连接参数更新事件句柄
    cp_init.error_handler                   = conn_params_error_handler;        // 注册连接参数 更新错误事件句柄

    err_code = ble_conn_params_init(&cp_init);
    APP_ERROR_CHECK(err_code);
}

static void gap_params_init(void)
{
    ret_code_t                  err_code;
    ble_gap_conn_params_t       gap_conn_params; // 定义连接参数结构体变量
    ble_gap_conn_sec_mode_t     sec_mode;

    // 设置GAP的安全模式
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);
    // 设置GAP的设备名称
    err_code = sd_ble_gap_device_name_set(&sec_mode, (const uint8_t *) DEVICE_NAME, strlen(DEVICE_NAME));
    APP_ERROR_CHECK(err_code);

    // 如果需要设置外观特征，在这里使用如下代码设置
    /*
    err_code = sd_ble_gap_appearance_set(BLE_APPEARANCE_x);
    APP_ERROR_CHECK(err_code); */

    // 设置首选连接参数，设置前先清零gap_conn_params
    memset(&gap_conn_params, 0, sizeof(gap_conn_params));

    gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
    gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
    gap_conn_params.slave_latency     = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT;

    // 调用协议栈API来配置GAP参数
    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
}

static void on_adv_evt(ble_adv_evt_t ble_adv_evt)
{
    ret_code_t err_code;
    switch (ble_adv_evt) {
        // 快速广播启动事件：快速广播时会触发这个事件
        case BLE_ADV_EVT_FAST:
            NRF_LOG_INFO("Fast advertising.");
            err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING);
            break;
        // 广播IDEL事件: 广播超时后会触发此事件
        case BLE_ADV_EVT_IDLE:
            NRF_LOG_INFO("Advertising idle.");
            err_code = bsp_indication_set(BSP_INDICATE_IDLE);
            break;

        default:
            break;
    }
}

static void advertising_init(void)
{
    ret_code_t err_code;
    ble_advertising_init_t init;

    memset(&init, 0, sizeof(init));
    init.advdata.name_type             = BLE_ADVDATA_FULL_NAME;                         // 设备名称类型：全称
    init.advdata.include_appearance    = true;                                          // 是否包含外观: true
    init.advdata.flags                 = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;   // Flag: 一般为可发现模式，不支持BR/EDR
    init.config.ble_adv_fast_enabled   = true;
    init.config.ble_adv_fast_interval  = APP_ADV_INTERVAL;
    init.config.ble_adv_fast_timeout   = APP_ADV_DURATION;
    init.evt_handler                   = on_adv_evt;

    err_code = ble_advertising_init(&m_advertising, &init);
    APP_ERROR_CHECK(err_code);

    // 设置广播标记。APP_BLE_CONN_CFG_TAG是用于跟踪广播配置的标记，这是为未来预留的一个参数，在将来的SoftDevice版本中，
    // 可以使用sd_ble_gap_adv_set_configure(()配置新的广播配置
    // 当前SoftDevice版本(S132 V7.2.0版本)支持最大的广播集数量为1，因此APP_BLE_CONN_CFG_TAG只可以写1
    ble_advertising_conn_cfg_tag_set(&m_advertising, APP_BLE_CONN_CFG_TAG);
}

static void advertising_start(void)
{
    ret_code_t err_code = ble_advertising_start(&m_advertising, BLE_ADV_MODE_FAST);
    APP_ERROR_CHECK(err_code);
}
