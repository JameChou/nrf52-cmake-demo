## nRF52(pca10040) cmake demo

本项目基于[https://github.com/danielTobon43/nRF5-cmake-framework](https://github.com/danielTobon43/nRF5-cmake-framework) 根据自身情况进行一些改变。

芯片是nRF52832，使用国内的艾克姆开发板。这个板子的接口电路与pca10040是一致的，所以sdk中的example都可以顺利使用，**boards.h**里的方法也可以同步使用。

```cmake
add_definitions(-DNRF52 -DNRF52832 -DNRF52832_XXAA -DNRF52_PAN_74 -DNRF52_PAN_64 -DNRF52_PAN_12 -DNRF52_PAN_58 -DNRF52_PAN_54 -DNRF52_PAN_31 -DNRF52_PAN_51 -DNRF52_PAN_36 -DNRF52_PAN_15 -DNRF52_PAN_20 -DNRF52_PAN_55 -DBOARD_PCA10040)
```
在`./cmake/nRF5x.cmake`这个文件中如上面代码所示，`-DBOARD_PCA10040`指定了开发板的型号。

* [nRF Clion CMake](https://blog.jetbrains.com/clion/2020/01/using-nrf52-with-clion/) 这篇文章介绍了如何在macOS下使用gcc和CLion来进行nRF5x相关系列的嵌入式开发。

1. tcp:localhost:2331
2. Mac: /usr/local/bin/JLinkGDBServer
3. -device nrf52 -strict -timeout 0 -nogui -if swd -speed 1000 -endian little -s
4. svd file location: \<ur location\>nRF5_SDK_xxxx/modules/nrfx/mdk

## demo

-- 基本功能

* [闪烁LED](./projects/blinky)
* [开发板便捷工具](./projects/board)
* [GPIOTE输出](./projects/gpiote_out)
* [GPIOTE输入](./projects/gpiote_in)
* [GPIOTE PORT模式](./projects/gpiote_port)
* [UART](./projects/uart)
* [Timer定时器](./projects/timer)
* [Counter计数器](./projects/counter)
* [PPI](./projects/ppi)
* [PPI次级任务](./projects/ppi_sec_task)
* [PPI组](./projects/ppi_group)
* [NVMC](./projects/nvmc)
* [WDT](./projects/wdt)
* [SAADC阻塞式非差分采样](./projects/saadc)
* [SAADC阻塞式差分采样](./projects/saadc_differential)
* [SAADC非阻塞式双缓存](./projects/saadc_unblock_cache)
* [SAADC门限监测](./projects/saadc_limit)
* [SOC温度监测](./projects/soc_temp)
* [MPU6050 DMP(TWI&I2C)](./projects/i2c_mp6050)

-- BLE相关

---- 从机(peripheral)
* [BLE基本功能](./projects/ble_peripheral_basic)
* [BLE广播](./projects/ble_peripheral_adv)
* [BLE标准Profile](./projects/ble_peripheral_basic_profile)
* [BLE RSC](./projects/ble_peripheral_rsc)

---- 主机(center)

-- ANT+ 2.4G传输相关