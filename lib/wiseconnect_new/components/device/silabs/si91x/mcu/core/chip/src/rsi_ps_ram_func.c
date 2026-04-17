/*******************************************************************************
* @file  rsi_ps_ram_func.c
* @brief 
*******************************************************************************
* # License
* <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "rsi_ccp_user_config.h"
#include "rsi_ps_ram_func.h"
#if defined(DEBUG_UART)
#include "rsi_debug.h"
#endif // DEBUG_UART
#include "rsi_rom_table_si91x.h"

/*Note : Enable the WiSe_MCU_MODE if it is WiSeMCU product, and comment off the MCU_MODE define*/
#define MCU_MODE
/*Note : Update the FLASH define based on product integrated flash */
#define GIGA_DEVICE
/*Note please update this stack pointer based on application stack/RAM usage*/
#define INTERMEDIATE_SP 0x8000
/*Note please update this FLASH_VECTOR_OFFSET based on Application vector offset, default is 0x8012000*/
#define FLASH_VECTOR_OFFSET 0x8012000
//!qspi hardware address
#define QSPI_HW_ADDR 0x12000000

/*QSPI configuration structure*/
spi_config_t spi_configs_giga;

#ifdef WiSe_MCU_MODE
#define PadSelectionEnable 15
#define QSPI_MODE          2
/*M4 QSPI  pin set */
#define M4SS_QSPI_CLK  58
#define M4SS_QSPI_CSN0 59
#define M4SS_QSPI_D0   60
#define M4SS_QSPI_D1   61
#define M4SS_QSPI_D2   62
#define M4SS_QSPI_D3   63
#endif //WiSe_MCU_MODE

#ifdef MCU_MODE
#define PadSelectionEnable 0
#define QSPI_MODE          1
/*M4 QSPI  pin set */
#define M4SS_QSPI_CLK  0
#define M4SS_QSPI_CSN0 1
#define M4SS_QSPI_D0   2
#define M4SS_QSPI_D1   3
#define M4SS_QSPI_D2   4
#define M4SS_QSPI_D3   5
#endif //MCU_MODE

