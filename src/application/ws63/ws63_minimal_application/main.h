/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2022-2023. All rights reserved.
 * Description: Minimal WS63 LiteOS application entry.
 */

#ifndef WS63_MINIMAL_MAIN_H
#define WS63_MINIMAL_MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

__attribute__((section(".text.runtime.init"))) void copy_bin_to_ram(unsigned int *start_addr,
    const unsigned int *const load_addr, unsigned int size);
__attribute__((section(".text.runtime.init"))) void init_mem_value(unsigned int *start_addr,
    const unsigned int *const end_addr, unsigned int init_val);
__attribute__((section(".text.runtime.init"))) void do_relocation(void);
__attribute__((section(".text.runtime.init"))) void runtime_init(void);

extern void LOS_PrepareMainTask(void);

#ifdef __cplusplus
}
#endif

#endif
