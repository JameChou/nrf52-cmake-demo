# BLE广播

## 基本概念和流程
广播就是设备将自身意愿展示的信息按照伙一定的间隔以“扫描者”可理解的方式向周边发射。

广播有如下四种类型，广播报文的报头中有4个位专门用于指示广播报文的类型。
1. 通用广播：最常用的广播方式，可以被扫描，接收到连接请求时可以作为从设备进入一个连接。
2. 定向广播：针对于快速建立连接的需求，定向广播会占满整个广播信道，定向广播的数据净荷只包含广播者和发起者地址，发起者收到发给自己的定向广播后，会立即发送连接请求。定向广播最长时间不能超过1.28s，并且不能被主动扫描。
3. 不可连接广播：广播数据，而不进入连接态，也不响应扫描，这是唯一一个允许硬件设备只有发射机的广播类型，因为它不会需要接收任何数据。
4. 可发现广播：不可连接，但可以响应扫描。

Nordic SDK中实现广播流程
1. 广播的配置：应用程序首先要根据自己的需求配置广播的间隔、广播的模式以及广播中包含哪些数据，这称为广播初始化，广播初始化配置好各项参数之后，最终调用API函数sd_ble_gap_adv_set_configure()将配置传递给SoftDevice完成配置。配置成功之后，SoftDevice会返回"NRF_SUCCESS"。
2. 广播的启动：应用程序初始化广播成功后，广播并没有启动，应用程序需要根据初始化中设置的模式启动广播，应用程序启动广播最终是调用API函数sd_ble_gap_adv_start()通知SoftDevice启动广播，SoftDevice启动广播成功后会返回"NRF_SUCCESS"。
3. 广播的停止：广播的停止有2种放课后，一是应用程序主动停止广播，即应用程序通过调用API函数sd_ble_gap_adv_stop()通知SoftDevice停止广播，SoftDevice成功停止广播后返回"NRF_SUCCESS"；二是超时停止广播，即广播设置了超时时间，在设定的超时时间 内没有和中心设备建立连接，广播即会超时停止，这时SoftDevice会向应用程序提交"BLE_GAP_EVT_ADV_SET_TERMINATED"事件。

![广播流程时序图](imgs/ble_adv_sequence.png)

`
APP: Application应用程序
SD: SoftDevice
SCANNERS: 扫描设备
`

## 广播数据包报文结构
因为有的域的长度超过了一个字节，所以在传输的过程中就涉及到多字节中哪个字节先传输的问题，BLE报文传输时的字节序和比特序如下：
* 字节序：大多数字节域是从低字节开始传输的。注意，并不是所有的多字节域都是从低字节开始传输的。
* 比特序：各个字节传输时，每个字节都是从低位开始。

![广播数据包报文结构](imgs/ble_pdu_data_structure.png)

## 广播可以包含的数据
SDK里面定义了广播数据结构体ble_advdata_t，该结构体描述了广播可以包含的数据，ble_advdata_t结构体声明如下：
```c
typedef struct
{
    ble_advdata_name_type_t      name_type;                           // 设备名称类型
    uint8_t                      short_name_len;                      // 裁剪的设备名称的长度
    bool                         include_appearance;                  // 是否包含外观
    uint8_t                      flags;                               // flags
    int8_t *                     p_tx_power_level;                    // 发射功率等级
    ble_advdata_uuid_list_t      uuids_more_available;                // 服务UUID部分列表
    ble_advdata_uuid_list_t      uuids_complete;                      // 服务UUID完整列表
    ble_advdata_uuid_list_t      uuids_solicited;                     // 服务请求UUID列表
    ble_advdata_conn_int_t *     p_slave_conn_int;                    // 从机连接间隔范围
    ble_advdata_manuf_data_t *   p_manuf_specific_data;               // 制造商自定义数据
    ble_advdata_service_data_t * p_service_data_array;                // 服务数据
    uint8_t                      service_data_count;                  // 服务数据的长度(字节数)
    bool                         include_ble_device_addr;             // 是否包含设备地址
    ble_advdata_le_role_t        le_role;                             // 公用于NFC，BLE广播设备为NULL
    ble_advdata_tk_value_t *     p_tk_value;                          // 公用于NFC，BLE广播设备为NULL
    uint8_t *                    p_sec_mgr_oob_flags;                 // 公用于NFC，BLE广播设备为NULL
    ble_gap_lesc_oob_data_t *    p_lesc_data;                         // 公用于NFC，BLE广播设备为NULL
} ble_advdata_t;
```

