/******************************************************************************
* @file  rsi_qspi.c
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

// Include Files

#include "rsi_ccp_user_config.h"

#include "rsi_rom_table_si91x.h"
#include "rsi_qspi.h"
#include "rsi_efuse.h"
#include "rsi_rom_timer.h"

// static function prototype
static void qspi_aes_encrypt_decrypt_standalone(qspi_reg_t *qspi_reg,
                                                uint8_t aes_mode,
                                                bool encrypt,
                                                uint32_t *in_data,
                                                uint32_t *out_data,
                                                uint32_t *key1,
                                                uint32_t *key2,
                                                uint32_t key_len,
                                                uint32_t *iv,
                                                bool kh_enable,
                                                uint32_t data_length,
                                                uint32_t flip_data);
#ifndef QSPI_ROMDRIVER_PRESENT
/*==============================================*/
/**
 * @fn           void initialise_m4_efuse_in_io_mode()
 * @brief        This API used to initialise the Efuse program of input output mode  
 * @return       none
 */
void initialise_m4_efuse_in_io_mode()
{
  M4SS_CLK_ENABLE_SET_3_REG = EFUSE_CLK_BIT | EFUSE_PCLK_BIT;
  // Program Timing params
  M4_EFUSE_RD_TMNG_PARAM_REG = 0x5A2;
  // Program Mem map mode to byte read
  M4_EFUSE_MEM_MAP_LENGTH = 0;
  // Enable Efuse
  M4_EFUSE_CTRL_REG = 0x1;
}

/*==============================================*/
/**
 * @fn           void rsi_cmemcpy(uint8_t *dst, uint8_t *src, uint32_t len)
 * @brief        This API used to copy the command from source to destination
 * @param[in]    dst : pointer to the destination command  
 * @param[in]    src : pointer to the source command
 * @param[in]    len : length 
 * @return       none
 */
void rsi_cmemcpy(uint8_t *dst, uint8_t *src, uint32_t len)
{
  while (len--) {
    *dst++ = *src++;
  }
}

/*==============================================*/
/**
 * @fn           void qspi_write_to_flash(qspi_reg_t *qspi_reg, uint32_t len_in_bits, uint32_t cmd_addr_data, uint32_t cs_no)
 * @brief        This API used to QSPI register read and write to the flash
 * @param[in]    qspi_reg      :  qspi register pointer to the qspi manual
 * @param[in]    len_in_bits   :  length of the word
 * @param[in]    cmd_addr_data :  command data address
 * @param[in]    cs_no         :  chip select number  
 * @return       none
 */
void qspi_write_to_flash(qspi_reg_t *qspi_reg, uint32_t len_in_bits, uint32_t cmd_addr_data, uint32_t cs_no)
{
  // length of the word to be programmed
  qspi_reg->QSPI_MANUAL_WRITE_DATA_2_REG = len_in_bits;
  // cmd/data/addr to be written
  qspi_reg->QSPI_MANUAL_RD_WR_DATA_REG = cmd_addr_data;
  qspi_reg->QSPI_MANUAL_CONFIG_REG =
    (((qspi_reg->QSPI_MANUAL_CONFIG_REG & ((uint32_t)~0x6007)) & ~TAKE_LEN_FRM_REG) | WRITE_TRIGGER | (cs_no << 13));
  // wait till QSPI becomes idle
  while (qspi_reg->QSPI_STATUS_REG & 1)
    ;
}

/*==============================================*/
/**
 * @fn           void qspi_switch_qspi2(qspi_reg_t *qspi_reg, uint32_t mode, uint32_t cs_no)
 * @brief        This API used to qspi switch to another qspi 2
 * @param[in]    qspi_reg  :  qspi register pointer to the qspi manual
 * @param[in]    mode      :  bus mode in bits
 * @param[in]    cs_no     :  chip select number  
 * @return       none
 */
void qspi_switch_qspi2(qspi_reg_t *qspi_reg, uint32_t mode, uint32_t cs_no)
{
  uint32_t qspi_manual_config_reg;

  // clearing existing read count and bus mode
  qspi_reg->QSPI_MANUAL_CONFIG_REG &= (uint32_t)~0x1ffe;
  if (cs_no == CHIP_ZERO) {
    // for chip select zero, configure bus mode
    qspi_reg->QSPI_BUS_MODE_REG = ((qspi_reg->QSPI_BUS_MODE_REG & ((uint32_t)~0x6)) | (mode << 1));
  } else {
    // read the reg
    qspi_manual_config_reg = qspi_reg->QSPI_MANUAL_CONFIG_2_REG;
    // mask the bus mode bits
    qspi_manual_config_reg &= (uint16_t)(~(0x3 << (((cs_no - 1) * 2) + 8)));
    // write the bus mode bits
    qspi_manual_config_reg |= ((mode) << (((cs_no - 1) * 2) + 8));
    // write back to the register
    qspi_reg->QSPI_MANUAL_CONFIG_2_REG = qspi_manual_config_reg;
  }
}

/*==============================================*/
/**
 * @fn           uint32_t qspi_wait_flash_status_Idle(qspi_reg_t *qspi_reg, spi_config_t *spi_config, uint32_t wr_reg_delay_ms)
 * @brief        This API used to qspi wait for flash status is Idle
 * @param[in]    qspi_reg        :   qspi register pointer to the qspi manual
 * @param[in]    spi_config      :   pointer to the SPI configuration 
 * @param[in]    wr_reg_delay_ms :   read and write register delay in milisecond 
 * @return       return flash_status for success
 */
uint32_t qspi_wait_flash_status_Idle(qspi_reg_t *qspi_reg, spi_config_t *spi_config, uint32_t wr_reg_delay_ms)
{
  (void)wr_reg_delay_ms;
  uint32_t busy, tmp_dummy;
  uint32_t qspi_operational_mode;
  uint32_t cs_no, flash_type, status_reg_read_cmd, busy_bit_pos;
  uint32_t cmd_len, dummy_bytes = 0;
  volatile uint32_t flash_status;

  cs_no               = spi_config->spi_config_2.cs_no;
  flash_type          = spi_config->spi_config_1.flash_type;
  status_reg_read_cmd = spi_config->spi_config_7.status_reg_read_cmd;

  qspi_operational_mode = QSPI_MANUAL_BUS_SIZE(cs_no);

  // SST_QFLASH supports status reg read in Quad mode
  busy_bit_pos = spi_config->spi_config_5.busy_bit_pos;
  if (QSPI_DUAL_FLASH_MODE) {
    if ((busy_bit_pos < 4)) {
      busy = (BIT(busy_bit_pos)) | (BIT(busy_bit_pos) << 4);
    } else {
      busy_bit_pos -= 4;
      busy = (((BIT(busy_bit_pos)) | (BIT(busy_bit_pos) << 4)) << 8);
    }
  } else {
    // flashes which support status reg read only in SPI mode
    busy = BIT(busy_bit_pos);
  }
  // ensure previous operation is terminated
  DEASSERT_CSN;

  // status reg read cmd

  cmd_len = ((spi_config->spi_config_3._16bit_cmd_valid) && (qspi_operational_mode == OCTA_MODE)) ? 16 : 8;
  if ((cmd_len == 8) && (spi_config->spi_config_3._16bit_cmd_valid)) {
    status_reg_read_cmd >>= 8;
  }
#if 0
	if(cmd_len > 8) {
		status_reg_read_cmd = status_reg_read_cmd | (RDSR << 8);
	}
#endif

  qspi_write_to_flash(qspi_reg, cmd_len, status_reg_read_cmd, cs_no);

  if ((flash_type == MX_OCTA_FLASH) && (qspi_operational_mode == OCTA_MODE)) {
    qspi_write_to_flash(qspi_reg, QSPI_8BIT_LEN * 4, 0x00, cs_no);
  }

  if ((qspi_operational_mode == OCTA_MODE) || (qspi_operational_mode == QUAD_MODE)) {
    dummy_bytes = spi_config->spi_config_5.dummy_bytes_for_rdsr;
  }
  if (dummy_bytes) {
    do {
      tmp_dummy = (dummy_bytes & 0x3) ? (dummy_bytes & 3) : 4;

      qspi_write_to_flash(qspi_reg, (QSPI_8BIT_LEN * tmp_dummy), 0x00, cs_no);
      dummy_bytes -= tmp_dummy;

    } while (dummy_bytes);
  }

  if (spi_config->spi_config_4.dual_flash_mode) {
    qspi_reg->OCTA_SPI_BUS_CONTROLLER2 &= ~BIT(19);
    qspi_switch_qspi2(qspi_reg, OCTA_MODE, cs_no);
  }

  do {
#ifdef CHIP_9118
    if (QSPI_DATA_DDR_MODE || QSPI_DUAL_FLASH_MODE)
#else
    if (QSPI_DUAL_FLASH_MODE)
#endif
    {
      READ_4M_FLASH(2, cs_no, _16BIT);
    } else {
      READ_4M_FLASH(1, cs_no, 0);
    }
    // wait till the fifo empty is deasserted
    while (qspi_reg->QSPI_STATUS_REG & QSPI_FIFO_EMPTY_RFIFO_S)
      ;

      // read status
#ifdef CHIP_9118
    if (QSPI_DATA_DDR_MODE || QSPI_DUAL_FLASH_MODE)
#else
    if (QSPI_DUAL_FLASH_MODE)
#endif
    {
      flash_status = (uint16_t)qspi_reg->QSPI_MANUAL_RD_WR_DATA_REG;
    } else {
      flash_status = (uint8_t)qspi_reg->QSPI_MANUAL_RD_WR_DATA_REG;
    }
    // if flash is busy, continue reading till it becomes idle
  } while (flash_status & busy);

  if (spi_config->spi_config_4.dual_flash_mode) {
    qspi_reg->OCTA_SPI_BUS_CONTROLLER2 |= BIT(19);
  }
  qspi_switch_qspi2(qspi_reg, qspi_operational_mode, cs_no);
  DEASSERT_CSN;

  return flash_status;
}

/*==============================================*/
/**
 * @fn           void qspi_enable_status_reg_write(qspi_reg_t *qspi_reg, uint32_t flash_type, spi_config_t *spi_config, uint32_t cs_no)
 * @brief        This API used to qspi enable the status register for write
 * @param[in]    qspi_reg       : qspi register pointer to the qspi manual
 * @param[in]    flash_type     : flash type
 * @param[in]    spi_config     : pointer to the SPI configuration 
 * @param[in]    cs_no          : chip select 
 * @return       none
 */
void qspi_enable_status_reg_write(qspi_reg_t *qspi_reg, uint32_t flash_type, spi_config_t *spi_config, uint32_t cs_no)
{
  (void)flash_type;
  uint32_t qspi_operational_mode;
  qspi_operational_mode = QSPI_MANUAL_BUS_SIZE(cs_no);
  // write enable added by Samson after verifying with LiteFi
  qspi_write_to_flash(qspi_reg, QSPI_8BIT_LEN, WREN, cs_no);
  if (spi_config->spi_config_3._16bit_cmd_valid && (qspi_operational_mode == OCTA_MODE)) {
    qspi_write_to_flash(qspi_reg, QSPI_8BIT_LEN, WREN2, cs_no);
  }
  DEASSERT_CSN;
}

/*==============================================*/
/**
 * @fn           void qspi_status_reg_write( qspi_reg_t *qspi_reg,
                           uint32_t write_value,
                           spi_config_t *spi_config,
                           uint32_t wr_reg_delay_ms )
 * @brief        This API used to qspi status register for write
 * @param[in]    qspi_reg        :   qspi register pointer to the qspi manual
 * @param[in]    write_value     :   write the value
 * @param[in]    spi_config      :   pointer to the SPI configuration 
 * @param[in]    wr_reg_delay_ms :   register delay in milisecond 
 * @return       none
 */
void qspi_status_reg_write(qspi_reg_t *qspi_reg,
                           uint32_t write_value,
                           spi_config_t *spi_config,
                           uint32_t wr_reg_delay_ms)
{
  uint32_t cs_no;
  uint32_t cmd_len;
  uint32_t flash_type, qspi_operational_mode, status_reg_write_cmd;
  uint32_t send_16bit_data = 0, send_24bit_data = 0, do_non_volatile_write = 0;

  cs_no                 = spi_config->spi_config_2.cs_no;
  flash_type            = spi_config->spi_config_1.flash_type;
  qspi_operational_mode = QSPI_MANUAL_BUS_SIZE(cs_no);
  status_reg_write_cmd  = spi_config->spi_config_7.status_reg_write_cmd;

  if (wr_reg_delay_ms & BIT(29)) {
    wr_reg_delay_ms &= ~BIT(29);
    wr_reg_delay_ms &= ~BIT(31);
    send_24bit_data = 1;
  }
  if (wr_reg_delay_ms & BIT(31)) {
    wr_reg_delay_ms &= ~BIT(31);
    send_16bit_data = 1;
  }

  if (wr_reg_delay_ms & BIT(30)) {
    wr_reg_delay_ms &= ~BIT(30);
    do_non_volatile_write = 1;
  }
  if (spi_config->spi_config_4.dual_flash_mode) {
    qspi_reg->OCTA_SPI_BUS_CONTROLLER2 |= BIT(19);
  }
  if (((flash_type == SST_SPI_FLASH) || (flash_type == GIGA_DEVICE_FLASH)) && (!do_non_volatile_write)) {
    // cmd to enable status reg write
    qspi_write_to_flash(qspi_reg, QSPI_8BIT_LEN, EWSR, cs_no);
    DEASSERT_CSN;
  } else {
    // enable status reg write
    qspi_enable_status_reg_write(qspi_reg, spi_config->spi_config_1.flash_type, spi_config, cs_no);
#if 0       
		if ((flash_type == WBOND_QUAD_FLASH)) {
			qspi_func->write_to_flash(qspi_reg, CMD_LEN, WREN, cs_no);   
			DEASSERT_CSN;
		}
#endif
  }

  cmd_len = ((flash_type == MX_OCTA_FLASH) && (qspi_operational_mode == OCTA_MODE)) ? 16 : 8;
  cmd_len = ((spi_config->spi_config_3._16bit_cmd_valid) && (qspi_operational_mode == OCTA_MODE)) ? 16 : 8;
  if ((cmd_len == 8) && (spi_config->spi_config_3._16bit_cmd_valid)) {
    status_reg_write_cmd >>= 8;
  }
  qspi_write_to_flash(qspi_reg, cmd_len, status_reg_write_cmd, cs_no);

  if ((flash_type == MX_OCTA_FLASH) && (qspi_operational_mode == OCTA_MODE)) {
    qspi_write_to_flash(qspi_reg, QSPI_8BIT_LEN * 4, 0x00, cs_no);
  }

  if ((flash_type == WBOND_QUAD_FLASH) || (flash_type == GIGA_DEVICE_FLASH) || send_16bit_data) {
    cmd_len = 16;
  } else {
    cmd_len = 8;
  }
  if (send_24bit_data) {
    cmd_len = 24;
  }
// status reg is written with 0 to remove protection of memory
#ifdef CHIP_9118
  if (QSPI_DATA_DDR_MODE) {
    qspi_write_to_flash(qspi_reg, QSPI_16BIT_LEN, (write_value << 8), cs_no);
  } else {
    qspi_write_to_flash(qspi_reg, cmd_len, write_value, cs_no);
  }
#else
  qspi_write_to_flash(qspi_reg, cmd_len, write_value, cs_no);
#endif
  DEASSERT_CSN;

  // wait till qspi_status_reg_write is done
  qspi_wait_flash_status_Idle(qspi_reg, spi_config, wr_reg_delay_ms);

  if (spi_config->spi_config_4.dual_flash_mode) {
    qspi_reg->OCTA_SPI_BUS_CONTROLLER2 &= ~BIT(19);
  }
}

/*==============================================*/
/**
 * @fn           uint32_t qspi_flash_reg_read(qspi_reg_t *qspi_reg, uint8_t reg_read_cmd, uint32_t cs_no, spi_config_t *spi_config)
 * @brief        This API used to qspi flash register for read
 * @param[in]    qspi_reg        :   qsi register pointer to the qspi read data
 * @param[in]    reg_read_cmd    :   register read the command
 * @param[in]    cs_no           :   chip select number
 * @param[in]    spi_config      :   pointer to the different configuration 
 * @return       return rd_config if success
 */
uint32_t qspi_flash_reg_read(qspi_reg_t *qspi_reg, uint8_t reg_read_cmd, uint32_t cs_no, spi_config_t *spi_config)
{
  (void)spi_config;
  uint32_t rd_config;
  uint32_t read_len = 1;
#if defined(SLI_SI917)
  rd_config = 0;
#endif
  if (cs_no & BIT(31)) {
    cs_no &= ~BIT(31);
    read_len = 2;
  }

  // config reg read cmd
  qspi_write_to_flash(qspi_reg, QSPI_8BIT_LEN, reg_read_cmd, cs_no);

  // trigger qspi to read one byte from flash
  READ_4M_FLASH(read_len, cs_no, 0);

  // read the config reg
  do {
    // wait till the fifo empty is deasserted
    while (qspi_reg->QSPI_STATUS_REG & QSPI_FIFO_EMPTY_RFIFO_S)
      ;
#if defined(SLI_SI917)
    // This is a bug fix as this func is returning 0th byte as 0 always in readl_len=2
    // This Fix is required for Giga flash only and Not necessary for Macronix
    rd_config <<= 8;
    rd_config |= (uint8_t)qspi_reg->QSPI_MANUAL_RD_WR_DATA_REG;
#endif
#ifdef CHIP_9118
    rd_config = (uint8_t)qspi_reg->QSPI_MANUAL_RD_WR_DATA_REG;
    rd_config <<= 8;
#endif
  } while (--read_len);

  DEASSERT_CSN;

  return rd_config;
}

/*==============================================*/
/**
 * @fn           void qspi_flash_reg_write( qspi_reg_t *qspi_reg,
                          uint32_t reg_write_cmd,
                          uint32_t reg_write_value,
                          uint32_t cs_no,
                          uint32_t wr_reg_delay_ms )
 * @brief        This API used to qspi flash rester for write
 * @param[in]    qspi_reg            :   qsi register pointer to the qspi read data
 * @param[in]    reg_write_cmd       :   register write command 
 * @param[in]    reg_write_value     :   register write the value
 * @param[in]    cs_no               :   chip select number
 * @param[in]    wr_reg_delay_ms     :   register delay in milisecond
 * @return       none
 */

