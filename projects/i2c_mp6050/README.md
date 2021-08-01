# TWI(I2C) 读写MPU6050

## I2C相关内容

[查看51单片机相关内容](https://github.com/JameChou/prechin-demo/blob/master/src/i2c/README.md)

## nRF52832 TWI特点
* 兼容I2C
* 速率: 100kbps、250kbps或400kbps。
* 支持时钟延伸
* 带EasyDMA
* TWI的SCL和SDA信号可以通过配置寄存器连接到任何一个GPIO，这样可以灵活地实现器件引脚排列，并有效利用电路板空间和信号路由。

### EasyDMA
TWI主机通过EasyDMA实现数据传输，因此TWI的接收和发送缓存必须位于数据RAM区域，如果TXD.PTR的RXD.PTR未指向数据RAM区域，则EasyDMA传输可能导致HardFault或RAM损坏。

.PTR和.MAXCNT寄存器是双缓冲的，因此在收到RXSTARTED/TXSTARTED事件后，可以立即更新并准备下一个RX/TX传输。STOPPED事件表示EasyDMA已完成访问RAM中的缓冲区。


### 低功耗
当系统处于低功耗且不需要TWI外设时，为了尽可能降低功耗，应先停止TWI，然后禁用TWI外设来实现最低功耗。

如果TWI已经停止，则不需要触发STOP任务停止TWI，但是如TWI正在执行发送，则应等待直到收到STOPPED事件作为响应，然后再通过ENABLED寄存器禁用外设。

### 引脚配置
TWI的SCL和SDA信号可以自由映射，通过设置PSEL.SCL和PESL.SDA寄存器可以将它们映射到做生意的物理引脚。

PSEL.SCL和PSEL.SDA寄存器及其配置仅在TWI使能时使用，并且仅在器件处于ON模式时保留。TWI禁用后，引脚将作为常规GPIO使用，即可以使用GPIO的OUT位字段和PIN_CNF[n]寄存器中的配置。
PSEL.SCL和PSEL.SDA必须在禁用TWI后才可以配置。

为确保SPIM中的正确行为，在使用SPIM前必须要正确配置SPIM的引脚，但是需要注意的是不能同时将多个SPIM外设的信号映射到同一个引脚，如不能将SPIM0和SPIM1的MISO同时映射到P0.05，否则，可能会导致不可预测的行为。


当系统处于OFF模式时以及TWI禁用时，为确保TWI主机使用的引脚上的信号电平正确，这些引脚应按照下表所示配置：

|TWI信号|SPIM引脚|方向|输出值|驱动能力|
|:----:|:-----:|:----:|:-----:|:-----:|
|SCL|由PSEL.SCK寄存器设置|输入|不适用|S0D1|
|SDA|由PSEL.SDA寄存器设置|输入|不适用|S0D1|