## 设备地址
BLE设备地址可以使用公共地址(Public Device Address)或随机地址(Random Device Address)两种地址类型，一个BLE至少使用一种地址类型，当然也可以同时
具备两种地址类型。公共地址和随机地址的长度一样，都是48位(6个字节)的。BLE设备地址类型的关系如下图。
![设备地址类型](imgs/ble_adv_address_type.png)

* 公共地址：从IEEE申请(购买)，IEEE保证地址分发的唯一性。
* 随机静态地址(Static Device Address)：自己定义，上电初始化完成后不能修改。
* 随机不可解析私有地址(Non-resolvable private address)：定时更新地址，蓝牙核心规范建议15分钟更新一次。
* 随机可解析私有地址(RPA: Resolvable Private Address)：通信双方使用共享的身份解析密钥(IRK: Identity Resolving Key)，生成和解析可解析私
有地址。只有一台设备拥有另一台广播设备的IRK时，才能跟踪该广播设备的活动。
  
### 公共地址
公共地址由两部分组成，公共地址由制造商从IEEE申请(购买)，由IEEE注册机构为该制造商分配的机构唯一标识符OUI(Organizationally Unique Identifier)。
这个地址是独一无二的，不能修改。
LSB => company_assigned(24 bits)

MSB => company_id(24 bits)

公共地址能明确的指示出设备，同时具有唯一性，但是安全度不够，试想当我们携带使用公共地址的BLE设备时，不法分子可以通过技术手段跟踪该唯一的巫颂地址，即可跟踪
到该BLE设备的使用者。

为了加强隐私保护，蓝牙内核协议中提供了另外一种地址：随机地址，随机地址是随机产生的，不是固定分配的，随机地址又为多种类型，以适应不同的应用场景对隐私的需求。

### 随机地址

#### 随机静态地址
1. 随机静态地址的定义
随机静态地址是随机生成的48位地址，随机静态地址必须符合以下要求:
   * 静态地址的最高2位有效位必须是1。
   * 静态地址最高2位有效位之后的其余部分不能全为0。
   * 静态地址最高2位有效位之外的其余部分不能全为1。
   * 一个上电周期内不变。
   
![img.png](imgs/ble_static_address_bit.png)

```c
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
    // 11111110
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
#endif
```

#### 不可解析私有地址
1. 不可解析私有地址的定义
设备生成不可解析私有地址时必须符合以下要求：
   * 地址的最高2位有效位必须是0
   * 地址最高2位有效位之外的其余部分不能全为0 
   * 地址最高2位有效位之外的其余部分不能全为1
   * 不可解析私有地址不能和公共地址一样
   
![img.png](imgs/ble_static_address_private_bit.png)

不可解析私有地址相当于周期性改变的随机静态地址，不可解析私有地址一直在变化， 并且该地址是个随机数，没有提供任何可解析的信息，因此，很难通过跟踪地址来跟踪设备， 所以具有很高的安全性。但是因为地址一直变化，又没有可解析的信息，这就导致受信任的 设备也没法分辨该地址的真实身份。

由此可见，不可解析私有地址在隐私上“敌我”不分，不管是谁，统统让你无法分辨我 的真实身份。因此不可解析私有地址在实际应用中使用的不多。