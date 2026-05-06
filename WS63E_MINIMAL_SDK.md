# WS63E Driver SDK Baseline

This workspace is now shaped as a clean driver/protocol SDK base for custom application development.

Detailed build-to-runtime architecture: [WS63E_BUILD_TO_RUNTIME_ARCHITECTURE.md](WS63E_BUILD_TO_RUNTIME_ARCHITECTURE.md)

## Application Layout

- Custom entry: `src/application/ws63/ws63_minimal_application`
- Built component: `ws63_liteos_min_app`
- Removed from `src/application`:
  - `samples`
  - `ws63_liteos_application`
  - `ws63_liteos_mfg`

The application CMake now only adds the WS63 minimal application entry.

## Kept Driver And Protocol Layers

The WS63 build configuration keeps the SDK driver base and protocol stacks, including:

- peripheral drivers: UART, GPIO, I2C, SPI, DMA, PWM, ADC, SFC, EFUSE, watchdog, timer, systick, TCXO, security, PMP, SIO/I2S
- system base: LiteOS, OSAL, partition/NV/update/factory/DFX pieces used by the stack
- wireless stacks: Wi-Fi, BT host/controller, BLE, SLE/GLE, BG common, BGTP, CHBA/SLE netdev, TIOT transport support

Removed from linked components:

- `samples`

## Removed Vendor Example Code

Vendor demo/sample/product application folders were removed. Vendor documentation folders remain only as reference material.

Removed example roots include:

- `vendor/*/demo`
- `vendor/*/peripheral`
- `vendor/*/products`
- `vendor/BearPi-Pico_H3863/wifi`
- `vendor/Hqyj_Ws63/Farsight`
- `vendor/build_sample.py`

## Build Command

The intended build target is:

```powershell
cd src
$env:PATH = 'D:\DevTools_CFBB_V1.0.12\thirdparty\ninja;D:\DevTools_CFBB_V1.0.12\thirdparty\python\Lib\site-packages\cmake\data\bin;' + $env:PATH
python build.py -c ws63-liteos-app
```

The build target name intentionally remains `ws63-liteos-app` because several closed driver/protocol libraries are shipped under target-name directories such as `drivers/chips/ws63/porting/patch/ws63-liteos-app`. The target still links the minimal application component `ws63_liteos_min_app`; the removed samples are not linked.

## Burn And Serial Output

The minimal SDK uses the original `ws63-liteos-app` target name. After building, burn this package:

```text
src/output/ws63/fwpkg/ws63-liteos-app/ws63-liteos-app_all.fwpkg
```

HiSpark/DevEco project metadata has been switched to this package path.

Burn settings:

- target/chip: `WS63` (WS63E uses the WS63 burn target)
- protocol: `serial`
- upload baud: `921600`
- enable `Auto Burn` and `Auto disconnect`
- click connect/program load, then press the board `RST` key when the tool waits for reset

Serial monitor settings after burn:

- baud: `115200`
- data bits: `8`
- parity: `N`
- stop bits: `1`

Close the burn connection before opening the serial monitor. If the monitor is open during burning, or if it uses the upload baud `921600` for normal boot logs, binary burn traffic and boot text can appear as garbled characters.

For the clean SDK baseline, `ws63-liteos-app`, `ws63-flashboot`, and `ws63-loaderboot` keep debug/log UART output on UART0 at `115200`. The application target also disables dynamic UART binding, disables HSO binary logging, disables DFX print output, and disables binary exception memory dumps. This keeps normal boot/application output as plain text on the same serial settings.

Expected minimal application logs include lines like:

```text
APP|dbg uart init ok.
APP|minimal heartbeat init ok.
APP|watchdog disabled.
APP|kernel initialize state=...
APP|main heartbeat start.
APP|hello
APP|hello
```

Messages such as `secure verify disable` come from the boot security check path when secure boot verification is disabled. They are not application logs. `0x80001341` means the SFC driver did not match the flash ID and fell back to the default flash description. This baseline treats that fallback as usable and prints `Flash Init Default! ret = 0x80001341`; if `Flash Init Fail! ret = 0x80001341` still appears, the board is still running an old boot image or only the application partition was updated. Burn the full `ws63-liteos-app_all.fwpkg` package after rebuilding.

The minimal app also initializes and disables the bootloader-started watchdog during early startup so the board does not reboot just because no final user application is feeding the watchdog yet.
