/*******************************************************************************
* @file  rsi_pll.h
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/

/**
 * Includes
 */
#ifndef __RSI_PLL_H__
#define __RSI_PLL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "rsi_ccp_common.h"
#include "rsi_error.h"
#include "rsi_ipmu.h"
#include "rsi_reg_spi.h"

#define MISC_CFG_MISC_CTRL (*((uint32_t volatile *)(MISC_CONFIG_BASE + 0x14)))

typedef void (*cdDelay)(uint32_t delay);

#define CCI_SYNC_MODE BIT(16) /*  Enables CCI_SYNC_MODE */
/*M4 Clock configuration base address */

/*SET Reg 1  SET / CLR*/
#define USART1_PCLK_ENABLE BIT(0) /*  Enables USART1_PCLK_ENABLE */
#define USART1_SCLK_ENABLE BIT(1) /*  Enables USART1_SCLK_ENABLE */
#define USART2_PCLK_ENABLE BIT(2) /*  Enables USART2_PCLK_ENABLE */
#define USART2_SCLK_ENABLE BIT(3) /*  Enables USART2_SCLK_ENABLE */
#if defined(SLI_SI917B0)
#define QSPI_2_CLK_ENABLE        BIT(4) /*  Enables QSPI_CLK_ENABLE */
#define QSPI_2_HCLK_ENABLE       BIT(5) /*  Enables QSPI_HCLK_ENABLE */
#define QSPI_2_M4_SOC_SYNC       BIT(6) /*  Enables QSPI_M4_SOC_SYNC */
#define QSPI_2_CLK_ONEHOT_ENABLE BIT(7) /*  Enables QSPI_CLK_ONEHOT_ENABLE */
#endif
#define CT_CLK_ENABLE          BIT(9)  /*  Enables CT_CLK_ENABLE */
#define CT_PCLK_ENABLE         BIT(10) /*  Enables CT_PCLK_ENABLE */
#define ICACHE_CLK_ENABLE      BIT(11) /*  Enables ICACHE_CLK_ENABLE */
#define ICACHE_CLK_2X_ENABLE   BIT(12) /*  Enables ICACHE_CLK_2X_ENABLE */
#define RPDMA_HCLK_ENABLE      BIT(13) /*  Enables RPDMA_HCLK_ENABLE */
#define SOC_PLL_SPI_CLK_ENABLE BIT(14) /*  Enables SOC_PLL_SPI_CLK_ENABLE */
#define IID_CLK_ENABLE         BIT(16) /*  Enables IID_CLK_ENABLE */
#define SDIO_SYS_HCLK_ENABLE   BIT(17) /*  Enables SDIO_SYS_HCLK_ENABLE */
#define CRC_CLK_ENABLE_M4      BIT(18) /*  Enables CRC_CLK_ENABLE_M4 */
#define M4SS_UM_CLK_STATIC_EN  BIT(19) /*  Enables M4SS_UM_CLK_STATIC_EN */
#define ETH_HCLK_ENABLE        BIT(21) /*  Enables ETH_HCLK_ENABLE */
#define HWRNG_PCLK_ENABLE      BIT(22) /*  Enables HWRNG_PCLK_ENABLE */
#define GNSS_MEM_CLK_ENABLE    BIT(23) /*  Enables GNSS_MEM_CLK_ENABLE */
#define CCI_PCLK_ENABLE        BIT(24) /*  Enables CCI_PCLK_ENABLE */
#define CCI_HCLK_ENABLE        BIT(25) /*  Enables CCI_HCLK_ENABLE */
#define CCI_CLK_ENABLE         BIT(26) /*  Enables CCI_CLK_ENABLE */
#define MASK_HOST_CLK_WAIT_FIX BIT(27) /*  Enables MASK_HOST_CLK_WAIT_FIX */
#define MASK31_HOST_CLK_CNT    BIT(28) /*  Enables MASK31_HOST_CLK_CNT */
#define SD_MEM_INTF_CLK_ENABLE \
  BIT(29) /*  Static Clock gating Enable for sd_mem_intf clk1'b1 => Clock is enabled1'b0 => Invalid*/
#define MASK_HOST_CLK_AVAILABLE_FIX BIT(30) /*  Enables MASK_HOST_CLK_AVAILABLE_FIX */
#define ULPSS_CLK_ENABLE            BIT(31) /*  Enables ULPSS_CLK_ENABLE */

/*SER Reg2  SET / CLR*/
#define GEN_SPI_MST1_HCLK_ENABLE BIT(0)  /*  Enables GEN_SPI_MST1_HCLK_ENABLE */
#define CAN1_PCLK_ENABLE         BIT(2)  /*  Enables CAN1_PCLK_ENABLE */
#define CAN1_CLK_ENABLE          BIT(3)  /*  Enables CAN1_CLK_ENABLE */
#define UDMA_HCLK_ENABLE         BIT(6)  /*  Enables UDMA_HCLK_ENABLE */
#define I2C_BUS_CLK_ENABLE       BIT(7)  /*  Enables I2C_1_BUS_CLK_ENABLE */
#define I2C_2_BUS_CLK_ENABLE     BIT(8)  /*  Enables I2C_2_BUS_CLK_ENABLE */
#define SSI_SLV_PCLK_ENABLE      BIT(9)  /*  Enables SSI_SLV_PCLK_ENABLE */
#define SSI_SLV_SCLK_ENABLE      BIT(10) /*  Enables SSI_SLV_SCLK_ENABLE */
#define QSPI_CLK_ENABLE          BIT(11) /*  Enables QSPI_CLK_ENABLE */
#define QSPI_HCLK_ENABLE         BIT(12) /*  Enables QSPI_HCLK_ENABLE */
#define I2SM_SCLK_ENABLE         BIT(13) /*  Enables I2SM_SCLK_ENABLE */
#define I2SM_INTF_SCLK_ENABLE    BIT(14) /*  Enables I2SM_INTF_SCLK_ENABLE */
#define I2SM_PCLK_ENABLE         BIT(15) /*  Enables I2SM_PCLK_ENABLE */
#define QE_PCLK_ENABLE           BIT(17) /*  Enables QE_PCLK_ENABLE */
#define MCPWM_PCLK_ENABLE        BIT(18) /*  Enables MCPWM_PCLK_ENABLE */
#define SGPIO_PCLK_ENABLE        BIT(20) /*  Enables SGPIO_PCLK_ENABLE */
#define EGPIO_PCLK_ENABLE        BIT(21) /*  Enables EGPIO_PCLK_ENABLE */
#define ARM_CLK_ENABLE           BIT(22) /*  Enables ARM_CLK_ENABLE */
#define SSI_MST_PCLK_ENABLE      BIT(23) /*  Enables SSI_MST_PCLK_ENABLE */
#define SSI_MST_SCLK_ENABLE      BIT(24) /*  Enables SSI_MST_SCLK_ENABLE */
#define MEM2_CLK_ENABLE          BIT(25) /*  Enables MEM2_CLK_ENABLE */
#define MEM_CLK_ULP_ENABLE       BIT(26) /*  Enables MEM_CLK_ULP_ENABLE */
#define ROM_CLK_ENABLE           BIT(27) /*  Enables ROM_CLK_ENABLE */
#define PLL_INTF_CLK_ENABLE      BIT(28) /*  Enables PLL_INTF_CLK_ENABLE */
#define SEMAPHORE_CLK_ENABLE     BIT(29) /*  Enables SEMAPHORE_CLK_ENABLE */
#define TOT_CLK_ENABLE           BIT(30) /*  Enables TOT_CLK_ENABLE */
#define RMII_SOFT_RESET          BIT(31) /*  Enables RMII_SOFT_RESET */

