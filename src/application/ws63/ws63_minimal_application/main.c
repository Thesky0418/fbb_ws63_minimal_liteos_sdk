/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2022-2023. All rights reserved.
 * Description: Minimal LiteOS application for WS63.
 */

#include <stdint.h>
#include "cmsis_os2.h"
#include "arch/cache.h"
#include "arch_port.h"
#include "chip_core_irq.h"
#include "chip_io.h"
#include "clock_init.h"
#include "debug_print.h"
#include "dyn_mem.h"
#include "flash_patch.h"
#include "los_builddef.h"
#include "main.h"
#include "gpio.h"
#include "partition.h"
#include "pinctrl.h"
#include "pmp_cfg.h"
#include "share_mem_config.h"
#include "soc_porting.h"
#include "systick.h"
#include "tcxo.h"
#include "timer.h"
#include "timer_patch.h"
#include "uart.h"
#include "uart_porting.h"
#include "watchdog.h"

#define MIN_WDT_TIMEOUT_SEC         15U
#define MIN_HEARTBEAT_STACK_SIZE    0x1000U

#define APP_PRINT(fmt, ...)         PRINT(fmt, ##__VA_ARGS__)

#define PATCH_NUM                   194
#define PATCH_REMAP_TAB_WORD_NUM    2
#define PATCH_CMP_HEADINFO_NUM      3

static uint32_t patch_remap[PATCH_NUM * PATCH_REMAP_TAB_WORD_NUM] __attribute__((section(".patch_remap"))) = { 0 };
static uint32_t patch_cmp[PATCH_NUM + PATCH_CMP_HEADINFO_NUM] __attribute__((section(".patch_cmp"))) = { 0 };

static void patch_init(void)
{
    riscv_cfg_t patch_cfg;

    patch_cfg.cmp_start_addr = (uint32_t)(uintptr_t)((void *)patch_cmp);
    patch_cfg.remap_addr = (uint32_t)(uintptr_t)((void *)patch_remap);
    patch_cfg.off_region = true;
    patch_cfg.flplacmp0_en = 0;
    patch_cfg.flplacmp1_en = 0;
    riscv_patch_init(patch_cfg);
}

static void cpu_cache_init(void)
{
    ArchICacheFlush();
    ArchDCacheInvalidate();
    ArchICacheEnable(CACHE_32KB);
    ArchICachePrefetchEnable(CACHE_PREF_1_LINES);
    ArchDCacheEnable(CACHE_4KB);
}

#ifdef BOARD_ASIC
static void minimal_bypass_uart_auto_gate(void)
{
#define IP_AUTO_CG_BYPASS          0x44000244
#define BIT_UART0_CK_EN_HW         10
#define BIT_UART1_CK_EN_HW         11
#define BIT_UART2_CK_EN_HW         12
#define BIT_UART0_AUTO_CG_BYPASS   0
#define BIT_UART1_AUTO_CG_BYPASS   1
#define BIT_UART2_AUTO_CG_BYPASS   2
    reg_setbit(IP_AUTO_CG_BYPASS, 0, BIT_UART0_CK_EN_HW);
    reg_setbit(IP_AUTO_CG_BYPASS, 0, BIT_UART1_CK_EN_HW);
    reg_setbit(IP_AUTO_CG_BYPASS, 0, BIT_UART2_CK_EN_HW);
    reg_setbit(IP_AUTO_CG_BYPASS, 0, BIT_UART0_AUTO_CG_BYPASS);
    reg_setbit(IP_AUTO_CG_BYPASS, 0, BIT_UART1_AUTO_CG_BYPASS);
    reg_setbit(IP_AUTO_CG_BYPASS, 0, BIT_UART2_AUTO_CG_BYPASS);
}
#endif

static void minimal_debug_hw_init(void)
{
#ifdef BOARD_ASIC
    set_uart_tcxo_clock_period();
    open_rf_power();
    switch_clock();
    minimal_bypass_uart_auto_gate();
#endif

    uapi_pin_init();
    uapi_gpio_init();

#if defined(SW_UART_DEBUG)
    sw_debug_uart_init(CONFIG_DEBUG_UART_BAUDRATE);
    APP_PRINT("dbg uart init ok.\r\n");
#endif
}

static void minimal_hw_init(void)
{
    minimal_debug_hw_init();

    timer_patch_init();
    uapi_timer_init();
    uapi_timer_adapter(1, TIMER_1_IRQN, irq_prio(TIMER_1_IRQN));
    uapi_systick_init();
    uapi_tcxo_init();
    APP_PRINT("minimal heartbeat init ok.\r\n");
}