/**
* @fn           void Init_QspiStc(void)
* @brief        This function is used to initialize the QSPI configuration structure of selected flash.
* @param[in]    None
* @return       None.
*
*/
void Init_QspiStc(void)
{
  //! Default SPI configuration
  spi_configs_giga.spi_config_1.inst_mode         = QUAD_MODE;
  spi_configs_giga.spi_config_1.addr_mode         = QUAD_MODE;
  spi_configs_giga.spi_config_1.data_mode         = QUAD_MODE;
  spi_configs_giga.spi_config_1.dummy_mode        = QUAD_MODE;
  spi_configs_giga.spi_config_1.extra_byte_mode   = QUAD_MODE;
  spi_configs_giga.spi_config_1.prefetch_en       = DIS_PREFETCH;
  spi_configs_giga.spi_config_1.dummy_W_or_R      = DUMMY_READS;
  spi_configs_giga.spi_config_1.extra_byte_en     = 0;
  spi_configs_giga.spi_config_1.d3d2_data         = 3;
  spi_configs_giga.spi_config_1.continuous        = CONTINUOUS;
  spi_configs_giga.spi_config_1.read_cmd          = FREAD_QUAD_O;
  spi_configs_giga.spi_config_1.flash_type        = MICRON_QUAD_FLASH;
  spi_configs_giga.spi_config_1.no_of_dummy_bytes = 1;

  spi_configs_giga.spi_config_2.addr_width_valid            = 0;
  spi_configs_giga.spi_config_2.addr_width                  = _24BIT_ADDR;
  spi_configs_giga.spi_config_2.auto_mode                   = EN_AUTO_MODE;
  spi_configs_giga.spi_config_2.cs_no                       = CHIP_ZERO;
  spi_configs_giga.spi_config_2.neg_edge_sampling           = NEG_EDGE_SAMPLING;
  spi_configs_giga.spi_config_2.qspi_clk_en                 = QSPI_FULL_TIME_CLK;
  spi_configs_giga.spi_config_2.protection                  = DNT_REM_WR_PROT;
  spi_configs_giga.spi_config_2.dma_mode                    = NO_DMA;
  spi_configs_giga.spi_config_2.swap_en                     = SWAP;
  spi_configs_giga.spi_config_2.full_duplex                 = IGNORE_FULL_DUPLEX;
  spi_configs_giga.spi_config_2.wrap_len_in_bytes           = NO_WRAP;
  spi_configs_giga.spi_config_2.addr_width_valid            = 0;
  spi_configs_giga.spi_config_2.addr_width                  = _24BIT_ADDR;
  spi_configs_giga.spi_config_2.pinset_valid                = 0;
  spi_configs_giga.spi_config_2.flash_pinset                = GPIO_0_TO_5;
  spi_configs_giga.spi_config_2.dummy_cycles_for_controller = 0;

  spi_configs_giga.spi_config_3._16bit_cmd_valid  = 0;
  spi_configs_giga.spi_config_3._16bit_rd_cmd_msb = 0;
#ifdef CHIP_9118
  spi_configs_giga.spi_config_3.ddr_mode_en = 0;
#endif
  spi_configs_giga.spi_config_3.wr_cmd        = 0x2;
  spi_configs_giga.spi_config_3.wr_inst_mode  = QUAD_MODE;
  spi_configs_giga.spi_config_3.wr_addr_mode  = QUAD_MODE;
  spi_configs_giga.spi_config_3.wr_data_mode  = QUAD_MODE;
  spi_configs_giga.spi_config_3.dummys_4_jump = 1;

  spi_configs_giga.spi_config_4.high_perf_mode_en = 0;
#ifdef CHIP_9118 //!Removed ddr mode in 9117
  spi_configs_giga.spi_config_4.qspi_manual_ddr_phasse = 0;
  spi_configs_giga.spi_config_4.ddr_data_mode          = 0;
  spi_configs_giga.spi_config_4.ddr_addr_mode          = 0;
  spi_configs_giga.spi_config_4.ddr_inst_mode          = 0;
  spi_configs_giga.spi_config_4.ddr_dummy_mode         = 0;
  spi_configs_giga.spi_config_4.ddr_extra_byte         = 0;
#endif
  spi_configs_giga.spi_config_4.valid_prot_bits = 4;

  spi_configs_giga.spi_config_5.busy_bit_pos         = 0;
  spi_configs_giga.spi_config_5.d7_d4_data           = 0xf;
  spi_configs_giga.spi_config_5.dummy_bytes_for_rdsr = 0x0;
  spi_configs_giga.spi_config_5.reset_type           = 0x0;
  spi_configs_giga.spi_config_5.block_erase_cmd      = BLOCK_ERASE;

  spi_configs_giga.spi_config_6.chip_erase_cmd   = CHIP_ERASE;
  spi_configs_giga.spi_config_6.sector_erase_cmd = SECTOR_ERASE;

  spi_configs_giga.spi_config_7.status_reg_read_cmd  = 0x05;
  spi_configs_giga.spi_config_7.status_reg_write_cmd = 0x01;

#if ((defined GIGA_DEVICE) || (defined MICRON) || (defined EN_MACRONIX_QUAD_FLASH) || (defined MACRONIX_RSERIES))
  //! Refer spi configs from QSPI PRM
  //
#ifdef MACRONIX_RSERIES
  spi_configs_giga.spi_config_1.inst_mode  = SINGLE_MODE;
  spi_configs_giga.spi_config_1.addr_mode  = SINGLE_MODE;
  spi_configs_giga.spi_config_1.dummy_mode = SINGLE_MODE;
#else
  spi_configs_giga.spi_config_1.inst_mode  = QUAD_MODE;
  spi_configs_giga.spi_config_1.addr_mode  = QUAD_MODE;
  spi_configs_giga.spi_config_1.dummy_mode = QUAD_MODE;
#endif

  spi_configs_giga.spi_config_1.data_mode       = QUAD_MODE;
  spi_configs_giga.spi_config_1.extra_byte_mode = QUAD_MODE;
  spi_configs_giga.spi_config_1.prefetch_en     = DIS_PREFETCH;
  spi_configs_giga.spi_config_1.dummy_W_or_R    = DUMMY_READS;
  spi_configs_giga.spi_config_1.extra_byte_en   = 0;
  spi_configs_giga.spi_config_1.d3d2_data       = 3;
  spi_configs_giga.spi_config_1.continuous      = CONTINUOUS;
#ifdef GIGA_DEVICE
  spi_configs_giga.spi_config_1.read_cmd          = 0x0B;
  spi_configs_giga.spi_config_1.flash_type        = GIGA_DEVICE_FLASH;
  spi_configs_giga.spi_config_1.no_of_dummy_bytes = 4;
#endif

#ifdef EN_MACRONIX_QUAD_FLASH
  spi_configs_giga.spi_config_1.read_cmd          = 0x0B;
  spi_configs_giga.spi_config_1.flash_type        = MX_QUAD_FLASH;
  spi_configs_giga.spi_config_1.no_of_dummy_bytes = 2;
#endif

#ifdef MICRON
  spi_configs_giga.spi_config_1.read_cmd          = FREAD_QUAD_O;
  spi_configs_giga.spi_config_1.flash_type        = MICRON_QUAD_FLASH;
  spi_configs_giga.spi_config_1.no_of_dummy_bytes = 1;
#endif

#ifdef MACRONIX_RSERIES
  spi_configs_giga.spi_config_1.read_cmd          = 0x6B;
  spi_configs_giga.spi_config_1.flash_type        = MX_QUAD_FLASH;
  spi_configs_giga.spi_config_1.no_of_dummy_bytes = 1;
  spi_configs_giga.spi_config_1.extra_byte_en     = 0;
#endif

  spi_configs_giga.spi_config_2.addr_width_valid = 0;
  spi_configs_giga.spi_config_2.addr_width       = _24BIT_ADDR;

  spi_configs_giga.spi_config_3._16bit_cmd_valid  = 0;
  spi_configs_giga.spi_config_3._16bit_rd_cmd_msb = 0;
#ifdef CHIP_9118
  spi_configs_giga.spi_config_3.ddr_mode_en = 0;
#endif
  spi_configs_giga.spi_config_3.wr_cmd = 0x2;

#ifdef MACRONIX_RSERIES
  spi_configs_giga.spi_config_3.wr_inst_mode = SINGLE_MODE;
  spi_configs_giga.spi_config_3.wr_addr_mode = SINGLE_MODE;
  spi_configs_giga.spi_config_3.wr_data_mode = SINGLE_MODE;
#else

  spi_configs_giga.spi_config_3.wr_inst_mode      = QUAD_MODE;
  spi_configs_giga.spi_config_3.wr_addr_mode      = QUAD_MODE;
  spi_configs_giga.spi_config_3.wr_data_mode      = QUAD_MODE;

#endif
  spi_configs_giga.spi_config_3.dummys_4_jump = 1;

#ifdef MACRONIX_RSERIES
  spi_configs_giga.spi_config_4.high_perf_mode_en = 1;
#else
  spi_configs_giga.spi_config_4.high_perf_mode_en = 0;
#endif
#ifdef CHIP_9118
  spi_configs_giga.spi_config_4.qspi_manual_ddr_phasse = 0;
  spi_configs_giga.spi_config_4.ddr_data_mode          = 0;
  spi_configs_giga.spi_config_4.ddr_addr_mode          = 0;
  spi_configs_giga.spi_config_4.ddr_inst_mode          = 0;
  spi_configs_giga.spi_config_4.ddr_dummy_mode         = 0;
  spi_configs_giga.spi_config_4.ddr_extra_byte         = 0;
#endif
#ifdef GIGA_DEVICE
  spi_configs_giga.spi_config_4.valid_prot_bits = 5;
#else
  spi_configs_giga.spi_config_4.valid_prot_bits   = 4;
#endif

  spi_configs_giga.spi_config_5.busy_bit_pos         = 0;
  spi_configs_giga.spi_config_5.d7_d4_data           = 0xf;
  spi_configs_giga.spi_config_5.dummy_bytes_for_rdsr = 0x0;
  spi_configs_giga.spi_config_5.reset_type           = 0x3;
#ifdef EN_MACRONIX_QUAD_FLASH
  spi_configs_giga.spi_config_5.reset_type = 0x4;
#endif

#ifdef MACRONIX_RSERIES
  spi_configs_giga.spi_config_5.reset_type = 0x0;
#endif
  spi_configs_giga.spi_config_5.block_erase_cmd = BLOCK_ERASE;

  spi_configs_giga.spi_config_6.chip_erase_cmd   = CHIP_ERASE;
  spi_configs_giga.spi_config_6.sector_erase_cmd = SECTOR_ERASE;

  spi_configs_giga.spi_config_7.status_reg_read_cmd  = 0x05;
  spi_configs_giga.spi_config_7.status_reg_write_cmd = 0x01;
#endif

#ifdef EN_ADESTO_OCTA_FLASH

  spi_configs_giga.spi_config_1.inst_mode         = OCTA_MODE;
  spi_configs_giga.spi_config_1.addr_mode         = OCTA_MODE;
  spi_configs_giga.spi_config_1.data_mode         = OCTA_MODE;
  spi_configs_giga.spi_config_1.dummy_mode        = OCTA_MODE;
  spi_configs_giga.spi_config_1.extra_byte_mode   = OCTA_MODE;
  spi_configs_giga.spi_config_1.prefetch_en       = DIS_PREFETCH;
  spi_configs_giga.spi_config_1.dummy_W_or_R      = DUMMY_WRITES;
  spi_configs_giga.spi_config_1.extra_byte_en     = 0;
  spi_configs_giga.spi_config_1.d3d2_data         = 3;
  spi_configs_giga.spi_config_1.continuous        = CONTINUOUS;
  spi_configs_giga.spi_config_1.read_cmd          = 0x0B;
  spi_configs_giga.spi_config_1.flash_type        = ADESTO_OCTA_FLASH;
  spi_configs_giga.spi_config_1.no_of_dummy_bytes = 8;

  spi_configs_giga.spi_config_3._16bit_cmd_valid  = 0;
  spi_configs_giga.spi_config_3._16bit_rd_cmd_msb = 0;
  spi_configs_giga.spi_config_3.ddr_mode_en       = 0;
  spi_configs_giga.spi_config_3.wr_cmd            = 0x2;
  spi_configs_giga.spi_config_3.wr_inst_mode      = OCTA_MODE;
  spi_configs_giga.spi_config_3.wr_addr_mode      = OCTA_MODE;
  spi_configs_giga.spi_config_3.wr_data_mode      = OCTA_MODE;

  spi_configs_giga.spi_config_4._16bit_wr_cmd_msb      = 0;
  spi_configs_giga.spi_config_4.qspi_ddr_clk_en        = 0;
  spi_configs_giga.spi_config_4.qspi_manual_ddr_phasse = 0;
  spi_configs_giga.spi_config_4.ddr_data_mode          = 0;
  spi_configs_giga.spi_config_4.ddr_addr_mode          = 0;
  spi_configs_giga.spi_config_4.ddr_inst_mode          = 0;
  spi_configs_giga.spi_config_4.ddr_dummy_mode         = 0;
  spi_configs_giga.spi_config_4.ddr_extra_byte         = 0;
  spi_configs_giga.spi_config_4.valid_prot_bits        = 4;

  spi_configs_giga.spi_config_5.busy_bit_pos         = 0;
  spi_configs_giga.spi_config_5.d7_d4_data           = 0xf;
  spi_configs_giga.spi_config_5.dummy_bytes_for_rdsr = 4;
  //spi_configs_giga.spi_config_5.auto_address_27_24     = 4;
  spi_configs_giga.spi_config_5.reset_type      = 0x3;
  spi_configs_giga.spi_config_5.block_erase_cmd = BLOCK_ERASE;

  spi_configs_giga.spi_config_6.chip_erase_cmd   = CHIP_ERASE;
  spi_configs_giga.spi_config_6.sector_erase_cmd = SECTOR_ERASE;

  spi_configs_giga.spi_config_7.status_reg_read_cmd  = 0x05;
  spi_configs_giga.spi_config_7.status_reg_write_cmd = 0x01;

  mbr->qspi_ctrl_flags |= OCTA_FLASH;
#endif
#ifdef EN_MACRONIX_OCTA_FLASH
  spi_configs_giga.spi_config_2.addr_width_valid = 1;
  spi_configs_giga.spi_config_2.addr_width       = _32BIT_ADDR;

  spi_configs_giga.spi_config_1.inst_mode         = OCTA_MODE;
  spi_configs_giga.spi_config_1.addr_mode         = OCTA_MODE;
  spi_configs_giga.spi_config_1.data_mode         = OCTA_MODE;
  spi_configs_giga.spi_config_1.dummy_mode        = OCTA_MODE;
  spi_configs_giga.spi_config_1.extra_byte_mode   = OCTA_MODE;
  spi_configs_giga.spi_config_1.prefetch_en       = DIS_PREFETCH;
  spi_configs_giga.spi_config_1.dummy_W_or_R      = DUMMY_WRITES;
  spi_configs_giga.spi_config_1.extra_byte_en     = 0;
  spi_configs_giga.spi_config_1.d3d2_data         = 3;
  spi_configs_giga.spi_config_1.continuous        = CONTINUOUS;
  spi_configs_giga.spi_config_1.read_cmd          = 0xEC;
  spi_configs_giga.spi_config_1.flash_type        = MX_OCTA_FLASH;
  spi_configs_giga.spi_config_1.no_of_dummy_bytes = 4;

  spi_configs_giga.spi_config_3._16bit_cmd_valid  = 1;
  spi_configs_giga.spi_config_3._16bit_rd_cmd_msb = 0x13;
  spi_configs_giga.spi_config_3.ddr_mode_en       = 0;
  spi_configs_giga.spi_config_3.wr_cmd            = 0x12;
  spi_configs_giga.spi_config_3.wr_inst_mode      = OCTA_MODE;
  spi_configs_giga.spi_config_3.wr_addr_mode      = OCTA_MODE;
  spi_configs_giga.spi_config_3.wr_data_mode      = OCTA_MODE;

  spi_configs_giga.spi_config_4._16bit_wr_cmd_msb      = 0xED;
  spi_configs_giga.spi_config_4.qspi_ddr_clk_en        = 0;
  spi_configs_giga.spi_config_4.qspi_manual_ddr_phasse = 0;
  spi_configs_giga.spi_config_4.ddr_data_mode          = 0;
  spi_configs_giga.spi_config_4.ddr_addr_mode          = 0;
  spi_configs_giga.spi_config_4.ddr_inst_mode          = 0;
  spi_configs_giga.spi_config_4.ddr_dummy_mode         = 0;
  spi_configs_giga.spi_config_4.ddr_extra_byte         = 0;
  spi_configs_giga.spi_config_4.valid_prot_bits        = 4;
  spi_configs_giga.spi_config_4.no_of_ms_dummy_bytes   = 1;

  spi_configs_giga.spi_config_5.busy_bit_pos         = 0;
  spi_configs_giga.spi_config_5.d7_d4_data           = 0xf;
  spi_configs_giga.spi_config_5.dummy_bytes_for_rdsr = 4;
  //spi_configs_giga.spi_config_5.auto_address_27_24     = 4;
  spi_configs_giga.spi_config_5.reset_type      = 0x7;
  spi_configs_giga.spi_config_5.block_erase_cmd = 0xDC23;

  spi_configs_giga.spi_config_6.chip_erase_cmd   = 0xC738;
  spi_configs_giga.spi_config_6.sector_erase_cmd = 0x21DE;

  spi_configs_giga.spi_config_7.status_reg_read_cmd  = 0x05FA;
  spi_configs_giga.spi_config_7.status_reg_write_cmd = 0x01FE;
#endif
}