/*SET Reg 3 SET / CLR*/
#define BUS_CLK_ENABLE                   BIT(0)  /*  Enables BUS_CLK_ENABLE */
#define M4_CORE_CLK_ENABLE               BIT(1)  /*  Enables M4_CORE_CLK_ENABLE */
#define CM_BUS_CLK_ENABLE                BIT(2)  /*  Enables CM_BUS_CLK_ENABLE */
#define MISC_CONFIG_PCLK_ENABLE          BIT(4)  /*  Enables MISC_CONFIG_PCLK_ENABLE */
#define EFUSE_CLK_ENABLE                 BIT(5)  /*  Enables EFUSE_CLK_ENABLE */
#define ICM_CLK_ENABLE                   BIT(6)  /*  Enables ICM_CLK_ENABLE */
#define MEM1_CLK_ENABLE                  BIT(7)  /*  Enables MEM1_CLK_ENABLE */
#define MEM3_CLK_ENABLE                  BIT(8)  /*  Enables MEM3_CLK_ENABLE */
#define USB_PHY_CLK_IN_ENABLE            BIT(12) /*  Enables USB_PHY_CLK_IN_ENABLE */
#define QSPI_CLK_ONEHOT_ENABLE           BIT(13) /*  Enables QSPI_CLK_ONEHOT_ENABLE */
#define QSPI_M4_SOC_SYNC                 BIT(14) /*  Enables QSPI_M4_SOC_SYNC */
#define EGPIO_CLK_ENABLE                 BIT(16) /*  Enables EGPIO_CLK_ENABLE */
#define I2C_CLK_ENABLE                   BIT(17) /*  Enables I2C_CLK_ENABLE */
#define I2C_2_CLK_ENABLE                 BIT(18) /*  Enables I2C_2_CLK_ENABLE */
#define EFUSE_PCLK_ENABLE                BIT(19) /*  Enables EFUSE_PCLK_ENABLE */
#define SGPIO_CLK_ENABLE                 BIT(20) /*  Enables SGPIO_CLK_ENABLE */
#define TASS_M4SS_64K_SWITCH_CLK_ENABLE  BIT(21) /*  Enables TASS_M4SS_64K_SWITCH_CLK_ENABLE */
#define TASS_M4SS_128K_SWITCH_CLK_ENABLE BIT(22) /*  Enables TASS_M4SS_128K_SWITCH_CLK_ENABLE */
#define TASS_M4SS_SDIO_SWITCH_CLK_ENABLE BIT(23) /*  Enables TASS_M4SS_SDIO_SWITCH_CLK_ENABLE */
#define TASS_M4SS_USB_SWITCH_CLK_ENABLE  BIT(24) /*  Enables TASS_M4SS_USB_SWITCH_CLK_ENABLE */
#define ROM_MISC_STATIC_ENABLE           BIT(25) /*  Enables ROM_MISC_STATIC_ENABLE */
#define M4_SOC_CLK_FOR_OTHER_ENABLE      BIT(26) /*  Enables M4_SOC_CLK_FOR_OTHER_ENABLE */
#define ICACHE_ENABLE                    BIT(27) /*  Enables ICACHE_ENABLE */

/*DYN_CLK_GATE_DISABLE_REG */

#define SDIO_SYS_HCLK_DYN_CTRL_DISABLE    BIT(0)  /*  Enables SDIO_SYS_HCLK_DYN_CTRL_DISABLE */
#define BUS_CLK_DYN_CTRL_DISABLE          BIT(1)  /*  Enables BUS_CLK_DYN_CTRL_DISABLE */
#define GPDMA_HCLK_DYN_CTRL_DISABLE       BIT(4)  /*  Enables GPDMA_HCLK_DYN_CTRL_DISABLE */
#define EGPIO_PCLK_DYN_CTRL_DISABLE       BIT(5)  /*  Enables EGPIO_PCLK_DYN_CTRL_DISABLE */
#define SGPIO_PCLK_DYN_CTRL_DISABLE       BIT(6)  /*  Enables SGPIO_PCLK_DYN_CTRL_DISABLE */
#define TOT_CLK_DYN_CTRL_DISABLE          BIT(7)  /*  Enables TOT_CLK_DYN_CTRL_DISABLE */
#define HWRNG_PCLK_DYN_CTRL_DISABLE       BIT(8)  /*  Enables HWRNG_PCLK_DYN_CTRL_DISABLE */
#define USART1_SCLK_DYN_CTRL_DISABLE      BIT(9)  /*  Enables USART1_SCLK_DYN_CTRL_DISABLE */
#define USART1_PCLK_DYN_CTRL_DISABLE      BIT(10) /*  Enables USART1_PCLK_DYN_CTRL_DISABLE */
#define USART2_SCLK_DYN_CTRL_DISABLE      BIT(11) /*  Enables USART2_SCLK_DYN_CTRL_DISABLE */
#define USART2_PCLK_DYN_CTRL_DISABLE      BIT(12) /*  Enables USART2_PCLK_DYN_CTRL_DISABLE */
#define SSI_SLV_SCLK_DYN_CTRL_DISABLE     BIT(15) /*  Enables SSI_SLV_SCLK_DYN_CTRL_DISABLE */
#define SSI_SLV_PCLK_DYN_CTRL_DISABLE     BIT(16) /*  Enables SSI_SLV_PCLK_DYN_CTRL_DISABLE */
#define I2SM_INTF_SCLK_DYN_CTRL_DISABLE   BIT(18) /*  Enables I2SM_INTF_SCLK_DYN_CTRL_DISABLE */
#define SEMAPHORE_CLK_DYN_CTRL_DISABLE    BIT(19) /*  Enables SEMAPHORE_CLK_DYN_CTRL_DISABLE */
#define ARM_CLK_DYN_CTRL_DISABLE          BIT(20) /*  Enables ARM_CLK_DYN_CTRL_DISABLE */
#define SSI_MST_SCLK_DYN_CTRL_DISABLE     BIT(21) /*  Enables SSI_MST_SCLK_DYN_CTRL_DISABLE */
#define MEM1_CLK_DYN_CTRL_DISABLE         BIT(22) /*  Enables MEM1_CLK_DYN_CTRL_DISABLE */
#define MEM2_CLK_DYN_CTRL_DISABLE         BIT(23) /*  Enables MEM2_CLK_DYN_CTRL_DISABLE */
#define MEM_CLK_ULP_DYN_CTRL_DISABLE      BIT(24) /*  Enables MEM_CLK_ULP_DYN_CTRL_DISABLE */
#define MEM3_CLK_DYN_CTRL_DISABLE         BIT(25) /*  Enables MEM3_CLK_DYN_CTRL_DISABLE */
#define SSI_MST_PCLK_DYN_CTRL_DISABLE     BIT(28) /*  Enables SSI_MST_PCLK_DYN_CTRL_DISABLE */
#define ICACHE_DYN_GATING_DISABLE         BIT(29) /*  Enables ICACHE_DYN_GATING_DISABLE */
#define CCI_PCLK_DYN_CTRL_DISABLE         BIT(30) /*  Enables CCI_PCLK_DYN_CTRL_DISABLE */
#define MISC_CONFIG_PCLK_DYN_CTRL_DISABLE BIT(31) /*  Enables MISC_CONFIG_PCLK_DYN_CTRL_DISABLE */

/*DYN_CLK_GATE_DISABLE_REG2 */
#define SOC_PLL_SPI_CLK_DYN_CTRL_DISABLE BIT(0) /*  Enables SOC_PLL_SPI_CLK_DYN_CTRL_DISABLE */
#define I2C_BUS_DYN_CTRL_DISABLE         BIT(1) /*  Enables I2C_BUS_DYN_CTRL_DISABLE */
#define I2C_2_BUS_CLK_DYN_CTRL_DISABLE   BIT(2) /*  Enables I2C_2_BUS_CLK_DYN_CTRL_DISABLE */
#define CT_PCLK_DYN_CTRL_DISABLE         BIT(3) /*  Enables SCT_PCLK_DYN_CTRL_DISABLE */
#define CAN1_PCLK_DYN_CTRL_DISABLE       BIT(4) /*  Enables CAN1_PCLK_DYN_CTRL_DISABLE */
#define I2SM_PCLK_DYN_CTRL_DISABLE       BIT(5) /*  Enables I2SM_PCLK_DYN_CTRL_DISABLE */
#define EFUSE_CLK_DYN_CTRL_DISABLE       BIT(6) /*  Enables EFUSE_CLK_DYN_CTRL_DISABLE */
#define EFUSE_PCLK_DYN_CTRL_DISABLE      BIT(7) /*  Enables EFUSE_PCLK_DYN_CTRL_DISABLE */
#define PWR_CTRL_CLK_DYN_CTRL_DISABLE    BIT(8) /*  Enables PWR_CTRL_CLK_DYN_CTRL_DISABLE */