static void minimal_watchdog_stop(void)
{
#if defined(CONFIG_DRIVER_SUPPORT_WDT)
    if (uapi_watchdog_init(MIN_WDT_TIMEOUT_SEC) == ERRCODE_SUCC) {
        (void)uapi_watchdog_disable();
        APP_PRINT("watchdog disabled.\r\n");
    } else {
        APP_PRINT("watchdog init failed.\r\n");
    }
#endif
}

static void minimal_heartbeat_task(void *argument)
{
    uint32_t delay_ticks = osKernelGetTickFreq();

    (void)argument;
    if (delay_ticks == 0) {
        delay_ticks = 100;
    }

    APP_PRINT("liteos heartbeat task start.\r\n");
    for (;;) {
        APP_PRINT("liteos task hello tick=%u.\r\n", osKernelGetTickCount());
        (void)uapi_watchdog_kick();
        (void)osDelay(delay_ticks);
    }
}

static osThreadId_t minimal_create_heartbeat_task(void)
{
    const osThreadAttr_t attr = {
        .name = "heartbeat",
        .stack_size = MIN_HEARTBEAT_STACK_SIZE,
        .priority = osPriorityNormal,
    };

    return osThreadNew(minimal_heartbeat_task, NULL, &attr);
}

LITE_OS_SEC_TEXT_INIT int main(void)
{
    osStatus_t kernel_state;
    osThreadId_t heartbeat_task;

    patch_init();
    uapi_partition_init();
    pmp_enable();
    cpu_cache_init();

    LOS_PrepareMainTask();
    kernel_state = osKernelInitialize();
    minimal_hw_init();
    minimal_watchdog_stop();

    APP_PRINT("kernel initialize state=%d.\r\n", kernel_state);
    heartbeat_task = minimal_create_heartbeat_task();
    if (heartbeat_task == NULL) {
        APP_PRINT("create heartbeat task failed.\r\n");
        for (;;) {
            (void)uapi_watchdog_kick();
            (void)uapi_tcxo_delay_ms(1000);
        }
    }

    APP_PRINT("create heartbeat task ok.\r\n");
    APP_PRINT("kernel start.\r\n");
    kernel_state = osKernelStart();
    APP_PRINT("kernel start returned state=%d.\r\n", kernel_state);

    for (;;) {
        (void)uapi_watchdog_kick();
        (void)uapi_tcxo_delay_ms(1000);
    }
    return 0;
}

__attribute__((section(".text.runtime.init"))) void copy_bin_to_ram(unsigned int *start_addr,
    const unsigned int *const load_addr, unsigned int size)
{
    unsigned int i;

    for (i = 0; i < size / sizeof(unsigned int); i++) {
        *(start_addr + i) = *(load_addr + i);
    }
}

__attribute__((section(".text.runtime.init"))) void init_mem_value(unsigned int *start_addr,
    const unsigned int *const end_addr, unsigned int init_val)
{
    unsigned int *dest = start_addr;

    while (dest < end_addr) {
        *dest = init_val;
        dest++;
    }
}

__attribute__((section(".text.runtime.init"))) void do_relocation(void)
{
    copy_bin_to_ram(&__rom_data_begin__, &__rom_data_load__, (unsigned int)&__rom_data_size__);
    init_mem_value(&__rom_bss_begin__, &__rom_bss_end__, 0);

    copy_bin_to_ram(&__rom_patch_begin__, &__rom_patch_load__, (unsigned int)&__rom_patch_size__);

    copy_bin_to_ram(&__tcm_text_begin__, &__tcm_text_load__, (unsigned int)&__tcm_text_size__);
    copy_bin_to_ram(&__tcm_data_begin__, &__tcm_data_load__, (unsigned int)&__tcm_data_size__);
    init_mem_value(&__tcm_bss_begin__, &__tcm_bss_end__, 0);

    copy_bin_to_ram(&__sram_text_begin__, &__sram_text_load__, (unsigned int)&__sram_text_size__);
    copy_bin_to_ram(&__data_begin__, &__data_load__, (unsigned int)&__data_size__);

#ifdef CONFIG_MEMORY_CUSTOMIZE_RSV
    init_mem_value(&__mem_rsv_begin__, &__mem_rsv_end__, 0);
#endif

    init_mem_value(&__bss_begin__, &__bss_end__, 0);
}

__attribute__((section(".text.runtime.init"))) void runtime_init(void)
{
    dyn_mem_cfg();
#ifndef CHIP_EDA
    do_relocation();
#endif
    main();
}