void qspi_flash_reg_write(qspi_reg_t *qspi_reg,
                          uint32_t reg_write_cmd,
                          uint32_t reg_write_value,
                          uint32_t cs_no,
                          uint32_t wr_reg_delay_ms)
{
  // write enable
  qspi_write_to_flash(qspi_reg, QSPI_8BIT_LEN, WREN, cs_no);
  DEASSERT_CSN;

  // write cmd + reg configuration
  qspi_write_to_flash(qspi_reg, QSPI_16BIT_LEN, ((reg_write_cmd << 8) | reg_write_value), cs_no);

  DEASSERT_CSN;
  if (wr_reg_delay_ms) {
    qspi_usleep(wr_reg_delay_ms);
  }
}

/*==============================================*/
/**
 * @fn           void RSI_QSPI_UpdateOperatingMode_and_ResetType(qspi_reg_t *qspi_reg, uint32_t operating_mode) 
 * @brief        This API used to qspi update operating mode and reset type
 * @param[in]    qspi_reg            :   qsi register pointer to the qspi read data
 * @param[in]    operating_mode      :   operating mode
 * @return       none
 */
void RSI_QSPI_UpdateOperatingMode_and_ResetType(qspi_reg_t *qspi_reg, uint32_t operating_mode)
{
  uint32_t bbff_storage2;
  if (qspi_reg == (qspi_reg_t *)QSPI_BASE_ADDRESS) {
    bbff_storage2 = M4_BBFF_STORAGE2;
    bbff_storage2 &= (uint32_t)(~0xff);
    bbff_storage2 |= operating_mode;
    M4_BBFF_STORAGE2 = bbff_storage2;

  } else {
    bbff_storage2 = TA_BBFF_STORAGE2;
    bbff_storage2 &= (uint32_t)(~0xff);
    bbff_storage2 |= operating_mode;
    TA_BBFF_STORAGE2 = bbff_storage2;
  }
}

/*==============================================*/
/**
 * @fn           void RSI_QSPI_ResetFlash(qspi_reg_t *qspi_reg, uint32_t cs_no)  
 * @brief        This API used to qspi reset flash
 * @param[in]    qspi_reg   :   qsi register pointer to the qspi read data
 * @param[in]    cs_no      :   chip select number
 * @return       note
 */
void RSI_QSPI_ResetFlash(qspi_reg_t *qspi_reg, uint32_t cs_no)
{
  uint32_t operating_mode, reset_type, flash_oper_mode;
#ifdef CHIP_9118
  uint32_t ddr_mode;
#endif
  if (qspi_reg == (qspi_reg_t *)QSPI_BASE_ADDRESS) {
    operating_mode = M4_BBFF_STORAGE2;
  } else {
    operating_mode = TA_BBFF_STORAGE2;
  }
  reset_type      = operating_mode & 0xf;
  flash_oper_mode = (operating_mode >> 5) & 0x3;
#ifdef CHIP_9118
  ddr_mode = operating_mode & BIT(7);
#endif
  if (!reset_type) {
    // Giving reset in all possible modes(Single/Quad/Octa) for resetting flash.
    qspi_switch_qspi2(qspi_reg, SINGLE_MODE, cs_no);
    qspi_write_to_flash(qspi_reg, QSPI_8BIT_LEN, 0xFF, cs_no);
    DEASSERT_CSN;
    qspi_switch_qspi2(qspi_reg, QUAD_MODE, cs_no);
    qspi_write_to_flash(qspi_reg, QSPI_8BIT_LEN, 0xFF, cs_no);
    DEASSERT_CSN;
#if 0      

		qspi_switch_qspi2(qspi_reg, OCTA_MODE, cs_no);
		qspi_write_to_flash(qspi_reg, QSPI_8BIT_LEN, 0xFF, cs_no);
		DEASSERT_CSN;
#endif
  } else {
#ifdef CHIP_9118
    if (ddr_mode) {
      qspi_reg->QSPI_MANUAL_CONFIG_2_REG |= QSPI_DDR_CLK_EN;
      qspi_reg->QSPI_MANUAL_CONFIG_2_REG |= QSPI_MANUAL_DDR_PHASSE;
    }
#endif
    qspi_switch_qspi2(qspi_reg, (flash_oper_mode & 0x3), cs_no);
    if (reset_type == 1) {
      // Switch off flash LDO
    } else if (reset_type == 2) { // Godavari mode of resetting
      qspi_write_to_flash(qspi_reg, QSPI_8BIT_LEN, 0xFF, cs_no);
      DEASSERT_CSN;
      qspi_write_to_flash(qspi_reg, QSPI_16BIT_LEN, 0xFFFF, cs_no);
      DEASSERT_CSN;
    } else if (reset_type == 3) { // giga device flash
      qspi_write_to_flash(qspi_reg, QSPI_8BIT_LEN, 0xFF, cs_no);
      DEASSERT_CSN;
    } else if (reset_type == 4) { // macronix quad
      if (flash_oper_mode != SINGLE_MODE) {
        qspi_write_to_flash(qspi_reg, QSPI_8BIT_LEN, 0xF5, cs_no);
        DEASSERT_CSN;
        if (operating_mode & BIT(4)) {
          qspi_usleep(150);
        } else {
          qspi_usleep(50);
        }
      }
    } else if (reset_type == 5) { // Adesto_flash
      qspi_write_to_flash(qspi_reg, QSPI_8BIT_LEN, WREN, cs_no);
      DEASSERT_CSN;
      qspi_write_to_flash(qspi_reg, QSPI_8BIT_LEN, 0xFF, cs_no);
      DEASSERT_CSN;
    } else if (reset_type == 6) { // mx octa flash in single
      if (flash_oper_mode != SINGLE_MODE) {
        qspi_write_to_flash(qspi_reg, QSPI_8BIT_LEN, 0x66, cs_no);
        DEASSERT_CSN;
        qspi_write_to_flash(qspi_reg, QSPI_8BIT_LEN, 0x99, cs_no);
        DEASSERT_CSN;
        if (operating_mode & BIT(4)) {
          qspi_usleep(150);
        } else {
          qspi_usleep(50);
        }
      }
    } else if (reset_type == 7) { // mx octa flash in octa. it can be used for ddr mode also
      // This is intended for Macronix OCTA flash if already flash is in single bit mode not giving reset.
      if (flash_oper_mode != SINGLE_MODE) {
        qspi_write_to_flash(qspi_reg, QSPI_16BIT_LEN, 0x6699, cs_no);
        DEASSERT_CSN;
        qspi_write_to_flash(qspi_reg, QSPI_16BIT_LEN, 0x9966, cs_no);
        DEASSERT_CSN;
        if (operating_mode & BIT(4)) {
          qspi_usleep(150);
        } else {
          qspi_usleep(50);
        }
      }
    } else if (reset_type == 8) {
      egpio_set_pin_mux(EGPIO, 0, 13, 0);
      egpio_set_pin(EGPIO, 0, 13, 0);
      egpio_set_dir(EGPIO, 0, 13, 0);
      if (operating_mode & BIT(4)) {
        qspi_usleep(150);
      } else {
        qspi_usleep(50);
      }
      egpio_set_pin(EGPIO, 0, 13, 1);
    } else if (reset_type == 9) {
      egpio_set_pin_mux(EGPIO, 0, 14, 0);
      egpio_set_pin(EGPIO, 0, 14, 0);
      egpio_set_dir(EGPIO, 0, 14, 0);
      if (operating_mode & BIT(4)) {
        qspi_usleep(150);
      } else {
        qspi_usleep(50);
      }
      egpio_set_pin(EGPIO, 0, 14, 1);
    } else if (reset_type == 10) {
      qspi_write_to_flash(qspi_reg, QSPI_8BIT_LEN, 0xFF, cs_no);
      DEASSERT_CSN;
      qspi_write_to_flash(qspi_reg, QSPI_8BIT_LEN, 0xF5, cs_no);
      DEASSERT_CSN;
    } else if (reset_type == 11) {

      // DWORD0
      // No of commands[1:0],
      // cmd1_len[3:2],
      // cmd2_len[5:4],
      // Reserved[7:6],
      // delay[13:8],
      // Reserved[15:14]
      // reserved cmd1[15:0]
      // DWORD1
      // cmd2[15:0]
      uint16_t flash_reset_info[3];
      uint32_t delay, no_of_commands, cmd_len, command, inx, src;

      initialise_m4_efuse_in_io_mode();

      src = M4_EFUSE_IO_BASE_ADDR;

      rsi_cmemcpy((uint8_t *)flash_reset_info, (uint8_t *)(src), 6);

      no_of_commands = flash_reset_info[0] & 0x3;
      inx            = 0;

      while (inx < no_of_commands) {
        // 0 : 32-bit, 1 : 8-bit, 2 : 16-bit
        cmd_len = (flash_reset_info[0] >> ((inx + 1) * 2)) & 0x3;
        command = flash_reset_info[inx + 1];

        qspi_write_to_flash(qspi_reg, (8 * cmd_len), command, cs_no);
        DEASSERT_CSN;
        inx++;
      }

      // If Bit(13) is set delay is ms with 1ms ganularity else delay is in us with 16us granularity.
      if (flash_reset_info[0] & BIT(13)) {
        delay = (((flash_reset_info[0] >> 8) & 0x1f) * 1000);
      } else {
        delay = (((flash_reset_info[0] >> 8) & 0x1f) * 16);
      }

      if (delay) {
        qspi_usleep(delay);
      }
    }
#ifdef CHIP_9118
    if (ddr_mode) {
      qspi_reg->QSPI_MANUAL_CONFIG_2_REG &= ~QSPI_DDR_CLK_EN;
      qspi_reg->QSPI_MANUAL_CONFIG_2_REG &= ~QSPI_MANUAL_DDR_PHASSE;
    }
#endif
  }
  qspi_switch_qspi2(qspi_reg, SINGLE_MODE, cs_no);
}

/*==============================================*/
/**
 * @fn           void qspi_set_flash_mode(qspi_reg_t *qspi_reg,
                             uint32_t data_mode,
                             uint32_t cs_no,
                             uint32_t ddr_mode_en,
                             uint32_t flash_type )
 * @brief        This API used to qspi set the flash mode
 * @param[in]    qspi_reg        :   qsi register pointer to the qspi read data 
 * @param[in]    data_mode       :   data mode
 * @param[in]    cs_no			 :   chip select number
 * @param[in]    ddr_mode_en     :   mode enable
 * @param[in]    flash_type      :   flash type
 * @return       none
 */
void qspi_set_flash_mode(qspi_reg_t *qspi_reg,
                         uint32_t data_mode,
                         uint32_t cs_no,
                         uint32_t ddr_mode_en,
                         uint32_t flash_type)
{
  (void)ddr_mode_en;
  uint32_t enable_bus_mode         = 0;
  volatile uint32_t reset_bus_mode = 0;
  // FIXME AS SST26VF016 supports only SINGLE and QUAD MODE
  // support for Dual mode is not added
  // Add it if flash supports

  if (flash_type == MX_QUAD_FLASH) {
    enable_bus_mode = 0x35;
    reset_bus_mode  = 0xF5;
  } else if (flash_type == MX_OCTA_FLASH) {
    reset_bus_mode = 0x9966;
  } else {
    enable_bus_mode = EQIO;
    reset_bus_mode  = RSTQIO;
  }
  if ((data_mode == QUAD_MODE)) {
    qspi_write_to_flash(qspi_reg, QSPI_8BIT_LEN, enable_bus_mode, cs_no);

    DEASSERT_CSN;
    qspi_switch_qspi2(qspi_reg, data_mode, cs_no);

  } else {

#if 1
    RSI_QSPI_ResetFlash(qspi_reg, cs_no);
  }
#else

    if (flash_type == ADESTO_OCTA_FLASH) {
      qspi_write_to_flash(qspi_reg, QSPI_8BIT_LEN, WREN, cs_no);
      DEASSERT_CSN;
    }
    if (flash_type == MX_OCTA_FLASH) {
      qspi_switch_qspi2(qspi_reg, OCTA_MODE, cs_no);
      qspi_write_to_flash(qspi_reg, QSPI_16BIT_LEN, ((reset_bus_mode << 8) | (reset_bus_mode >> 8)), cs_no);
      DEASSERT_CSN;
      qspi_write_to_flash(qspi_reg, QSPI_16BIT_LEN, reset_bus_mode, cs_no);
      DEASSERT_CSN;
      qspi_switch_qspi2(qspi_reg, SINGLE_MODE, cs_no);
      qspi_write_to_flash(qspi_reg, QSPI_8BIT_LEN, reset_bus_mode, cs_no);
      DEASSERT_CSN;
      qspi_write_to_flash(qspi_reg, QSPI_8BIT_LEN, (reset_bus_mode >> 8), cs_no);
      DEASSERT_CSN;
    } else {
      qspi_switch_qspi2(qspi_reg, SINGLE_MODE, cs_no);
      qspi_write_to_flash(qspi_reg, QSPI_8BIT_LEN, reset_bus_mode, cs_no);
      DEASSERT_CSN;
      qspi_switch_qspi2(qspi_reg, QUAD_MODE, cs_no);
      qspi_write_to_flash(qspi_reg, QSPI_8BIT_LEN, reset_bus_mode, cs_no);
      DEASSERT_CSN;
      qspi_switch_qspi2(qspi_reg, OCTA_MODE, cs_no);
      qspi_write_to_flash(qspi_reg, QSPI_8BIT_LEN, reset_bus_mode, cs_no);
      DEASSERT_CSN;
    }
    qspi_switch_qspi2(qspi_reg, SINGLE_MODE, cs_no);
  }
#endif
  (void)reset_bus_mode;
}

/*==============================================*/
/**
 * @fn           void qspi_status_control_reg_write( spi_config_t *spi_config,
                                   qspi_reg_t *qspi_reg,
                                   uint16_t write_command,
                                   uint32_t addr,
                                   uint16_t write_value,
                                   uint32_t cs_no,
                                   uint32_t wr_reg_delay_ms )
 * @brief        This API used to qspi control the status register for write
 * @param[in]    spi_config      :   pointer to the SPI configuration 
 * @param[in]    qspi_reg        :   qsi register pointer to the qspi read data
 * @param[in]    write_command   :   write the command
 * @param[in]    addr            :   address of status control
 * @param[in]    write_value     :   write the value
 * @param[in]    cs_no           :   chip select number
 * @param[in]    wr_reg_delay_ms :   register delay in milisecond 
 * @return       none
 */
void qspi_status_control_reg_write(spi_config_t *spi_config,
                                   qspi_reg_t *qspi_reg,
                                   uint16_t write_command,
                                   uint32_t addr,
                                   uint16_t write_value,
                                   uint32_t cs_no,
                                   uint32_t wr_reg_delay_ms)
{
  // write enable
  qspi_write_to_flash(qspi_reg, QSPI_8BIT_LEN, WREN, cs_no);
  DEASSERT_CSN;
  // write cmd + reg configuration
  qspi_write_to_flash(qspi_reg, QSPI_8BIT_LEN, write_command, cs_no);

  if (spi_config->spi_config_1.flash_type == MX_OCTA_FLASH) {
    qspi_write_to_flash(qspi_reg, QSPI_32BIT_ADDR, addr, cs_no);
  } else {
    qspi_write_to_flash(qspi_reg, QSPI_8BIT_LEN, addr, cs_no);
  }
  qspi_write_to_flash(qspi_reg, QSPI_8BIT_LEN, write_value, cs_no);
  DEASSERT_CSN;
  if (spi_config->spi_config_1.flash_type == ADESTO_OCTA_FLASH) {
    qspi_wait_flash_status_Idle(qspi_reg, spi_config, 0);
  }

  if (wr_reg_delay_ms) {
    qspi_usleep(wr_reg_delay_ms);
  }
}

/*==============================================*/
/**
 * @fn           void qspi_write_block_protect( qspi_reg_t *qspi_reg,
                              uint32_t protect,
                              uint32_t cs_no,
                              uint32_t num_prot_bytes,
                              uint32_t wr_reg_delay_ms )
 * @brief        This API used to qspi write the block protection
 * @param[in]    qspi_reg        :   qsi register pointer to the qspi read data
 * @param[in]    protect         :   protect
 * @param[in]    cs_no           :   chip select number
 * @param[in]    num_prot_bytes  :   number of protection bytes
 * @return       none
 */
void qspi_write_block_protect(qspi_reg_t *qspi_reg,
                              uint32_t protect,
                              uint32_t cs_no,
                              uint32_t num_prot_bytes,
                              uint32_t wr_reg_delay_ms)
{
  uint32_t i;
  // write enable
  qspi_write_to_flash(qspi_reg, QSPI_8BIT_LEN, WREN, cs_no);
  DEASSERT_CSN;
  // RSI_QSPI_WriteBlockProtection_reg_cmd
  qspi_write_to_flash(qspi_reg, QSPI_8BIT_LEN, WBPR, cs_no);
  for (i = 0; i < num_prot_bytes; i++) {
    // Writing all Zeros to BPR to disable memory protection
    // or all Ones to enable memory protection
    qspi_write_to_flash(qspi_reg, QSPI_8BIT_LEN, protect, cs_no);
  }
  DEASSERT_CSN;

  // if required (non-zero) wait for the delay time
  if (wr_reg_delay_ms) {
    qspi_usleep(wr_reg_delay_ms);
  }
}

/*==============================================*/
/**
 * @fn           void qspi_config_qflash4_read(qspi_reg_t *qspi_reg, spi_config_t *spi_config, uint32_t addr)  
 * @brief        This API used to qspi configuration flash for read
 * @param[in]    qspi_reg        :   qsi register pointer to the qspi read data
 * @param[in]    spi_config      :   pointer to the SPI configuration
 * @param[in]    addr            :   address of configuration flash 
 * @return       none
 */