/*SOC_Pll Clock frequency checks */
#define SOC_PLL_MIN_FREQUECY 1000000   /*  Minimum frequency for SOC_PLL*/
#define SOC_PLL_MAX_FREQUECY 300000000 /*  Maximum frequency for SOC_PLL*/

/*SOC_Pll Clock frequency checks */
#define INTF_PLL_MIN_FREQUECY 1000000   /*  Minimum frequency for SOC_PLL*/
#define INTF_PLL_MAX_FREQUECY 300000000 /*  Maximum frequency for SOC_PLL*/

#define I2S_DCO_FREQ1 73728000 /*  i2s_dco_freq1*/
#define I2S_DCO_FREQ2 67737600 /*  i2s_dco_freq2*/

/*SOCPLL_MACRO_REG_ACCESS*/
#define SOCPLLMACROREG1 0x00 /*  Address for SOCPLLMACROREG1 register Access*/
#define SOCPLLMACROREG2 0x01 /*  Address for SOCPLLMACROREG2 register Access*/
#define SOCPLLMACROREG3 0x02 /*  Address for SOCPLLMACROREG3 register Access*/
#define SOCPLLMACROREG4 0x03 /*  Address for SOCPLLMACROREG4 register Access*/
#define SOCPLLMACROREG5 0x04 /*  Address for SOCPLLMACROREG5 register Access*/

/*SOCPLLMACROREG*/
#define LDO_PROG_SOCPLL  (0xE000) /*  Mask value for LDO_PROG_SOCPLL*/
#define LDO_PROG_INTFPLL (0x1C00) /*  Mask value for LDO_PROG_INTFPLL*/
#define LDO_PROG_I2SPLL  (0x0380) /*  Mask value for LDO_PROG_I2SPLL*/

/* SOC_PLL_REG_ACCESS */
#define SOC_PLL_500_CTRL_REG1  0x10 /*  Address for SOC_PLL_500_CTRL_REG1 register Access*/
#define SOC_PLL_500_CTRL_REG2  0x11 /*  Address for SOC_PLL_500_CTRL_REG2 register Access*/
#define SOC_PLL_500_CTRL_REG3  0x12 /*  Address for SOC_PLL_500_CTRL_REG3 register Access*/
#define SOC_PLL_500_CTRL_REG4  0x13 /*  Address for SOC_PLL_500_CTRL_REG4 register Access*/
#define SOC_PLL_500_CTRL_REG5  0x14 /*  Address for SOC_PLL_500_CTRL_REG5 register Access*/
#define SOC_PLL_500_CTRL_REG6  0x15 /*  Address for SOC_PLL_500_CTRL_REG6 register Access*/
#define SOC_PLL_500_CTRL_REG7  0x16 /*  Address for SOC_PLL_500_CTRL_REG7 register Access*/
#define SOC_PLL_500_CTRL_REG8  0x17 /*  Address for SOC_PLL_500_CTRL_REG8 register Access*/
#define SOC_PLL_500_CTRL_REG9  0x18 /*  Address for SOC_PLL_500_CTRL_REG9 register Access*/
#define SOC_PLL_500_CTRL_REG10 0x19 /*  Address for SOC_PLL_500_CTRL_REG10 register Access*/
#define SOC_PLL_500_CTRL_REG11 0x1A /*  Address for SOC_PLL_500_CTRL_REG11 register Access*/
#define SOC_PLL_500_CTRL_REG12 0x1B /*  Address for SOC_PLL_500_CTRL_REG12 register Access*/
#define SOC_PLL_500_CTRL_REG13 0x1C /*  Address for SOC_PLL_500_CTRL_REG13 register Access*/

/* INTF_PLL_REG_ACCESS */
#define INTF_PLL_500_CTRL_REG1  0x20 /*  Address for INTF_PLL_500_CTRL_REG1 register Access*/
#define INTF_PLL_500_CTRL_REG2  0x21 /*  Address for INTF_PLL_500_CTRL_REG2 register Access*/
#define INTF_PLL_500_CTRL_REG3  0x22 /*  Address for INTF_PLL_500_CTRL_REG3 register Access*/
#define INTF_PLL_500_CTRL_REG4  0x23 /*  Address for INTF_PLL_500_CTRL_REG4 register Access*/
#define INTF_PLL_500_CTRL_REG5  0x24 /*  Address for INTF_PLL_500_CTRL_REG5 register Access*/
#define INTF_PLL_500_CTRL_REG6  0x25 /*  Address for INTF_PLL_500_CTRL_REG6 register Access*/
#define INTF_PLL_500_CTRL_REG7  0x26 /*  Address for INTF_PLL_500_CTRL_REG7 register Access*/
#define INTF_PLL_500_CTRL_REG8  0x27 /*  Address for INTF_PLL_500_CTRL_REG8 register Access*/
#define INTF_PLL_500_CTRL_REG9  0x28 /*  Address for INTF_PLL_500_CTRL_REG9 register Access*/
#define INTF_PLL_500_CTRL_REG10 0x29 /*  Address for INTF_PLL_500_CTRL_REG10 register Access*/
#define INTF_PLL_500_CTRL_REG11 0x2A /*  Address for INTF_PLL_500_CTRL_REG11 register Access*/
#define INTF_PLL_500_CTRL_REG12 0x2B /*  Address for INTF_PLL_500_CTRL_REG12 register Access*/
#define INTF_PLL_500_CTRL_REG13 0x2C /*  Address for INTF_PLL_500_CTRL_REG13 register Access*/

/* I2S_PLL_REG_ACCESS */
#define I2S_PLL_CTRL_REG1  0x30 /*  Address for I2S_PLL_CTRL_REG1 register Access*/
#define I2S_PLL_CTRL_REG2  0x31 /*  Address for I2S_PLL_CTRL_REG2 register Access*/
#define I2S_PLL_CTRL_REG3  0x32 /*  Address for I2S_PLL_CTRL_REG3 register Access*/
#define I2S_PLL_CTRL_REG4  0x33 /*  Address for I2S_PLL_CTRL_REG4 register Access*/
#define I2S_PLL_CTRL_REG5  0x34 /*  Address for I2S_PLL_CTRL_REG5 register Access*/
#define I2S_PLL_CTRL_REG6  0x35 /*  Address for I2S_PLL_CTRL_REG6 register Access*/
#define I2S_PLL_CTRL_REG7  0x36 /*  Address for I2S_PLL_CTRL_REG7 register Access*/
#define I2S_PLL_CTRL_REG8  0x37 /*  Address for I2S_PLL_CTRL_REG8 register Access*/
#define I2S_PLL_CTRL_REG9  0x38 /*  Address for I2S_PLL_CTRL_REG9 register Access*/
#define I2S_PLL_CTRL_REG10 0x39 /*  Address for I2S_PLL_CTRL_REG10 register Access*/
#define I2S_PLL_CTRL_REG11 0x3A /*  Address for I2S_PLL_CTRL_REG11 register Access*/
#define I2S_PLL_CTRL_REG12 0x3B /*  Address for I2S_PLL_CTRL_REG12 register Access*/
#define I2S_PLL_CTRL_REG13 0x3C /*  Address for I2S_PLL_CTRL_REG13 register Access*/

/* AFE_PLL_CTRL_REG_REG_ACCESS */
#define AFEPLLCTRLREG1 0x07 /*  Address for AFEPLLCTRLREG1 register Access*/
#define AFEPLLCTRLREG2 0x08 /*  Address for AFEPLLCTRLREG1 register Access*/

#define MEMS_REF_CLK_ENABLE BIT(6)

