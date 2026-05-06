/**
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2022-2023. All rights reserved.
 *
 * Description: WS63 clock initialization helpers.
 */
#include "chip_io.h"
#include "tcxo.h"
#include "soc_porting.h"
#include "platform_core.h"
#include "timer_porting.h"
#include "watchdog_porting.h"
#include "tcxo_porting.h"
#include "uart_porting.h"
#include "i2c_porting.h"
#include "clock_init.h"

#ifdef BOARD_ASIC
#define PMU_RXLDO_1P0       0x40003134
#define PMU_PLLLDO_1P0      0x40003148
#define PMU_LNALDO_1P0      0x40003138
#define PMU_VCOLDO_1P0      0x40003140
#define PMU_TXLDO_1P8       0x40003168
#define PMU_ABBLDO_1P0      0x40003144
#define ISO_SIG             0x40003230
#define CLDO_SUB_CRG_CKEN_CTL1 0x44001104
#define DBG_CMU_XO_PD       0x40003400
#define CLDO_CRG_CLK_SEL    0x44001134
#define DELAY_120_US        120
#define DELAY_10_US         10
#define DELAY_1_US          1
#define LDO_EN              0x4070
#define LDO_EN_DELAY_ENABLE 0x6070
#define PMU_VCOLDO_1P0_EN   0x4000
#define PMU_VCOLDO_1P0_EN_DELAY   0x6000
#define PMU_TXLDO_1P8_EN    0x4030
#define PMU_TXLDO_1P8_EN_DELAY    0x6030
#define RELEASE_ADDA_ISOLATION_SIGNAL  0x3
#define ONEN_RF_PLL         0x5
#define OFFSET_0 0
#define TRNG_DIV2 2
#define TRNG_DIV4 4
#define TRNG_LEN 3
#define TSENSOR_DIV_LEN 6
#define TSENSOR_24_DIV  0x18
#define TSENSOR_40_DIV  0x28

void open_rf_power(void)
{
    writel(CMU_XO_SIG, 0X30000);
    writel(CMU_XO_SIG, 0X3083C);

    writel(PMU_RXLDO_1P0, LDO_EN);
    uapi_tcxo_delay_us(DELAY_120_US);
    writel(PMU_RXLDO_1P0, LDO_EN_DELAY_ENABLE);
    uapi_tcxo_delay_us(DELAY_10_US);
    writel(PMU_PLLLDO_1P0, 0x4080);
    uapi_tcxo_delay_us(DELAY_120_US);
    writel(PMU_PLLLDO_1P0, 0x6080);
    uapi_tcxo_delay_us(DELAY_10_US);
    writel(PMU_LNALDO_1P0, LDO_EN);
    uapi_tcxo_delay_us(DELAY_120_US);
    writel(PMU_LNALDO_1P0, LDO_EN_DELAY_ENABLE);
    uapi_tcxo_delay_us(DELAY_10_US);
    writel(PMU_VCOLDO_1P0, PMU_VCOLDO_1P0_EN);
    uapi_tcxo_delay_us(DELAY_120_US);
    writel(PMU_VCOLDO_1P0, PMU_VCOLDO_1P0_EN_DELAY);
    uapi_tcxo_delay_us(DELAY_10_US);
    writel(PMU_TXLDO_1P8, PMU_TXLDO_1P8_EN);
    uapi_tcxo_delay_us(DELAY_120_US);
    writel(PMU_TXLDO_1P8, PMU_TXLDO_1P8_EN_DELAY);
    uapi_tcxo_delay_us(DELAY_10_US);
    writel(PMU_ABBLDO_1P0, LDO_EN);
    uapi_tcxo_delay_us(DELAY_120_US);
    writel(PMU_ABBLDO_1P0, LDO_EN_DELAY_ENABLE);
    writel(ISO_SIG, RELEASE_ADDA_ISOLATION_SIGNAL);
}