void qspi_config_qflash4_read(qspi_reg_t *qspi_reg, spi_config_t *spi_config, uint32_t addr)
{
  uint32_t dummy_cnt, tmp_dummy;
  volatile uint32_t junk;
  uint32_t prev_bus_mode;
  uint32_t cs_no;

  cs_no = spi_config->spi_config_2.cs_no;
  DEASSERT_CSN;

  // switch qspi to inst mode
  qspi_switch_qspi2(qspi_reg, spi_config->spi_config_1.inst_mode, cs_no);
  prev_bus_mode = spi_config->spi_config_1.inst_mode;

  // incase of 9bit addressing include the A8 bit in read_cmd
  if (spi_config->spi_config_3._16bit_cmd_valid) {
    qspi_write_to_flash(
      qspi_reg,
      QSPI_16BIT_LEN,
      (((uint32_t)(spi_config->spi_config_1.read_cmd << 8)) | spi_config->spi_config_3._16bit_rd_cmd_msb),
      cs_no);
  } else {
    qspi_write_to_flash(qspi_reg, QSPI_8BIT_LEN, (spi_config->spi_config_1.read_cmd | A8_BIT), cs_no);
  }

  // if addr mode is not same as prev bus mode, switch qspi to addr mode
  if (spi_config->spi_config_1.addr_mode != prev_bus_mode) {
    qspi_switch_qspi2(qspi_reg, spi_config->spi_config_1.addr_mode, cs_no);
    prev_bus_mode = spi_config->spi_config_1.addr_mode;
  }
  // write addr to flash

  qspi_write_to_flash(qspi_reg, ADDR_LEN, addr, cs_no);

  // if extra byte is enabled send it
  if (spi_config->spi_config_1.extra_byte_en) {

    if (spi_config->spi_config_1.extra_byte_mode != prev_bus_mode) {
      qspi_switch_qspi2(qspi_reg, spi_config->spi_config_1.extra_byte_mode, cs_no);
      prev_bus_mode = spi_config->spi_config_1.extra_byte_mode;
    }

    qspi_write_to_flash(qspi_reg, QSPI_8BIT_LEN, 0x00, cs_no);
  }

  // if no_of_dummy_bytes is non-zero take care
  if ((spi_config->spi_config_1.no_of_dummy_bytes) || (spi_config->spi_config_4.no_of_ms_dummy_bytes)) {
    dummy_cnt = spi_config->spi_config_1.no_of_dummy_bytes;
    dummy_cnt |= (spi_config->spi_config_4.no_of_ms_dummy_bytes << 4);

    if (spi_config->spi_config_1.dummy_mode != prev_bus_mode) {
      qspi_switch_qspi2(qspi_reg, spi_config->spi_config_1.dummy_mode, cs_no);
      prev_bus_mode = spi_config->spi_config_1.dummy_mode;
    }

    // dummy read
    if (spi_config->spi_config_1.dummy_W_or_R == DUMMY_READS) {
      // trigger qspi to read 1 byte from flash
#ifdef CHIP_9118
      if (spi_config->spi_config_3.ddr_mode_en) {
        READ_4M_FLASH(dummy_cnt, cs_no, _16BIT);
      } else {
        READ_4M_FLASH(dummy_cnt, cs_no, 0);
      }
#else
      READ_4M_FLASH(dummy_cnt, cs_no, 0);
#endif
      do {
        while (qspi_reg->QSPI_STATUS_REG & QSPI_FIFO_EMPTY_RFIFO_S)
          ;
#ifdef CHIP_9118
        if (spi_config->spi_config_3.ddr_mode_en) {
          junk = (uint16_t)qspi_reg->QSPI_MANUAL_RD_WR_DATA_REG;
        } else {
          junk = (uint8_t)qspi_reg->QSPI_MANUAL_RD_WR_DATA_REG;
        }
#else
        junk = (uint8_t)qspi_reg->QSPI_MANUAL_RD_WR_DATA_REG;
#endif
        dummy_cnt--;
        if (dummy_cnt == 0) {
          break;
        }
      } while (1);
    } else {
      do {

        tmp_dummy = (dummy_cnt & 0x3) ? (dummy_cnt & 3) : 4;

        qspi_write_to_flash(qspi_reg, (QSPI_8BIT_LEN * tmp_dummy), 0x00, cs_no);
        dummy_cnt -= tmp_dummy;

      } while (dummy_cnt);
    }
  }

  if (spi_config->spi_config_2.dummy_cycles_for_controller) {

    tmp_dummy = spi_config->spi_config_2.dummy_cycles_for_controller;
    // Switching to OCTA mode to wait in terms of dummy cycles , since 1 bytes is read in cycle in octa mode
    qspi_switch_qspi2(qspi_reg, OCTA_MODE, cs_no);
#ifdef CHIP_9118
    if (spi_config->spi_config_3.ddr_mode_en) {
      READ_4M_FLASH(spi_config->spi_config_2.dummy_cycles_for_controller, cs_no, _16BIT);
    } else {
      READ_4M_FLASH(spi_config->spi_config_2.dummy_cycles_for_controller, cs_no, 0);
    }
#else
    READ_4M_FLASH(spi_config->spi_config_2.dummy_cycles_for_controller, cs_no, 0);
#endif

    do {
      while (qspi_reg->QSPI_STATUS_REG & QSPI_FIFO_EMPTY_RFIFO_S)
        ;
#ifdef CHIP_9118
      if (spi_config->spi_config_3.ddr_mode_en) {
        junk = (uint16_t)qspi_reg->QSPI_MANUAL_RD_WR_DATA_REG;
      } else {
        junk = (uint8_t)qspi_reg->QSPI_MANUAL_RD_WR_DATA_REG;
      }
#else
      junk = (uint8_t)qspi_reg->QSPI_MANUAL_RD_WR_DATA_REG;
#endif

    } while (--tmp_dummy);

    qspi_switch_qspi2(qspi_reg, spi_config->spi_config_1.dummy_mode, cs_no);
    // prev_bus_mode = spi_config->spi_config_1.dummy_mode;
  }

  if (spi_config->spi_config_1.data_mode != prev_bus_mode) {
    qspi_switch_qspi2(qspi_reg, spi_config->spi_config_1.data_mode, cs_no);
  }

  //XXX CSN must not be deassert here...
  (void)junk;
}

/*==============================================*/
/**
 * @fn           void RSI_QSPI_GPDMA_Init(uint32_t hsize, uint32_t ch_no, uint32_t mode)     
 * @brief        This API used to qspi general purpose DMA
 * @param[in]    hsize        :  size of GPDMA channels
 * @param[in]    ch_no        :  channel number
 * @param[in]    mode         :  mode
 * @return       none
 */
void RSI_QSPI_GPDMA_Init(uint32_t hsize, uint32_t ch_no, uint32_t mode)
{
  GPDMA_C->CHANNEL_CONFIG[ch_no].FIFO_CONFIG_REGS_b.FIFO_SIZE      = 8;
  GPDMA_C->CHANNEL_CONFIG[ch_no].FIFO_CONFIG_REGS_b.FIFO_STRT_ADDR = (unsigned int)((8 * ch_no) & 0x3F);

  // Default values
  GPDMA_C->CHANNEL_CONFIG[ch_no].PRIORITY_CHNL_REGS_b.PRIORITY_CH      = 0;
  GPDMA_C->CHANNEL_CONFIG[ch_no].CHANNEL_CTRL_REG_CHNL_b.DMA_FLOW_CTRL = DMA_FLW_CTRL;

  if (mode) {
    GPDMA_C->CHANNEL_CONFIG[ch_no].CHANNEL_CTRL_REG_CHNL_b.TRNS_TYPE            = MEMORY_MEMORY;
    GPDMA_C->CHANNEL_CONFIG[ch_no].MISC_CHANNEL_CTRL_REG_CHNL_b.AHB_BURST_SIZE  = 0x3;
    GPDMA_C->CHANNEL_CONFIG[ch_no].MISC_CHANNEL_CTRL_REG_CHNL_b.SRC_DATA_BURST  = 0x10;
    GPDMA_C->CHANNEL_CONFIG[ch_no].MISC_CHANNEL_CTRL_REG_CHNL_b.DEST_DATA_BURST = 0x10;
    GPDMA_C->CHANNEL_CONFIG[ch_no].CHANNEL_CTRL_REG_CHNL_b.SRC_FIFO_MODE        = 0;
  } else {
    GPDMA_C->CHANNEL_CONFIG[ch_no].CHANNEL_CTRL_REG_CHNL_b.TRNS_TYPE            = PERIPHERAL_MEMORY;
    GPDMA_C->CHANNEL_CONFIG[ch_no].MISC_CHANNEL_CTRL_REG_CHNL_b.AHB_BURST_SIZE  = 0x2;
    GPDMA_C->CHANNEL_CONFIG[ch_no].MISC_CHANNEL_CTRL_REG_CHNL_b.SRC_DATA_BURST  = 0x7;
    GPDMA_C->CHANNEL_CONFIG[ch_no].MISC_CHANNEL_CTRL_REG_CHNL_b.DEST_DATA_BURST = 0x7;
    GPDMA_C->CHANNEL_CONFIG[ch_no].CHANNEL_CTRL_REG_CHNL_b.SRC_FIFO_MODE        = 1;
  }

  // Configure Misc Config reg
  GPDMA_C->CHANNEL_CONFIG[ch_no].CHANNEL_CTRL_REG_CHNL_b.SRC_DATA_WIDTH =
    (unsigned int)(((hsize == 3) ? 2 : hsize) & 0x03);
  GPDMA_C->CHANNEL_CONFIG[ch_no].CHANNEL_CTRL_REG_CHNL_b.DEST_DATA_WIDTH =
    (unsigned int)(((hsize == 3) ? 2 : hsize) & 0x03);
  //QSPI Peripheral ID is 16
  GPDMA_C->CHANNEL_CONFIG[ch_no].MISC_CHANNEL_CTRL_REG_CHNL_b.SRC_CHNL_ID = 16;
}

/*==============================================*/
/**
 * @fn           void RSI_QSPI_GPDMA_ReadFromFifo(uint32_t src, uint32_t dst, uint32_t len, uint32_t ch_no)
 * @brief        This API used to qspi General Purpose read by FIFO
 * @param[in]    src        :   source address of register channel 
 * @param[in]    dst        :   destination address of register channel 
 * @param[in]    len        :   length of register channel
 * @param[in]    ch_no      :   channel number
 * @return       none
 */
void RSI_QSPI_GPDMA_ReadFromFifo(uint32_t src, uint32_t dst, uint32_t len, uint32_t ch_no)
{
  // Set src and dest in given channel
  GPDMA_C->CHANNEL_CONFIG[ch_no].SRC_ADDR_REG_CHNL                    = src;
  GPDMA_C->CHANNEL_CONFIG[ch_no].DEST_ADDR_REG_CHNL                   = dst;
  GPDMA_C->CHANNEL_CONFIG[ch_no].CHANNEL_CTRL_REG_CHNL_b.DMA_BLK_SIZE = (unsigned int)(len & 0x0FFF);

  // Trigger DMA
  GPDMA_G->GLOBAL.DMA_CHNL_ENABLE_REG = SET_BIT(ch_no);

  // Wait for DMA done
  while ((GPDMA_G->GLOBAL.DMA_CHNL_ENABLE_REG & SET_BIT(ch_no)))
    ;
}

/*==============================================*/
/**
 * @fn           void RSI_QSPI_ReadFromFifo(uint32_t udma_read, void *udmaHandle, void *gpdmaHandle, uint32_t ch_no)
 * @brief        This API used to qspi read by FIFO
 * @param[in]    udma_read      :   micro DMA read 
 * @param[in]    udmaHandle     :   pointer to DMA controller
 * @param[in]    gpdmaHandle    :   pointer to general purpose DMA controller
 * @param[in]    ch_no          :   channel number
 * @return       none
 */
void RSI_QSPI_ReadFromFifo(uint32_t udma_read, void *udmaHandle, void *gpdmaHandle, uint32_t ch_no)
{
  if (udma_read) {
    RSI_UDMA_ChannelEnable(udmaHandle, (uint8_t)ch_no);
    // Enable DMA controller
    RSI_UDMA_UDMAEnable(udmaHandle);

    // Software trigger for transfer
    RSI_UDMA_ChannelSoftwareTrigger(udmaHandle, (uint8_t)ch_no);
    while (!(RSI_UDMA_ChannelIsEnabled(udmaHandle, (uint8_t)ch_no)))
      ;

  } else {

    gpdma_dma_channel_trigger(gpdmaHandle, (uint8_t)ch_no);
    while ((gpdma_channel_is_enabled(gpdmaHandle, (uint8_t)ch_no)))
      ;
  }
}
#if defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wcast-align"
#endif
/*==============================================*/
/**
 * @fn           void qspi_manual_read( qspi_reg_t *qspi_reg,
                      spi_config_t *spi_config,
                      uint32_t addr,
                      uint8_t *data,
                      uint32_t hsize,
                      uint32_t len_in_bytes,
                      uint32_t dma_flags,
                      void *udmaHandle,
                      void *gpdmaHandle )
 * @brief        This API used to qspi manual read
 * @param[in]    qspi_reg        :   qsi register pointer to the qspi read data
 * @param[in]    spi_config      :   pointer to the SPI configuration
 * @param[in]    addr            :   address
 * @param[in]    data            :   pointer to manual read
 * @param[in]    len_in_bytes    :   length in bytes
 * @param[in]    dma_flags       :   DMA flags for manual read
 * @param[in]    udmaHandle      :   pointer to DMA controller
 * @param[in]    gpdmaHandle     :   pointer to general purpose DMA controller
 * @return       none
 */
void qspi_manual_read(qspi_reg_t *qspi_reg,
                      spi_config_t *spi_config,
                      uint32_t addr,
                      uint8_t *data,
                      uint32_t hsize,
                      uint32_t len_in_bytes,
                      uint32_t dma_flags,
                      void *udmaHandle,
                      void *gpdmaHandle)
{
  uint32_t loop_count;
  uint32_t split_length;
  uint32_t once;
  uint32_t qspi_manual_config_reg;
  uint32_t prev_state     = 0;
  uint32_t qspi_data_mode = 0;
  uint16_t *data_16bit;
  uint32_t *data_32bit;
  uint32_t ch_no;

  if (spi_config->spi_config_2.addr_width == 4) {
    // Shifting more than 31 bit won't work in our processor. So not using below logic.
    if (hsize & BIT(31)) {
    } else {
      addr &= 0x3FFFFFF;
    }
  } else {
    addr &= (uint32_t)((1 << (spi_config->spi_config_2.addr_width * 8)) - 1);
  }
  hsize &= ~BIT(31);

  // Check if already auto mode enabled
  if (qspi_reg->QSPI_BUS_MODE_REG & AUTO_MODE) {
    qspi_reg->QSPI_BUS_MODE_REG &= ~AUTO_MODE;
    while (qspi_reg->QSPI_STATUS_REG & AUTO_MODE_ENABLED)
      ;
    prev_state = 1;
  }
  data_16bit = (uint16_t *)data;
  data_32bit = (uint32_t *)data;
  // split_length
  if (dma_flags & DEFAULT_DESC_MODE) {
    split_length = (4 * 1024) - 4;
  } else {
    split_length = (32 * 1024) - 4;
  }
  // variable to keep state of configurations
  once                   = 0;
  qspi_manual_config_reg = 0;

  if (!(qspi_reg->QSPI_STATUS_REG & HW_CTRLD_QSPI_MODE_CTRL_SCLK)) {
    qspi_reg->QSPI_BUS_MODE_REG &= ~AUTO_MODE;
    while (!(qspi_reg->QSPI_STATUS_REG & AUTO_MODE_FSM_IDLE_SCLK)) {
      // wait till auto mode becomes idle
    }
  }
  if (spi_config->spi_config_2.swap_en == NO_SWAP) {
    // swap en are at 7:4 for csn 3:0
    qspi_reg->QSPI_MANUAL_CONFIG_2_REG &= (uint16_t)(~(SWAP << (4 + spi_config->spi_config_2.cs_no)));
  }
  if (hsize) {
    // pad length for making align
    len_in_bytes = ((len_in_bytes + hsize) & (~hsize));
  }

  ch_no = (dma_flags & 0xFF);

  if (dma_flags & DEFAULT_DESC_MODE) {
    RSI_QSPI_GPDMA_Init(hsize, ch_no, 0);
  }

  while (len_in_bytes) {
    if (!once) {
      if (spi_config->spi_config_4.dual_flash_mode) {
        qspi_reg->OCTA_SPI_BUS_CONTROLLER2 |= BIT(19);
      }
      qspi_config_qflash4_read(qspi_reg, spi_config, addr);
      if (spi_config->spi_config_4.dual_flash_mode) {
        qspi_reg->OCTA_SPI_BUS_CONTROLLER2 &= ~BIT(19);
        qspi_data_mode = spi_config->spi_config_1.data_mode;
        qspi_switch_qspi2(qspi_reg, OCTA_MODE, spi_config->spi_config_2.cs_no);
      }
      // configure qspi for read
      qspi_manual_config_reg = (qspi_reg->QSPI_MANUAL_CONFIG_REG & ~0xF8387FFF);
      qspi_manual_config_reg |= (spi_config->spi_config_2.cs_no << 13) | (hsize << 19) | TAKE_LEN_FRM_REG;
      // if not continuous repeat this block
      once = spi_config->spi_config_1.continuous;
    }
    if (spi_config->spi_config_1.continuous == DIS_CONTINUOUS) {
      loop_count = 1 + hsize;
      addr += loop_count;
    } else {

      loop_count = XMIN(len_in_bytes, split_length);
      addr += loop_count;
    }
    // reducing total length by loop_count
    len_in_bytes -= loop_count;
    // trigger qspi to read loop_count bytes from flash
    qspi_reg->QSPI_MANUAL_CONFIG_REG = qspi_manual_config_reg | READ_TRIGGER | ((loop_count & 0x3FF) << 3)
                                       | (((loop_count >> 10) & 0x1F) << 27);

    // check whether dma mode is configured
    if (spi_config->spi_config_2.dma_mode) {
      if (dma_flags & DEFAULT_DESC_MODE) {
        // read using dma
        RSI_QSPI_GPDMA_ReadFromFifo((uint32_t) & (qspi_reg->QSPI_MANUAL_RD_WR_DATA_REG),
                                    (uint32_t)data,
                                    loop_count,
                                    ch_no);
        data += loop_count;
        // continue is used so that PC doesn't go inside the next loop
        // and starts from the beginning of the current loop
        continue;
      } else {
        RSI_QSPI_ReadFromFifo((dma_flags & USE_UDMA_MODE), udmaHandle, gpdmaHandle, ch_no);
        continue;
      }
    }
    // IO reads
    if (hsize == _32BIT) {
      do {
        while (qspi_reg->QSPI_STATUS_REG & QSPI_FIFO_EMPTY_RFIFO_S) {
          // wait till the fifo empty is deasserted
        }
        *(data_32bit) = (uint32_t)qspi_reg->QSPI_MANUAL_RD_WR_DATA_REG;
        data_32bit++;
        loop_count -= 4;
      } while (loop_count);
    } else if (hsize == _16BIT) {
      do {
        while (qspi_reg->QSPI_STATUS_REG & QSPI_FIFO_EMPTY_RFIFO_S) {
          // wait till the fifo empty is deasserted
        }
        *(data_16bit) = (uint16_t)qspi_reg->QSPI_MANUAL_RD_WR_DATA_REG;
        data_16bit++;
        loop_count -= 2;
      } while (loop_count);
    } else if (hsize == _8BIT) {
      do {
        while (qspi_reg->QSPI_STATUS_REG & QSPI_FIFO_EMPTY_RFIFO_S) {
          // wait till the fifo empty is deasserted
        }
        *(data) = (uint8_t)qspi_reg->QSPI_MANUAL_RD_WR_DATA_REG;
        data++;
        loop_count -= 1;
      } while (loop_count);
    }
  }
  DEASSERT_CSN;
  if (spi_config->spi_config_4.dual_flash_mode) {
    spi_config->spi_config_3.wr_data_mode = (unsigned int)(qspi_data_mode & 0x03);
    qspi_switch_qspi2(qspi_reg, spi_config->spi_config_3.wr_data_mode, spi_config->spi_config_2.cs_no);
  }
  if (prev_state == 1) {
    qspi_reg->QSPI_BUS_MODE_REG |= AUTO_MODE;
    while (!(qspi_reg->QSPI_STATUS_REG & AUTO_MODE_ENABLED))
      ;
  }
}

