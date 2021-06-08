# BLE基本功能

## 此工程实现的参数及功能
<table>
    <thead>
        <tr>
            <th>项目</th>
            <th>名称</th>
            <th>内容</th>
        </tr>
    </thead>
    <tbody>
        <tr>
            <td rowspan="3">广播参数</td>
            <td>广播间隔</td>
            <td>187.5ms</td>
        </tr>
        <tr>
            <td>广播超时时间</td>
            <td>无限广播不超时</td>
        </tr>
        <tr>
            <td>广播模式</td>
            <td>快速广播</td>
        </tr>
        <tr>
            <td rowspan="3">广播数据</td>
            <td>设备名称</td>
            <td>James' BLE</td>
        </tr>
        <tr>
            <td>外观</td>
            <td>包含</td>
        </tr>
        <tr>
            <td>Flags</td>
            <td>一般可发现模式，不支持BR/EDR</td>
        </tr>
        <tr>
            <td rowspan="4">首选连接参数</td>
            <td>最小连接间隔</td>
            <td>100ms</td>
        </tr>
        <tr>
            <td>最大连接间隔</td>
            <td>200ms</td>
        </tr>
        <tr>
            <td>从机延迟</td>
            <td>0</td>
        </tr>
        <tr>
            <td>监督超时</td>
            <td>4s</td>
        </tr>
        <tr>
            <td rowspan="3">连接参数协商</td>
            <td>首次协延时</td>
            <td>5s</td>
        </tr>
        <tr>
            <td>每次协商之间的间隔</td>
            <td>30s</td>
        </tr>
        <tr>
            <td>最次协商最大尝试次数</td>
            <td>3次</td>
        </tr>
        <tr>
            <td rowspan="2">包含的服务</td>
            <td>GAP服务</td>
            <td><strong>强制包含</strong></td>
        </tr>
        <tr>
            <td>GATT服务</td>
            <td><strong>强制包含</strong></td>
        </tr>
        <tr>
            <td rowspan="2">指示灯</td>
            <td>D1闪烁</td>
            <td>指示正在广播</td>
        </tr>
        <tr>
            <td>D1常亮</td>
            <td>指示已经和中断设备建立连接</td>
        </tr>
    </tbody>
</table>

## BLE程序流程

一个BLE程序通常需要至少包含4个必要的部分: 系统初始化、启动、空闲管理和事件处理。

![BLE程序流程](imgs/ble_peripheral_basic_start_sequence.png)

### 流程概述

1. 日志打印初始化`tri_app_log.h(app_log_init)`
```c
// 设置log输出终端伙(根据sdk_config.h中的配置设置输出终端为UART或者RTT)
NRF_LOG_DEFAULT_BACKENDS_INIT();
```
2. APP定时器初始化`tri_app_timer.h(timers_init)`
3. 指示灯初始化`tri_app_leds.h(leds_init)`
4. 电源管理初始化`tri_app_power.h(power_management_init)`
5. 协议栈初始化`tri_app_ble.h(ble_stack_init)`
```c
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
```
注意这里有几点非常重要，如果想使用SoftDevice一定要要先烧入相应的SoftDevice hex文件，然后再将SoftDevice使能。
另外需要注意的是我们使用的是gcc来进行烧写，内存分配的。所以需要更改`config/gcc_nrf52.ld`文件，否则在`nrf_sdh_ble_enable`
这个方法执行的时候会抛出`err_code = 4`错误，这个错误是表示没有足够的内存来分配数据。

```
MEMORY
{
  FLASH (rx) : ORIGIN = 0x26000, LENGTH = 0x5a000
  RAM (rwx) :  ORIGIN = 0x200022f0, LENGTH = 0xdd10
}
```

上面的表示了烧写的起始地址等数据。

6. GAP初始化`tri_app_ble.h(gap_params_init)`
7. GATT初始化`tri_app_ble.h(gatt_init)`
8. 广播初始化`tri_app_ble.h(advertising_init)`
9. 服务初始化`tri_app_ble.h(services_init)`
10. 连接参数协商初始化`tri_app_ble.h(conn_params_init)`
11. 启动广播`main.c(advertising_start)`
12. 空闲状态处理`main.c(idle_state_handle)`

