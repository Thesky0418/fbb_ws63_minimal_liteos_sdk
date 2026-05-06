/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2022-2023. All rights reserved.
 * Description: Clock initialization helpers used by WS63 porting code.
 */

#ifndef CLOCK_INIT_H
#define CLOCK_INIT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CMU_XO_SIG 0X40003404
#define XO_TRIM_ENABLE 1
#define SIZE_8_BITS 8
#define EFUSE_GROUP_MAX 3
#define EXT_EFUSE_LOCK_XO_TRIM_BIT_POS 7
#define EXT_EFUSE_XO_TRIM_1_ID  144
#define EXT_EFUSE_XO_TRIM_2_ID  162
#define EXT_EFUSE_XO_TRIM_3_ID  180
#define EXT_EFUSE_RSSI_BAND3_1_ID  160
#define EXT_EFUSE_RSSI_BAND3_2_ID  178
#define EXT_EFUSE_RSSI_BAND3_3_ID  196

typedef union {
    struct {
        uint32_t rg_cmu_xo_trim_fine : 8;
        uint32_t rg_cmu_xo_trim_coarse : 4;
        uint32_t rg_cmu_xo_trim_rev : 4;
        uint32_t rg_cmu_xo_trim_fine_sel : 1;
        uint32_t rg_cmu_xo_trim_coarse_sel : 1;
        uint32_t rg_cmu_xo_trim_rev_sel : 1;
        uint32_t reserved_0 : 13;
    } bits;
    uint32_t u32;
} u_cmu_xo_sig;

extern void LOS_SetSysClosk(uint32_t clock);

void open_rf_power(void);
void switch_clock(void);
void set_uart_tcxo_clock_period(void);

#ifdef __cplusplus
}
#endif

#endif