#if 1 // ef SST_QFLASH
#if 0

/*==============================================*/
/**
 * @fn           void RSI_QSPI_WrapInit(qspi_reg_t *qspi_reg,uint32_t wrap_len_in_bytes,uint32_t cs_no)
 * @brief        This API used to qspi flash to initialise wrap
 * @param[in]    qspi_reg          :   qsi register pointer to the qspi read data
 * @param[in]    wrap_len_in_bytes :   wrap length in bytes 
 * @param[in]    cs_no             : chip select number
 * @return       none
 */
void RSI_QSPI_WrapInit( qspi_reg_t *qspi_reg, uint32_t wrap_len_in_bytes, uint32_t cs_no)
{
	// send SET_BURST cmd to flash to initialize wrap
	qspi_write_to_flash(qspi_reg, QSPI_16BIT_LEN,
			((SET_BURST << 8) | wrap_len_in_bytes), cs_no);
	DEASSERT_CSN;
}
#endif
#endif

/*==============================================*/
/**
 * @fn           void RSI_QSPI_AutoModeEn(qspi_reg_t *qspi_reg)  
 * @brief        This API used to qspi enable the auto mode
 * @param[in]    qspi_reg        :   qsi register pointer to the qspi enable auto mode 
 * @return       none
 */
void RSI_QSPI_AutoModeEn(qspi_reg_t *qspi_reg)
{
  if (!(qspi_reg->QSPI_STATUS_REG & HW_CTRLD_QSPI_MODE_CTRL_SCLK)) {
    while (!(qspi_reg->QSPI_MANUAL_CONFIG_REG & CSN_ACTIVE)) {
      //wait till manual becomes idle
    }
    qspi_reg->QSPI_BUS_MODE_REG |= AUTO_MODE;
  }
}

/*==============================================*/
/**
 * @fn           void qspi_auto_init(qspi_reg_t *qspi_reg, spi_config_t *spi_config)  
 * @brief        This API used to qspi function initializes auto mode
 * @param[in]    qspi_reg        :   qsi register pointer to the qspi read data
 * @param[in]    spi_config      :   pointer to the SPI configuration 
 * @return       none
 */
void qspi_auto_init(qspi_reg_t *qspi_reg, spi_config_t *spi_config)
{
  uint32_t offset;
  uint32_t *auto_1_ptr;
  uint32_t *auto_2_ptr;
  uint32_t *auto_3_ptr;
  uint32_t auto_3_data;
  uint32_t dummy_count = 0;
  // enable or disable prefetch
  if (spi_config->spi_config_1.prefetch_en) {
    qspi_reg->QSPI_BUS_MODE_REG |= QSPI_PREFETCH_EN;
  } else {
    qspi_reg->QSPI_BUS_MODE_REG &= ~QSPI_PREFETCH_EN;
  }

  if (spi_config->spi_config_2.addr_width == _32BIT_ADDR) {
    qspi_reg->QSPI_AUTO_CONFIG3 |= QSPI_ADR_SIZE_32BIT_AUTO_MODE;
  } else {
    qspi_reg->QSPI_AUTO_CONFIG3 &= ~QSPI_ADR_SIZE_32BIT_AUTO_MODE;
  }

  if (spi_config->spi_config_2.cs_no) {
    auto_3_ptr = ((uint32_t *)&qspi_reg->QSPI_AUTO_CONFIG3_CSN1);
  } else {
    auto_3_ptr = ((uint32_t *)&qspi_reg->QSPI_AUTO_CONFIG3);
  }
  auto_3_data = *auto_3_ptr;

  if (spi_config->spi_config_3._16bit_cmd_valid) {
    auto_3_data |= QSPI_CMD_SIZE_16BIT_CSN0 | (spi_config->spi_config_1.read_cmd << QSPI_RD_INST_CSN0_MSB);
  } else {
    auto_3_data &= ~QSPI_CMD_SIZE_16BIT_CSN0;
  }
#ifdef CHIP_9118
  if (spi_config->spi_config_4.ddr_data_mode) {
    auto_3_data |= (1 << DDR_DATA);
  } else {
    auto_3_data &= ~(1 << DDR_DATA);
  }
  if (spi_config->spi_config_4.ddr_addr_mode) {
    auto_3_data |= (1 << DDR_ADDR);
  } else {
    auto_3_data &= ~(1 << DDR_ADDR);
  }
  if (spi_config->spi_config_4.ddr_inst_mode) {
    auto_3_data |= (1 << DDR_CMD);
  } else {
    auto_3_data &= ~(1 << DDR_CMD);
  }
  if (spi_config->spi_config_4.ddr_dummy_mode) {
    auto_3_data |= (1 << DDR_DUMMY);
  } else {
    auto_3_data &= ~(1 << DDR_DUMMY);
  }
  if (spi_config->spi_config_4.ddr_extra_byte) {
    auto_3_data |= (1 << DDR_EXTRA_BYTE);
  } else {
    auto_3_data &= ~(1 << DDR_EXTRA_BYTE);
  }
#endif
  dummy_count =
    (uint16_t)((spi_config->spi_config_1.no_of_dummy_bytes) | (spi_config->spi_config_4.no_of_ms_dummy_bytes << 4));

  if (spi_config->spi_config_2.dummy_cycles_for_controller) {

    // Program dummy cycles in bit mode
    auto_3_data |= DUMMY_BYTE_OR_BIT_MODE;

    dummy_count = ((dummy_count * 8)
                   + (uint16_t)((spi_config->spi_config_2.dummy_cycles_for_controller
                                 * (1 << spi_config->spi_config_1.dummy_mode))));
  } else {
    auto_3_data &= ~DUMMY_BYTE_OR_BIT_MODE;
  }

  auto_3_data |= (((dummy_count >> 4) & 0xF) << 1);

  if (spi_config->spi_config_3.en_word_swap) {
    auto_3_data |= (1 << WORD_SWAP_EN);
  } else {
    auto_3_data &= (uint32_t)(~(1 << WORD_SWAP_EN));
  }
  *auto_3_ptr = auto_3_data;

  // address difference between auto config reg for csn 0 and 1 is 12regs
  offset     = 12 * spi_config->spi_config_2.cs_no;
  auto_1_ptr = ((uint32_t *)&qspi_reg->QSPI_AUTO_CTRL_CONFIG_1_REG + offset);
  auto_2_ptr = ((uint32_t *)&qspi_reg->QSPI_AUTO_CTRL_CONFIG_2_REG + offset);

  if (spi_config->spi_config_3._16bit_cmd_valid) {
    *auto_2_ptr = (spi_config->spi_config_3._16bit_rd_cmd_msb << 8);
  } else {
    *auto_2_ptr = (spi_config->spi_config_1.read_cmd << 8);
  }

  // enable read data swapping for auto mode
  if (spi_config->spi_config_2.swap_en == SWAP) {
    *auto_2_ptr |= AUTO_RD_SWAP;
  } else {
    *auto_2_ptr &= ~AUTO_RD_SWAP;
  }

  *auto_2_ptr |= (uint32_t)AUTO_ADDR_WIDTH //< address width 8, 9, 16 or 24 bit
                 //	|  (spi_config->spi_config_2.jump_inst << 24)          //< cmd to be used in case of jump
                 | (spi_config->spi_config_1.read_cmd << 16)     //< read cmd is used for wrap reads too
                 | (spi_config->spi_config_3.dummys_4_jump << 4) //< no. of dummy bytes in case of jump reads
                 | (spi_config->spi_config_1.dummy_W_or_R << 3); //< dummy writes or reads
#if defined(SLI_SI917)
  if (spi_config->spi_config_1.flash_type != MX_QUAD_FLASH) {
    *auto_2_ptr |= (spi_config->spi_config_1.continuous << 2); //< continuous read mode enable
  }
#endif

  *auto_1_ptr = ((dummy_count & 0xF) << 24)                               //< no. of dummy bytes for read
                | (spi_config->spi_config_3.no_of_dummy_bytes_wrap << 28) //< no. of dummy bytes for wrap
                //		| (spi_config->spi_config_2.jump_en << 23)  					//< jump enable
                | (0 << 23)                                        //< removed jump enable
                | (spi_config->spi_config_1.extra_byte_en << 18)   //< extra byte enable
                | (uint16_t)(EXTRA_BYTE << 10)                     //< extra byte value
                | (spi_config->spi_config_1.inst_mode << 6)        //< cmd mode
                | (spi_config->spi_config_1.addr_mode << 4)        //< addr mode
                | (spi_config->spi_config_1.dummy_mode << 2)       //< dummy mode
                | (spi_config->spi_config_1.extra_byte_mode << 0); //< extra mode

  if (spi_config->spi_config_4.dual_flash_mode) {
    *auto_1_ptr |= (OCTA_MODE << 8); //< data mode
  } else {
    *auto_1_ptr |= (spi_config->spi_config_1.data_mode << 8); //< data mode
  }
  if (spi_config->spi_config_2.wrap_len_in_bytes != NO_WRAP) {
    // Enable burst mode read in qspi
    qspi_reg->QSPI_BUS_MODE_REG |= QSPI_WRAP_EN;
    // no. of dummy bytes for wrap
    *auto_1_ptr |= 1 << 28;
    // read cmd for wrap mode
    *auto_2_ptr |= READ_BURST << 16;
  } else {
    // disable burst mode read in qspi
    qspi_reg->QSPI_BUS_MODE_REG &= ~QSPI_WRAP_EN;
  }
  RSI_QSPI_AutoModeEn(qspi_reg);
}

/*==============================================*/
/**
 * @fn           void qspi_auto_read( uint32_t cs_no, uint32_t addr,
                    uint8_t *data,
                    uint32_t hsize, 
                    uint32_t len_in_bytes,
                    spi_config_t *spi_config,
                    uint32_t dma_flags )
 * @brief        This API used to qspi auto read base address
 * @param[in]    cs_no         :   chip select number
 * @param[in]    addr          :   address of auto read
 * @param[in]    data          :   pointer to the base address 
 * @param[in]    len_in_bytes  :   length in bytes
 * @param[in]    spi_config    :   pointer to the SPI configuration 
 * @param[in]    dma_flags     :   dma flags 
 * @return       none
 */
void qspi_auto_read(uint32_t cs_no,
                    uint32_t addr,
                    uint8_t *data,
                    uint32_t hsize,
                    uint32_t len_in_bytes,
                    spi_config_t *spi_config,
                    uint32_t dma_flags)
{
  uint16_t *data_16bit;
  uint32_t *data_32bit;
  uint32_t pad;
  uint32_t split_length;
  uint32_t loop_count;
  uint32_t ch_no;

  data_16bit = (uint16_t *)data;
  data_32bit = (uint32_t *)data;

  if (cs_no == CHIP_ZERO) {
    // RS9116 CS_1 qspi auto base address
    addr += QSPI_AUTOM_CHIP0_ADDRESS;
  } else {
    // RS9116 CS_0 qspi auto base address
    addr += QSPI_AUTOM_CHIP1_ADDRESS;
  }
  if (spi_config->spi_config_2.dma_mode) {

    ch_no = (dma_flags & 0xFF);

    RSI_QSPI_GPDMA_Init(hsize, ch_no, 1);

    // split_length cannot be more than 1023, RS9116 QSPI hw limitation
    split_length = (4 * 1024) - 4;

    while (len_in_bytes) {

      loop_count = XMIN(len_in_bytes, split_length);

      // reducing total length by loop_count
      len_in_bytes -= loop_count;
      // read using dma
      RSI_QSPI_GPDMA_ReadFromFifo((uint32_t)addr, (uint32_t)data, loop_count, ch_no);
      addr += loop_count;
      data += loop_count;
    }
  } else {

    // pad length to hsize
    pad = len_in_bytes % (hsize + 1);
    if (pad) {
      len_in_bytes += (hsize + 1) - pad;
    }

    if (hsize == _8BIT) {
      while (len_in_bytes) {
        *(data) = *(uint8_t *)addr;
        data++;
        addr++;
        len_in_bytes -= 1;
      }
    } else if (hsize == _16BIT) {
      while (len_in_bytes) {
        *(data_16bit) = *(uint16_t *)addr;
        len_in_bytes -= 2;
        data_16bit++;
        if (spi_config->spi_config_4.dual_flash_mode) {
          addr += 1;
        } else {
          addr += 2;
        }
      }
    } else if (hsize == _32BIT) {
      while (len_in_bytes) {
        *(data_32bit) = *(uint32_t *)addr;
        len_in_bytes -= 4;
        data_32bit++;
        if (spi_config->spi_config_4.dual_flash_mode) {
          addr += 2;
        } else {
          addr += 4;
        }
      }
    }
  }
}

/*==============================================*/
/**
 * @fn           void qspi_flash_init(qspi_reg_t *qspi_reg, spi_config_t *spi_config, uint32_t wr_reg_delay_ms)   
 * @brief        This API used to qspi flash initialisation
 * @param[in]    qspi_reg        :   qsi register pointer to the qspi read data
 * @param[in]    spi_config      :   pointer to the SPI configuration 
 * @param[in]    wr_reg_delay_ms :   register delay in milisecond 
 * @return       none
 */