/*SOC_PLL_500_CTRL_REG1/INTF_PLL_500_CTRL_REG1/I2S_PLL_CTRL_REG_1 */
#define DCO_FIX_SEL_MASK   (0x0003) /*  Mask value for DCO_FIX_SEL_MASK*/
#define PLL_500_BYPASS     BIT(2)   /*  Enables PLL_500_BYPASS */
#define PLL_500_CLK_ENABLE BIT(3)   /*  Enables PLL_500_CLK_ENABLE */
#define PLL_500_PD         BIT(4)   /*  Enables PLL_500_PD */
#define PLL_500_RST        BIT(5)   /*  Enables PLL_500_RST */
#define PLL_500_M_MASK     (0xFFC0) /*  Mask value for PLL_500_M_MASK*/
/*I2S_PLL_CTRL_REG_1*/
#define I2S_PLL_CLK_ENABLE BIT(2) /*  Enables I2S_PLL_CLK_ENABLE */
#define I2S_PLL_BYPASS     BIT(3) /*  Enables I2S_PLL_BYPASS */

/*SOC_PLL_500_CTRL_REG2/INTF_PLL_500_CTRL_REG 2 */
#define PLL_500_N_MASK (0x01F8) /*  Mask value for PLL_500_N_MASK*/
#define PLL_500_P_MASK (0xFE00) /*  Mask value for PLL_500_P_MASK*/

/*I2S_PLL_CTRL_REG_2*/
#define N_DIV_MASK  (0x00FE) /*  Mask value for N_DIV_MASK*/
#define P_DIV2_MASK (0x0700) /*  Mask value for P_DIV2_MASK*/
#define P_DIV1_MASK (0xF800) /*  Mask value for P_DIV1_MASK*/

/*SOC_PLL_500_CTRL_REG3/INTF_PLL_500_CTRL_REG_3/ I2S_PLL_CTRL_REG_3 */
#define FCW_F_MASK (0xFFFC) /*  Mask value for FCW_F_MASK*/

/*SOC_PLL_500_CTRL_REG_4/INTF_PLL_500_CTRL_REG_4 */
#define LDO_BY_PASS     BIT(1)   /*  Enables LDO_BY_PASS */
#define SD_CLK_SEL_MASK (0x000C) /*  Mask value for SD_CLK_SEL_MASK*/
#define SD_LEN          BIT(4)   /*  Enables SD_LEN */
#define FILTER_TYPE     BIT(5)   /*  Enables FILTER_TYPE */
#define BETA_MASK       (0x07C0) /*  Mask value for BETA_MASK*/
#define ALPHA_MASK      (0xF800) /*  Mask value for ALPHA_MASK*/
/*SOC_PLL_500_CTRL_REG_5/INTF_PLL_500_CTRL_REG_5/ I2S_PLL_CTRL_REG_5 */
#define LOCK_LIMIT_MASK          (0x3FFF) /*  Mask value for LOCK_LIMIT_MASK*/
#define ENABLE_PHASE_LOCK_DETECT BIT(14)  /*  Enables ENABLE_PHASE_LOCK_DETECT */
#define ENABLE_FREQ_LOCK_DETECT  BIT(15)  /*  Enables ENABLE_FREQ_LOCK_DETECT */
/*SOC_PLL_500_CTRL_REG_6/INTF_PLL_500_CTRL_REG_6/ I2S_PLL_CTRL_REG_6 */
#define RETIMER_COUNT_MASK    (0x00F0) /*  Mask value for RETIMER_COUNT_MASK*/
#define TDC_PWRSAV_COUNT_MASK (0x7F00) /*  Mask value for TDC_PWRSAV_COUNT_MASK*/
#define TDC_PWRSAV_EN         BIT(15)  /*  Enables TDC_PWRSAV_EN */

/*SOC_PLL_500_CTRL_REG_7/INTF_PLL_500_CTRL_REG_7/ I2S_PLL_CTRL_REG_7*/
#define SPI_INP_RD_EN   BIT(4)   /*  Enables SPI_INP_RD_EN */
#define OCW_MANUAL_MASK (0xFFE0) /*  Mask value for OCW_MANUAL_MASK */
#define DCO_TESTMODE    BIT(15)  /*  Enables DCO_TESTMODE */

/*SOC_PLL_500_CTRL_REG_8/INTF_PLL_500_CTRL_REG_8/I2S_PLL_CTRL_REG_8 */
#define ISOLATION_ENABLE            BIT(0)   /*  Enables ISOLATION_ENABLE */
#define BYPASS_ISO_GEN              BIT(1)   /*  Enables BYPASS_ISO_GEN */
#define BYPASS_LOCK_FLAG            BIT(2)   /*  Enables BYPASS_LOCK_FLAG */
#define BYPASS_PWR_GOOD             BIT(3)   /*  Enables BYPASS_PWR_GOOD */
#define LOCK_COUNT_LIMIT_FREQ_MASK  (0x00F0) /*  Mask value for LOCK_COUNT_LIMIT_FREQ_MASK */
#define LOCK_COUNT_LIMIT_PHASE_MASK (0xFF00) /*  Mask value for LOCK_COUNT_LIMIT_PHASE_MASK*/

/*SOC_PLL_500_CTRL_REG_9/INTF_PLL_500_CTRL_REG_9/ I2S_PLL_CTRL_REG_9 */
#define MM_COUNT_LIMIT_MASK (0x3FC0) /*  Mask value for MM_COUNT_LIMIT_MASK */
#define BYPASS_LOCK_PLL     BIT(14)  /*  Enables BYPASS_LOCK_PLL */
#define MANUAL_LOCK_ENABLE  BIT(15)  /*  Enables MANUAL_LOCK_ENABLE */

/*SOC_PLL_500_CTRL_REG_10/INTF_PLL_500_CTRL_REG_10/ I2S_PLL_CTRL_REG_10 */
#define CKR_TEST_EN                BIT(3)   /*  Enables CKR_TEST_EN */
#define SELOUT_SA_RETIMER          BIT(4)   /*  Enables SELOUT_SA_RETIMER */
#define EN_STD_RETIMER             BIT(5)   /*  Enables EN_STD_RETIMER */
#define EN_SA_RETIMER              BIT(6)   /*  Enables EN_SA_RETIMER */
#define RETIMER_PWRSAV_COUNT2_MASK (0x0780) /*  Mask value for RETIMER_PWRSAV_COUNT2_MASK  */
#define RETIMER_PWRSAV_COUNT1_MASK (0x7800) /*  Mask value for RETIMER_PWRSAV_COUNT1_MASK  */
#define RETIMER_PWRSAV_EN          BIT(15)  /*  Enables RETIMER_PWRSAV_EN */

/*SOC_PLL_500_CTRL_REG_11/INTF_PLL_500_CTRL_REG_11/ I2S_PLL_CTRL_REG_11 */
#define PU_SD_DIV          BIT(1)  /*  Enables PU_SD_DIV */
#define PU_POST_DIV        BIT(2)  /*  Enables PU_POST_DIV */
#define PU_INP_DIV         BIT(3)  /*  Enables PU_INP_DIV */
#define PU_INDO            BIT(4)  /*  Enables PU_INDO */
#define PU_RETIMER         BIT(5)  /*  Enables PU_RETIMER */
#define PU_TDC             BIT(6)  /*  Enables PU_TDC */
#define PU_DCO             BIT(7)  /*  Enables PU_DCO */
#define PU_DIGITAL_TOP     BIT(8)  /*  Enables PU_DIGITAL_TOP */
#define RESETN_SD_DIV      BIT(9)  /*  Enables RESETN_SD_DIV */
#define RESETN_INP_DIV     BIT(10) /*  Enables RESETN_INP_DIV */
#define RESETN_POST_DIV    BIT(11) /*  Enables RESETN_POST_DIV */
#define RESETN_RETIMER     BIT(12) /*  Enables RESETN_RETIMER */
#define RESETN_LOCK_DETECT BIT(13) /*  Enables RESETN_LOCK_DETECT */
#define RESETN_LOOP        BIT(14) /*  Enables RESETN_LOOP */
#define RESETN_TDC         BIT(15) /*  Enables RESETN_TDC */

/*SOC_PLL_500_CTRL_REG_12/INTF_PLL_500_CTRL_REG_12/ I2S_PLL_CTRL_REG_12 */
#define DELTF_MASK (0x003E) /*  Mask value for DELTF_MASK  */
#define DELTR_MASK (0x07C0) /*  Mask value for DELTR_MASK  */
#define TV_MASK    (0XF800) /*  Mask value for TV_MASK  */

