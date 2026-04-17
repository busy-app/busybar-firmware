/*******************************************************************************
* @file  rsi_reg_spi.h
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
#ifndef __RSI_REG_SPI_H__
#define __RSI_REG_SPI_H__

#ifdef __cplusplus
extern "C" {
#endif
/**
 * \cond HIDDEN_SYMBOLS 
 */

#define REG_SPI_BASE_ADDR_ULP              0x24050000
#define REG_SPI_BASE_ADDR_PLL              0x46180000
#define TASS_AFE_REG_ACC_MEM_MAP_BASE_ADDR 0x41138000

#include "rsi_ccp_common.h"

/*Register memory mapped reg read write*/

/*REG ADDRESS : Address of a target register ULP*/
#define ULP_SPI_MEM_MAP(REG_ADR) (*((uint32_t volatile *)(REG_SPI_BASE_ADDR_ULP + (0xa000 + (REG_ADR * 4)))))

/*REG ADDRESS : Address of a target register PMU*/
#define PMU_SPI_MEM_MAP(REG_ADR) (*((uint32_t volatile *)(REG_SPI_BASE_ADDR_ULP + (0x00008000 + (REG_ADR * 4)))))

/*Memory mapped SPI for M4 PLL configuration */
#define SPI_MEM_MAP_PLL(REG_ADR) (*((uint16_t volatile *)(REG_SPI_BASE_ADDR_PLL + 0x00008000 + (REG_ADR << 2))))

/*Memory mapped SPI for  PLL_480 configuration */
#define TASS_PLL_CTRL_SET_REG(REG_ADR) (*((uint16_t volatile *)(TASS_AFE_REG_ACC_MEM_MAP_BASE_ADDR + (REG_ADR << 2))))

/*Configuration parameters*/
#define GSPI_RF_N_ULP   BIT(12)
#define GSPI_ACTIVE     BIT(8)
#define GSPI_TRIG       BIT(7)
#define GSPI_READ       BIT(6)
#define GSPI_DATA_FIFO  GSPI_DATA_REG0
#define READ_INDICATION BIT(15)
#define ADDR_DATA_LEN   31

/*GSPI Configuration*/
typedef struct stc_reg_spi_config {
  uint16_t u16GspiClockRatio;
  uint16_t u16GspiCsbSetupTime;
  uint16_t u16GspiCsbHoldTime;
  uint16_t u16GspiCsbHighTime;
  boolean_t bGspiSpiMode;
  boolean_t bGspiClockPhase;
  boolean_t bGspiAfeIpmun;
  boolean_t bDmaMode;
  boolean_t bDma32_48Bitn;
} stc_reg_spi_config_t;

/*Target selection*/
typedef enum en_select_target { PmuBlock = 0, UlpBlock = 1 } en_select_target_t;

/*Base address assignment */
#define REG_SPI_ULP (*((REG_SPI_Type_T *)REG_SPI_BASE_ADDR_ULP)) /*Memory map for ULPSS Reg access SPI*/
#define REG_SPI_PLL (*((REG_SPI_Type_T *)REG_SPI_BASE_ADDR_PLL)) /*Memory map for ULPSS Reg access SPI*/

/**
 * \endcond
 */

#ifdef __cplusplus
}
#endif

#endif /*__RSI_REG_SPI_H__*/