void qspi_flash_init(qspi_reg_t *qspi_reg, spi_config_t *spi_config, uint32_t wr_reg_delay_ms)
{
  uint32_t reg_cfg  = 0;
  uint32_t xip_mode = DIS_XIP;
  uint32_t is_quad_mode;
  uint32_t is_octa_mode;
  uint32_t flash_type;
  uint32_t opi_qpi, wr_status_reg = 0, flash_status2 = 0;
  uint32_t dummy_7_0, dummy_7_4;
  uint32_t str_dtr, dummy_clks, status, operating_mode;

  // store quad indication to optimize
  is_quad_mode = CHK_QUAD_MODE;
  is_octa_mode = CHK_OCTA_MODE;
  opi_qpi      = (spi_config->spi_config_1.inst_mode == OCTA_MODE)
                   ? BIT(3)
                   : ((spi_config->spi_config_1.inst_mode == QUAD_MODE) ? BIT(2) : 0);
  flash_type   = spi_config->spi_config_1.flash_type;

  // Reset flash to single bit mode
  qspi_set_flash_mode(qspi_reg, 0, spi_config->spi_config_2.cs_no, 0, flash_type);

// Store flash operating mode and reset type in battery backup flipflops.
#ifdef CHIP_9118
  operating_mode = (spi_config->spi_config_1.inst_mode | (spi_config->spi_config_3.ddr_mode_en ? BIT(2) : 0));
#else
  operating_mode = (spi_config->spi_config_1.inst_mode);
#endif
  RSI_QSPI_UpdateOperatingMode_and_ResetType(qspi_reg, ((operating_mode << 5) | (spi_config->spi_config_5.reset_type)));

  switch (flash_type) {
    case SST_QUAD_FLASH:
    case SST_SPI_FLASH:
      // Configure flash to quad/single mode as requested
      //qspi_set_flash_mode(qspi_reg, spi_config->spi_config_1.data_mode,
      //		spi_config->spi_config_2.cs_no, 0, flash_type);
#if 0      
		if(spi_config->spi_config_2.protection) {
			// Removes write protection, so that memory can be written
			// OR Enables write protection, so that unwanted writes are ignored
			qspi_write_block_protect(qspi_reg, SST_PROTECTION,
					spi_config->spi_config_2.cs_no,
					spi_config->spi_config_2.num_prot_bytes, wr_reg_delay_ms);
		}
#endif
      break;

    case AT_QUAD_FLASH:
      // QUAD_MODE en is at BIT(7), so enable it if asked
      if (is_quad_mode) {
        qspi_flash_reg_write(qspi_reg, WCON, ATMEL_QEN, spi_config->spi_config_2.cs_no, wr_reg_delay_ms);
      }
#if 0      
		if(spi_config->spi_config_2.protection) {
			// Removes write protection, so that memory can be written
			// OR Enables write protection, so that unwanted writes are ignored
			qspi_status_reg_write(qspi_reg, AT_PROT,
					spi_config, wr_reg_delay_ms);
		}
#endif
      break;

    case MX_QUAD_FLASH:

      if ((is_quad_mode) || (spi_config->spi_config_4.prot_top_bottom)
          || (spi_config->spi_config_4.high_perf_mode_en)) {
        status = qspi_wait_flash_status_Idle(qspi_reg, spi_config, wr_reg_delay_ms);

        if ((!!(status & BIT(6))) != is_quad_mode) {
          wr_status_reg = 1;
        }

        status |= (is_quad_mode << 6);
        if ((spi_config->spi_config_4.prot_top_bottom) || (spi_config->spi_config_4.high_perf_mode_en)) {
          flash_status2 = qspi_flash_reg_read(qspi_reg, 0x15, spi_config->spi_config_2.cs_no | BIT(31), spi_config);
          status        = (status << 16) | flash_status2;
          if (spi_config->spi_config_4.high_perf_mode_en) {
            status |= HIGH_PERF_MODE;
            wr_reg_delay_ms |= BIT(29);
            wr_status_reg = 1;
          } else {
            status >>= 8;
          }

          if (spi_config->spi_config_4.prot_top_bottom) {

            status |= (PROT_FROM_TOP << (spi_config->spi_config_4.high_perf_mode_en) ? 8 : 0);

            wr_reg_delay_ms |= BIT(31);
            wr_status_reg = 1;
          }

          else if (!(spi_config->spi_config_4.high_perf_mode_en)) {
            status >>= 8;
          }
        }

        if (wr_status_reg) {
          qspi_status_reg_write(qspi_reg, status, spi_config, wr_reg_delay_ms);
        }
      }
      wr_reg_delay_ms &= ~BIT(31);
      if (spi_config->spi_config_1.inst_mode == QUAD_MODE) {
        // Send QPI/OPI enable command
        qspi_set_flash_mode(qspi_reg,
                            spi_config->spi_config_1.inst_mode,
                            spi_config->spi_config_2.cs_no,
                            0,
                            flash_type);
      }
      break;

    case MX_OCTA_FLASH:
#ifdef CHIP_9118
      str_dtr = is_octa_mode ? (spi_config->spi_config_3.ddr_mode_en ? BIT(1) : BIT(0)) : 0;
#else
      str_dtr = is_octa_mode ? BIT(0) : 0;
#endif
      //qspi_set_flash_mode(qspi_reg, 0, spi_config->spi_config_2.cs_no, 0, flash_type);
      dummy_7_4 = spi_config->spi_config_4.no_of_ms_dummy_bytes;
      dummy_7_0 = (spi_config->spi_config_1.no_of_dummy_bytes | (dummy_7_4 << 4));
      if (dummy_7_0 <= 8) {
        dummy_clks = 6;
      } else if (dummy_7_0 <= 16) {
        dummy_clks = 2;
      } else {
        dummy_clks = 0;
      }

      if (spi_config->spi_config_4.prot_top_bottom) {
        status        = qspi_wait_flash_status_Idle(qspi_reg, spi_config, wr_reg_delay_ms);
        flash_status2 = qspi_flash_reg_read(qspi_reg, 0x15, spi_config->spi_config_2.cs_no, spi_config);
        if (flash_status2 & BIT(3)) {
        } else {
          status = ((status << 8) | 0xF);
          wr_reg_delay_ms |= BIT(31);
          qspi_status_reg_write(qspi_reg, status, spi_config, wr_reg_delay_ms);
        }
      }
      wr_reg_delay_ms &= ~BIT(31);

      qspi_status_control_reg_write(spi_config,
                                    qspi_reg,
                                    WCFG2,
                                    0x300,
                                    (uint16_t)dummy_clks,
                                    spi_config->spi_config_2.cs_no,
                                    wr_reg_delay_ms);
      qspi_status_control_reg_write(spi_config,
                                    qspi_reg,
                                    WCFG2,
                                    0,
                                    (uint16_t)str_dtr,
                                    spi_config->spi_config_2.cs_no,
                                    wr_reg_delay_ms);
      break;

    case MICRON_QUAD_FLASH:
      //qspi_set_flash_mode(qspi_reg, 0, spi_config->spi_config_2.cs_no, 0, flash_type);

      if (!spi_config->spi_config_3.xip_mode) {
        xip_mode = DIS_XIP;
      } else {
        xip_mode = XIP_MODE;
      }
      dummy_clks = spi_config->spi_config_1.no_of_dummy_bytes;
      if (spi_config->spi_config_1.dummy_mode == DUAL_MODE) {
        dummy_clks = dummy_clks * 4;
      } else if (spi_config->spi_config_1.dummy_mode == QUAD_MODE) {
        dummy_clks = dummy_clks * 2;
      } else {
        dummy_clks = spi_config->spi_config_1.no_of_dummy_bytes * 8;
      }
      // configure the no_of_dummy_bytes
      qspi_flash_reg_write(qspi_reg,
                           WR_VOL_CON_REG,

                           ((dummy_clks << 4) | xip_mode | spi_config->spi_config_2.wrap_len_in_bytes),
                           spi_config->spi_config_2.cs_no,
                           wr_reg_delay_ms);

      if (is_quad_mode) {
        // default configs + Quad mode enable
        reg_cfg = 0x5F;
      } else if (CHK_DUAL_MODE) {
        // default configs + Dual mode enable
        reg_cfg = 0x9F;
      } else {
        // default configs + SPI mode enable
        reg_cfg = 0xDF;
      }
      if (reg_cfg) {
        // writing enhanced volatile config reg only if Quad/dual mode are enabled
        qspi_flash_reg_write(qspi_reg, WR_ENHN_VOL_CON_REG, reg_cfg, spi_config->spi_config_2.cs_no, wr_reg_delay_ms);
      }
      break;

    case GIGA_DEVICE_FLASH:
    case WBOND_QUAD_FLASH:
      // qspi_set_flash_mode(qspi_reg, 0,
      // spi_config->spi_config_2.cs_no, 0, flash_type);
      // Set QUAD ENABLE bit in status register(BIT(9))
      status = qspi_wait_flash_status_Idle(qspi_reg, spi_config, wr_reg_delay_ms);
#if !defined(SLI_SI917)
      status <<= 8;
      status |= (is_quad_mode << 1);

      qspi_status_reg_write(qspi_reg, status, spi_config, wr_reg_delay_ms);
#else
      flash_status2 = qspi_flash_reg_read(qspi_reg, SR2_READ, spi_config->spi_config_2.cs_no, spi_config);
      // Checking whether flash configuration and user configurations are same, if not same we make it equal below
      if (is_quad_mode != ((flash_status2 & QUAD_EN) >> 1)) {
        // Preparing 2byte status reg data; upper byte will be fed first
        status <<= 8;
        // If quad mode is set in spi config we enable quad in flash otherwise we disable
        if (is_quad_mode) {
          status |= (flash_status2 | QUAD_EN);
        } else {
          status |= (flash_status2 & ~QUAD_EN);
        }
        wr_status_reg = 1;
        // This 31st bit is to enable qspi_status_reg_write func to initiate 2byte status reg write
        wr_reg_delay_ms |= BIT(31);
        // This programming enables the status_re_write func to do non-volatile reg write
        wr_reg_delay_ms |= BIT(30);
      }
      if (wr_status_reg)
        qspi_status_reg_write(qspi_reg, status, spi_config, wr_reg_delay_ms);
      wr_reg_delay_ms &= ~(BIT(30) | BIT(31));
#endif
      if (spi_config->spi_config_1.inst_mode == QUAD_MODE) {
        // Send QPI enable command
        qspi_set_flash_mode(qspi_reg,
                            spi_config->spi_config_1.inst_mode,
                            spi_config->spi_config_2.cs_no,
                            0,
                            flash_type);
        // Set read parameters. Setting number of dummy bytes and Wrap bytes.
#if defined(SLI_SI917)
        // This is to fix the dummy cycles configuration which was not happening properly
        uint32_t total_dummy_bytes =
          spi_config->spi_config_1.no_of_dummy_bytes + spi_config->spi_config_1.extra_byte_en;
        if (total_dummy_bytes == 4) {
          // 8 dummy cycles
          dummy_clks = (3 << 4);
        } else if (total_dummy_bytes == 3) {
          // 6 dummy cycles
          dummy_clks = (2 << 4);
        } else {
          // 4 dummy cycles
          dummy_clks = 0;
        }
#else
        if (spi_config->spi_config_1.no_of_dummy_bytes > 2) {
          if (spi_config->spi_config_1.no_of_dummy_bytes == 3) {
            // 6 clocks dummy cycles
            dummy_clks = (0x2 << 4);
          } else {
            // 8 clocks dummy cycles
            dummy_clks = (0x3 << 4);
          }
        } else {
          // 4 clocks dummy cycles
          dummy_clks = 0;
        }
#endif
        qspi_flash_reg_write(qspi_reg, 0xC0, (dummy_clks | 0x00), 0, wr_reg_delay_ms);
      }
      break;

#if defined(SLI_SI917)
    case ADESTO_QUAD_FLASH:
      //Writing control reg for quad mode
      if (is_quad_mode) {
        qspi_status_control_reg_write(spi_config,
                                      qspi_reg,
                                      STS_CTRL,
                                      2,
                                      (uint16_t)(is_quad_mode << 1),
                                      spi_config->spi_config_2.cs_no,
                                      wr_reg_delay_ms);
      }
      //Writing control reg for xip mode and wrap lenth
      if (spi_config->spi_config_3.xip_mode) {

        qspi_status_control_reg_write(spi_config,
                                      qspi_reg,
                                      STS_CTRL,
                                      4,
                                      (uint16_t)(spi_config->spi_config_2.wrap_len_in_bytes | BIT(3)),
                                      spi_config->spi_config_2.cs_no,
                                      wr_reg_delay_ms);
      }
      //Writing control reg for dummy cycles incase of 0-4-4 command
      if (spi_config->spi_config_2.dummy_cycles_for_controller) {
        qspi_status_control_reg_write(spi_config,
                                      qspi_reg,
                                      STS_CTRL,
                                      5,
                                      (spi_config->spi_config_2.dummy_cycles_for_controller << 4),
                                      spi_config->spi_config_2.cs_no,
                                      wr_reg_delay_ms);
      }
      break;
#endif
    case ADESTO_OCTA_FLASH:
      str_dtr = 0;
      if (opi_qpi) {
        if (spi_config->spi_config_1.dummy_mode == OCTA_MODE) {
          dummy_clks = spi_config->spi_config_1.no_of_dummy_bytes;
        } else {
          dummy_clks = spi_config->spi_config_1.no_of_dummy_bytes * 2;
        }
      } else {
        dummy_clks = 0;
      }
      if (dummy_clks <= 8) {
        dummy_clks = 0;
      } else if (dummy_clks <= 16) {
        dummy_clks = 4;
      }
      //qspi_set_flash_mode(qspi_reg, 0,
      //		spi_config->spi_config_2.cs_no, 0, flash_type);
      // config dummy and wrap bytes
      qspi_status_control_reg_write(spi_config,
                                    qspi_reg,
                                    STS_CTRL,
                                    3,
                                    (uint16_t)(dummy_clks | (spi_config->spi_config_2.wrap_len_in_bytes << 5)),
                                    spi_config->spi_config_2.cs_no,
                                    wr_reg_delay_ms);
      // enabling opi or qpi and str or dtr
      if (opi_qpi || str_dtr) {
        qspi_flash_reg_write(qspi_reg, STS_BYT2, (opi_qpi | str_dtr), spi_config->spi_config_2.cs_no, wr_reg_delay_ms);
      }
      break;
    case ISSI_FLASH:
      status = qspi_wait_flash_status_Idle(qspi_reg, spi_config, wr_reg_delay_ms);
      if (is_quad_mode != ((status >> 6) & 1)) {
        if (is_quad_mode) {
          status |= BIT(6);
        } else {
          status &= ~BIT(6);
        }
        qspi_status_reg_write(qspi_reg, status, spi_config, wr_reg_delay_ms);
      }
      if (spi_config->spi_config_1.inst_mode == QUAD_MODE) {
        qspi_set_flash_mode(qspi_reg,
                            spi_config->spi_config_1.inst_mode,
                            spi_config->spi_config_2.cs_no,
                            0,
                            flash_type);
      }
      if (spi_config->spi_config_1.dummy_mode == QUAD_MODE)
        dummy_clks = 2;
      else if (spi_config->spi_config_1.dummy_mode == DUAL_MODE)
        dummy_clks = 4;
      else
        dummy_clks = 8;
      dummy_clks = (spi_config->spi_config_1.no_of_dummy_bytes + spi_config->spi_config_1.extra_byte_en) * dummy_clks
                   + spi_config->spi_config_2.dummy_cycles_for_controller;
      qspi_flash_reg_write(qspi_reg, 0xC0, (dummy_clks << 3), 0, wr_reg_delay_ms);
      break;
    case XMC_FLASH:

      //! Reading Second status register
      flash_status2 = qspi_flash_reg_read(qspi_reg, SR2_READ, spi_config->spi_config_2.cs_no, spi_config);
      //! Checking whether flash configuration and user configurations are same, if not same we make it equal below
      if (is_quad_mode != ((flash_status2 & QUAD_EN) >> 1)) {
        //! If quad mode is set in spi config we enable quad in flash otherwise we disable
        if (is_quad_mode) {
          flash_status2 |= QUAD_EN;
        } else {
          flash_status2 &= ~QUAD_EN;
        }

        qspi_flash_reg_write(qspi_reg, SR2_WRITE, flash_status2, spi_config->spi_config_2.cs_no, wr_reg_delay_ms);

        qspi_wait_flash_status_Idle(qspi_reg, spi_config, wr_reg_delay_ms);
      }
      if (spi_config->spi_config_1.inst_mode == QUAD_MODE) {
        //! Send QPI enable command

        qspi_set_flash_mode(qspi_reg,
                            spi_config->spi_config_1.inst_mode,
                            spi_config->spi_config_2.cs_no,
                            0,
                            flash_type);
        //! Set read parameters. Setting number of dummy bytes and Wrap bytes.
        //! This is to fix the dummy cycles configuration which was not happening properly
        uint32_t total_dummy_bytes =
          spi_config->spi_config_1.no_of_dummy_bytes + spi_config->spi_config_1.extra_byte_en;
        if (total_dummy_bytes == 4) {
          //! 8 dummy cycles
          dummy_clks = (3 << 4);
        } else if (total_dummy_bytes == 3) {
          //! 6 dummy cycles
          dummy_clks = (2 << 4);
        } else if (total_dummy_bytes == 2) {
          //! 4 dummy cycles
          dummy_clks = (1 << 4);
        } else {
          //! 2 dummy cycles
          dummy_clks = 0;
        }
        qspi_flash_reg_write(qspi_reg, 0xC0, (dummy_clks | 0x00), 0, wr_reg_delay_ms);
      }
      break;
    default:;
      // XXX UNKNOWN FLASH TYPE
      // If the flash type is other than the current list
      // then caller must take care of flash configuration
  }
}

/*==============================================*/
/**
 * @fn           void RSI_QSPI_ConfigQspiDll(spi_config_t *spi_config, qspi_reg_t *qspi_reg)  
 * @brief        This API used to qspi transmit calibration enable
 * @param[in]    spi_config      :   pointer to the SPI configuration
 * @param[in]    qspi_reg        :   qsi register pointer to the qspi read data 
 * @return       none
 */
void RSI_QSPI_ConfigQspiDll(spi_config_t *spi_config, qspi_reg_t *qspi_reg)
{
  uint32_t delay = 10;
  M4SS_QSPI_TX_DLL_TEST_REG |= BIT(5); // tx calib enable
  qspi_reg->QSPI_CLK_CONFIG_REG =
    (QSPI_DLL_CALIB | QSPI_DLL_TX_EN | QSPI_DLL_RX_EN | (0x6 << 22) | (0x6 << 12) | BIT(18)
     | (spi_config->spi_config_2.qspi_clk_en << 8)); // Octa_mode_enable_with_dqs
  //wait till 10 clocks
  while (delay) {
    __ASM("nop");
    delay--;
  }

  M4SS_QSPI_TX_DLL_TEST_REG &= ~BIT(5); // tx calib enable
  qspi_reg->QSPI_CLK_CONFIG_REG &= ~(0xFE539EFF);
  qspi_reg->QSPI_CLK_CONFIG_REG |= ~(0xFE539EFE);
}

/*==============================================*/
/**
 * @fn           void qspi_spi_init(qspi_reg_t *qspi_reg, 
                   spi_config_t *spi_config,
                   uint32_t flash_init_req,
                   uint32_t wr_reg_delay_ms,
                   uint8_t fifo_thrsld )  
 * @brief        This API used to qspi configure dataline SPI initialisation
 * @param[in]    qspi_reg        :   qsi register pointer to the qspi read data
 * @param[in]    spi_config      :   pointer to the SPI configuration 
 * @param[in]    flash_init_req  :   flash initialisation
 * @param[in]    wr_reg_delay_ms :   register delay in milisecond
 * @param[in]    fifo_thrsld     :   threshold FIFO
 * @return       none
 */