/*SOC_PLL_500_CTRL_REG_13/INTF_PLL_500_CTRL_REG_13/ I2S_PLL_CTRL_REG_13*/
#define OCW_MASK        (0x3FF0) /*  Mask value for OCW_MASK  */
#define LOCK_FLAG_PHASE BIT(14)  /*  Enables LOCK_FLAG_PHASE */
#define LOCK_FLAG_FREQ  BIT(15)  /*  Enables LOCK_FLAG_FREQ */

/*SOC Clock division factor checks */
#define SOC_MAX_CLK_DIVISION_FACTOR 63 /* Maximum division factor value for SOC clock*/
#define SOC_MIN_CLK_DIVISION_FACTOR 0  /* Minimum division factor value for SOC clock*/

/*SDMEM Clock division factor checks */
#define SDMEM_MAX_CLK_DIVISION_FACTOR 63 /* Maximum division factor value for SD_MEM clock*/
#define SDMEM_MIN_CLK_DIVISION_FACTOR 0  /* Minimum division factor value for SD_MEM clock*/

/*CT Clock division factor checks */
#define CT_MAX_CLK_DIVISION_FACTOR 63 /* Maximum division factor value for CT clock*/
#define CT_MIN_CLK_DIVISION_FACTOR 0  /* Minimum division factor value for CT clock*/

/*I2S Clock division factor checks */
#define I2S_MAX_CLK_DIVISION_FACTOR 63 /* Maximum division factor value for I2S clock*/
#define I2S_MIN_CLK_DIVISION_FACTOR 0  /* Minimum division factor value for I2S clock*/

/*USB Clock division factor checks */
#define USB_MAX_CLK_DIVISION_FACTOR 3 /* Maximum division factor value for USB clock*/
#define USB_MIN_CLK_DIVISION_FACTOR 0 /* Minimum division factor value for USB clock*/

/*CAN Clock division factor checks */
#define CAN_MAX_CLK_DIVISION_FACTOR 255 /* Maximum division factor value for CAN clock*/
#define CAN_MIN_CLK_DIVISION_FACTOR 0   /* Minimum division factor value for CAN clock*/

/*I2S Clock division factor checks */
#define MCU_CLKOUT_MAX_CLK_DIVISION_FACTOR 63 /* Maximum division factor value for MCU_CLKOUT clock*/
#define MCU_CLKOUT_MIN_CLK_DIVISION_FACTOR 0  /* Minimum division factor value for MCU_CLKOUT clock*/
#define MCU_CLKOUT_SEL_MAX                 15 /* Maximum Seletion value for MCU_CLKOUT clock source*/

/*QSPI Clock Division factor checks */
#define QSPI_MAX_CLK_DIVISION_FACTOR 63 /* Maximum division factor value for QSPI clock*/
#define QSPI_MIN_CLK_DIVISION_FACTOR 0  /* Minimum division factor value for QSPI clock*/

/*USART Clock Division factor checks */
#define USART_MAX_CLK_DIVISION_FACTOR 15 /* Maximum division factor value for USART clock*/
#define USART_MIN_CLK_DIVISION_FACTOR 0  /* Minimum division factor value for USART clock*/

/*SSI Clock Division factor checks */
#define SSI_MAX_CLK_DIVISION_FACTOR 15 /* Maximum division factor value for SSI clock*/
#define SSI_MIN_CLK_DIVISION_FACTOR 0  /* Minimum division factor value for SSI clock*/

/*CCI Clock Division factor checks */
#define CCI_MAX_CLK_DIVISION_FACTOR 15 /* Maximum division factor value for CCI clock*/
#define CCI_MIN_CLK_DIVISION_FACTOR 0  /* Minimum division factor value for CCI clock*/

/*PLL_INTF Clock Division factor checks */
#define PLL_INTF_MAX_CLK_DIVISION_FACTOR 15 /* Maximum division factor value for PLL_INTF clock*/
#define PLL_INTF_MIN_CLK_DIVISION_FACTOR 0  /* Minimum division factor value for PLL_INTF clock*/

/*Sleep Clock selection checks */
#define SLP_MAX_SEL 3 /* Maximum Seletion value for Sleep clock source*/
#define SLP_MIN_SEL 0 /* Minimum Seletion value for Sleep clock source*/

#define MAX_SLP_CYCLES 3 /* Maximum Cycles for Sleep clock*/
#define MIN_SLP_CYCLES 0 /* Minimum Cycles for Sleep clock*/

#define MISC_CFG_MISC_CTRL1   (*((uint32_t volatile *)(MISC_CONFIG_BASE + 0x44))) /*  address of MISC_CFG_MISC_CTRL1 */
#define I2S_MASTER_SLAVE_MODE (1 << 23)                                           /* Sets I2S/I2S PCM master mode  */

#define MCU_ULP_40MHZ_CLK_EN_TURN_ON_DELAY          10   /*  delay to enable the ULP 40MHZ  CLK*/
#define MCU_ULP_DOUBLER_CLK_EN_TURN_ON_DELAY        10   /*  delay to enable the ULP DOUBLER CLK*/
#define MCU_ULP_20MHZ_RING_OSC_CLK_EN_TURN_ON_DELAY 10   /*  delay to enable the ULP 20MHZ_RING_OSC CLK*/
#define MCU_ULP_MHZ_RC_CLK_EN_TURN_ON_DELAY         2    /*  delay to enable the ULP MHZ_RC CLK*/
#define MCU_ULP_32KHZ_XTAL_CLK_EN_TURN_ON_DELAY_1   500  /*  delay to enable the ULP 32KHZ_XTAL CLK*/
#define MCU_ULP_32KHZ_XTAL_CLK_EN_TURN_ON_DELAY_2   1500 /*  delay to enable the ULP 32KHZ_XTAL CLK*/
#define MCU_ULP_32KHZ_RO_CLK_EN_TURN_ON_DELAY       250  /*  delay to enable the ULP 32KHZ_RO CLK*/
#define MCU_ULP_32KHZ_RC_CLK_EN_TURN_ON_DELAY       150  /*  delay to enable the ULP 32KHZ_RC CLK*/

/**
 *@brief Reference clock selection
 **/
typedef enum REF_CLK_ENABLE {
  MCU_ULP_40MHZ_CLK_EN,          /*!< Enables ULP_40MHZ_CLK when it is passed */
  MCU_ULP_DOUBLER_CLK_EN,        /*!< Enables ULP_DOUBLER_CLK when it is passed */
  MCU_ULP_20MHZ_RING_OSC_CLK_EN, /*!< Enables ULP_20MHZ_RING_OSC_CLK when it is passed */
  MCU_ULP_MHZ_RC_CLK_EN,         /*!< Enables ULP_MHZ_RC_CLK when it is passed */
  MCU_ULP_32KHZ_XTAL_CLK_EN,     /*!< Enables ULP_32KHZ_XTAL_CLK when it is passed */
  MCU_ULP_32KHZ_RO_CLK_EN,       /*!< 32KHZ RO clock not supported */
  MCU_ULP_32KHZ_RC_CLK_EN        /*!< Enables ULP_32KHZ_RC_CLK when it is passed */
} REF_CLK_ENABLE_T;
/**
 *@brief list of peripherals, particular clock that to be enabled
 **/
typedef enum PERIPHERALS_CLK {
  USART1_CLK,    /*!< Enables or Disables USART1 Peripheral clock when it is passed */
  USART2_CLK,    /*!< Enables or Disables USART2 Master Peripheral clock when it is passed */
  SSIMST_CLK,    /*!< Enables or Disables SSI Master Peripheral clock when it is passed */
  SSISLAVE_CLK,  /*!< Enables or Disables SSI Slave Peripheral clock when it is passed */
  CT_CLK,        /*!< Enables or Disables CT Peripheral clock when it is passed */
  SD_MEM_CLK,    /*!< Enables or Disables SD_MEM Peripheral clock when it is passed */
  CCI_CLK,       /*!< Enables or Disables CCI Peripheral clock when it is passed */
  QSPI_CLK,      /*!< Enables or Disables QSPI Peripheral clock when it is passed */
  RPDMA_CLK,     /*!< Enables or Disables RPDMA Peripheral clock when it is passed */
  UDMA_CLK,      /*!< Enables or Disables UDMA Peripheral clock when it is passed */
  PWM_CLK,       /*!< Enables or Disables PWM Peripheral clock when it is passed */
  CAN_CLK,       /*!< Enables or Disables CAN Peripheral clock when it is passed */
  GSPI_CLK,      /*!< Enables or Disables GSPI Peripheral clock when it is passed */
  EGPIO_CLK,     /*!< Enables or Disables EGPIO Peripheral clock when it is passed */
  ETHERNET_CLK,  /*!< Enables or Disables ETHERNET Peripheral clock when it is passed */
  MCUCLKOUT_CLK, /*!< Enables or Disables MCUCLKOUT Peripheral clock when it is passed */
  HWRNG_CLK,     /*!< Enables or Disables HWRNG Peripheral clock when it is passed */
  I2SM_CLK,      /*!< Enables or Disables I2SM Peripheral clock when it is passed */
#if defined(SLI_SI917B0)
  QSPI_2_CLK, /*!< Enables or Disables QSPI 2 Peripheral clock when it is passed */
#endif
} PERIPHERALS_CLK_T;

