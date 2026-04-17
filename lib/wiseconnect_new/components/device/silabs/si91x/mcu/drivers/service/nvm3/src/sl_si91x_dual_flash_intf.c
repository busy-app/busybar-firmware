/*******************************************************************************
 * @file  Flash_Intf.c
 * @brief
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/
#include "sl_si91x_dual_flash_intf.h"
#include "si91x_device.h"
#include "rsi_debug.h"
#include "rsi_rom_qspi.h"

#include "system_si91x.h"

/*******************************************************************************
 ***************************  DEFINES / MACROS   ********************************
 ******************************************************************************/

#if defined(SLI_SI917)
#define PadSelectionEnable_CLK  16
#define PadSelectionEnable_D0   17
#define PadSelectionEnable_D1   18
#define PadSelectionEnable_CSN0 19
#define PadSelectionEnable_D2   20
#define PadSelectionEnable_D3   21

#define QSPI_MODE 9

/*M4 QSPI  pin set */
#define M4SS_QSPI_CLK  52
#define M4SS_QSPI_D0   53
#define M4SS_QSPI_D1   54
#define M4SS_QSPI_CSN0 55
#define M4SS_QSPI_D2   56
#define M4SS_QSPI_D3   57
#endif

/*******************************************************************************
* @brief: Flash access APIs used for NVM3 for Si911x dual flash

* @details:
* These APIs are used for flash erase, flash write and flash read operations
* For erase operation supplied address should be 4k aligned. For write operation
* address should be word aligned
*******************************************************************************/

/*******************************************************************************
 ***********************  Global function Definitions *************************
 ******************************************************************************/

/***************************************************************************/ /**
 *  Uninitialize QSPI module
 ******************************************************************************/
void rsi_flash_uninitialize(void)
{
  /* This function not supported */
}

/***************************************************************************/ /**
 * Configures the pin MUX for QSPI  pins
 ******************************************************************************/
static void rsi_qspi_pin_mux_init(void)
{
#ifdef CHIP_9118
  /*Pad selection enable */
  RSI_EGPIO_PadSelectionEnable(15);
#endif

#if defined(SLI_SI917)
  /*Pad selection enable */
  RSI_EGPIO_PadSelectionEnable(PadSelectionEnable_CLK);
  RSI_EGPIO_PadSelectionEnable(PadSelectionEnable_D0);
  RSI_EGPIO_PadSelectionEnable(PadSelectionEnable_D1);
  RSI_EGPIO_PadSelectionEnable(PadSelectionEnable_CSN0);
  RSI_EGPIO_PadSelectionEnable(PadSelectionEnable_D2);
  RSI_EGPIO_PadSelectionEnable(PadSelectionEnable_D3);
#endif

  /*Receive enable for QSPI GPIO*/
  RSI_EGPIO_PadReceiverEnable(M4SS_QSPI_CLK);
  RSI_EGPIO_PadReceiverEnable(M4SS_QSPI_CSN0);
  RSI_EGPIO_PadReceiverEnable(M4SS_QSPI_D0);
  RSI_EGPIO_PadReceiverEnable(M4SS_QSPI_D1);
  RSI_EGPIO_PadReceiverEnable(M4SS_QSPI_D2);
  RSI_EGPIO_PadReceiverEnable(M4SS_QSPI_D3);

  /*Set GPIO pin MUX for QSPI*/
  RSI_EGPIO_SetPinMux(EGPIO, 0, M4SS_QSPI_CLK, QSPI_MODE);
  RSI_EGPIO_SetPinMux(EGPIO, 0, M4SS_QSPI_CSN0, QSPI_MODE);
  RSI_EGPIO_SetPinMux(EGPIO, 0, M4SS_QSPI_D0, QSPI_MODE);
  RSI_EGPIO_SetPinMux(EGPIO, 0, M4SS_QSPI_D1, QSPI_MODE);
  RSI_EGPIO_SetPinMux(EGPIO, 0, M4SS_QSPI_D2, QSPI_MODE);
  RSI_EGPIO_SetPinMux(EGPIO, 0, M4SS_QSPI_D3, QSPI_MODE);
}

/***************************************************************************/ /**
 * This function initializes GPIO, QSPI and flash
 ******************************************************************************/
/* QSPI HAL wrapper for NVM3 */
void rsi_flash_init(void)
{
  /*Init the QSPI configurations structure */
  spi_config_t spi_configs_init;

  /*Get QSPI  structures configuration     */
  get_qspi_config(&spi_configs_init);

  /* Configures the pin MUX for QSPI  pins*/
  rsi_qspi_pin_mux_init();

  /* initializes QSPI  */
  RSI_QSPI_SpiInit((qspi_reg_t *)QSPI_BASE, &spi_configs_init, 1, 0, 0);
}

/***************************************************************************/ /**
 * This function erases the flash
 ******************************************************************************/
sl_status_t rsi_flash_erase_sector(uint32_t *sector_address)
{
  spi_config_t spi_configs_erase;
  uint32_t status = SL_STATUS_OK;
  get_qspi_config(&spi_configs_erase);

  /* Erases the SECTOR   */
  if (sector_address == NULL) {
    status = SL_STATUS_NULL_POINTER;
  } else {
    RSI_QSPI_SpiErase((qspi_reg_t *)QSPI_BASE, &spi_configs_erase, SECTOR_ERASE, (uint32_t)sector_address, 1, 0);
  }

  /* The erase function does the erase and takes care of any missing
   * configurations to successfully erase the sector*/
  return status;
}