void qspi_spi_init(qspi_reg_t *qspi_reg,
                   spi_config_t *spi_config,
                   uint32_t flash_init_req,
                   uint32_t wr_reg_delay_ms,
                   uint8_t fifo_thrsld)
{
  uint32_t dataline_pos, d7_d4_data_pos;
  uint32_t cs_no         = spi_config->spi_config_2.cs_no;
  uint32_t secondary_csn = spi_config->spi_config_4.secondary_csn;

  // d2d3 dataline initialization
  dataline_pos   = GET_POS;
  d7_d4_data_pos = GET_POS_D7_D4;
  qspi_reg->QSPI_BUS_MODE_REG =
    ((qspi_reg->QSPI_BUS_MODE_REG & (uint16_t)(MASK_D3_D2(dataline_pos)))
     | (spi_config->spi_config_1.d3d2_data << dataline_pos) | (spi_config->spi_config_2.neg_edge_sampling << 16));

  qspi_reg->OCTA_SPI_BUS_CONTROLLER = ((qspi_reg->OCTA_SPI_BUS_CONTROLLER & (uint16_t)(MASK_D7_D4(d7_d4_data_pos)))
                                       | (spi_config->spi_config_5.d7_d4_data << d7_d4_data_pos));

  //for micron, qspi should not be switched to spi mode, as flash might be in quad mode already
  qspi_switch_qspi2(qspi_reg, SINGLE_MODE, spi_config->spi_config_2.cs_no);

  if (spi_config->spi_config_4.dual_flash_mode) {
    qspi_reg->OCTA_SPI_BUS_CONTROLLER2 |= (DUAL_FLASH_MODE | (secondary_csn << (4 + (cs_no * 2))));
  } else {
    if (((qspi_reg->OCTA_SPI_BUS_CONTROLLER2 & DUAL_FLASH_MODE) != 0)
        || ((qspi_reg->OCTA_SPI_BUS_CONTROLLER2 & (0x3 << (4 + (cs_no * 2)))) != 0)) {
      qspi_reg->OCTA_SPI_BUS_CONTROLLER2 &= ~(DUAL_FLASH_MODE | (secondary_csn << (4 + (cs_no * 2))));
    }
  }

  if (spi_config->spi_config_4.dual_flash_mode) {
    qspi_reg->OCTA_SPI_BUS_CONTROLLER2 |= BIT(19);
  }

  if (flash_init_req) {
    // flash initialization
    qspi_flash_init(qspi_reg, spi_config, wr_reg_delay_ms);
    if (spi_config->spi_config_2.wrap_len_in_bytes != NO_WRAP) {
      // send SET_BURST cmd to flash to initialize wrap
      qspi_write_to_flash(qspi_reg,
                          16,
                          (uint16_t)((SET_BURST << 8) | (spi_config->spi_config_2.wrap_len_in_bytes)),
                          spi_config->spi_config_2.cs_no);
      DEASSERT_CSN;
    }
  }
  if (spi_config->spi_config_4.dual_flash_mode) {
    qspi_reg->OCTA_SPI_BUS_CONTROLLER2 &= ~BIT(19);
  }

  qspi_switch_qspi2(qspi_reg, spi_config->spi_config_1.inst_mode, spi_config->spi_config_2.cs_no);

// only enabling synchros logic & dynamic clk
#ifdef CHIP_9118
  if (spi_config->spi_config_4.ddr_dll_en) {
    RSI_QSPI_ConfigQspiDll(spi_config, qspi_reg);
  } else {
    qspi_reg->QSPI_CLK_CONFIG_REG = (spi_config->spi_config_2.qspi_clk_en << 8);
  }
#else
  qspi_reg->QSPI_CLK_CONFIG_REG = (spi_config->spi_config_2.qspi_clk_en << 8);
#endif

  if (spi_config->spi_config_4.polarity_mode) {
    qspi_reg->QSPI_CLK_CONFIG_REG |= BIT(20);
  } else {
    qspi_reg->QSPI_CLK_CONFIG_REG &= ~BIT(20);
  }

  if (spi_config->spi_config_4.auto_csn_based_addr_en) {
    qspi_reg->QSPI_BUS_MODE_REG |= AUTO_CSN_BASED_ADDR_ENABLE;
    qspi_reg->QSPI_AUTO_BASE_ADDR_UNMASK_CSN0 = 0xFC000000;
  } else {
    qspi_reg->QSPI_BUS_MODE_REG &= ~AUTO_CSN_BASED_ADDR_ENABLE;
  }

// Keeping csn high for one SOC clock cycle, during CSN assertions. Range is 0 to 31. /
// This is fix for QSPI read issue at > 75 degrees, if prefetch is enabled.
#ifdef CHIP_9118
  if (!(spi_config->spi_config_4.ddr_dll_en)) {
    qspi_reg->QSPI_CLK_CONFIG_REG |= 1;
  }
#else
  qspi_reg->QSPI_CLK_CONFIG_REG |= 1;
#endif
  // Enable HW_CTRL_MODE
  qspi_reg->QSPI_MANUAL_CONFIG_REG |= (spi_config->spi_config_2.cs_no << 13) | HW_CTRL_MODE;

  if (spi_config->spi_config_4.qspi_loop_back_mode_en) {
    qspi_reg->QSPI_MANUAL_CONFIG_2_REG |= QSPI_LOOP_BACK_MODE_EN;
  } else {
    qspi_reg->QSPI_MANUAL_CONFIG_2_REG &= ~QSPI_LOOP_BACK_MODE_EN;
  }
#ifdef CHIP_9118
  if (spi_config->spi_config_4.qspi_manual_ddr_phasse) {
    qspi_reg->QSPI_MANUAL_CONFIG_2_REG |= QSPI_MANUAL_DDR_PHASSE;
  } else {
    qspi_reg->QSPI_MANUAL_CONFIG_2_REG &= ~QSPI_MANUAL_DDR_PHASSE;
  }

  if (spi_config->spi_config_3.ddr_mode_en) {
    qspi_reg->QSPI_MANUAL_CONFIG_2_REG |= QSPI_DDR_CLK_EN;
  } else {
    qspi_reg->QSPI_MANUAL_CONFIG_2_REG &= ~QSPI_DDR_CLK_EN;
  }
#endif
  if (fifo_thrsld) {
    // configure QSPI fifo thresholds
    qspi_reg->QSPI_FIFO_THRESHOLD_REG = (QSPI_FIFO_AFULL_TH << 4) | QSPI_FIFO_AEMPTY_TH;
  }

  // Enable full duplex mode if asked
  if (spi_config->spi_config_2.full_duplex == EN_FULL_DUPLEX) {
    qspi_reg->QSPI_MANUAL_CONFIG_REG |= FULL_DUPLEX_EN;
  } else if (spi_config->spi_config_2.full_duplex == DIS_FULL_DUPLEX) {
    qspi_reg->QSPI_MANUAL_CONFIG_REG &= ~FULL_DUPLEX_EN;
  }

  if (spi_config->spi_config_4.continue_fetch_en) {
    qspi_reg->QSPI_AUTO_CONITNUE_FETCH_CTRL_REG |= CONTINUE_FETCH_EN;
  } else {
    qspi_reg->QSPI_AUTO_CONITNUE_FETCH_CTRL_REG &= ~CONTINUE_FETCH_EN;
  }

  qspi_auto_init(qspi_reg, spi_config);
  // If auto mode is requested call auto_init
  if (spi_config->spi_config_2.auto_mode) {
    RSI_QSPI_AutoModeEn(qspi_reg);
  }
}

/*==============================================*/
/**
 * @fn           void qspi_spi_erase(qspi_reg_t *qspi_reg, 
                    spi_config_t *spi_config,
                    uint32_t erase_cmd,
                    uint32_t blk_sec_addr,
                    uint32_t dis_hw_ctrl,
                    uint32_t wr_reg_delay_ms )
 * @brief        This API used to qspi erase command flash
 * @param[in]    qspi_reg        :   qsi register pointer to the qspi read data
 * @param[in]    spi_config      :   pointer to the SPI configuration
 * @param[in]    erase_cmd       :   erase command is sent to flash
 * @param[in]    blk_sec_addr    :   address of sector/block
 * @param[in]    dis_hw_ctrl     :   hardware control needs to be disabled
 * @param[in]    wr_reg_delay_ms :   register delay in milisecond
 * @return       none
 */
void qspi_spi_erase(qspi_reg_t *qspi_reg,
                    spi_config_t *spi_config,
                    uint32_t erase_cmd,
                    uint32_t blk_sec_addr,
                    uint32_t dis_hw_ctrl,
                    uint32_t wr_reg_delay_ms)
{
  uint32_t cs_no;
  uint32_t prev_state = 0, flash_type, cmd_len;
  uint32_t cmd_to_drive;
  (void)flash_type;
  cs_no      = spi_config->spi_config_2.cs_no;
  flash_type = spi_config->spi_config_1.flash_type;

  if (spi_config->spi_config_2.addr_width == 4) {
    // Shifting more than 31 bit won't work in our processor. So not using below logic.
    if (dis_hw_ctrl & BIT(31)) {
    } else {
      blk_sec_addr &= 0x3FFFFFF;
    }
  } else {
    blk_sec_addr &= (uint16_t)((1 << (spi_config->spi_config_2.addr_width * 8)) - 1);
  }
  dis_hw_ctrl &= ~BIT(31);

  // Check if already auto mode enabled
  if (qspi_reg->QSPI_BUS_MODE_REG & AUTO_MODE) {
    qspi_reg->QSPI_BUS_MODE_REG &= ~AUTO_MODE;
    while (qspi_reg->QSPI_STATUS_REG & AUTO_MODE_ENABLED)
      ;
    prev_state = 1;
  }
  if (spi_config->spi_config_4.continue_fetch_en) {
    qspi_reg->QSPI_AUTO_CONITNUE_FETCH_CTRL_REG &= ~CONTINUE_FETCH_EN;
  }
  if (spi_config->spi_config_4.dual_flash_mode) {
    qspi_reg->OCTA_SPI_BUS_CONTROLLER2 |= BIT(19);
  }

  // switch qspi to data mode
  qspi_switch_qspi2(qspi_reg, spi_config->spi_config_3.wr_inst_mode, cs_no);

  // if hardware control needs to be disabled, do it here.
  if (dis_hw_ctrl) {
    qspi_reg->QSPI_MANUAL_CONFIG_REG &= ~HW_CTRL_MODE;
    while (qspi_reg->QSPI_STATUS_REG & HW_CTRLD_QSPI_MODE_CTRL_SCLK)
      ;
  }

// write enable
#ifdef CHIP_9118
  if (spi_config->spi_config_3.ddr_mode_en) {
    if (flash_type == MX_OCTA_FLASH) {
      qspi_write_to_flash(qspi_reg, QSPI_16BIT_LEN, (WREN << 8) | WREN2, cs_no);
    } else {
      qspi_write_to_flash(qspi_reg, QSPI_8BIT_LEN, WREN, cs_no);
    }
  } else {
    qspi_write_to_flash(qspi_reg, QSPI_8BIT_LEN, WREN, cs_no);
    if (spi_config->spi_config_3._16bit_cmd_valid) {
      qspi_write_to_flash(qspi_reg, QSPI_8BIT_LEN, WREN2, cs_no);
    }
  }
#else
  qspi_write_to_flash(qspi_reg, QSPI_8BIT_LEN, WREN, cs_no);
  if (spi_config->spi_config_3._16bit_cmd_valid) {
    qspi_write_to_flash(qspi_reg, QSPI_8BIT_LEN, WREN2, cs_no);
  }
#endif
  DEASSERT_CSN;

  // erase command is sent to flash
  // erase command is sent to flash
  cmd_len = spi_config->spi_config_3._16bit_cmd_valid ? 16 : 8;

  if (erase_cmd == SECTOR_ERASE) {
    cmd_to_drive = spi_config->spi_config_6.sector_erase_cmd;
  } else if (erase_cmd == BLOCK_ERASE) {
    cmd_to_drive = spi_config->spi_config_5.block_erase_cmd;
  } else { // for sector erase and block erase
    cmd_to_drive = spi_config->spi_config_6.chip_erase_cmd;
  }

  qspi_write_to_flash(qspi_reg, cmd_len, cmd_to_drive, cs_no);

  // if not chip erase then send the address of sector/block
  if (erase_cmd != CHIP_ERASE) {
    qspi_write_to_flash(qspi_reg, ADDR_LEN, blk_sec_addr, cs_no);
  }
  DEASSERT_CSN;

  // wait for flash to become idle
  qspi_wait_flash_status_Idle(qspi_reg, spi_config, wr_reg_delay_ms);

  if (spi_config->spi_config_4.dual_flash_mode) {
    qspi_reg->OCTA_SPI_BUS_CONTROLLER2 &= ~BIT(19);
  }

  // if hardware control was disabled, enable it here.
  if (dis_hw_ctrl) {
    qspi_reg->QSPI_MANUAL_CONFIG_REG |= HW_CTRL_MODE;
    while (!(qspi_reg->QSPI_STATUS_REG & HW_CTRLD_QSPI_MODE_CTRL_SCLK))
      ;
  }
  if (prev_state == 1) {
    qspi_reg->QSPI_BUS_MODE_REG |= AUTO_MODE;
    while (!(qspi_reg->QSPI_STATUS_REG & AUTO_MODE_ENABLED))
      ;
  }
  if (spi_config->spi_config_4.continue_fetch_en) {
    qspi_reg->QSPI_AUTO_CONITNUE_FETCH_CTRL_REG |= CONTINUE_FETCH_EN;
  }
}

/*==============================================*/
/**
 * @fn           uint32_t qspi_spi_write(qspi_reg_t *qspi_reg,
                        spi_config_t *spi_config,
                        uint32_t write_cmd,
                        uint32_t addr,
                        uint8_t *data,
                        uint32_t len_in_bytes,
                        uint16_t page_size,
                        uint32_t hsize,
                        uint32_t dis_hw_ctrl,
                        uint32_t wr_reg_delay_ms,
                        uint32_t check_en,
                        uint32_t dma_flags,
                        void *udmaHandle,
                        void *gpdmaHandle)
 * @brief        This API used to qspi write command sent to flash
 * @param[in]    qspi_reg        :   qsi register pointer to the qspi read data
 * @param[in]    spi_config      :   pointer to the SPI configuration
 * @param[in]    write_cmd       :   write command is sent to flash
 * @param[in]    addr            :   address 
 * @param[in]    data            :   pointer to the write
 * @param[in]    len_in_bytes    :   length in bytes
 * @param[in]    page_size       :   page size
 * @param[in]    hsize           :   size
 * @param[in]    dis_hw_ctrl     :   hardware control needs to be disabled
 * @param[in]    wr_reg_delay_ms :   register delay in milisecond
 * @param[in]    check_en        :   check enable write
 * @param[in]    dma_flags       :   DMA flags
 * @param[in]    udmaHandle      :   pointer to DMA controller
 * @param[in]    gpdmaHandle     :   pointer to general purpose DMA controller
 * @return       status
 */
uint32_t qspi_spi_write(qspi_reg_t *qspi_reg,
                        spi_config_t *spi_config,
                        uint32_t write_cmd,
                        uint32_t addr,
                        uint8_t *data,
                        uint32_t len_in_bytes,
                        uint16_t page_size,
                        uint32_t hsize,
                        uint32_t dis_hw_ctrl,
                        uint32_t wr_reg_delay_ms,
                        uint32_t check_en,
                        uint32_t dma_flags,
                        void *udmaHandle,
                        void *gpdmaHandle)
{

  uint32_t loop_count;
  uint16_t *data_16bit;
  uint32_t *data_32bit;
  uint32_t once;
  uint32_t cs_no;
  uint32_t prev_state     = 0;
  uint32_t qspi_data_mode = 0;
  uint32_t address;
  uint32_t length, status, len_in_loop, flash_type;
  volatile uint32_t auto_mode_address = 0;
  uint8_t *data_in;
  uint32_t ch_no;
  status     = RSI_OK;
  once       = 1;
  data_16bit = (uint16_t *)data;
  data_32bit = (uint32_t *)data;
  cs_no      = spi_config->spi_config_2.cs_no;
  flash_type = spi_config->spi_config_1.flash_type;
  ch_no      = (dma_flags & 0xFF);
  (void)write_cmd;
  (void)flash_type;

  // Ignoring bits more than address width.
  if (spi_config->spi_config_2.addr_width == 4) {
    // Shifting more than 31 bit won't work in our processor. So not using below logic.
    if (dis_hw_ctrl & BIT(31)) {
    } else {
      addr &= 0x3FFFFFF;
    }
  } else {
    addr &= (uint32_t)((1 << (spi_config->spi_config_2.addr_width * 8)) - 1);
  }
  dis_hw_ctrl &= ~BIT(31);

  address = addr;
  length  = len_in_bytes;
  data_in = (uint8_t *)data;
  // Check if already auto mode enabled
  if (qspi_reg->QSPI_BUS_MODE_REG & AUTO_MODE) {
    qspi_reg->QSPI_BUS_MODE_REG &= ~AUTO_MODE;
    while (qspi_reg->QSPI_STATUS_REG & AUTO_MODE_ENABLED)
      ;
    prev_state = 1;
  }
  if (spi_config->spi_config_4.continue_fetch_en) {
    qspi_reg->QSPI_AUTO_CONITNUE_FETCH_CTRL_REG &= ~CONTINUE_FETCH_EN;
  }

  if (spi_config->spi_config_4.dual_flash_mode) {
    qspi_reg->OCTA_SPI_BUS_CONTROLLER2 |= BIT(19);
  }

  while (len_in_bytes) {
    // pick loop_count as minimum of len_in_bytes and page_size
    loop_count = XMIN(len_in_bytes, page_size);
    // switch qspi to inst mode
    qspi_switch_qspi2(qspi_reg, spi_config->spi_config_3.wr_inst_mode, cs_no);
#ifdef CHIP_9118
    if (spi_config->spi_config_3.ddr_mode_en) {
      if (flash_type == MX_OCTA_FLASH) {
        qspi_write_to_flash(qspi_reg, QSPI_16BIT_LEN, (WREN << 8) | WREN2, cs_no);
      } else {
        qspi_write_to_flash(qspi_reg, QSPI_8BIT_LEN, WREN, cs_no);
      }
    } else {
      // write enable
      qspi_write_to_flash(qspi_reg, CMD_LEN, WREN, cs_no);
      if (spi_config->spi_config_3._16bit_cmd_valid) {
        qspi_write_to_flash(qspi_reg, CMD_LEN, WREN2, cs_no);
      }
    }
#else
    // write enable
    qspi_write_to_flash(qspi_reg, CMD_LEN, WREN, cs_no);
    if (spi_config->spi_config_3._16bit_cmd_valid) {
      qspi_write_to_flash(qspi_reg, CMD_LEN, WREN2, cs_no);
    }
#endif
    DEASSERT_CSN;

    // write command is send to flash
#ifdef CHIP_9118
    if (spi_config->spi_config_3.ddr_mode_en) {
      if (flash_type == MX_OCTA_FLASH) {
        qspi_write_to_flash(qspi_reg,
                            QSPI_16BIT_LEN,
                            ((spi_config->spi_config_3.wr_cmd << 8) | spi_config->spi_config_4._16bit_wr_cmd_msb),
                            cs_no);
      } else {
        qspi_write_to_flash(qspi_reg, QSPI_8BIT_LEN, spi_config->spi_config_3.wr_cmd, cs_no);
      }
    } else {
      qspi_write_to_flash(qspi_reg, CMD_LEN, spi_config->spi_config_3.wr_cmd, cs_no);
      if (spi_config->spi_config_3._16bit_cmd_valid) {
        qspi_write_to_flash(qspi_reg, QSPI_8BIT_LEN, spi_config->spi_config_4._16bit_wr_cmd_msb, cs_no);
      }
    }
#else
    qspi_write_to_flash(qspi_reg, CMD_LEN, spi_config->spi_config_3.wr_cmd, cs_no);
    if (spi_config->spi_config_3._16bit_cmd_valid) {
      qspi_write_to_flash(qspi_reg, QSPI_8BIT_LEN, spi_config->spi_config_4._16bit_wr_cmd_msb, cs_no);
    }
#endif

    // if write cmd is not AAI then send addr or send atleast once
    if (once) {
      qspi_switch_qspi2(qspi_reg, spi_config->spi_config_3.wr_addr_mode, cs_no);
      qspi_write_to_flash(qspi_reg, ADDR_LEN, addr, cs_no);
      if (spi_config->spi_config_3.wr_cmd == AAI) {
        once = 0;
      }
    }

    if (spi_config->spi_config_4.dual_flash_mode) {
      qspi_reg->OCTA_SPI_BUS_CONTROLLER2 &= ~BIT(19);
      qspi_data_mode                        = spi_config->spi_config_3.wr_data_mode;
      spi_config->spi_config_3.wr_data_mode = OCTA_MODE;
    }
    if (ODD_PAGE_BOUNDARY) {
      loop_count = (uint16_t)(page_size - ODD_PAGE_BOUNDARY);
      loop_count = XMIN(loop_count, len_in_bytes);
    }
    // reducing total length by loop_count
    len_in_bytes -= loop_count;
    // increment addr by loop_count, so that during next iteration addr will be ready
    addr += loop_count;

    while (loop_count) {
      // start writing data
      qspi_switch_qspi2(qspi_reg, spi_config->spi_config_3.wr_data_mode, cs_no);
      // write swap en is done only for data
      qspi_reg->QSPI_MANUAL_CONFIG_2_REG = (qspi_reg->QSPI_MANUAL_CONFIG_2_REG & (uint32_t)~0xF)
                                           | (spi_config->spi_config_2.swap_en << cs_no);

      if (spi_config->spi_config_4.dma_write) {
        qspi_reg->QSPI_MANUAL_WRITE_DATA_2_REG = hsize;
        qspi_reg->QSPI_MANUAL_CONFIG_REG       = 0x200000 | WRITE_TRIGGER | (256 << 3);

        if (dma_flags & USE_UDMA_MODE) {

          RSI_UDMA_ChannelEnable(udmaHandle, (uint8_t)ch_no);
          // Enable DMA controller
          RSI_UDMA_UDMAEnable(udmaHandle);
          while (!(RSI_UDMA_ChannelIsEnabled(udmaHandle, (uint8_t)ch_no)))
            ;

        } else {
          gpdma_dma_channel_trigger(gpdmaHandle, (uint8_t)ch_no);
          while ((gpdma_channel_is_enabled(gpdmaHandle, (uint8_t)ch_no)))
            ;
        }

        loop_count -= 256;
        qspi_wait_flash_status_Idle(qspi_reg, spi_config, cs_no);

      } else {
        if (hsize == _8BIT) {
          qspi_write_to_flash(qspi_reg, 8, *(data), cs_no);
          data++;
          loop_count -= 1;
        } else if (hsize == _16BIT) {
          qspi_write_to_flash(qspi_reg, 16, *(data_16bit), cs_no);
          data_16bit++;
          loop_count -= 2;
        } else if (hsize == _32BIT) {
          qspi_write_to_flash(qspi_reg, 32, *(data_32bit), cs_no);
          data_32bit++;
          loop_count -= 4;
        }
      }
    }

    // if hardware control needs to be disabled, do it here
    if (dis_hw_ctrl) {
      qspi_reg->QSPI_MANUAL_CONFIG_REG &= ~HW_CTRL_MODE;
      while (qspi_reg->QSPI_STATUS_REG & HW_CTRLD_QSPI_MODE_CTRL_SCLK)
        ;
    }

    // dual flash mode
    if (spi_config->spi_config_4.dual_flash_mode) {
      qspi_reg->OCTA_SPI_BUS_CONTROLLER2 |= BIT(19);
      spi_config->spi_config_3.wr_data_mode = (unsigned int)(qspi_data_mode & 0x03);
      qspi_switch_qspi2(qspi_reg, spi_config->spi_config_3.wr_data_mode, cs_no);
    }

    qspi_reg->QSPI_MANUAL_CONFIG_2_REG &= (uint32_t)~0xF;
    qspi_switch_qspi2(qspi_reg, spi_config->spi_config_1.inst_mode, cs_no);
    qspi_wait_flash_status_Idle(qspi_reg, spi_config, cs_no);

    // if required (non-zero) wait for the delay time
    if (wr_reg_delay_ms) {
      qspi_usleep(wr_reg_delay_ms);
    }

    // if hardware control was disabled, enable it here
    if (dis_hw_ctrl) {
      qspi_reg->QSPI_MANUAL_CONFIG_REG |= HW_CTRL_MODE;
      while (!(qspi_reg->QSPI_STATUS_REG & HW_CTRLD_QSPI_MODE_CTRL_SCLK))
        ;
    }
  }

// disable write command is issued
#ifdef CHIP_9118
  if (spi_config->spi_config_3.ddr_mode_en) {
    if (flash_type == MX_OCTA_FLASH) {
      qspi_write_to_flash(qspi_reg, QSPI_16BIT_LEN, (WRDI << 8) | WRDI2, cs_no);
    } else {
      qspi_write_to_flash(qspi_reg, QSPI_8BIT_LEN, WRDI, cs_no);
    }
  } else {
    qspi_write_to_flash(qspi_reg, CMD_LEN, WRDI, cs_no);
    if (spi_config->spi_config_3._16bit_cmd_valid) {
      qspi_write_to_flash(qspi_reg, CMD_LEN, WRDI2, cs_no);
    }
  }
#else
  qspi_write_to_flash(qspi_reg, CMD_LEN, WRDI, cs_no);
  if (spi_config->spi_config_3._16bit_cmd_valid) {
    qspi_write_to_flash(qspi_reg, CMD_LEN, WRDI2, cs_no);
  }
#endif
  DEASSERT_CSN;
  if (spi_config->spi_config_4.dual_flash_mode) {
    qspi_reg->OCTA_SPI_BUS_CONTROLLER2 &= ~BIT(19);
  }
  if (prev_state == 1) {
    qspi_reg->QSPI_BUS_MODE_REG |= AUTO_MODE;
    while (!(qspi_reg->QSPI_STATUS_REG & AUTO_MODE_ENABLED))
      ;
  }
  if (spi_config->spi_config_4.continue_fetch_en) {
    qspi_reg->QSPI_AUTO_CONITNUE_FETCH_CTRL_REG |= CONTINUE_FETCH_EN;
  }

  if (check_en) {
    if (qspi_reg == (qspi_reg_t *)TA_QSPI_BASE_ADDRESS) {
      auto_mode_address = TA_QSPI_AUTOM_CHIP0_ADDRESS;
    } else if (qspi_reg == (qspi_reg_t *)M4_QSPI_BASE_ADDRESS) {
      auto_mode_address = M4_QSPI_AUTOM_CHIP0_ADDRESS;
    }
    len_in_loop = 0;
    while (length >= 4) {
      if (*(volatile uint32_t *)(auto_mode_address + address + len_in_loop)
          == *(uint32_t *)((uint32_t)data_in + len_in_loop)) {
        len_in_loop += 4;
        length -= 4;
      } else {
        status = (uint32_t)RSI_FAIL;
        break;
      }
    }

    while (length > 0) {
      if (*(volatile uint8_t *)(auto_mode_address + address + len_in_loop)
          == *(uint8_t *)((uint32_t)data_in + len_in_loop)) {
        len_in_loop += 1;
        length -= 1;
      } else {
        status = (uint32_t)RSI_FAIL;
        break;
      }
    }
  }
  return status;
}

