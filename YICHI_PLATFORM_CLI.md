# Yichi：Platform 与 CLI 修改说明

本次直接基于上传的 redundant-fiber-converter-main.zip 修改。没有操作你另一个 CubeIDE 工作目录，也没有烧录硬件。

## 先理解一条路径

电脑输入 `failover status` → UART 接收字节 → cli.c 拼成一行并识别命令 → path_manager_get_state() 返回软件状态 → CLI 格式化文字 → Platform UART 发送。

CLI 不决定是否切换，也不选择 UART 引脚。应用层现在通过 bxp_time/bxp_gpio 等接口访问硬件；main.c 是组装这些模块的入口。驱动层仍有 HAL 依赖，这不影响 app 层隔离。

## 检查发现与修改

| 模块 | 原有状态 | 本次处理 |
|---|---|---|
| GPIO | 有真实 HAL 调用，但引脚未验证；CS 未初始化；中断直接执行较重工作 | 保留原映射，默认禁用未经确认的映射；增加安全初始输出、CS 初始化、分开配置输入端口、EXTI 入口；中断工作移到主循环 |
| I2C | 空读写却返回成功，可能使用未初始化内存 | 实现 HAL memory read/write 包装、参数检查、超时/忙/错误返回；缺驱动或未绑定时不可用；保持原接口的左移一位地址约定 |
| SPI | 返回固定 0x96455001 芯片 ID | 删除伪造数据；增加原始全双工 HAL 传输及完整帧 CS；寄存器协议尚未确认，read_reg/write_reg 明确不可用 |
| UART | 无 | 增加硬件无关接口和可选 HAL 后端；单字节非阻塞接收、最长 50 ms 阻塞发送；要求 8 位无校验 |
| CLI | 无 | 七条命令全部接入，支持 CR/LF/CRLF、退格、空白整理、未知命令和超长整行丢弃 |
| Diagnostics | 有日志环，但无读取接口；初始化会清掉启动日志 | 提供只读日志访问；提前初始化；硬件计数器显示不可用 |
| Peer | 启动就声明在线 | 默认离线，收到回调后才在线；明确 Ethernet 收发仍不完整 |
| Failover | 启动就声明主链路已激活 | 保持 INIT，明确 switch 尚未初始化，不宣称实际路径已建立 |
| Optical health | 默认 OK；把原始光功率计数当成 dBm | 默认不可用；读取失败不会保留 OK；暂时停用未经验证的光功率阈值判断 |

现有 VLAN/切换逻辑仍有忽略错误返回的问题；由于 switch 协议未完成，本版 path manager 不进入活动状态。恢复真实自动切换前还必须修复这些错误传播，不能只把 INIT 改回 PRIMARY_ACTIVE。

## 七条命令

- `help`：列出命令。
- `system status`：运行时间、GPIO/I2C/UART 配置可用性，不把绑定句柄当成硬件健康。
- `sfp status`：现有 optical_health 状态；未配置/转换未验证时显示 UNAVAILABLE，不输出假 dBm。仓库只提供单个 SFP 接口，不虚构 SFP0/SFP1。
- `switch status`：NOT INITIALIZED，并说明寄存器协议待验证。
- `failover status`：调用 path_manager API，显示软件状态与硬件路径不可用。
- `peer status`：调用 peer API，并说明 Ethernet transport 尚不完整。
- `diagnostics`：打印实际日志环；未实现的硬件统计不打印成有效的零。

## 为什么现在接串口不会立刻有输出

上传 ZIP 只有部分 STM32N6 HAL 文件，缺少 I2C、SPI、UART 头文件和实现。因此默认构建使用明确返回 UNAVAILABLE 的分支。HAL 后端代码已经写入，但本次无法在这份缺依赖的 ZIP 上编译这些启用分支，也未做板上验证。

下一步需要：

1. 从与项目匹配的 STM32CubeN6 包补齐 I2C/SPI/UART 及其依赖，加入 CubeIDE 构建，再启用 hal_conf.h 对应模块。
2. 在板级文件里覆盖弱函数 bxp_board_init()，配置确认过的时钟、引脚复用、I2C timing、SPI mode、UART baud；初始化静态 HAL 句柄后调用 bxp_i2c_bind / bxp_uart_bind / bxp_spi_bind。应用和 CLI 不需要 HAL 句柄。
3. 核对 board_pins.h 中已有映射、初始电平、上电时序及 EXTI IRQ 后，再启用 BXP_BOARD_PINS_CONFIRMED。更改 EXTI 引脚时同步修改 IRQ 入口，不能只改宏。
4. 确认交换机准确型号、SPI 命令格式、字节序、寄存器表、芯片 ID 校验，再实现寄存器适配和初始化状态 API。
5. 验证 SFP 校准/单位转换、peer 真实收发、VLAN 错误传播后再启用自动切换。

这是可构建的 bring-up 基础版本，尚不是硬件功能完成版。UART 当前为轮询接收，输出会阻塞主循环；日志较多或连续输入时可能丢字节。真实实时运行前应改为中断/DMA 接收环和分批输出，并验证调度预算。

## 构建结果与复现

使用电脑已安装的 GNU Arm 14.3.1，针对 Cortex-M55 / STM32N657xx，从源代码编译并链接了 38 个文件。C 编译没有警告；链接成功，有一条 LOAD segment RWX 警告，需在后续检查现有链接脚本的段权限。

本次没有硬件运行、没有 CLI 动态测试、没有 FSBL 重建，也没有验证启用 HAL 总线后的分支。不要将 ZIP 原有 Debug 二进制当成本次构建结果；交付包已排除旧 Debug/Release 构建目录。

仓库内 tools/check_build.py 可复现独立构建：

```text
python tools/check_build.py --gcc "你的 arm-none-eabi-gcc.exe 完整路径"
```

或者用 CubeIDE 导入 firmware/Appli，Refresh 后 Clean/Build，让 IDE 重新生成构建列表；不要运行旧 Debug/makefile（包含原作者电脑的绝对路径）。

## 接通 UART 后的检查顺序

1. 输入 help，以 CR、LF、CRLF 分别结束，均应只执行一次。
2. 逐一输入七条命令，确认未初始化设备没有 READY、虚假芯片 ID 或虚假功率值。
3. 输入错误命令、空行、带多余空格命令、退格修正命令。
4. 输入超过 95 字节的一行，应整行丢弃；下一行 help 应正常。
5. 测试串口拔插、连续粘贴、长日志输出对 heartbeat 调度的影响。
6. 完成 HAL 依赖后测试未绑定、参数错误、超时、总线忙和正常传输；SPI 失败时确认 CS 释放。

建议阅读顺序：cli.c → bxp_uart.h → bxp_uart.c → bxp_board.h → main.c。先看“谁调用谁”，再看 HAL 细节。