/**
* @fn           void RSI_PS_Restore(void)
* @brief        This API is used to configure the QSPI in auto mode and to resume the CPU execution after wake up when it is flash based execution.
* @param[in]    None
* @return       None
* @Note         This function must be in RAM, and user application needs point this function address to bootloader before going to 
*               Sleep. Up on wake up bootloader branches to this function in RAM to configure the QSPI and resumes to flash execution.
*
*/
void RSI_PS_Restore(void)
{
  /*This macro needs to be enabled when flash based retention sleep is needed*/
#ifdef FLASH_BASED_RET_SLEEP
  /*Use Dummy stack pointer for QSPI initialization*/
  __set_MSP(INTERMEDIATE_SP);
  /*Init the QSPI configurations structure */
  Init_QspiStc();

  ROMAPI_EGPIO_API->egpio_pad_selection_enable(PadSelectionEnable);
  /*Receive enable for GPIO0 58 to 63*/
  ROMAPI_EGPIO_API->egpio_pad_receiver_enable(M4SS_QSPI_CLK);
  ROMAPI_EGPIO_API->egpio_pad_receiver_enable(M4SS_QSPI_CSN0);
  ROMAPI_EGPIO_API->egpio_pad_receiver_enable(M4SS_QSPI_D0);
  ROMAPI_EGPIO_API->egpio_pad_receiver_enable(M4SS_QSPI_D1);
  ROMAPI_EGPIO_API->egpio_pad_receiver_enable(M4SS_QSPI_D2);
  ROMAPI_EGPIO_API->egpio_pad_receiver_enable(M4SS_QSPI_D3);

  /*Set GPIO pin MUX for QSPI*/
  ROMAPI_EGPIO_API->egpio_set_pin_mux(EGPIO, 0, M4SS_QSPI_CLK, QSPI_MODE);
  ROMAPI_EGPIO_API->egpio_set_pin_mux(EGPIO, 0, M4SS_QSPI_CSN0, QSPI_MODE);
  ROMAPI_EGPIO_API->egpio_set_pin_mux(EGPIO, 0, M4SS_QSPI_D0, QSPI_MODE);
  ROMAPI_EGPIO_API->egpio_set_pin_mux(EGPIO, 0, M4SS_QSPI_D1, QSPI_MODE);
  ROMAPI_EGPIO_API->egpio_set_pin_mux(EGPIO, 0, M4SS_QSPI_D2, QSPI_MODE);
  ROMAPI_EGPIO_API->egpio_set_pin_mux(EGPIO, 0, M4SS_QSPI_D3, QSPI_MODE);

  spi_configs_giga.spi_config_2.auto_mode = EN_AUTO_MODE;
  /*Enable QSPI clock*/
  //RSI_CLK_PeripheralClkEnable(M4CLK ,QSPI_CLK,ENABLE_STATIC_CLK);
  /*Initialize QSPI*/
  ROMAPI_QSPI_API->qspi_spi_init((qspi_reg_t *)(QSPI_BASE_ADDRESS), &spi_configs_giga, 1, 0, 0);
  /*Update the VTOR*/
  (*(volatile uint32_t *)0xE000ED08) = FLASH_VECTOR_OFFSET;
#ifdef DEBUG_UART
  /*Note : This Print is for debug purpose , please comment off if it is not required*/
  DEBUGOUT("\n****IN QSPI CONFIG FUNC****\n");
#endif // DEBUG_UART
#endif //FLASH_BASED_RET_SLEEP
  /*Restore the actual CPU context to resume from flash execution*/
  RSI_PS_RestoreCpuContext();
  return;
}
#if defined(SLI_SI91X_MCU_COMMON_FLASH_MODE)
/**
* @fn           void GetQspiConfig(spi_config_t *spi_config)
* @brief        This function is used to get the QSPI configuration structure of selected flash.
* @param[in]    None
* @return       None.
*
*/
void GetQspiConfig(spi_config_t *spi_config)
{
  memset(spi_config, 0, sizeof(spi_config_t));
#ifdef GIGA_DEVICE
  spi_config->spi_config_1.inst_mode  = SINGLE_MODE; // For Radio Board , instr mode should be in SINGLE_MODE
  spi_config->spi_config_1.addr_mode  = QUAD_MODE;
  spi_config->spi_config_1.dummy_mode = QUAD_MODE;
#else
  spi_config->spi_config_1.inst_mode              = SINGLE_MODE;
  spi_config->spi_config_1.addr_mode              = SINGLE_MODE;
  spi_config->spi_config_1.dummy_mode             = SINGLE_MODE;
#endif
  spi_config->spi_config_1.data_mode       = QUAD_MODE;
  spi_config->spi_config_1.extra_byte_mode = QUAD_MODE;
  spi_config->spi_config_1.prefetch_en     = DIS_PREFETCH;
  spi_config->spi_config_1.dummy_W_or_R    = DUMMY_READS;
  spi_config->spi_config_1.d3d2_data       = 3;
  spi_config->spi_config_1.continuous      = DIS_CONTINUOUS;
#ifdef GIGA_DEVICE
  spi_config->spi_config_1.read_cmd          = 0xEB;
  spi_config->spi_config_1.flash_type        = GIGA_DEVICE_FLASH;
  spi_config->spi_config_1.no_of_dummy_bytes = 2;
  spi_config->spi_config_1.extra_byte_en     = 1;
#else
  spi_config->spi_config_1.read_cmd               = 0x6B;
  spi_config->spi_config_1.flash_type             = MX_QUAD_FLASH;
  spi_config->spi_config_1.no_of_dummy_bytes      = 1;
  spi_config->spi_config_1.extra_byte_en          = 0;
#endif
  spi_config->spi_config_2.auto_mode                   = EN_AUTO_MODE;
  spi_config->spi_config_2.cs_no                       = CHIP_ZERO;
  spi_config->spi_config_2.neg_edge_sampling           = NEG_EDGE_SAMPLING;
  spi_config->spi_config_2.qspi_clk_en                 = QSPI_FULL_TIME_CLK;
  spi_config->spi_config_2.protection                  = DNT_REM_WR_PROT;
  spi_config->spi_config_2.dma_mode                    = NO_DMA;
  spi_config->spi_config_2.swap_en                     = SWAP;
  spi_config->spi_config_2.full_duplex                 = IGNORE_FULL_DUPLEX;
  spi_config->spi_config_2.wrap_len_in_bytes           = NO_WRAP;
  spi_config->spi_config_2.addr_width_valid            = 0;
  spi_config->spi_config_2.addr_width                  = _24BIT_ADDR;
  spi_config->spi_config_2.pinset_valid                = 0;
  spi_config->spi_config_2.dummy_cycles_for_controller = 0;

  spi_config->spi_config_3.xip_mode          = 0;
  spi_config->spi_config_3._16bit_cmd_valid  = 0;
  spi_config->spi_config_3._16bit_rd_cmd_msb = 0;
  spi_config->spi_config_3.reserved          = 0;
  spi_config->spi_config_3.wr_cmd            = 0x2;
  spi_config->spi_config_3.wr_inst_mode      = SINGLE_MODE;
  spi_config->spi_config_3.wr_addr_mode      = SINGLE_MODE;
  spi_config->spi_config_3.wr_data_mode      = SINGLE_MODE;
  spi_config->spi_config_3.dummys_4_jump     = 1;

  spi_config->spi_config_4._16bit_wr_cmd_msb = 0;
  spi_config->spi_config_4.dual_flash_mode   = 0;
  spi_config->spi_config_4.secondary_csn     = 1;
  spi_config->spi_config_4.polarity_mode     = 0;
#ifdef GIGA_DEVICE
  spi_config->spi_config_4.valid_prot_bits = 5;
#else
  spi_config->spi_config_4.valid_prot_bits        = 4;
#endif
  spi_config->spi_config_4.no_of_ms_dummy_bytes = 0;
  spi_config->spi_config_4.continue_fetch_en    = 0;

  spi_config->spi_config_5.block_erase_cmd      = BLOCK_ERASE;
  spi_config->spi_config_5.busy_bit_pos         = 0;
  spi_config->spi_config_5.d7_d4_data           = 0xf;
  spi_config->spi_config_5.dummy_bytes_for_rdsr = 0x0;
  spi_config->spi_config_5.reset_type           = 0x0;

  spi_config->spi_config_6.chip_erase_cmd   = CHIP_ERASE;
  spi_config->spi_config_6.sector_erase_cmd = SECTOR_ERASE;

  spi_config->spi_config_7.status_reg_write_cmd = 0x1;
  spi_config->spi_config_7.status_reg_read_cmd  = 0x5;
}
/**
* @fn           void RSI_FLASH_Initialize(void)
* @brief        This function is used to initialize QSPI .
* @param[in]    None
* @return       None.
*
*/
void RSI_FLASH_Initialize(void)
{
  spi_config_t spi_configs_init;
  // Get QSPI configurations
  GetQspiConfig(&spi_configs_init);
  // QSPI Initialization
  ROMAPI_QSPI_API->qspi_spi_init((qspi_reg_t *)QSPI_HW_ADDR, &spi_configs_init, 0, 0, 1);
}
#endif

/**
* @fn           void RSI_PS_Restore_WiseAoc(void)
* @brief        This API is used to configure the memory configurations and resumes the execution from ROM in WISEAOC mode.
* @param[in]    None
* @return       None
* @Note         This function must be in ROM, and user application needs point this function address to bootloader before going to 
*               Sleep. Up on wake up bootloader branches to this function in ROM to configure the Memories and resumes to ROM execution.
*               This is valid only in WiSeAOC products.
*
*/
#if defined(WISE_AOC_4)
void RSI_PS_Restore_WiseAoc(void)
{
  /*Keep intermediate Stack*/
  __set_MSP(INTERMEDIATE_SP);
  /**/
  M4SS_CLK_ENABLE_SET_REG1 = M4SS_STATIC_CLK_SWITCH;
  __asm("nop");
  __asm("nop");
  __asm("nop");
  __asm("nop");
  M4SS_CLK_ENABLE_CLEAR_REG1 = M4SS_STATIC_CLK_SWITCH;
  __asm("nop");
  __asm("nop");
  __asm("nop");
  __asm("nop");
  /*Restore the CPU context*/
  RSI_PS_RestoreCpuContext();
  return;
}
#endif //defined (WISE_AOC_4)
