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

/*
 * ADV广播测试实验程序
 *
 * 0 -> 随机静态地址读写实验
 *
 */
#define ADDRESS_TEST 1
#define ENABLE_TX_POWER_TEST
#define ENABLE_UUID_LIST_TEST

void ble_stack_init(void)
{
    ret_code_t err_code;
    // 请求使能SoftDevice，该函数会根据sdk_config.h文件中低频时钟的设置来配置低频时钟
    err_code = nrf_sdh_enable_request();
    APP_ERROR_CHECK(err_code);

    // 定义保存应用程序RAM起始地址的变量
    uint32_t  ram_start = 0;

    err_code = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start);
    APP_ERROR_CHECK(err_code);

    // 使能ble协议栈
    err_code = nrf_sdh_ble_enable(&ram_start);
    APP_ERROR_CHECK(err_code);

    // 注册BLE事件回调函数
    NRF_SDH_BLE_OBSERVER(m_observer, APP_BLE_OBSERVER_PRIO, ble_evt_handler, NULL);
}

void ble_evt_handler(ble_evt_t const * p_ble_evt, void * p_context)
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

void gatt_init(void)
{
    // 初始化GATT程序模块
    ret_code_t err_code = nrf_ble_gatt_init(&m_gatt, NULL);
    APP_ERROR_CHECK(err_code);
}

void conn_params_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}

void on_conn_params_evt(ble_conn_params_evt_t * p_evt)
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

void conn_params_init(void)
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

void gap_params_init(void) {
    ret_code_t err_code;
    ble_gap_conn_params_t gap_conn_params; // 定义连接参数结构体变量
    ble_gap_conn_sec_mode_t sec_mode;

    // 设置GAP的安全模式
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);
    // 设置GAP的设备名称
    err_code = sd_ble_gap_device_name_set(&sec_mode, (const uint8_t *) DEVICE_NAME, strlen(DEVICE_NAME));
    APP_ERROR_CHECK(err_code);

    // 设置GAP的外观属性
    // 如果需要设置其他外观可以查看ble_types.h这个头文件
    err_code = sd_ble_gap_appearance_set(BLE_APPEARANCE_GENERIC_RUNNING_WALKING_SENSOR);
    APP_ERROR_CHECK(err_code);

    // 设置首选连接参数，设置前先清零gap_conn_params
    memset(&gap_conn_params, 0, sizeof(gap_conn_params));

    gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
    gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
    gap_conn_params.slave_latency = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout = CONN_SUP_TIMEOUT;

    // 调用协议栈API来配置GAP参数
    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);


#if ADDRESS_TEST == 0
    // 定义地址结构体变量my_addr
    static ble_gap_addr_t my_addr;

    /*--------------设置设备地址: 随机静态地址----------------*/
    my_addr.addr[0] = 0x11;
    my_addr.addr[1] = 0x22;
    my_addr.addr[2] = 0x33;
    my_addr.addr[3] = 0x44;
    my_addr.addr[4] = 0x55;
    // 注意地址最高位必须为1,其它所有的位不能同时为0，也不能同时为1
    my_addr.addr[5] = 0xFE;
    // 地址类型设置为随机静态地址
    my_addr.addr_type = BLE_GAP_ADDR_TYPE_RANDOM_STATIC;
    // 写入地址
    err_code = sd_ble_gap_addr_set(&my_addr);
    if (err_code != NRF_SUCCESS) {
        NRF_LOG_INFO("Set address failed!");
    }

    err_code = sd_ble_gap_addr_get(&my_addr);
    if (err_code == NRF_SUCCESS) {
        NRF_LOG_INFO("Address Type: %02X\r\n", my_addr.addr_type);

        NRF_LOG_INFO("Address: %02X:%02X:%02X:%02X:%02X:%02X\r\n",
                        my_addr.addr[0], my_addr.addr[1],
                        my_addr.addr[2], my_addr.addr[3],
                        my_addr.addr[4], my_addr.addr[5]
                     );
    }

#elif ADDRESS_TEST == 1
    static ble_gap_privacy_params_t my_addr = {0};

    // 初始化地址模式、地址类型和循环周期，私有地址由协议栈自动生成
    my_addr.privacy_mode = BLE_GAP_PRIVACY_MODE_DEVICE_PRIVACY;
    my_addr.private_addr_type = BLE_GAP_ADDR_TYPE_RANDOM_PRIVATE_NON_RESOLVABLE;

    // 地址循环周期设置为15秒，蓝牙内核协议推荐值是15分钟。
    my_addr.private_addr_cycle_s = 15;
    my_addr.p_device_irk = NULL;

    err_code = sd_ble_gap_privacy_set(&my_addr);

    if (err_code != NRF_SUCCESS) {
        NRF_LOG_INFO("")
    }
#endif
}

void on_adv_evt(ble_adv_evt_t ble_adv_evt)
{
    ret_code_t err_code;
    switch (ble_adv_evt) {
        // 快速广播启动事件：快速广播时会触发这个事件
        case BLE_ADV_EVT_FAST:
            NRF_LOG_INFO("Fast advertising.");
            err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING);
            APP_ERROR_CHECK(err_code);
            break;
        // 广播IDEL事件: 广播超时后会触发此事件
        case BLE_ADV_EVT_IDLE:
            NRF_LOG_INFO("Advertising idle.");
            err_code = bsp_indication_set(BSP_INDICATE_IDLE);
            APP_ERROR_CHECK(err_code);
            break;

        default:
            break;
    }
}

void advertising_init(void)
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

#ifdef ENABLE_TX_POWER_TEST
    // 定义保存想发射功率等级的变量
    int8_t tx_power_level = 0;
    // 发送功率等级
    init.advdata.p_tx_power_level = &tx_power_level;
#endif

#ifdef ENABLE_UUID_LIST_TEST
    static ble_uuid_t m_adv_uuids[] = {
            {BLE_UUID_RUNNING_SPEED_AND_CADENCE, BLE_UUID_TYPE_BLE},
            {BLE_UUID_HEART_RATE_SERVICE, BLE_UUID_TYPE_BLE}
    };

    init.advdata.uuids_complete.uuid_cnt = sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]);
    init.advdata.uuids_complete.p_uuids = m_adv_uuids;

    // 广播数据中包含部分的服务UUID
    /*
    init.advdata.uuids_more_available.uuid_cnt = (sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0])) - 1;
    init.advdata.uuids_more_available.p_uuids = m_adv_uuids;
     */

#endif

    err_code = ble_advertising_init(&m_advertising, &init);
    APP_ERROR_CHECK(err_code);

    // 设置广播标记。APP_BLE_CONN_CFG_TAG是用于跟踪广播配置的标记，这是为未来预留的一个参数，在将来的SoftDevice版本中，
    // 可以使用sd_ble_gap_adv_set_configure(()配置新的广播配置
    // 当前SoftDevice版本(S132 V7.2.0版本)支持最大的广播集数量为1，因此APP_BLE_CONN_CFG_TAG只可以写1
    ble_advertising_conn_cfg_tag_set(&m_advertising, APP_BLE_CONN_CFG_TAG);
}

void advertising_start(void)
{
    ret_code_t err_code = ble_advertising_start(&m_advertising, BLE_ADV_MODE_FAST);
    APP_ERROR_CHECK(err_code);
}

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
    err_code = nrf_ble_qwr_init(&m_qwr, &qwr_init);
    APP_ERROR_CHECK(err_code);
}