/*==============================================*/
/**
 * @fn           void qspi_spi_read(qspi_reg_t *qspi_reg, 
                   spi_config_t *spi_config,
                   uint32_t addr,
                   uint8_t *data,
                   uint32_t hsize,
                   uint32_t len_in_bytes,
                   uint32_t dma_flags,
                   void *udmaHandle, 
                   void *gpdmaHandle)
 * @brief        This API used to qspi read command sent to flash
 * @param[in]    qspi_reg        :   qsi register pointer to the qspi read data
 * @param[in]    spi_config      :   pointer to the SPI configuration
 * @param[in]    addr            :   address 
 * @param[in]    data            :   pointer to the write
 * @param[in]    hsize           :   size
 * @param[in]    len_in_bytes    :   length in bytes
 * @param[in]    dma_flags       :   DMA flags
 * @param[in]    udmaHandle      :   pointer to DMA controller
 * @param[in]    gpdmaHandle     :   pointer to general purpose DMA controller 
 * @return       none
 */
void qspi_spi_read(qspi_reg_t *qspi_reg,
                   spi_config_t *spi_config,
                   uint32_t addr,
                   uint8_t *data,
                   uint32_t hsize,
                   uint32_t len_in_bytes,
                   uint32_t dma_flags,
                   void *udmaHandle,
                   void *gpdmaHandle)
{
  if (!spi_config->spi_config_2.auto_mode) {
    // if not auto mode call manual read function
    qspi_manual_read(qspi_reg, spi_config, addr, data, hsize, len_in_bytes, dma_flags, udmaHandle, gpdmaHandle);
  } else {
    // if auto mode call auto read function
    qspi_auto_read(spi_config->spi_config_2.cs_no, addr, data, hsize, len_in_bytes, spi_config, dma_flags);
  }
}

/*==============================================*/
/** 
 *  @fn      void RSI_QSPI_TIMER_Config(void)
 *  @brief   This API is used to configure the qspi timer.
 *  @return  none
 */
void RSI_QSPI_TIMER_Config(void)
{
  // Timer clock config 32Mhz clock
  ulpss_time_clk_config(ULPCLK, ENABLE_STATIC_CLK, 0, ULP_TIMER_MHZ_RC_CLK, 1);
  // Sets periodic mode
  RSI_TIMERS_SetTimerMode(TIMERS, PERIODIC_TIMER, TIMER_0);
  // Sets timer in 1 Micro second mode
  RSI_TIMERS_SetTimerType(TIMERS, MICRO_SEC_MODE, TIMER_0);
  // 1 Micro second timer configuration
  // Micro sec clock is 32 MHZ, but it may vary from 20MHZ to 47MHZ.
  // So we are programming max freq  for Timer to configure Time Period
  // FIXME , Option to configure from mbr
  RSI_TIMERS_MicroSecTimerConfig(TIMERS, TIMER_0, 80, 0, MICRO_SEC_MODE);
}

/*==============================================*/
/** 
 * @fn         void qspi_usleep(uint32_t delay)
 * @brief      This API is used to micro second delay by timer interrupt.
 * @param[in]  delay   : delay
 * @return     none
 */
void qspi_usleep(uint32_t delay)
{
  // Micro seconds delay
  RSI_TIMERS_SetMatch(TIMERS, TIMER_0, delay);
  // Start timer
  RSI_TIMERS_TimerStart(TIMERS, TIMER_0);
  // Wait for time out
  while (!RSI_TIMERS_InterruptStatus(TIMERS, TIMER_0))
    ;
}

#if defined(SLI_SI917)
#if defined(SLI_SI917B0)
void qspi_qspiload_key(qspi_reg_t *qspi_reg,
                       uint8_t mode,
                       uint32_t *key1,
                       uint32_t *key2,
                       uint32_t key_len,
                       uint32_t kh_enable)
{
  qspi_reg->QSPI_AES_CONFIG = mode | KEY_FLIP_FOR_REG_INTF | KEY_FLIP_FOR_KH_INTF;
  if (key_len == 32) {
    qspi_reg->QSPI_AES_CONFIG |= QSPI_KEY_SIZE_256;
  }
  if (kh_enable) {
    qspi_reg->QSPI_AES_SEC_KEY_FRM_KH |= LOAD_SEC_KEY_FRM_KH;
    while (qspi_reg->QSPI_AES_SEC_KEY_FRM_KH & LOAD_SEC_KEY_FRM_KH)
      ;
    qspi_reg->OCTA_SPI_BUS_CONTROLLER2 |= EN_KH_KEY | EN_SECURITY; // enabling security ;
  } else {
    qspi_reg->QSPI_AES_KEY1_10_13 = key1[3];
    qspi_reg->QSPI_AES_KEY1_14_17 = key1[2];
    qspi_reg->QSPI_AES_KEY1_18_1B = key1[1];
    qspi_reg->QSPI_AES_KEY1_1C_1F = key1[0];
    if (key_len == 32) {
      qspi_reg->QSPI_AES_KEY1_0_3 = key1[7];
      qspi_reg->QSPI_AES_KEY1_4_7 = key1[6];
      qspi_reg->QSPI_AES_KEY1_8_B = key1[5];
      qspi_reg->QSPI_AES_KEY1_C_F = key1[4];
    }
    if (mode == XTS_MODE) {
      qspi_reg->QSPI_AES_KEY2_10_13 = key2[3];
      qspi_reg->QSPI_AES_KEY2_14_17 = key2[2];
      qspi_reg->QSPI_AES_KEY2_18_1B = key2[1];
      qspi_reg->QSPI_AES_KEY2_1C_1F = key2[0];
      if (key_len == 32) {
        qspi_reg->QSPI_AES_KEY2_0_3 = key2[7];
        qspi_reg->QSPI_AES_KEY2_4_7 = key2[6];
        qspi_reg->QSPI_AES_KEY2_8_B = key2[5];
        qspi_reg->QSPI_AES_KEY2_C_F = key2[4];
      }
    }
    qspi_reg->OCTA_SPI_BUS_CONTROLLER2 |= EN_SECURITY; // enabling security
  }
  if (key_len == 32) {
    qspi_reg->QSPI_AES_KEY_IV_VALID = (LB_IV_VALID | KEY1_VALID_256);
  } else {
    qspi_reg->QSPI_AES_KEY_IV_VALID = (LB_IV_VALID | KEY1_VALID_128);
  }
  if (mode == XTS_MODE) {
    if (key_len == 32) {
      qspi_reg->QSPI_AES_KEY_IV_VALID |= (KEY2_VALID_256);
    } else {
      qspi_reg->QSPI_AES_KEY_IV_VALID |= (KEY2_VALID_128);
    }
    qspi_reg->QSPI_AES_CONFIG |= DECRYPT_KEY_CAL;
  }
}
#else
/* @fn          void qspi_qspiload_key(qspi_reg_t *qspi_reg, uint8_t mode, uint32_t *key, uint32_t kh_enable)
 * @brief       This API is used to load the security key.
 * @param[in]   qspi_reg       :   qsi register pointer to the qspi read data
 * @param[in]   mode           :   mode
 * @param[in]   key            :   pointer to security AES key
 * @param[in]   kh_enable      :   enable key
 * @return      none
 */
void qspi_qspiload_key(qspi_reg_t *qspi_reg, uint8_t mode, uint32_t *key, uint32_t kh_enable)
{
  uint32_t key_valid;
  qspi_reg->QSPI_AES_CONFIG = mode | KEY_FLIP_FOR_REG_INTF | KEY_FLIP_FOR_KH_INTF; // QSPI_AES_SOFT_RESET
  ;
  if (kh_enable) {
    qspi_reg->QSPI_AES_SEC_KEY_FRM_KH |= LOAD_SEC_KEY_FRM_KH;
    while (qspi_reg->QSPI_AES_SEC_KEY_FRM_KH & LOAD_SEC_KEY_FRM_KH)
      ;
    qspi_reg->OCTA_SPI_BUS_CONTROLLER2 |= EN_KH_KEY | EN_SECURITY; // enabling security ;
  } else {
    qspi_reg->QSPI_AES_KEY1_0_3 = key[3];
    qspi_reg->QSPI_AES_KEY1_4_7 = key[2];
    qspi_reg->QSPI_AES_KEY1_8_B = key[1];
    qspi_reg->QSPI_AES_KEY1_C_F = key[0];
    if (mode == XTS_MODE) {
      qspi_reg->QSPI_AES_KEY2_0_3 = key[7];
      qspi_reg->QSPI_AES_KEY2_4_7 = key[6];
      qspi_reg->QSPI_AES_KEY2_8_B = key[5];
      qspi_reg->QSPI_AES_KEY2_C_F = key[4];
    }
    qspi_reg->OCTA_SPI_BUS_CONTROLLER2 |= EN_SECURITY; // enabling security
  }
  qspi_reg->QSPI_AES_KEY_IV_VALID = (IV_VALID | KEY1_VALID);
  if (mode == XTS_MODE) {
    qspi_reg->QSPI_AES_KEY_IV_VALID |= (KEY2_VALID);
    qspi_reg->QSPI_AES_CONFIG |= DECRYPT_KEY_CAL;
  }
}
#endif
#endif
#ifdef CHIP_9118

/*==============================================*/
/** 
 * @fn          void qspi_qspiload_key(qspi_reg_t *qspi_reg, uint32_t *key, uint32_t kh_enable)
 * @brief       This API is used to load the security key.
 * @param[in]   qspi_reg       :   qsi register pointer to the qspi read data
 * @param[in]   key            :   pointer to security AES key
 * @param[in]   kh_enable      :   enable key
 * @return      none
 */
void qspi_qspiload_key(qspi_reg_t *qspi_reg, uint32_t *key, uint32_t kh_enable)
{
  qspi_reg->QSPI_AES_KEY_0_3 = *key++;
  qspi_reg->QSPI_AES_KEY_4_7 = *key++;
  qspi_reg->QSPI_AES_KEY_8_B = *key++;
  qspi_reg->QSPI_AES_KEY_C_F = *key;
  qspi_reg->OCTA_SPI_BUS_CONTROLLER2 |= EN_SECURITY; // enabling security
}
#endif

/*==============================================*/
/** 
 * @fn          void qspi_qspiload_nonce(qspi_reg_t *qspi_reg, uint32_t *nonce)
 * @brief       This API is used to load the bus controller.
 * @param[in]   qspi_reg       :   qsi register pointer to the qspi read data
 * @param[in]   nonce          :   pointer to bus controller
 * @return      none 
 */
void qspi_qspiload_nonce(qspi_reg_t *qspi_reg, uint32_t *nonce)
{
  (void)qspi_reg;
  (void)nonce;
#ifdef CHIP_9118
  qspi_reg->QSPI_AES_NONCE_0_3 = *nonce++;
  qspi_reg->QSPI_AES_NONCE_4_7 = *nonce++;
  qspi_reg->QSPI_AES_NONCE_8_B = *nonce;
  qspi_reg->OCTA_SPI_BUS_CONTROLLER2 |= NONCE_INIT; // nonce init
#endif
}
#ifdef CHIP_9118

/*==============================================*/
/** 
 * @fn          void qspi_seg_sec_en(qspi_reg_t *qspi_reg, uint32_t seg_no, uint32_t start_addr, uint32_t end_addr)
 * @brief       This API is used to enabling security for segment
 * @param[in]   qspi_reg       :   qsi register pointer to the qspi enabling security
 * @param[in]   seg_no         :   segment number
 * @param[in]   start_addr     :   start address
 * @param[in]   end_addr       :   end the address 
 * @return      none
 */
void qspi_seg_sec_en(qspi_reg_t *qspi_reg, uint32_t seg_no, uint32_t start_addr, uint32_t end_addr)
{
  qspi_reg->OCTA_SPI_BUS_CONTROLLER2 |= (BIT(seg_no) << EN_SEG_SEC); // enabling security for segment

  qspi_reg->QSPI_AES_SEC_SEG_ADDR[(7 - seg_no)] = start_addr;
  qspi_reg->QSPI_AES_SEC_SEG_ADDR[(6 - seg_no)] = end_addr;
}
#endif
#if defined(SLI_SI917)

/*==============================================*/
/** 
 * @fn          void qspi_seg_sec_en(qspi_reg_t *qspi_reg, uint32_t seg_no, uint32_t start_addr, uint32_t end_addr)
 * @brief       This API is used to enabling security for segment
 * @param[in]   qspi_reg       :   qsi register pointer to the qspi enabling security
 * @param[in]   seg_no         :   segment number
 * @param[in]   start_addr     :   start address
 * @param[in]   end_addr       :   end the address 
 * @return      none
 */