### 日志打印
日志打印有两种手段，一种是通过UART串口进行数据打印，一种是通过JLINK RTT来进行日志打印
日志有几个级别
* INFO
* WARNING
* DEBUG
* ERROR

选择什么样的方式来进行日志打印可以在`sdk_config.h`来进行配置

### APP定时器

#### APP定时器作用
APP 定时器基于实时计数器 RTC1 的软件定时器，APP 定时器允许用户同时创建多个 定时任务，APP 定时器在 RTC1 中断处理程序中检查超时并执行超时处理程序的调用，在软 中断（SWI0）处理定时列表，RTC1 中断和 SWI0 均使用低中断优先级 APP_LOW。 在 BLE 的程序中，APP 定时器的用途主要有以下几种：

1. 按键消抖以及实现长按和短按。
2. 定时驱动 LED 指示灯，和 BLE 事件配合实现 BLE 的状态指示（如指示灯 D1 闪烁指示 正在广播，常亮指示已经连接）。
3. 连接参数更新，连接成功后，开启 APP 定时器，超时后进行连接参数更新。
4. 用户创建定时任务，实现各种定时任务，如创建一个超时时间为 1 秒的 APP 定时器作 为实时时钟的时基。

### 电源管理
某些程序对于功耗要求较高，需要SOC进行休眠处理。

电源管理运行后，在使用API等待应用程序事件时，只要SoftDevice不使用CPU，CPU就会进入IDLE状态，此时，SoftDevice直接处理的中断不会唤醒应用程序。应用程序
中断将预期唤醒应用程序。另外，当系统进入System Off模式时，使用电源管理模块的API可以确保在断电之前停止SoftDevice服务。

使用电源管理功能时，首先要初始化电源管理模块，之后在主循环中运行电源管理，这样，当CPU空闲时会执行电源管理API，进入低功耗模式等待事件唤醒，从而实现低功耗。

### BLE协议栈
使用BLE功能之前，需要先初始化BLE协议栈，BLE协议栈初始化包括使能SoftDevice、配置BLE协议栈参数、使能BLE协议栈事件回调函数。

![BLE协议栈初始化步骤](imgs/ble_peripheral_basic_stack_init_sequence.png)

### GAP

#### GAP作用
GAP(Generic Access Profile)，通用访问配置文件。GAP定义了设备如何彼此发现、建立连接以及如何实现绑定，同时描述了设备如何成为广播者和观察者，并且实现
无需连接的传输。同时，GAP定义了如何用不同类型的地址来实现隐私性和可解析性。GAP和BLE体系结构中底层的关系。

![GAP和BLE体系结构中底层的关系](imgs/ble_peripheral_basic_gap_relation.png)

GAP的作用就是定义以下4个方面:
* GAP角色
* 可发现性模式和规程
* 连接模式和规程
* 安全模式和规程

从学习GAP开始，有两个非常重要的概念: 模式(Mode)规程(Procedure)，它们用来描述设备的行为，定义以及区别如下：
* 模式(Mode): 模式描述是设备的工作状态，当一个设备被配置为按照某种方式操作一段较长时间时，称为模式。广播模式，表示设备正处在广播状态，一般会持续很长时间。
* 规程(Procedure): 规程描述的是在有限时间内进行特定的操作，如连接参数更新规程，它是在较短的时间内执行了连接参数更新的操作。

#### GAP角色
BLE为设备在物理传输定义了4种GAP角色，一个设备可以支持多个GAP角色，可以是广播者也可以是外围设备:
* 广播者: 广播发送者，不是可连接的设备
* 观察者: 扫描广播，不能够启动连接
* 外围设备(peripheral): 广播发送者，是可连接的设备，连接后成为从设备
* 中心设备(center): 扫描广播启动连接，连接后成为主设备