/**
 *@brief PLL selection types
 **/
typedef enum PLL_TYPE {
  SOC_PLL,             /*!<SOC_PLL type selection*/
  INFT_PLL,            /*!<INFT_PLL type selection, maintaining for backward compatibility*/
  INTF_PLL = INFT_PLL, /*!<INTF_PLL type selection*/
  I2S_PLL              /*!<I2S_PLL type selection*/
} PLL_TYPE_T;
/**
 *@brief list of possible values of UART instances
 **/
typedef enum EN_USART {
  USART1 = 0, /*!<USART1 instance selection*/
  USART2 = 1, /*!<USART2 instance selection*/
} EN_USART_T;
/**
 *@brief list of possible values of I2C instances
 **/
typedef enum EN_I2C {
  I2C1_INSTAN = 0, /*!<I2C 1 instance selection*/
  I2C2_INSTAN = 1  /*!<I2C 2 instance selection*/
} EN_I2C_T;

/**
 *@brief enum values for proccessor and peripheral clk
 **/
typedef enum SRC_TYPE {
  ULP_PROCESSOR_CLK  = 0, /*!< ULP_PROCESSOR_CLK selection*/
  ULP_PERIPHERAL_CLK = 1  /*!< ULP_PERIPHERAL_CLK selection*/

} SRC_TYPE_T;
/**
 *@brief M4 SOC input clock sources selection
 **/
typedef enum M4_SOC_CLK_SRC_SEL {
  M4_ULPREFCLK    = 0, /*!< M4_ULPREFCLK selection*/
  M4_RESERVED     = 1, /*!< RESERVED*/
  M4_SOCPLLCLK    = 2, /*!< M4_SOCPLLCLK selection*/
  M4_MODEMPLLCLK1 = 3, /*!< M4_MODEMPLLCLK1 selection*/
  M4_INTFPLLCLK   = 4, /*!< M4_INTFPLLCLK selection*/
  M4_SLEEPCLK     = 5, /*!< M4_SLEEPCLK selection*/
} M4_SOC_CLK_SRC_SEL_T;
/**
 *@brief QSPI Input clock source selection
 **/
typedef enum QSPI_CLK_SRC_SEL {
  QSPI_ULPREFCLK                 = 0, /*!< QSPI_ULPREFCLK selection*/
  QSPI_MODELPLLCLK2              = 1, /*!< QSPI_MODELPLLCLK2 selection*/
  QSPI_INTFPLLCLK                = 2, /*!< QSPI_INTFPLLCLK selection*/
  QSPI_SOCPLLCLK                 = 3, /*!< QSPI_SOCPLLCLK selection*/
  M4_SOCCLKNOSWLSYNCCLKTREEGATED = 4  /*!< M4_SOCCLKNOSWLSYNCCLKTREEGATED selection*/
} QSPI_CLK_SRC_SEL_T;

/**
 *@brief Clock enable type
 **/
typedef enum CLK_ENABLE {
  ENABLE_DYN_CLK,   /*!< Dynamic clock enable for the peripherals*/
  ENABLE_STATIC_CLK /*!< Static clock enable for the peripherals*/
} CLK_ENABLE_T;
/**
 *@brief USART Input clock source selection
 **/
typedef enum USART_CLK_SRC_SEL {
  USART_ULPREFCLK         = 0, /*!< USART_ULPREFCLK selection*/
  USART_SOCPLLCLK         = 1, /*!< USART_SOCPLLCLK selection*/
  USART_MODELPLLCLK2      = 2, /*!< USART_MODELPLLCLK2 selection*/
  USART_INTFPLLCLK        = 3, /*!< USART_INTFPLLCLK selection*/
  M4_SOCCLKFOROTHERCLOCKS = 4, /*!< M4_SOCCLKFOROTHERCLOCKS selection*/
} USART_CLK_SRC_SEL_T;

/**
 *@brief USART Fractional clock selection
 **/
typedef enum USART_SCLK_FRAC_SEL {
  USART_FRACTIONAL_DIVIDER, /*!< Fractional Divider selected i.e  clk_out = clk_in/ (sclk_div_fac+0.5)*/
  USART_CLOCK_SWALLOW,      /*!< Clock Swallow selected i.e  clk_out = clk_in/ sclk_div_fac*/
} USART_SCLK_FRAC_SEL_T;

/**
 *@brief SSI Input clock source selection
 **/
typedef enum SSI_MST_CLK_SRC_SEL {
  SSI_ULPREFCLK         = 0, /*!< SSI_ULPREFCLK selection*/
  SSI_SOCPLLCLK         = 1, /*!< SSI_SOCPLLCLK selection*/
  SSI_MODEMPLLCLK1      = 2, /*!< SSI_MODEMPLLCLK1 selection*/
  SSI_INTFPLLCLK        = 3, /*!< SSI_INTFPLLCLK selection*/
  SSI_MODELPLLCLK2      = 4, /*!< SSI_MODELPLLCLK2 selection*/
  M4_SOCCLKFOROTHERCLKS = 5, /*!< M4_SOCCLKFOROTHERCLKS selection*/
} SSI_MST_CLK_SRC_SEL_T;
/**
 *@brief SD_MEM Input clock source selection
 **/
typedef enum SDMEM_CLK_SRC_SEL {
  SDMEM_SOCPLLCLK            = 0, /*!< SDMEM_SOCPLLCLK selection*/
  SDMEM_MODEMPLLCLK1         = 1, /*!< SDMEM_MODEMPLLCLK1 selection*/
  SDMEM_INTFPLLCLK           = 2, /*!< SDMEM_INTFPLLCLK selection*/
  M4_SOCCLKFOROTHERCLKSSDMEM = 3, /*!< M4_SOCCLKFOROTHERCLKSSDMEM selection*/
} SDMEM_CLK_SRC_SEL_T;
/**
 *@brief CT Input clock source selection
 **/
/*CT Input clock source selection*/
typedef enum CT_CLK_SRC_SEL {
  CT_ULPREFCLK            = 0, /*!< CT_ULPREFCLK selection*/
  CT_INTFPLLCLK           = 1, /*!< CT_INTFPLLCLK selection*/
  CT_SOCPLLCLK            = 2, /*!< CT_SOCPLLCLK selection*/
  M4_SOCCLKFOROTHERCLKSCT = 3, /*!< M4_SOCCLKFOROTHERCLKSCT selection*/
} CT_CLK_SRC_SEL_T;
/**
 *@brief GSPI Input clock source selection
 **/

typedef enum GSPI_CLK_SRC_SEL {
  GSPI_M4_SOC_CLK_FOR_OTHER_CLKS, /*!< GSPI_M4_SOC_CLK_FOR_OTHER_CLKS selection*/
  GSPI_ULP_REF_CLK,               /*!< GSPI_ULP_REF_CLK selection*/
  GSPI_SOC_PLL_CLK,               /*!< GSPI_SOC_PLL_CLK selection*/
  GSPI_MODEM_PLL_CLK2,            /*!< GSPI_MODEM_PLL_CLK2 selection*/
  GSPI_INTF_PLL_CLK,              /*!< GSPI_INTF_PLL_CLK selection*/
} GSPI_CLK_SRC_SEL_T;
/**
 *@brief MCU_CLKOUT Input clock source selection
 **/