/***************************************************************************/ /**
 * This function writes to destination flash address location
 ******************************************************************************/
sl_status_t rsi_flash_write(uint32_t *address, unsigned char *data, uint32_t length)
{
  spi_config_t spi_configs_program;
  sl_status_t status = SL_STATUS_OK;
  get_qspi_config(&spi_configs_program);

  /* writes the data to required address using qspi */
  if (address == NULL) {
    status = SL_STATUS_NULL_POINTER;
  } else {
    RSI_QSPI_SpiWrite((qspi_reg_t *)QSPI_BASE,
                      &spi_configs_program,
                      0x2,
                      (uint32_t)address,
                      (uint8_t *)&data[0],
                      length,
                      FLASH_PAGE_SIZE,
                      _1BYTE,
                      0,
                      0,
                      0,
                      0,
                      0,
                      0);
  }
  return status;
}

/***************************************************************************/ /**
 * Reads data from the address in selected mode
 ******************************************************************************/
sl_status_t rsi_flash_read(uint32_t *address, unsigned char *data, uint32_t length, uint8_t auto_mode)
{
  spi_config_t spi_configs_program;
  sl_status_t status = SL_STATUS_OK;
  get_qspi_config(&spi_configs_program);

  /* IO_READ - Manual Mode */
  if (address == NULL) {
    status = SL_STATUS_NULL_POINTER;
  } else {
    if (!auto_mode) {
      /* IO Read config */
      /* Reads from the address in manual mode */
      //       DEBUGOUT("\r\n Read Data From Flash Memory Using Manual Mode\r\n");
      //       RSI_QSPI_ManualRead((qspi_reg_t *)(QSPI_BASE), &spi_configs_program,
      //                           address, (uint8_t *)data, _8BIT, length, 0, 0, 0);
      memcpy((uint8_t *)data, (uint8_t *)address, length * 4);
    } else { /* DMA_READ - Auto mode */
             /* Read the data by using UDMA */
#if 0        // Need to Implement
      DEBUGOUT("\r\n Read Data From Flash Memory Using DMA \r\n");
      UDMA_Read();
      /* Wait till dma done */
      while (!done)
        ;
#endif
    }
  }
  return status;
}

/***************************************************************************/ /**
 * Configurations for QSPI module
 ******************************************************************************/
void get_qspi_config(spi_config_t *spi_config)
{
  //This structure members are used to configure qspi
  memset(spi_config, 0, sizeof(spi_config_t));
  spi_config->spi_config_1.inst_mode         = SINGLE_MODE;
  spi_config->spi_config_1.addr_mode         = SINGLE_MODE;
  spi_config->spi_config_1.data_mode         = SINGLE_MODE;
  spi_config->spi_config_1.dummy_mode        = SINGLE_MODE;
  spi_config->spi_config_1.extra_byte_mode   = SINGLE_MODE;
  spi_config->spi_config_1.prefetch_en       = DIS_PREFETCH;
  spi_config->spi_config_1.dummy_W_or_R      = DUMMY_READS;
  spi_config->spi_config_1.extra_byte_en     = 0;
  spi_config->spi_config_1.d3d2_data         = 3;
  spi_config->spi_config_1.continuous        = DIS_CONTINUOUS;
  spi_config->spi_config_1.read_cmd          = READ;
  spi_config->spi_config_1.flash_type        = SST_SPI_FLASH;
  spi_config->spi_config_1.no_of_dummy_bytes = 0;

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
  spi_config->spi_config_2.flash_pinset                = GPIO_58_TO_63;
  spi_config->spi_config_2.dummy_cycles_for_controller = 0;

  spi_config->spi_config_3.xip_mode          = 0;
  spi_config->spi_config_3._16bit_cmd_valid  = 0;
  spi_config->spi_config_3._16bit_rd_cmd_msb = 0;
#ifdef CHIP_9118
  spi_config->spi_config_3.ddr_mode_en = 0;
#endif
  spi_config->spi_config_3.wr_cmd        = 0x2;
  spi_config->spi_config_3.wr_inst_mode  = SINGLE_MODE;
  spi_config->spi_config_3.wr_addr_mode  = SINGLE_MODE;
  spi_config->spi_config_3.wr_data_mode  = SINGLE_MODE;
  spi_config->spi_config_3.dummys_4_jump = 1;

  spi_config->spi_config_4._16bit_wr_cmd_msb = 0;
#ifdef CHIP_9118
  spi_config->spi_config_4.qspi_manual_ddr_phasse = 0;
  spi_config->spi_config_4.ddr_data_mode          = 0;
  spi_config->spi_config_4.ddr_addr_mode          = 0;
  spi_config->spi_config_4.ddr_inst_mode          = 0;
  spi_config->spi_config_4.ddr_dummy_mode         = 0;
  spi_config->spi_config_4.ddr_extra_byte         = 0;
#endif
  spi_config->spi_config_4.dual_flash_mode      = 0;
  spi_config->spi_config_4.secondary_csn        = 1;
  spi_config->spi_config_4.polarity_mode        = 0;
  spi_config->spi_config_4.valid_prot_bits      = 4;
  spi_config->spi_config_4.no_of_ms_dummy_bytes = 0;
#ifdef CHIP_9118
  spi_config->spi_config_4.ddr_dll_en = 0;
#endif
  spi_config->spi_config_4.continue_fetch_en = 0;

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