#define CLDO_CRG_DIV_CTL9 0x4400112c
#define CLDO_CRG_DIV_CTL7 0x44001124
void switch_clock(void)
{
    writel(DBG_CMU_XO_PD, ONEN_RF_PLL);
    uapi_tcxo_delay_us(DELAY_10_US);
    reg_setbit(CLDO_CRG_CLK_SEL, OFFSET_0, POS_20);
    reg_clrbit(CLDO_SUB_CRG_CKEN_CTL1, OFFSET_0, POS_13);
    uapi_tcxo_delay_us(DELAY_1_US);
    reg_setbit(CLDO_CRG_CLK_SEL, OFFSET_0, POS_19);
    uapi_tcxo_delay_us(DELAY_1_US);
    reg_setbit(CLDO_SUB_CRG_CKEN_CTL1, OFFSET_0, POS_13);
    reg_setbit(CLDO_CRG_CLK_SEL, OFFSET_0, POS_0);

    reg_clrbit(CLDO_SUB_CRG_CKEN_CTL1, 0, POS_18);
    reg_clrbit(CLDO_SUB_CRG_CKEN_CTL1, 0, POS_19);
    reg_clrbit(CLDO_SUB_CRG_CKEN_CTL1, 0, POS_20);
    reg_setbit(CLDO_CRG_CLK_SEL, 0, POS_1);
    reg_setbit(CLDO_CRG_CLK_SEL, 0, POS_2);
    reg_setbit(CLDO_CRG_CLK_SEL, 0, POS_3);
    reg_setbit(CLDO_SUB_CRG_CKEN_CTL1, 0, POS_18);
    reg_setbit(CLDO_SUB_CRG_CKEN_CTL1, 0, POS_19);
    reg_setbit(CLDO_SUB_CRG_CKEN_CTL1, 0, POS_20);

    if (get_tcxo_freq() == CLK24M_TCXO) {
        reg_clrbit(CLDO_CRG_DIV_CTL9, 0, POS_9);
        reg32_setbits(CLDO_CRG_DIV_CTL9, POS_6, TRNG_LEN, TRNG_DIV2);
        reg_setbit(CLDO_CRG_DIV_CTL9, 0, POS_9);
        reg_clrbit(CLDO_CRG_DIV_CTL7, 0, POS_18);
        reg32_setbits(CLDO_CRG_DIV_CTL7, POS_6, TSENSOR_DIV_LEN, TSENSOR_24_DIV);
        reg_setbit(CLDO_CRG_DIV_CTL7, 0, POS_18);
    } else {
        reg_clrbit(CLDO_CRG_DIV_CTL9, 0, POS_9);
        reg32_setbits(CLDO_CRG_DIV_CTL9, POS_6, TRNG_LEN, TRNG_DIV4);
        reg_setbit(CLDO_CRG_DIV_CTL9, 0, POS_9);
        reg_clrbit(CLDO_CRG_DIV_CTL7, 0, POS_18);
        reg32_setbits(CLDO_CRG_DIV_CTL7, POS_6, TSENSOR_DIV_LEN, TSENSOR_40_DIV);
        reg_setbit(CLDO_CRG_DIV_CTL7, 0, POS_18);
    }
}

#define REQ_24M         24000000
#define REQ_40M         40000000
#define USEC_PER_SEC    1000000
#define UART_PLL_CLOCK  160000000
void set_uart_tcxo_clock_period(void)
{
    if (get_tcxo_freq() == CLK24M_TCXO) {
        tcxo_porting_ticks_per_usec_set(REQ_24M / USEC_PER_SEC);
        timer_porting_clock_value_set(REQ_24M);
        LOS_SetSysClosk(REQ_24M);
        watchdog_port_set_clock(REQ_24M);
        i2c_port_set_clock_value(I2C_BUS_0, REQ_24M);
        i2c_port_set_clock_value(I2C_BUS_1, REQ_24M);
    } else {
        tcxo_porting_ticks_per_usec_set(REQ_40M / USEC_PER_SEC);
        timer_porting_clock_value_set(REQ_40M);
        LOS_SetSysClosk(REQ_40M);
        watchdog_port_set_clock(REQ_40M);
        i2c_port_set_clock_value(I2C_BUS_0, REQ_40M);
        i2c_port_set_clock_value(I2C_BUS_1, REQ_40M);
    }
    uart_port_set_clock_value(UART_BUS_0, UART_PLL_CLOCK);
}
#endif