void qspi_seg_sec_en(qspi_reg_t *qspi_reg, uint32_t seg_no, uint32_t start_addr, uint32_t end_addr)
{
  qspi_reg->OCTA_SPI_BUS_CONTROLLER2 |= (BIT(seg_no) << EN_SEG_SEC); // enabling security for segment
  qspi_reg->QSPI_AES_SEC_SEG_ADDR[seg_no * 2]     = start_addr;
  qspi_reg->QSPI_AES_SEC_SEG_ADDR[seg_no * 2 + 1] = end_addr;
}

/*==============================================*/
/** 
 * @fn          void qspi_semi_auto_mode_config(qspi_reg_t *qspi_reg, uint32_t addr, uint32_t hsize, uint32_t bsize, uint32_t length)
 * @brief       This API is used to qspi configuration semi auto mode
 * @param[in]   qspi_reg       :   qsi register pointer to the qspi semi auto mode
 * @param[in]   addr           :   address
 * @param[in]   hsize          :   h size
 * @param[in]   bsize          :   b size
 * @param[in]   length         :   length of semi auto mode 
 * @return      none
 */
void qspi_semi_auto_mode_config(qspi_reg_t *qspi_reg, uint32_t addr, uint32_t hsize, uint32_t bsize, uint32_t length)
{
  while (qspi_reg->QSPI_STATUS_REG & BUSY)
    ; // waiting till MANUAL mode becomes IDLE
  qspi_reg->QSPI_SEMI_AUTO_ADDR_REG = addr;
  qspi_reg->QSPI_SEMI_AUTO_MODE_CONFIG_REG |= ((hsize << 8) | bsize);
  qspi_reg->QSPI_SEMI_AUTO_MODE_CONFIG2_REG = (length | SEMI_AUTO_MODE_EN);
}
#endif

/*==============================================*/
/** 
 * @fn          void RSI_QSPI_ProtectAdesto(spi_config_t *spi_config, qspi_reg_t *qspi_reg, uint32_t protection, uint32_t cs_no)
 * @brief       This API is used to qspi  protection of adesto
 * @param[in]   spi_config     :   pointer to the SPI configuration 
 * @param[in]   qspi_reg       :   qsi register pointer to the qspi semi auto mode
 * @param[in]   protection     :   protection
 * @param[in]   cs_no          :   chip select number
 * @return      none
 */
void RSI_QSPI_ProtectAdesto(spi_config_t *spi_config, qspi_reg_t *qspi_reg, uint32_t protection, uint32_t cs_no)
{
  uint32_t addr = 0;
  uint32_t reset_prot;
  if (!protection) {
    qspi_status_reg_write(qspi_reg, 0, spi_config, 0);
    return;
  } else if (protection == 0xFF) {
    qspi_status_reg_write(qspi_reg, 0xff, spi_config, 0);
    return;
  }
  reset_prot = protection >> 4;
  protection = protection & 0xf;

  while (protection--) {
    qspi_write_to_flash(qspi_reg, QSPI_8BIT_LEN, WREN, cs_no);
    DEASSERT_CSN;

    qspi_write_to_flash(qspi_reg, QSPI_8BIT_LEN, ADEST_PROTECT_CMD, cs_no);

    qspi_write_to_flash(qspi_reg, QSPI_32BIT_ADDR, addr, cs_no);

    DEASSERT_CSN;
    addr = addr + (256 * 1024);
  }

  while (reset_prot--) {
    qspi_write_to_flash(qspi_reg, QSPI_8BIT_LEN, WREN, cs_no);
    DEASSERT_CSN;

    qspi_write_to_flash(qspi_reg, QSPI_8BIT_LEN, ADEST_UNPROTECT_CMD, cs_no);

    qspi_write_to_flash(qspi_reg, QSPI_32BIT_ADDR, addr, cs_no);

    DEASSERT_CSN;
    addr = addr + (256 * 1024);
  }
}

/*==============================================*/
/**
 * @fn           void qspi_flash_protection(spi_config_t *spi_config, qspi_reg_t *qspi_reg, uint32_t prot, uint32_t wr_reg_delay_ms)
 * @brief        This API used to qspi status register for read
 * @param[in]    spi_config      :   pointer to the SPI configuration 
 * @param[in]    qspi_reg        :   qsi register pointer to the qspi read data 
 * @param[in]    prot            :   protection
 * @param[in]    wr_reg_delay_ms :   register delay in milisecond
 * @return       none
 */
void qspi_flash_protection(spi_config_t *spi_config, qspi_reg_t *qspi_reg, uint32_t prot, uint32_t wr_reg_delay_ms)
{
  uint32_t flash_status = 0, flash_status2 = 0, i = 0;
  uint32_t cs_no         = spi_config->spi_config_2.cs_no;
  uint32_t valid_bits    = spi_config->spi_config_4.valid_prot_bits;
  uint32_t prot_bit_mask = 0;
  uint32_t qspi_operational_mode;

  while (valid_bits--) {
    prot_bit_mask |= BIT(i++);
  }

  qspi_operational_mode = QSPI_MANUAL_BUS_SIZE(cs_no);
  if (qspi_operational_mode != spi_config->spi_config_1.inst_mode) {
    qspi_switch_qspi2(qspi_reg, spi_config->spi_config_1.inst_mode, cs_no);
  }

  if (spi_config->spi_config_1.flash_type == GIGA_DEVICE_FLASH) {
    flash_status = qspi_wait_flash_status_Idle(qspi_reg, spi_config, wr_reg_delay_ms);

    if (((flash_status >> 2) & prot_bit_mask) == prot) {
      goto skip_status_reg_write;
    }
    flash_status2 = qspi_flash_reg_read(qspi_reg, 0x35, cs_no, spi_config);

    flash_status = (flash_status << 8);
    flash_status |= flash_status2;
    prot_bit_mask = (prot_bit_mask << 10);
    prot          = prot << 10;
    flash_status &= ~(prot_bit_mask);
    prot = (prot & prot_bit_mask);
    flash_status |= prot;
    wr_reg_delay_ms |= BIT(30);
  } else if (spi_config->spi_config_1.flash_type == ADESTO_OCTA_FLASH) { // for adesto flash
    RSI_QSPI_ProtectAdesto(spi_config, qspi_reg, prot, cs_no);
    goto skip_status_reg_write;

  } else {
    flash_status = qspi_wait_flash_status_Idle(qspi_reg, spi_config, wr_reg_delay_ms);
    if (((flash_status >> 2) & prot_bit_mask) == prot) {
      goto skip_status_reg_write;
    }
    prot_bit_mask = (prot_bit_mask << 2);
    prot          = prot << 2;
    flash_status &= ~(prot_bit_mask); // 2 bytes reversed
    prot = (prot & prot_bit_mask);
    flash_status |= prot;
  }

  qspi_status_reg_write(qspi_reg, flash_status, spi_config, wr_reg_delay_ms);
skip_status_reg_write:
  qspi_switch_qspi2(qspi_reg, qspi_operational_mode, cs_no);
}
#ifdef CHIP_9118

/*==============================================*/
/**
 * @fn           void RSI_QSPI_DdrPadConfig()       
 * @brief        This API used to qspi data configuration
 * @return       none
 */
void RSI_QSPI_DdrPadConfig()
{
  M4_DDR_PAD_CONFIG(qspi_ddr_data_0) = 0x00;
  M4_DDR_PAD_CONFIG(qspi_ddr_data_1) = 0x00;
  M4_DDR_PAD_CONFIG(qspi_ddr_data_2) = 0x00;
  M4_DDR_PAD_CONFIG(qspi_ddr_data_3) = 0x00;
  M4_DDR_PAD_CONFIG(qspi_ddr_data_4) = 0x00;
  M4_DDR_PAD_CONFIG(qspi_ddr_data_5) = 0x00;
  M4_DDR_PAD_CONFIG(qspi_ddr_data_6) = 0x00;
  M4_DDR_PAD_CONFIG(qspi_ddr_data_7) = 0x00;
  M4_DDR_PAD_CONFIG(qspi_ddr_csn)    = 0x00;
  M4_DDR_PAD_CONFIG(qspi_ddr_clk)    = 0x00;
  M4_DDR_PAD_CONFIG(qspi_ddr_dqs)    = 0x00;
  M4_DDR_PAD_CONFIG(smih_wp)         = 0x00;
}

/*==============================================*/
/**
 * @fn           void RSI_QSPI_DdrPad()   
 * @brief        This API used to qspi octo mode control
 * @return       none
 */
void RSI_QSPI_DdrPad()
{
  M4SS_QSPI_OCTA_MODE_CTRL = BIT(0);
  RSI_QSPI_DdrPadConfig();
  M4SS_QSPI_RX_DLL_TEST_REG = 0x8;
}
#endif
/* ROM API Structure
const ROM_QSPI_API_T qspi_api =
{
		&qspi_write_to_flash,
		&qspi_switch_qspi2,
		&qspi_wait_flash_status_Idle,
		&qspi_enable_status_reg_write,
		&qspi_status_reg_write,
		&qspi_flash_reg_read,
		&qspi_flash_reg_write,
		&qspi_set_flash_mode,
		&qspi_config_qflash4_read,
		&qspi_manual_read,
		&qspi_auto_init,
		&qspi_auto_read,
		&qspi_flash_init,
		&qspi_spi_init,
		&qspi_spi_erase,
		&qspi_spi_write,
		&qspi_spi_read,
		&qspi_usleep,
		&qspi_write_block_protect,
		&qspi_qspiload_key,
		&qspi_qspiload_nonce,
		&qspi_seg_sec_en,
		&qspi_status_control_reg_write,
		&qspi_flash_protection,
		&RSI_QSPI_ConfigQspiDll,
		&RSI_QSPI_ResetFlash,
		&RSI_QSPI_UpdateOperatingMode_and_ResetType,
};
*/
#else
typedef int dummy; // To remove empty translation unit warning.
#endif // QSPI_ROMDRIVER_PRESENT

static void qspi_aes_encrypt_decrypt_standalone(qspi_reg_t *qspi_reg,
                                                uint8_t aes_mode,
                                                bool encrypt,
                                                uint32_t *in_data,
                                                uint32_t *out_data,
                                                uint32_t *key1,
                                                uint32_t *key2,
                                                uint32_t key_len,
                                                uint32_t *iv,
                                                bool kh_enable,
                                                uint32_t data_length,
                                                uint32_t flip_data)
{
  // Read the QSPI aes config data
  uint32_t aes_config_data = qspi_reg->QSPI_AES_CONFIG;
  //! Configuring mode, key flipping and enabling standalone aes with mode of encryption or decryption.
  qspi_reg->QSPI_AES_CONFIG = aes_mode | KEY_FLIP_FOR_REG_INTF | KEY_FLIP_FOR_KH_INTF;
  qspi_reg->QSPI_AES_CONFIG |= ((encrypt << 14) | EN_STANDALONE_AES | (flip_data ? FLIP_IN_LB : 0));
  ;
  if (key_len == KEY_LEN_256) {
    qspi_reg->QSPI_AES_CONFIG |= QSPI_KEY_SIZE_256;
  }
  //! If XTS mode is enabled with decryptiong, setting DECREYPT KEY CAL bit.
  if ((aes_mode == XTS_MODE) && encrypt)
    qspi_reg->QSPI_AES_CONFIG |= DECRYPT_KEY_CAL;

  //! If Key is present in keyholder, then configure qspi to fetch from keyholder.
  if (kh_enable) {
    qspi_reg->QSPI_AES_SEC_KEY_FRM_KH |= LOAD_SEC_KEY_FRM_KH;
    while (qspi_reg->QSPI_AES_SEC_KEY_FRM_KH & LOAD_SEC_KEY_FRM_KH)
      ;
    qspi_reg->OCTA_SPI_BUS_CONTROLLER2 |= EN_KH_KEY; // enabling security ;
  } else {
    qspi_reg->QSPI_AES_KEY1_10_13 = key1[3];
    qspi_reg->QSPI_AES_KEY1_14_17 = key1[2];
    qspi_reg->QSPI_AES_KEY1_18_1B = key1[1];
    qspi_reg->QSPI_AES_KEY1_1C_1F = key1[0];
    if (key_len == KEY_LEN_256) {
      qspi_reg->QSPI_AES_KEY1_0_3 = key1[7];
      qspi_reg->QSPI_AES_KEY1_4_7 = key1[6];
      qspi_reg->QSPI_AES_KEY1_8_B = key1[5];
      qspi_reg->QSPI_AES_KEY1_C_F = key1[4];
    }
    if (aes_mode == XTS_MODE) {
      qspi_reg->QSPI_AES_KEY2_10_13 = key2[3];
      qspi_reg->QSPI_AES_KEY2_14_17 = key2[2];
      qspi_reg->QSPI_AES_KEY2_18_1B = key2[1];
      qspi_reg->QSPI_AES_KEY2_1C_1F = key2[0];
      if (key_len == KEY_LEN_256) {
        qspi_reg->QSPI_AES_KEY2_0_3 = key2[7];
        qspi_reg->QSPI_AES_KEY2_4_7 = key2[6];
        qspi_reg->QSPI_AES_KEY2_8_B = key2[5];
        qspi_reg->QSPI_AES_KEY2_C_F = key2[4];
      }
    }
  }
  //! Loading Key valids to AES.
  if (key_len == KEY_LEN_256) {
    qspi_reg->QSPI_AES_KEY_IV_VALID = (LB_IV_VALID | KEY1_VALID_256);
  } else {
    qspi_reg->QSPI_AES_KEY_IV_VALID = (LB_IV_VALID | KEY1_VALID_128);
  }
  if (key_len == KEY_LEN_256) {
    qspi_reg->QSPI_AES_KEY_IV_VALID = (LB_IV_VALID | KEY1_VALID_256);
  }
  if (aes_mode == XTS_MODE) {
    if (key_len == KEY_LEN_256) {
      qspi_reg->QSPI_AES_KEY_IV_VALID |= (KEY2_VALID_256);
    } else {
      qspi_reg->QSPI_AES_KEY_IV_VALID |= (KEY2_VALID_128);
    }
    if (encrypt) {
      //! Resetting KEY valids to QSPI in XTS mode sothat pre key calculation doesn't happen for every block.
      qspi_reg->QSPI_AES_KEY_IV_VALID = LB_IV_VALID;
    }
  }

  if (data_length % 16 != 0)
    return;

  while (data_length != 0) {
    //! Feeding IV(address of flash ) for every block as we do decryption with address of flash
    qspi_reg->QSPI_AES_IV1_0_3 = iv[0];
    //! Feeding data to AES after DIN ready is high
    while (!(qspi_reg->QSPI_LB_STATUS & QSPI_AES_DIN_READY))
      ;
    qspi_reg->QSPI_AES_LB_DATA_C_F = in_data[0];
    qspi_reg->QSPI_AES_LB_DATA_8_B = in_data[1];
    qspi_reg->QSPI_AES_LB_DATA_4_7 = in_data[2];
    qspi_reg->QSPI_AES_LB_DATA_0_3 = in_data[3];

    //! RESETTING DECRYPT KEY CAL to avoid pre key calculation for every block.
    qspi_reg->QSPI_AES_CONFIG &= ~DECRYPT_KEY_CAL;
    //! SETTING this bit to indicate QSPI that FW is ready to take data from QSPI AES.
    qspi_reg->QSPI_LB_STATUS |= BIT(0);
    //! Fetching data from AES after checking for DATA VALID.
    while (!(qspi_reg->QSPI_LB_STATUS & QSPI_AES_DOUT_VALID))
      ;
    out_data[0] = qspi_reg->QSPI_AES_LB_DATA_C_F;
    out_data[1] = qspi_reg->QSPI_AES_LB_DATA_8_B;
    out_data[2] = qspi_reg->QSPI_AES_LB_DATA_4_7;
    out_data[3] = qspi_reg->QSPI_AES_LB_DATA_0_3;
    //! Incrementing the IV by 16 for every read of 128bit from AES as IV is used as address for QSPI INLINE DECRYPTION.
    iv[0] += 16;
    in_data += 4;
    out_data += 4;
    data_length -= 16;
  }
  // Program the QSPI AES default config data back to read data back in auto mode
  qspi_reg->QSPI_AES_CONFIG = aes_config_data;
}

/*==============================================*/
/**
 * @fn           void RSI_QSPI_Aes_Encrypt_Decrypt_Standalone()
 * @brief        This api is used to encrypt or decrypt the input data with QSPI standalone AES
 * @param[in]    qspi_reg pointer for QSPI controller reg address space
 * @param[in]    configs pointer for QSPI standalone encrypt/decrypt configs
 * @param[in]    in_data is a pointer contains the address of input data
 * @param[out]   out_data is a pointer updated after encryption or decryption
 * @param[in]    data_length is size of the image to be encrypted/decrypted, it should be multiple of 16 byptes,
 *                           if not have to pad the data with 0's and send
 * @return       error code if invalid parameter else RSI_OK
 *
 * @section description
 * This api is used to encrypt or decrypt the input data with QSPI standalone AES
 */
uint32_t RSI_QSPI_Aes_Encrypt_Decrypt_Standalone(qspi_reg_t *qspi_reg,
                                                 qspi_standalone_config_t *configs,
                                                 uint32_t *in_data,
                                                 uint32_t *out_data,
                                                 uint32_t data_length)
{
  uint32_t status = RSI_OK;
  // Check for valid aes mode
  if ((configs->aes_mode != CTR_MODE) && (configs->aes_mode != XTS_MODE)) {
    return INVALID_PARAMETERS;
  }
  // Check in_data and out_data buffers are valids
  if ((in_data == NULL) || (out_data == NULL)) {
    return INVALID_PARAMETERS;
  }
  // if kh_enable i.e key from keyholder then key1, key2 and key lenth are not required else validate those are proper
  if (!configs->kh_enable) {
    // Check key1 is valid
    if (configs->key1 == NULL)
      return INVALID_PARAMETERS;
    // Check key2 is valid only when key size is 32 bit
    if ((configs->key_len == KEY_LEN_256) && (configs->key2 == NULL))
      return INVALID_PARAMETERS;
    // Check for key length is proper or not
    if ((configs->key_len != KEY_LEN_128) && (configs->key_len != KEY_LEN_256))
      return INVALID_PARAMETERS;
  }

  if ((data_length % 16) != 0)
    return INVALID_PARAMETERS;

  // QSPI standalone aes encryption/decryption api
  qspi_aes_encrypt_decrypt_standalone(qspi_reg,
                                      configs->aes_mode,
                                      configs->encrypt,
                                      in_data,
                                      out_data,
                                      configs->key1,
                                      configs->key2,
                                      configs->key_len,
                                      configs->iv,
                                      configs->kh_enable,
                                      data_length,
                                      configs->flip_data);
  return status;
}