typedef enum MCU_CLKOUT_SRC_SEL {
  MCUCLKOUT_ULP_MHZ_RC_CLK,        /*!< MCUCLKOUT_ULP_MHZ_RC_CLK selection*/
  MCUCLKOUT_RF_REF_CLK,            /*!< MCUCLKOUT_RF_REF_CLK selection*/
  MCUCLKOUT_MEMS_REF_CLK,          /*!< MCUCLKOUT_MEMS_REF_CLK selection*/
  MCUCLKOUT_ULP_20MHZ_RINGOSC_CLK, /*!< MCUCLKOUT_ULP_20MHZ_RINGOSC_CLK selection*/
  MCUCLKOUT_ULP_DOUBLER_CLK,       /*!< MCUCLKOUT_ULP_DOUBLER_CLK selection*/
  MCUCLKOUT_ULP_32KHZ_RC_CLK,      /*!< MCUCLKOUT_ULP_32KHZ_RC_CLK selection*/
  MCUCLKOUT_ULP_32KHZ_XTAL_CLK,    /*!< MCUCLKOUT_ULP_32KHZ_XTAL_CLK selection*/
  MCUCLKOUT_ULP_32KHZ_RO_CLK,      /*!< 32KHZ RO clock not supported*/
  MCUCLKOUT_INTF_PLL_CLK,          /*!< MCUCLKOUT_INTF_PLL_CLK selection*/
  MCUCLKOUT_MODEM_PLL_CLK1,        /*!< MCUCLKOUT_MODEM_PLL_CLK1 selection*/
  MCUCLKOUT_MODEM_PLL_CLK2,        /*!< MCUCLKOUT_MODEM_PLL_CLK2 selection*/
  MCUCLKOUT_SOC_PLL_CLK,           /*!< MCUCLKOUT_SOC_PLL_CLK selection*/
  MCUCLKOUT_I2S_PLL_CLK,           /*!< MCUCLKOUT_I2S_PLL_CLK selection*/
  MCUCLKOUT_USB_PLL_CLK,           /*!< MCUCLKOUT_USB_PLL_CLK selection*/
} MCU_CLKOUT_SRC_SEL_T;
/**
 *@brief Ethernet Input clock source selection
 **/
typedef enum ETHERNET_CLK_SRC_SEL {
  ETH_INTF_PLL_CLK, /*!< ETH_INTF_PLL_CLK selection*/
  ETH_SOC_PLL_CLK,  /*!< ETH_SOC_PLL_CLK selection*/
} ETHERNET_CLK_SRC_SEL_T;
/**
 *@brief USB Input clock source selection
 **/
typedef enum USB_CLK_SRC_SEL {
  USB_MEMS_REF_CLK  = 0, /*!< USB_MEMS_REF_CLK selection*/
  USB_REFERENCE_CLK = 1, /*!< USB_REFERENCE_CLK selection*/
  USB_PLL_CLK       = 2  /*!< USB_PLL_CLK selection*/
} USB_CLK_SRC_SEL_T;
/**
 *@brief Sleep Clk Input clock source selection
 **/
typedef enum SLEEP_CLK_SRC_SEL {
  SLP_ULP_32KHZ_RC_CLK   = 0, /*!< SLP_ULP_32KHZ_RC_CLK selection*/
  SLP_ULP_32KHZ_XTAL_CLK = 1, /*!< SLP_ULP_32KHZ_XTAL_CLK selection*/
  SLP_CLK_GATED          = 2, /*!< Default Sleep Clk Gated*/
  SLP_ULP_32KHZ_RO_CLK   = 3  /*!< 32KHZ RO clock not supported*/
} SLEEP_CLK_SRC_SEL_T;
/**
 *@brief CCI Input clock source selection
 **/
typedef enum CCI_CLK_SRC_SEL {
  CCI_M4_SOC_CLK_FOR_OTHER_CLKS,      /*!< CCI_M4_SOC_CLK_FOR_OTHER_CLKS selection*/
  CCI_INTF_PLL_CLK,                   /*!< CCI_INTF_PLL_CLK selection*/
  CCI_M4_SOC_CLK_NO_SWL_SYNC_CLK_TREE /*!< CCI_M4_SOC_CLK_NO_SWL_SYNC_CLK_TREE selection*/
} CCI_CLK_SRC_SEL_T;
/**
 *@brief I2S Input clock source selection
 **/
typedef enum I2S_CLK_SRC_SEL {
  I2S_PLLCLK            = 0, /*!< I2S_PLLCLK selection*/
  I2S_M4SOCCLKFOROTHERS = 1, /*!< I2S_M4SOCCLKFOROTHERS selection*/
} I2S_CLK_SRC_SEL_T;

/**
 *@brief M4ss Reference Input clock source selection
 **/
typedef enum M4SS_REF_CLK_SEL {
  ULP_MHZ_RC_BYP_CLK    = 1, /*!< ULP_MHZ_RC_BYP_CLK selection*/
  ULP_MHZ_RC_CLK        = 2, /*!< ULP_MHZ_RC_CLK selection*/
  EXT_40MHZ_CLK         = 3, /*!< EXT_40MHZ_CLK selection*/
  MEMS_REF_CLK          = 4, /*!< MEMS_REF_CLK selection*/
  ULP_20MHZ_RINGOSC_CLK = 5, /*!< ULP_20MHZ_RINGOSC_CLK selection*/
  ULP_DOUBLER_CLK       = 6, /*!< ULP_DOUBLER_CLK selection*/
} M4SS_REF_CLK_SEL_T;

typedef enum CLK_PRESENT {
  SOC_PLL_CLK_PRESENT,   /*!< SOC CLock present       */
  INTF_PLL_CLK_PRESENT,  /*!< INTF PLL clock present  */
  I2S_PLL_CLK_PRESENT,   /*!< I2S PLL clock present   */
  MODEM_PLL_CLK_PRESENT, /*!< Modem PLL clock present */
} CLK_PRESENT_T;

STATIC INLINE void _usdelay(uint32_t delayUs, cdDelay delayCb)
{
  if (delayCb != NULL) {
    delayCb(delayUs);
  }
}

rsi_error_t clk_i2s_pll_clk_set(const M4CLK_Type *pCLK);

boolean_t clk_check_pll_lock(PLL_TYPE_T pllType);

rsi_error_t clk_soc_pll_clk_enable(boolean_t clkEnable);

rsi_error_t clk_soc_pll_set_freq_div(const M4CLK_Type *pCLK,
                                     boolean_t clk_en,
                                     uint16_t divFactor,
                                     uint16_t nFactor,
                                     uint16_t mFactor,
                                     uint16_t fcwF,
                                     uint16_t dcoFixSel,
                                     uint16_t ldoProg);

rsi_error_t clk_soc_pll_clk_set(const M4CLK_Type *pCLK);

rsi_error_t clk_soc_pll_clk_bypass_enable(boolean_t clkEnable);

rsi_error_t clk_soc_pll_clk_reset(void);

rsi_error_t clk_soc_pll_pd_enable(boolean_t en);

rsi_error_t clk_soc_pll_turn_off(void);

rsi_error_t clk_soc_pll_turn_on(void);

rsi_error_t clk_i2s_pll_clk_enable(boolean_t clkEnable);

rsi_error_t clk_i2s_pll_clk_bypass_enable(boolean_t clkEnable);

rsi_error_t clk_i2s_pll_pd_enable(boolean_t en);

rsi_error_t clk_i2s_pll_turn_off(void);

rsi_error_t clk_i2s_pll_turn_on(void);

rsi_error_t clk_i2s_pll_set_freq_div(const M4CLK_Type *pCLK,
                                     uint16_t u16DivFactor1,
                                     uint16_t u16DivFactor2,
                                     uint16_t nFactor,
                                     uint16_t mFactor,
                                     uint16_t fcwF);

rsi_error_t clk_i2s_pll_clk_reset(void);

rsi_error_t clk_i2s_pll_clk_set(const M4CLK_Type *pCLK);

rsi_error_t clk_intf_pll_clk_enable(boolean_t clkEnable);

rsi_error_t clk_intf_pll_pd_enable(boolean_t en);

rsi_error_t clk_intf_pll_turn_off(void);

