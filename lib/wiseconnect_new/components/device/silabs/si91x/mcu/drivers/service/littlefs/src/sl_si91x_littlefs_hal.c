/***************************************************************************/ /**
 * @file sl_si91x_littlefs_hal.c
 * @brief Little fs HAL API's implementation
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
#include "sl_si91x_littlefs_hal.h"
#include "sl_si91x_peripheral_gpio.h"
#include "rsi_qspi.h"
#include "rsi_rom_qspi.h"
#include "rsi_rom_egpio.h"
#include "rsi_rom_clks.h"
/*******************************************************************************
 ***************************  Defines / Macros  ********************************
 ******************************************************************************/

#ifdef SI91X_LITTLEFS_EXT_FLASH_PRESENT // Macro to configure the External flash configs in common flash mode
#include "sl_si91x_littlefs_ext_flash_config.h"
#define PAD_SELECTION_ENABLE_CLK  10
#define PAD_SELECTION_ENABLE_D0   11
#define PAD_SELECTION_ENABLE_D1   12
#define PAD_SELECTION_ENABLE_CSN0 13
#define PAD_SELECTION_ENABLE_D2   14
#define PAD_SELECTION_ENABLE_D3   15

// QSPI Mux
#define QSPI_MODE 11

/*M4 QSPI  pin set */
#define M4SS_QSPI_CLK      46
#define M4SS_QSPI_D0       47
#define M4SS_QSPI_D1       48
#define M4SS_QSPI_CSN0     49
#define M4SS_QSPI_D2       50
#define M4SS_QSPI_D3       51
#define LITTLEFS_QSPI_ADDR M4_QSPI_2_BASE_ADDRESS //QSPI base address for common flash parts

#define LITTLEFS_BASE SL_LITTLEFS_BASE_EXT //Littlefs base address for external flash
#else
#define PAD_SELECTION_ENABLE_CLK  10 // Pad no for qspi clock pin
#define PAD_SELECTION_ENABLE_D0   11 // Pad no for qspi D0 pin
#define PAD_SELECTION_ENABLE_D1   12 // Pad no for qspi D1 pin
#define PAD_SELECTION_ENABLE_CSN0 13 // Pad no for qspi CSN0 pin
#define PAD_SELECTION_ENABLE_D2   14 // Pad no for qspi D2 pin
#define PAD_SELECTION_ENABLE_D3   15 // Pad no for qspi D3 pin

#define QSPI_MODE          1         //Mode to configure the gpio in QSPI mode

/*M4 QSPI  pin set */
#define M4SS_QSPI_CLK      46        // Clock Pin
#define M4SS_QSPI_D0       47        // Data line 0 pin
#define M4SS_QSPI_D1       48        // Data line 1 pin
#define M4SS_QSPI_CSN0     49        // Chip select 0 pin
#define M4SS_QSPI_D2       50        // Data line 2 pin
#define M4SS_QSPI_D3       51        // Data line 3 pin
#define LITTLEFS_QSPI_ADDR QSPI_BASE //QSPI base address for dual flash parts

extern char linker_littlefs_begin;

__attribute__((used)) uint8_t littlefs_default_storage[LITTLEFS_DEFAULT_MEM_SIZE] __attribute__((section(".ltfs")));
#define LITTLEFS_BASE      (&linker_littlefs_begin)
#endif

#define MAX_FREQUENCY             33000000 // Max clock frequency for read ID command
#define FLASH_INIT_REQUEST_ENABLE 1        // QSPI init request
#define WRITE_REG_DELAY_NONE      0 // delay in ms provided after a register write operation is performed on flash
#define FIFO_THRESHOLD_NONE       0 // FIFO threshold 0 level 0
#define ODD_DIV_DISABLE           0 // Disable odd div functionality. See user manual for more info
#define SWALLOEN_DISABLE          0 // Disable the swallo functionality. See user manual for more info
#define EGPIO_PORT                0 // EGPIO port number
#define DISABLE_HW_CTRL           1 // Disable hw ctrl, while waiting for flash to go idle

/******************************************************************************
 * Configurations for QSPI module
 ******************************************************************************/
