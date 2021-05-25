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

* [闪烁LED](./projects/blinky)
* [开发板便捷工具](./projects/board)
* [GPIOTE输出](./projects/gpiote_out)
* [GPIOTE输入](./projects/gpiote_in)
* [GPIOTE PORT模式](./projects/gpiote_port)
* [UART](./projects/uart)
* [Timer定时器](./projects/timer)
* [Counter计数器](./projects/counter)