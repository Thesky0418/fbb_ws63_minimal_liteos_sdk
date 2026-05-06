# WS63E LiteOS Minimal SDK

这是从 HiSpark/FBB WS63 SDK 裁剪出来的最小 LiteOS 工程基线，目标是保留 WS63/WS63E 的启动链路、LiteOS、驱动层和无线协议栈，同时移除原厂 samples、vendor demo 和教学示例代码，让后续业务开发从一个干净入口开始。

当前仓库不是上游完整示例仓库的镜像。它已经改成最小应用工程，拉取后应直接使用原 SDK 构建流程编译 `ws63-liteos-app` 目标。

## 当前状态

- 最小应用入口：`src/application/ws63/ws63_minimal_application`
- 最小应用组件：`ws63_liteos_min_app`
- 构建目标名称仍然保留：`ws63-liteos-app`
- 已移除应用示例：`src/application/samples`
- 已移除原 WS63 LiteOS demo 应用：`src/application/ws63/ws63_liteos_application`
- 已移除制造/测试应用：`src/application/ws63/ws63_liteos_mfg`
- 已移除 vendor 下主要 demo/peripheral/products 示例目录
- 已保留驱动、LiteOS、boot、NV、分区、Wi-Fi、BT/BLE/SLE 等底层和协议相关组件

保留 `ws63-liteos-app` 这个目标名是有意的：部分闭源库、patch 库和构建路径依赖原目标名称。实际链接进去的应用组件已经换成 `ws63_liteos_min_app`。

## 目录说明

| 路径 | 说明 |
| --- | --- |
| `src/application/ws63/ws63_minimal_application` | 当前最小 LiteOS 应用入口 |
| `src/build/config/target_config/ws63` | WS63 构建目标配置 |
| `src/kernel/liteos` | LiteOS 内核、头文件和预编译库 |
| `src/drivers` | 芯片驱动和外设适配 |
| `src/protocol` | 无线协议栈相关代码和库 |
| `src/interim_binary` | 构建所需的预编译中间库 |
| `docs` / `tools` | 上游文档和工具说明，作为参考保留 |
| `WS63E_MINIMAL_SDK.md` | 裁剪说明和构建/烧录补充记录 |

## 构建

在 Windows 环境下，先确保已经安装原 WS63 SDK 需要的工具链、Python、CMake 和 Ninja。一个常见命令如下：

```powershell
cd src
$env:PATH = 'D:\DevTools_CFBB_V1.0.12\thirdparty\ninja;D:\DevTools_CFBB_V1.0.12\thirdparty\python\Lib\site-packages\cmake\data\bin;' + $env:PATH
python build.py -c ws63-liteos-app
```

如果你使用 HiSpark Studio / DevEco 插件，也继续选择 `WS63` 环境和 `ws63-liteos-app` 构建目标。

构建产物位于：

```text
src/output/ws63/acore/ws63-liteos-app/ws63-liteos-app.elf
src/output/ws63/acore/ws63-liteos-app/ws63-liteos-app.bin
src/output/ws63/acore/ws63-liteos-app/ws63-liteos-app-sign.bin
src/output/ws63/fwpkg/ws63-liteos-app/ws63-liteos-app_all.fwpkg
```

`src/output` 是本地构建输出目录，不提交到仓库。

## 烧录

推荐烧录完整包：

```text
src/output/ws63/fwpkg/ws63-liteos-app/ws63-liteos-app_all.fwpkg
```

不要只烧 app-only 包。当前工程涉及 boot、串口配置、应用入口等链路，烧录完整包更容易保证板上运行的 boot/app 与当前代码一致。

常用烧录配置：

```text
target/chip: WS63
protocol: serial
upload baud: 921600
package: ws63-liteos-app_all.fwpkg
```

烧录完成后关闭烧录连接，再打开串口监视器：

```text
baud: 115200
data bits: 8
parity: N
stop bits: 1
```

## 最小应用日志

当前最小应用会初始化调试串口、基础 timer/tick、关闭 watchdog，并创建一个 LiteOS 心跳任务。典型运行日志包含：

```text
APP|dbg uart init ok.
APP|minimal heartbeat init ok.
APP|watchdog disabled.
APP|kernel initialize state=...
APP|create heartbeat task ok.
APP|kernel start.
cpu 0 entering scheduler
APP|liteos heartbeat task start.
APP|liteos task hello tick=...
```

`cpu 0 entering scheduler` 来自 LiteOS 内核启动调度器阶段，不在当前仓库的普通源码里；本 SDK 中相关 LiteOS base 实现主要以预编译库形式提供，所以全局搜索源码可能搜不到这个字符串。

## 关于 Flash Init Fail

启动阶段可能看到：

```text
Flash Init Fail! ret = 0x80001341
```

`0x80001341` 对应 SFC flash ID 未匹配到已知型号。这个日志在未裁剪的完整 SDK 上也可能出现，因此当前仓库先不把它当作必须修复的问题。只要后续 boot 能继续跳转应用、LiteOS 调度器启动、心跳任务持续打印，就说明当前裁剪后的最小运行链路是通的。

## 后续开发入口

建议从这里开始添加自己的业务：

```text
src/application/ws63/ws63_minimal_application/main.c
```

如果后续要恢复某个外设、Wi-Fi、BLE、SLE 或其他协议示例，建议从上游完整 SDK 按需移植单个模块，不要直接把整套 samples/vendor demo 重新加回来。