void set_qspi_configs(spi_config_t *spi_config)
{
  memset(spi_config, 0, sizeof(spi_config_t));

  spi_config->spi_config_1.inst_mode         = SINGLE_MODE;
  spi_config->spi_config_1.addr_mode         = SINGLE_MODE;
  spi_config->spi_config_1.data_mode         = SINGLE_MODE;
  spi_config->spi_config_1.dummy_mode        = SINGLE_MODE;
  spi_config->spi_config_1.extra_byte_mode   = SINGLE_MODE;
  spi_config->spi_config_1.read_cmd          = READ;
  spi_config->spi_config_1.no_of_dummy_bytes = 0;
  spi_config->spi_config_1.prefetch_en       = DIS_PREFETCH;
  spi_config->spi_config_1.dummy_W_or_R      = DUMMY_READS;
  spi_config->spi_config_1.extra_byte_en     = 0;
  spi_config->spi_config_1.d3d2_data         = 3;
  spi_config->spi_config_1.continuous        = DIS_CONTINUOUS;
  spi_config->spi_config_1.flash_type        = MX_QUAD_FLASH;

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
  spi_config->spi_config_2.flash_pinset                = GPIO_46_TO_51;
  spi_config->spi_config_2.dummy_cycles_for_controller = 0;

  spi_config->spi_config_3.wr_cmd            = 0x2;
  spi_config->spi_config_3.wr_inst_mode      = SINGLE_MODE;
  spi_config->spi_config_3.wr_addr_mode      = SINGLE_MODE;
  spi_config->spi_config_3.wr_data_mode      = SINGLE_MODE;
  spi_config->spi_config_3.xip_mode          = 0;
  spi_config->spi_config_3._16bit_cmd_valid  = 0;
  spi_config->spi_config_3._16bit_rd_cmd_msb = 0;
  spi_config->spi_config_3.dummys_4_jump     = 1;

  spi_config->spi_config_4._16bit_wr_cmd_msb    = 0;
  spi_config->spi_config_4.high_perf_mode_en    = 0;
  spi_config->spi_config_4.dual_flash_mode      = 0;
  spi_config->spi_config_4.secondary_csn        = 1;
  spi_config->spi_config_4.polarity_mode        = 0;
  spi_config->spi_config_4.valid_prot_bits      = 4;
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

/******************************************************************************
 * QSPI gpio confiurations
 ******************************************************************************/
static void si91x_qspi_pin_mux_init(void)
{
  /*Pad selection enable */
  sl_si91x_gpio_enable_pad_selection(PAD_SELECTION_ENABLE_CLK);
  sl_si91x_gpio_enable_pad_selection(PAD_SELECTION_ENABLE_D0);
  sl_si91x_gpio_enable_pad_selection(PAD_SELECTION_ENABLE_D1);
  sl_si91x_gpio_enable_pad_selection(PAD_SELECTION_ENABLE_CSN0);
  sl_si91x_gpio_enable_pad_selection(PAD_SELECTION_ENABLE_D2);
  sl_si91x_gpio_enable_pad_selection(PAD_SELECTION_ENABLE_D3);

  /*Receive enable for QSPI GPIO*/
  sl_si91x_gpio_enable_pad_receiver(M4SS_QSPI_CLK);
  sl_si91x_gpio_enable_pad_receiver(M4SS_QSPI_CSN0);
  sl_si91x_gpio_enable_pad_receiver(M4SS_QSPI_D0);
  sl_si91x_gpio_enable_pad_receiver(M4SS_QSPI_D1);
  sl_si91x_gpio_enable_pad_receiver(M4SS_QSPI_D2);
  sl_si91x_gpio_enable_pad_receiver(M4SS_QSPI_D3);

  /*Set GPIO pin MUX for QSPI*/
  sl_gpio_set_pin_mode(EGPIO_PORT, M4SS_QSPI_CLK, QSPI_MODE, CLR);
  sl_gpio_set_pin_mode(EGPIO_PORT, M4SS_QSPI_CSN0, QSPI_MODE, CLR);
  sl_gpio_set_pin_mode(EGPIO_PORT, M4SS_QSPI_D0, QSPI_MODE, CLR);
  sl_gpio_set_pin_mode(EGPIO_PORT, M4SS_QSPI_D1, QSPI_MODE, CLR);
  sl_gpio_set_pin_mode(EGPIO_PORT, M4SS_QSPI_D2, QSPI_MODE, CLR);
  sl_gpio_set_pin_mode(EGPIO_PORT, M4SS_QSPI_D3, QSPI_MODE, CLR);
}

/******************************************************************************
 * Initialize the qspi for littlefs
 ******************************************************************************/
void sl_si91x_littlefs_qspi_init()
{
  uint16_t clkDivFactor       = 0;
  uint32_t system_clock_value = system_clocks.ulpss_ref_clk;

  /*Init the QSPI configurations structure */
  spi_config_t spi_configs_init;
  /* read ID command supports max frequency of 33MHz */
  if (system_clock_value > MAX_FREQUENCY) {
    clkDivFactor = (system_clock_value / MAX_FREQUENCY);
  }
  /*Clock Initialization*/
#ifndef SI91X_LITTLEFS_EXT_FLASH_PRESENT
  RSI_CLK_QspiClkConfig(M4CLK, QSPI_ULPREFCLK, SWALLOEN_DISABLE, ODD_DIV_DISABLE, clkDivFactor);
#else
  RSI_CLK_Qspi2ClkConfig(M4CLK, QSPI_ULPREFCLK, SWALLOEN_DISABLE, ODD_DIV_DISABLE, clkDivFactor);
#endif

  /*set the QSPI configuration structures*/
  set_qspi_configs(&spi_configs_init);

  /* Configures the pin MUX for QSPI  pins*/
  si91x_qspi_pin_mux_init();

  /* initializes QSPI  */
  RSI_QSPI_SpiInit((qspi_reg_t *)LITTLEFS_QSPI_ADDR,
                   &spi_configs_init,
                   FLASH_INIT_REQUEST_ENABLE,
                   WRITE_REG_DELAY_NONE,
                   FIFO_THRESHOLD_NONE);
}

/******************************************************************************
 * Read the data from flash
 ******************************************************************************/
int si91x_block_device_read(const struct lfs_config *cfg,
                            lfs_block_t block,
                            lfs_off_t off,
                            void *buffer,
                            lfs_size_t size)
{
  uint32_t flash_read_addr = 0, status = QSPI_OK;
  spi_config_t spi_configs_program;

  assert(block < cfg->block_count);

  //Calculate the flash read address based on block number and offset
  flash_read_addr = (uint32_t)LITTLEFS_BASE + (block * cfg->block_size) + off;
  set_qspi_configs(&spi_configs_program);
  if (flash_read_addr == 0) {
    status = QSPI_ERROR;
  }
  RSI_QSPI_ManualRead((qspi_reg_t *)LITTLEFS_QSPI_ADDR,
                      &spi_configs_program,
                      flash_read_addr,
                      (uint8_t *)buffer,
                      0 /*_32BIT*/,
                      size,
                      0,
                      0,
                      0);

  return status;
}

/******************************************************************************
 * Write the data to the flash
 ******************************************************************************/
int si91x_block_device_prog(const struct lfs_config *cfg,
                            lfs_block_t block,
                            lfs_off_t off,
                            const void *buffer,
                            lfs_size_t size)
{
  uint32_t flash_prog_addr = 0, status = QSPI_OK;
  spi_config_t spi_configs_program;
  assert(block < cfg->block_count);
  set_qspi_configs(&spi_configs_program);
  //Calculate the flash write address based on block number and offset
  flash_prog_addr = (uint32_t)LITTLEFS_BASE + (block * cfg->block_size) + off;

  if (flash_prog_addr == 0) {
    status = QSPI_ERROR;
  }
  //Call QSPI write API
  status = RSI_QSPI_SpiWrite((qspi_reg_t *)LITTLEFS_QSPI_ADDR,
                             &spi_configs_program,
                             0x2,
                             flash_prog_addr,
                             (uint8_t *)buffer,
                             size,
                             LITTLEFS_FLASH_PAGE_SIZE,
                             _1BYTE,
                             0,
                             0,
                             0,
                             0,
                             0,
                             0);

  return status;
}

/******************************************************************************
 * Erase the data from flash
 ******************************************************************************/
int si91x_block_device_erase(const struct lfs_config *cfg, lfs_block_t block)
{
  uint32_t flash_erase_addr = 0, status = QSPI_OK;
  spi_config_t spi_configs_erase;

  assert((block < cfg->block_count));

  //Calculate the flash erase address/block start address based on block number and offset
  flash_erase_addr = (uint32_t)LITTLEFS_BASE + (block * cfg->block_size);

  if (flash_erase_addr == 0)
    status = QSPI_ERROR;

  set_qspi_configs(&spi_configs_erase);
  //Call QSPI erase API
  RSI_QSPI_SpiErase((qspi_reg_t *)LITTLEFS_QSPI_ADDR,
                    &spi_configs_erase,
                    SECTOR_ERASE,
                    flash_erase_addr,
                    DISABLE_HW_CTRL,
                    WRITE_REG_DELAY_NONE);

  return status;
}

/*****************************************************************************
 *  Sync API
 ******************************************************************************/
int si91x_block_device_sync(const struct lfs_config *c)
{
  (void)c;
  return 0;
}