rsi_error_t clk_intf_pll_set_freq_div(const M4CLK_Type *pCLK,
                                      boolean_t clk_en,
                                      uint16_t divFactor,
                                      uint16_t nFactor,
                                      uint16_t mFactor,
                                      uint16_t fcwF,
                                      uint16_t dcoFixSel,
                                      uint16_t ldoProg);

rsi_error_t clk_intf_pll_clk_bypass_enable(boolean_t clkEnable);

rsi_error_t clk_intf_pll_turn_on(void);

rsi_error_t clk_intf_pll_clk_reset(void);

rsi_error_t clk_intf_pll_clk_set(const M4CLK_Type *pCLK);

rsi_error_t clk_peripheral_clk_enable1(M4CLK_Type *pCLK, uint32_t flags);

rsi_error_t clk_peripheral_clk_disable1(M4CLK_Type *pCLK, uint32_t flags);

rsi_error_t clk_peripheral_clk_enable2(M4CLK_Type *pCLK, uint32_t flags);

rsi_error_t clk_peripheral_clk_disable2(M4CLK_Type *pCLK, uint32_t flags);

rsi_error_t clk_peripheral_clk_enable3(M4CLK_Type *pCLK, uint32_t flags);

rsi_error_t clk_peripheral_clk_disable3(M4CLK_Type *pCLK, uint32_t flags);

rsi_error_t clk_dynamic_clk_gate_disable(M4CLK_Type *pCLK, uint32_t flags);

rsi_error_t clk_dynamic_clk_gate_disable2(M4CLK_Type *pCLK, uint32_t flags);

rsi_error_t clk_dynamic_clk_gate_enable(M4CLK_Type *pCLK, uint32_t flags);

rsi_error_t clk_dynamic_clk_gate_enable2(M4CLK_Type *pCLK, uint32_t flags);

rsi_error_t clk_qspi_clk_config(M4CLK_Type *pCLK,
                                QSPI_CLK_SRC_SEL_T clkSource,
                                boolean_t swalloEn,
                                boolean_t OddDivEn,
                                uint32_t divFactor);
#if defined(SLI_SI917B0)
rsi_error_t clk_qspi_2_clk_config(M4CLK_Type *pCLK,
                                  QSPI_CLK_SRC_SEL_T clkSource,
                                  boolean_t swalloEn,
                                  boolean_t OddDivEn,
                                  uint32_t divFactor);
#endif
rsi_error_t clk_usart_clk_config(M4CLK_Type *pCLK,
                                 CLK_ENABLE_T clkType,
                                 boolean_t FracDivEn,
                                 EN_USART_T enUsart,
                                 USART_CLK_SRC_SEL_T clkSource,
                                 uint32_t divFactor);

rsi_error_t clk_ssi_mst_clk_config(M4CLK_Type *pCLK,
                                   CLK_ENABLE_T clkType,
                                   SSI_MST_CLK_SRC_SEL_T clkSource,
                                   uint32_t divFactor);

rsi_error_t clk_sd_mem_clk_config(M4CLK_Type *pCLK,
                                  boolean_t swalloEn,
                                  SDMEM_CLK_SRC_SEL_T clkSource,
                                  uint32_t divFactor);

rsi_error_t clk_ct_clk_config(M4CLK_Type *pCLK, CT_CLK_SRC_SEL_T clkSource, uint32_t divFactor, CLK_ENABLE_T clkType);

rsi_error_t clk_cci_clk_config(M4CLK_Type *pCLK, CCI_CLK_SRC_SEL_T clkSource, uint32_t divFactor, CLK_ENABLE_T clkType);

rsi_error_t clk_i2s_clk_config(M4CLK_Type *pCLK, I2S_CLK_SRC_SEL_T clkSource, uint32_t divFactor);

rsi_error_t clk_mcu_clk_cut_config(M4CLK_Type *pCLK, MCU_CLKOUT_SRC_SEL_T clkSource, uint32_t divFactor);

rsi_error_t clk_can_clk_config(M4CLK_Type *pCLK, uint32_t divFactor, CLK_ENABLE_T clkType);

rsi_error_t clk_ethernet_clk_config(M4CLK_Type *pCLK,
                                    boolean_t swalloEn,
                                    ETHERNET_CLK_SRC_SEL_T clkSource,
                                    uint32_t divFactor);

rsi_error_t clk_m4_soc_clk_div(M4CLK_Type *pCLK, uint32_t divFactor);

rsi_error_t clk_qspi_clk_div(M4CLK_Type *pCLK, boolean_t u8SwallowEn, boolean_t u8OddDivEn, uint32_t divFactor);

rsi_error_t clk_ct_clk_div(M4CLK_Type *pCLK, uint32_t divFactor);

rsi_error_t clk_ssi_mst_clk_div(M4CLK_Type *pCLK, uint32_t divFactor);

rsi_error_t clk_cci_clk_div(M4CLK_Type *pCLK, uint32_t divFactor);

rsi_error_t clk_i2s_clk_div(M4CLK_Type *pCLK, uint32_t divFactor);

rsi_error_t clk_sd_mem_clk_div(M4CLK_Type *pCLK, boolean_t u8SwallowEn, uint32_t divFactor);

rsi_error_t clk_usart_clk_div(M4CLK_Type *pCLK, EN_USART_T enUsart, boolean_t u8FracDivEn, uint32_t divFactor);

rsi_error_t RSI_CLK_CanClkDiv(M4CLK_Type *pCLK, uint16_t divFactor);

uint32_t clk_slp_clk_calib_config(M4CLK_Type *pCLK, uint8_t clkCycles);

rsi_error_t clk_gspi_clk_config(M4CLK_Type *pCLK, GSPI_CLK_SRC_SEL_T clkSel);

rsi_error_t clk_slp_clk_config(M4CLK_Type *pCLK, SLEEP_CLK_SRC_SEL_T clkSrc);

rsi_error_t clk_i2c_clk_config(M4CLK_Type *pCLK, boolean_t clkEnable, EN_I2C_T enI2C);

rsi_error_t clk_xtal_clk_config(uint8_t xtalPin);

rsi_error_t clk_usb_clk_config(M4CLK_Type *pCLK, USB_CLK_SRC_SEL_T clkSource, uint16_t divFactor);

rsi_error_t clk_peripheral_clk_enable(M4CLK_Type *pCLK, PERIPHERALS_CLK_T module, CLK_ENABLE_T clkType);

rsi_error_t clk_peripheral_clk_disable(M4CLK_Type *pCLK, PERIPHERALS_CLK_T module);

rsi_error_t clk_set_soc_pll_freq(const M4CLK_Type *pCLK, uint32_t socPllFreq, uint32_t pllRefClk);

rsi_error_t clk_set_intf_pll_freq(const M4CLK_Type *pCLK, uint32_t intfPllFreq, uint32_t pllRefClk);

rsi_error_t ulpss_enable_ref_clks(REF_CLK_ENABLE_T enable, SRC_TYPE_T srcType, cdDelay delayFn);

rsi_error_t clk_set_i2s_pll_freq(const M4CLK_Type *pCLK, uint32_t i2sPllFreq, uint32_t fXtal);

static void _usdelay(uint32_t delayUs, cdDelay delayCb);

rsi_error_t ulpss_disable_ref_clks(REF_CLK_ENABLE_T clk_type);

void clk_config_pll_lock(boolean_t manual_lock, boolean_t bypass_manual_lock, uint8_t mm_count_limit);

void clk_config_pll_ref_clk(uint8_t ref_clk_src);
rsi_error_t clk_m4_soc_clk_config(M4CLK_Type *pCLK, M4_SOC_CLK_SRC_SEL_T clkSource, uint32_t divFactor);
uint32_t RSI_CLK_CheckPresent(const M4CLK_Type *pCLK, CLK_PRESENT_T clkPresent);
rsi_error_t clk_m4ss_ref_clk_config(const M4CLK_Type *pCLK, M4SS_REF_CLK_SEL_T clkSource);
rsi_error_t ulpss_disable_ref_clks(REF_CLK_ENABLE_T clk_type);

/*End of file not truncated*/

#ifdef __cplusplus
}
#endif
#endif // RSI_PLL_H