要理解 GAP 为什么分为 4 种角色，就要知道蓝牙标准制定时的考虑。我们知道BLE主打低功耗、所以 BLE 体系结构中，为了最大可能优化设备，节省功耗，所有的层都采用了
非对称的设计，对于物理层的无线电装置，可以是这 3 种形式。 
* 芯片只有发射机：只能发射无线信号，不能接收无线信号，硬件成本低。 
* 芯片只有接收机：只能接收无线信号，不能发射无线信号，硬件成本低。
* 芯片同时具有接收机和发射机：既可以接收无线信号，也可以发射无线信号，硬件成本较高。

这样，当某个应用只需要在设备之间单向传输数据时，其中一个设备可以作为广播者， 采用只有发射机的芯片，另外一个设备可以作为观察者，采用只有接收机的芯片，
这就构成 了非对称系统，如下图所示。它们实现了单向的传输，并且用了尽量少的硬件资源，所以，功耗会更低，同时理论上造价也会更低。

#### GAP安全模式

|安全模式和规程|广播者|观察者|外围设备|中心设备|
|:-----|:----:|:-----:|:-----:|:-----:|
|安全模式1|E|E|O|O|
|安全模式2|E|E|O|O|
|认证规程|E|E|O|O|
|授权规程|E|E|O|O|
|连接数据签名规程|E|E|O|O|
|认证签名数据规程|E|E|O|O|

`E: 不包含的 O: 可选择的`

安全模式和等级
<table>
    <thead>
        <tr>
            <th>安全模式</th>
            <th>等级</th>
            <th>描述</th>
        </tr>
    </thead>
    <tbody>
        <tr>
            <td>安全模式1</td>
            <td colspan="2">通过不同级别的加密保证安全</td>
        </tr>
        <tr>
            <td rowspan="4"></td>
            <td>等级1</td>
            <td>无安全性（无认证和加密）</td>
        </tr>
        <tr>
            <td>等级2</td>
            <td>带加密的未认证配对</td>
        </tr>
        <tr>
            <td>等级3</td>
            <td>带加密的认证配对</td>
        </tr>
        <tr>
            <td>等级4</td>
            <td>带加密的LE安全连接配对</td>
        </tr>
        <tr>
            <td>安全模式2</td>
            <td colspan="2">通过不同级别的加密保证安全</td>
        </tr>
        <tr>
            <td rowspan="2"></td>
            <td>等级1</td>
            <td>带数据签名的未认证配对</td>
        </tr>
        <tr>
            <td>等级2</td>
            <td>带数据签名的认证配对</td>
        </tr>
    </tbody>
</table>

#### GAP服务
GAP层定义了GAP服务，它的作用是用来确定设备的信息，GAP服务包含了5个特征：
设备名称、外观特征、外围设备首选连接参数、中心设备地址解析和可解析私有地址。4类GAP角色对想GAP服务的需求如下表所示，正如前面所述，广播者和观察者没有服务，
当然不能包含GAP服务，外围设备和中心设备对GAP服务是强制包含的。

| |广播者|观察者|外围设备|中心设备|
|:----:|:----:|:----:|:-----:|:----:|
|GAP服务|E|E|M|M|

对于每一个GAP角色，当其包含GAP服务时，对GAP的特征的需求也是不一样的，如下表所示，对于设备名称和外观特征，外围设备
和中心设备是强制包含的，对于外围设备首连接参数，外围设备是可选择是否包含的，而中心设备是不能包含的，对于中心设备地址解析和可解析私有地址是有条件包含的。

|特征|广播者|观察者|外围设备|中心设备|
|:----:|:----:|:----:|:----:|:----:|
|设备名称|E|E|M|M|
|外观特征|E|E|M|M|
|外围设备首选连接参数|E|E|O|E|
|中心设备地址解析|E|E|C3|C2|
|可解析私有地址|E|E|C3|C3|

`
C2: 如果支持链路层隐私，强制包含该特征，否则不包含
C3: 如果支持链路层徇私，此特征可选择包含，否则不包含
`