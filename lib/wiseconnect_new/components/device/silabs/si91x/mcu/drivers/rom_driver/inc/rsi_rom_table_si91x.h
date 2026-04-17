/******************************************************************************
* @file  rsi_rom_table_si91x.h
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
#ifndef __RSI_ROM_TABLE_RS9116_H__
#define __RSI_ROM_TABLE_RS9116_H__
#include "rsi_ccp_user_config.h"
#if defined(A11_ROM)
#include <stdint.h>
#include "rsi_error.h"
#include "rsi_timers.h"
#include "rsi_ulpss_clk.h"
#include "rsi_pll.h"
#include "rsi_power_save.h"
#include "rsi_egpio.h"
#include "rsi_crc.h"
#include "rsi_ulpss_clk.h"
#include "rsi_qspi_proto.h"
#include "rsi_qspi.h"
#include "rsi_rng.h"
#include "rsi_gpdma.h"
#include "rsi_ct.h"
#include "rsi_timers.h"
#include "rsi_udma.h"
#include "rsi_reg_spi.h"
#include "rsi_processor_sensor.h"
#include "rsi_retention.h"
#include "rsi_temp_sensor.h"
#include "rsi_time_period.h"
#include "rsi_wwdt.h"
#include "rsi_ipmu.h"
#if !defined(SLI_SI917B0)
#include "rsi_efuse.h"
#include "rsi_pwm.h"
#endif
#include "rsi_udma_wrapper.h"
#include "UDMA.h"
#include "USART.h"
#include "GSPI.h"
#include "SAI.h"
#include "I2C.h"
#include "SPI.h"
#ifdef __cplusplus
extern "C" {
#endif
/* @brief  ROM indirect function structure */

///////////////////EGPIO//////////////////////////
typedef struct {
  void (*egpio_set_dir)(EGPIO_Type *pEGPIO, uint8_t port, uint8_t pin, boolean_t dir);
  void (*egpio_set_pin)(EGPIO_Type *pEGPIO, uint8_t port, uint8_t pin, uint8_t val);
  boolean_t (*egpio_get_pin)(const EGPIO_Type *pEGPIO, uint8_t port, uint8_t pin);
  boolean_t (*egpio_get_dir)(const EGPIO_Type *pEGPIO, uint8_t port, uint8_t pin);
  void (*egpio_pin_int_sel)(EGPIO_Type *pEGPIO, uint8_t intCh, uint8_t port, uint8_t pin);
  void (*egpio_set_int_fall_edge_enable)(EGPIO_Type *pEGPIO, uint8_t intCh);
  void (*egpio_set_int_fall_edge_disable)(EGPIO_Type *pEGPIO, uint8_t intCh);
  void (*egpio_set_int_rise_edge_enable)(EGPIO_Type *pEGPIO, uint8_t intCh);
  void (*egpio_set_int_rise_edge_disable)(EGPIO_Type *pEGPIO, uint8_t intCh);
  void (*egpio_set_int_low_level_enable)(EGPIO_Type *pEGPIO, uint8_t intCh);
  void (*egpio_int_mask)(EGPIO_Type *pEGPIO, uint8_t intCh);
  void (*egpio_int_un_mask)(EGPIO_Type *pEGPIO, uint8_t intCh);
  void (*egpio_set_int_low_level_disable)(EGPIO_Type *pEGPIO, uint8_t intCh);
  void (*egpio_set_int_high_level_enable)(EGPIO_Type *pEGPIO, uint8_t intCh);
  void (*egpio_set_int_high_level_disable)(EGPIO_Type *pEGPIO, uint8_t intCh);
  uint8_t (*egpio_get_int_stat)(const EGPIO_Type *pEGPIO, uint8_t intCh);
  void (*egpio_int_clr)(EGPIO_Type *pEGPIO, uint8_t intCh, uint8_t flags);
  void (*egpio_set_pin_mux)(EGPIO_Type *pEGPIO, uint8_t port, uint8_t pin, uint8_t mux);
  void (*egpio_ulp_soc_gpio_mode)(ULPCLK_Type *pULPCLK, uint8_t gpio, uint8_t mode);
  void (*egpio_set_port_mask)(EGPIO_Type *pEGPIO, uint8_t port, uint8_t pin);
  void (*egpio_set_port_un_mask)(EGPIO_Type *pEGPIO, uint8_t port, uint8_t pin);
  void (*egpio_port_masked_load)(EGPIO_Type *pEGPIO, uint8_t port, uint16_t val);
  void (*egpio_set_port)(EGPIO_Type *pEGPIO, uint8_t port, uint16_t val);
  void (*egpio_port_load)(EGPIO_Type *pEGPIO, uint8_t port, uint16_t val);
  void (*egpio_word_load)(EGPIO_Type *pEGPIO, uint8_t pin, uint16_t val);
  void (*egpio_clr_port)(EGPIO_Type *pEGPIO, uint8_t port, uint16_t val);
  void (*egpio_toggle_port)(EGPIO_Type *pEGPIO, uint8_t port, uint16_t val);
  uint16_t (*egpio_get_port)(const EGPIO_Type *pEGPIO, uint8_t port);
  void (*egpio_group_int_one_enable)(EGPIO_Type *pEGPIO, uint8_t port, uint8_t pin);
  void (*egpio_group_int_one_disable)(EGPIO_Type *pEGPIO, uint8_t port, uint8_t pin);
  void (*egpio_group_int_two_enable)(EGPIO_Type *pEGPIO, uint8_t port, uint8_t pin);
  void (*egpio_group_int_mask)(EGPIO_Type *pEGPIO, uint8_t grpInt);
  void (*egpio_group_int_un_Mask)(EGPIO_Type *pEGPIO, uint8_t grpInt);
  void (*egpio_group_int_enable)(EGPIO_Type *pEGPIO, uint8_t grpInt);
  void (*egpio_group_int_disable)(EGPIO_Type *pEGPIO, uint8_t grpInt);
  void (*egpio_group_int_level)(EGPIO_Type *pEGPIO, uint8_t grpInt);
  void (*egpio_group_int_edge)(EGPIO_Type *pEGPIO, uint8_t grpInt);
  void (*egpio_group_int_and)(EGPIO_Type *pEGPIO, uint8_t grpInt);
  void (*egpio_group_int_or)(EGPIO_Type *pEGPIO, uint8_t grpInt);
  uint32_t (*egpio_group_int_stat)(const EGPIO_Type *pEGPIO, uint8_t grpInt);
  void (*egpio_group_int_wkeup_Enable)(EGPIO_Type *pEGPIO, uint8_t grpInt);
  void (*egpio_group_int_wkeup_disable)(EGPIO_Type *pEGPIO, uint8_t grpInt);
  void (*egpio_group_int_clr)(EGPIO_Type *pEGPIO, uint8_t grpInt, uint8_t flags);
  void (*egpio_group_int_two_disable)(EGPIO_Type *pEGPIO, uint8_t port, uint8_t pin);
  void (*egpio_set_group_int_one_pol)(EGPIO_Type *pEGPIO, uint8_t port, uint8_t pin, uint8_t pol);
  void (*egpio_set_group_int_two_pol)(EGPIO_Type *pEGPIO, uint8_t port, uint8_t pin, uint8_t pol);
  void (*egpio_host_pads_gpio_mode_enable)(uint8_t u8GpioNum);
  void (*egpio_host_pads_gpio_mode_disable)(uint8_t u8GpioNum);
  void (*egpio_pad_selection_enable)(uint8_t padNum);
  void (*egpio_pad_selection_disable)(uint8_t padNum);
  void (*egpio_pad_receiver_enable)(uint8_t u8GpioNum);
  void (*egpio_pad_receiver_disable)(uint8_t u8GpioNum);
  void (*egpio_pad_sdio_connected)(void);
  void (*egpio_pad_driver_disable_state)(uint8_t u8GpioNum, en_driver_state_t endstate);
  void (*egpio_pad_driver_strength_select)(uint8_t u8GpioNum, en_driver_strength_select_t strength);
  void (*egpio_pad_power_on_start_enable)(uint8_t u8GpioNum, uint8_t val);
  void (*egpio_pad_active_high_schmitt_trigger)(uint8_t u8GpioNum, uint8_t val);
  void (*egpio_pad_slew_rate_controll)(uint8_t u8GpioNum, uint8_t val);
  void (*egpio_ulp_pad_receiver_enable)(uint8_t u8GpioNum);
  void (*egpio_ulp_pad_receiver_disable)(uint8_t u8GpioNum);
  void (*egpio_ulp_pad_driver_disable_state)(uint8_t u8GpioNum, en_ulp_driver_disable_state_t disablestate);
  void (*egpio_ulp_pad_driver_strength_select)(uint8_t u8GpioNum, en_ulp_driver_strength_select_t strength);
  void (*egpio_ulp_pad_power_on_start_enable)(uint8_t u8GpioNum, uint8_t val);
  void (*egpio_ulp_pad_active_high_schmitt_trigger)(uint8_t u8GpioNum, uint8_t val);
  void (*egpio_ulp_pad_slew_rate_controll)(uint8_t u8GpioNum, uint8_t val);
} ROM_EGPIO_API_T;
///////////////////EGPIO END//////////////////////////

/** @brief CRC ROM indirect function structure */
typedef PRE_PACK struct POST_PACK {
  void (*crc_set_gen_control)(CRC_Type *pCRC);
  uint32_t (*crc_get_gen_status)(CRC_Type *pCRC);
  void (*crc_polynomial)(CRC_Type *pCRC, RSI_CRC_PARAMS_T *pCRC_Params);
  uint32_t (*crc_polynomial_width)(CRC_Type *pCRC, RSI_CRC_PARAMS_T *pCRC_Params);
  void (*crc_lfsr_init)(CRC_Type *pCRC, RSI_CRC_PARAMS_T *pCRC_Params);
  uint32_t (*crc_use_swapped_init)(CRC_Type *pCRC, RSI_CRC_PARAMS_T *pCRC_Params);
  uint32_t (*crc_set_data_width_type)(CRC_Type *pCRC, RSI_CRC_PARAMS_T *pCRC_Params);
  uint32_t (*crc_set_fifo_thresholds)(CRC_Type *pCRC, RSI_CRC_PARAMS_T *pCRC_Params);
  uint32_t (*crc_write_data)(CRC_Type *pCRC, RSI_CRC_PARAMS_T *pCRCParams, uint32_t data);
  uint32_t (*monitor_crc_calc)(CRC_Type *pCRC, RSI_CRC_PARAMS_T *pCRC_Params);
  void (*crc_lfsr_dynamic_write)(CRC_Type *pCRC, RSI_CRC_PARAMS_T *pCRCParams);
  void (*crc_reset_fifo)(CRC_Type *pCRC);
  uint32_t (*crc_get_fifo_status)(CRC_Type *pCRC);
} ROM_CRC_API_T;
//////////////CRC END/////////////////////////////

/** @brief RNG ROM indirect function structure */
typedef PRE_PACK struct POST_PACK {
  uint32_t (*rng_start)(HWRNG_Type *pRNG, uint8_t rngMode);
  void (*rng_stop)(HWRNG_Type *pRNG);
  void (*rng_get_bytes)(HWRNG_Type *pRNG, uint32_t *randomBytes, uint32_t numberOfBytes);
} ROM_RNG_API_T;
///////////////RNG END//////////////////
#if !defined(SLI_SI917B0)
/* @brief  ROM indirect function structure */
typedef PRE_PACK struct POST_PACK {
  void (*efuse_enable)(EFUSE_Type *pstcEfuse);
  void (*efuse_Disable)(EFUSE_Type *pstcEfuse);
  uint8_t (*efuse_read_data)(EFUSE_Type *pstcEfuse);
  void (*efuse_write_addr)(EFUSE_Type *pstcEfuse, uint16_t u16Addr);
  rsi_error_t (*efuse_write_bit)(EFUSE_Type *pstcEfuse, uint16_t u16Addr, uint8_t u8BitPos, uint32_t hold_time);
  rsi_error_t (*efuse_fsm_read_byte)(EFUSE_Type *pstcEfuse, uint16_t u16Addr, uint8_t *pu8Byte, uint32_t SocClk);
  rsi_error_t (*efuse_mem_map_read_byte)(EFUSE_Type *pstcEfuse, uint16_t u16Addr, uint8_t *pu8Byte, uint32_t SocClk);
  rsi_error_t (*efuse_mem_map_read_word)(EFUSE_Type *pstcEfuse, uint16_t u16Addr, uint16_t *pu16Word, uint32_t SocClk);
} ROM_EFUSE_API_T;
///////////////EFUSE END//////////////////
#endif
typedef PRE_PACK struct POST_PACK {
  void (*qspi_write_to_flash)(qspi_reg_t *qspi_reg, uint32_t len_in_bits, uint32_t cmd_addr_data, uint32_t cs_no);
  void (*qspi_switch_qspi2)(qspi_reg_t *qspi_reg, uint32_t mode, uint32_t cs_no);
  uint32_t (*qspi_wait_flash_status_Idle)(qspi_reg_t *qspi_reg, spi_config_t *spi_config, uint32_t wr_reg_delay_ms);
  void (*qspi_enable_status_reg_write)(qspi_reg_t *qspi_reg,
                                       uint32_t flash_type,
                                       spi_config_t *spi_config,
                                       uint32_t cs_no);
  void (*qspi_status_reg_write)(qspi_reg_t *qspi_reg,
                                uint32_t write_value,
                                spi_config_t *spi_config,
                                uint32_t wr_reg_delay_ms);
  uint32_t (*qspi_flash_reg_read)(qspi_reg_t *qspi_reg, uint8_t reg_read_cmd, uint32_t cs_no, spi_config_t *spi_config);
  void (*qspi_flash_reg_write)(qspi_reg_t *qspi_reg,
                               uint32_t reg_write_cmd,
                               uint32_t reg_write_value,
                               uint32_t cs_no,
                               uint32_t wr_reg_delay_ms);
  void (*qspi_set_flash_mode)(qspi_reg_t *qspi_reg,
                              uint32_t data_mode,
                              uint32_t cs_no,
                              uint32_t ddr_en,
                              uint32_t flash_type);
  void (*qspi_config_qflash4_read)(qspi_reg_t *qspi_reg, spi_config_t *spi_config, uint32_t addr);
  void (*qspi_manual_read)(qspi_reg_t *qspi_reg,
                           spi_config_t *spi_config,
                           uint32_t addr,
                           uint8_t *data,
                           uint32_t hsize,
                           uint32_t len_in_bytes,
                           uint32_t manual_udma_read,
                           void *udmaHandle,
                           void *gpdmaHandle);
  void (*qspi_auto_init)(qspi_reg_t *qspi_reg, spi_config_t *spi_config);
  void (*qspi_auto_read)(uint32_t cs_no,
                         uint32_t addr,
                         uint8_t *data,
                         uint32_t hsize,
                         uint32_t len_in_bytes,
                         spi_config_t *spi_config,
                         uint32_t dma_flags);
  void (*qspi_flash_init)(qspi_reg_t *qspi_reg, spi_config_t *spi_config, uint32_t wr_reg_delay_ms);
  void (*qspi_spi_init)(qspi_reg_t *qspi_reg,
                        spi_config_t *spi_config,
                        uint32_t flash_init_req,
                        uint32_t wr_reg_delay_ms,
                        uint8_t fifo_thrsld);
  void (*qspi_spi_erase)(qspi_reg_t *qspi_reg,
                         spi_config_t *spi_config,
                         uint32_t erase_cmd,
                         uint32_t blk_sec_addr,
                         uint32_t dis_hw_ctrl,
                         uint32_t wr_reg_delay_ms);
  uint32_t (*qspi_spi_write)(qspi_reg_t *qspi_reg,
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
                             uint32_t udma_enable,
                             void *udmaHandle,
                             void *gpdmaHandle);
  void (*qspi_spi_read)(qspi_reg_t *qspi_reg,
                        spi_config_t *spi_config,
                        uint32_t addr,
                        uint8_t *data,
                        uint32_t hsize,
                        uint32_t len_in_bytes,
                        uint32_t manual_udma_read,
                        void *udmaHandle,
                        void *gpdmaHandle);
  void (*qspi_usleep)(uint32_t delay_us); //!< function ptr for halting processor for delay (us) specified
  void (*qspi_write_block_protect)(qspi_reg_t *qspi_reg,
                                   uint32_t protect,
                                   uint32_t cs_no,
                                   uint32_t num_prot_bytes,
                                   uint32_t wr_reg_delay_ms);
#if defined(SLI_SI917)
#if defined(SLI_SI917B0)
  void (*qspi_qspiload_key)(qspi_reg_t *qspi_reg,
                            uint8_t mode,
                            uint32_t *key1,
                            uint32_t *key2,
                            uint32_t key_len,
                            uint32_t kh_enable);
#else
  void (*qspi_qspiload_key)(qspi_reg_t *qspi_reg, uint8_t mode, uint32_t *key, uint32_t kh_enable);
#endif
#else
  void (*qspi_qspiload_key)(qspi_reg_t *qspi_reg, uint32_t *key, uint32_t kh_enable);
#endif
  void (*qspi_qspiload_nonce)(qspi_reg_t *qspi_reg, uint32_t *nonce);
  void (*qspi_seg_sec_en)(qspi_reg_t *qspi_reg, uint32_t seg_no, uint32_t start_addr, uint32_t end_addr);
  void (*qspi_status_control_reg_write)(spi_config_t *spi_config,
                                        qspi_reg_t *qspi_reg,
                                        uint16_t write_command,
                                        uint32_t addr,
                                        uint16_t write_value,
                                        uint32_t cs_no,
                                        uint32_t wr_reg_delay_ms);
  void (*qspi_flash_protection)(spi_config_t *spi_config,
                                qspi_reg_t *qspi_reg,
                                uint32_t protection,
                                uint32_t wr_reg_delay_ms);
  void (*RSI_QSPI_ConfigQspiDll)(spi_config_t *spi_config, qspi_reg_t *qspi_reg);
  void (*RSI_QSPI_ResetFlash)(qspi_reg_t *qspi_reg, uint32_t cs_no);
  void (*RSI_QSPI_UpdateOperatingMode_and_ResetType)(qspi_reg_t *qspi_reg, uint32_t operating_mode);
} ROM_QSPI_API_T;
//////////////////////////QSPI END//////////////////////////

/** @brief TIMERS ROM indirect function structure */
typedef PRE_PACK struct POST_PACK {
  RSI_UDMA_HANDLE_T (*udma_init)(void *mem, const RSI_UDMA_INIT_T *pInit);

  uint32_t (*udma_get_channel_transfer_mode)(RSI_UDMA_HANDLE_T pHandle, const RSI_UDMA_CHA_CFG_T *pCfg);
  rsi_error_t (*udma_setup_channel_transfer)(RSI_UDMA_HANDLE_T pHandle,
                                             const RSI_UDMA_CHA_CFG_T *pCfg,
                                             RSI_UDMA_CHA_CONFIG_DATA_T vsUdmaChaConfigData,
                                             void *pSrcAddr,
                                             void *pDstAddr);

  rsi_error_t (*udma_set_channel_scatter_gather_transfer)(RSI_UDMA_HANDLE_T pHandle,
                                                          uint8_t dmaCh,
                                                          uint32_t taskCount,
                                                          void *pTaskList,
                                                          uint32_t transferType);

  uint32_t (*udma_get_channel_transfer_length)(RSI_UDMA_HANDLE_T pHandle,
                                               const RSI_UDMA_CHA_CFG_T *pCfg,
                                               RSI_UDMA_CHA_CONFIG_DATA_T vsUDMAChaConfigData);

  rsi_error_t (*udma_setup_channel)(RSI_UDMA_HANDLE_T pHandle, const RSI_UDMA_CHA_CFG_T *pCfg);

  void (*udma_deInit)(RSI_UDMA_HANDLE_T pHandle, const RSI_UDMA_CHA_CFG_T *pCfg);

  void (*udma_interrupt_handler)(RSI_UDMA_HANDLE_T pHandle);

  rsi_error_t (*udma_interrupt_enable)(RSI_UDMA_HANDLE_T pHandle, uint8_t dmaCh);

} ROM_UDMA_API_T;

//////////////////////////UDMA END//////////////////////////

//////////////////////////UDMA WRAPPERS //////////////////////////
typedef PRE_PACK struct POST_PACK {
  RSI_UDMA_HANDLE_T(*uDMAx_Initialize)
  (const UDMA_RESOURCES *udma, RSI_UDMA_DESC_T *UDMA_Table, RSI_UDMA_HANDLE_T udmaHandle, uint32_t *mem);
  int32_t (*uDMAx_Uninitialize)(const UDMA_RESOURCES *udma);
  int32_t (*uDMAx_ChannelConfigure)(const UDMA_RESOURCES *udma,
                                    uint8_t ch,
                                    uint32_t src_addr,
                                    uint32_t dest_addr,
                                    uint32_t size,
                                    RSI_UDMA_CHA_CONFIG_DATA_T control,
                                    const RSI_UDMA_CHA_CFG_T *config,
                                    UDMA_SignalEvent_t cb_event,
                                    UDMA_Channel_Info *chnl_info,
                                    RSI_UDMA_HANDLE_T udmaHandle);
  int32_t (*uDMAx_ChannelEnable)(uint8_t ch, const UDMA_RESOURCES *udma, RSI_UDMA_HANDLE_T udmaHandle);
  int32_t (*uDMAx_DMAEnable)(const UDMA_RESOURCES *udma, RSI_UDMA_HANDLE_T udmaHandle);
  int32_t (*uDMAx_ChannelDisable)(uint8_t ch, const UDMA_RESOURCES *udma, RSI_UDMA_HANDLE_T udmaHandle);
  uint32_t (*uDMAx_ChannelGetCount)(uint8_t ch,
                                    RSI_UDMA_CHA_CONFIG_DATA_T control,
                                    RSI_UDMA_CHA_CFG_T config,
                                    const UDMA_RESOURCES *udma,
                                    RSI_UDMA_HANDLE_T udmaHandle);
  void (*uDMAx_IRQHandler)(UDMA_RESOURCES *udma, RSI_UDMA_DESC_T *UDMA_Table, UDMA_Channel_Info *chnl_info);

} ROM_UDMA_WRAPPER_API_T;
//////////////////////////UDMA WRAPPERS END//////////////////////////
#if !defined(SLI_SI917B0)
typedef PRE_PACK struct POST_PACK {
  rsi_error_t (*mcpwm_counter_reset)(RSI_MCPWM_T *pMCPWM, uint8_t chnlNum);

  rsi_error_t (*mcpwm_channel_reset)(RSI_MCPWM_T *pMCPWM, uint8_t chnlNum);

  rsi_error_t (*mcpwm_start)(RSI_MCPWM_T *pMCPWM, uint8_t chnlNum);

  rsi_error_t (*mcpwm_stop)(RSI_MCPWM_T *pMCPWM, uint8_t chnlNum);

  rsi_error_t (*mcpwm_set_time_period)(RSI_MCPWM_T *pMCPWM, uint8_t chnlNum, uint16_t period, uint16_t initVal);

  void (*mcpwm_special_event_trigger_config)(RSI_MCPWM_T *pMCPWM,
                                             boolean_t svtDir,
                                             RSI_MCPWM_SVT_CONFIG_T *pMCPWMSVTConfig);

  rsi_error_t (*mcpwm_dead_time_value_set)(RSI_MCPWM_T *pMCPWM,
                                           RSI_MCPWM_DT_CONFIG_T *pMCPWMDeadTimeConfig,
                                           uint8_t chnlNum);

  rsi_error_t (*mcpwm_period_control_config)(RSI_MCPWM_T *pMCPWM,
                                             uint32_t postScale,
                                             uint32_t preScale,
                                             uint8_t chnlNum);

  rsi_error_t (*mcpwm_fault_avalue_set)(RSI_MCPWM_T *pMCPWM, uint8_t pwmOutput, uint8_t value);

  rsi_error_t (*mcpwm_fault_bvalue_set)(RSI_MCPWM_T *pMCPWM, uint8_t pwmOutput, uint8_t value);

  rsi_error_t (*mcpwm_set_base_timer_mode)(RSI_MCPWM_T *pMCPWM, uint8_t mode, uint8_t chnlNum);

  rsi_error_t (*mcpwm_set_output_mode)(RSI_MCPWM_T *pMCPWM, boolean_t mode, uint8_t chnlNum);

  void (*mcpwm_set_output_polarity)(RSI_MCPWM_T *pMCPWM, boolean_t polL, boolean_t polH);

  void (*mcpwm_interrupt_handler)(RSI_MCPWM_T *pMCPWM, RSI_MCPWM_CALLBACK_T *pCallBack);

} ROM_MCPWM_API_T;
#endif
////////////MCPWM END///////////////////////

/* RPDMA Function Pointer Table */
typedef PRE_PACK struct POST_PACK {
  uint32_t (*gpdma_get_mem_size)(void);

  RSI_GPDMA_HANDLE_T (*gpdma_init)(void *mem, const RSI_GPDMA_INIT_T *pInit);

  void (*gpdma_register_callback)(RSI_GPDMA_HANDLE_T pHandle, uint32_t cbIndex, gpdmaTransferCompleteCB pCB);

  rsi_error_t (*gpdma_abort_channel)(RSI_GPDMA_HANDLE_T pHandle, uint8_t dmaCh);

  rsi_error_t (*gpdma_setup_channel)(RSI_GPDMA_HANDLE_T pHandle, RSI_GPDMA_CHA_CFG_T *pCfg);

  rsi_error_t (*gpdma_build_descriptors)(RSI_GPDMA_HANDLE_T pHandle,
                                         RSI_GPDMA_DESC_T *pXferCfg,
                                         RSI_GPDMA_DESC_T *pDesc,
                                         RSI_GPDMA_DESC_T *pDescPrev);

  rsi_error_t (*gpdma_setup_channelTransfer)(RSI_GPDMA_HANDLE_T pHandle, uint8_t dmaCh, RSI_GPDMA_DESC_T *pDesc);

  void (*gpdma_interrupt_handler)(RSI_GPDMA_HANDLE_T pHandle);

  void (*gpdma_deInit)(RSI_GPDMA_HANDLE_T pHandle, RSI_GPDMA_CHA_CFG_T *pCfg);

  rsi_error_t (*gpdma_dma_channel_trigger)(RSI_GPDMA_HANDLE_T pHandle, uint8_t dmaCh);

  uint32_t (*gpdma_channel_is_enabled)(RSI_GPDMA_HANDLE_T pHandle, uint8_t dmaCh);

  rsi_error_t (*gpdma_interrupt_disable)(RSI_GPDMA_HANDLE_T pHandle, RSI_GPDMA_CHA_CFG_T *pCfg);

  rsi_error_t (*gpdma_interrupt_enable)(RSI_GPDMA_HANDLE_T pHandle, RSI_GPDMA_CHA_CFG_T *pCfg);

  rsi_error_t (*gpdma_error_status_clear)(RSI_GPDMA_HANDLE_T pHandle, RSI_GPDMA_CHA_CFG_T *pCfg);

  uint32_t (*gpdma_get_error_status)(RSI_GPDMA_HANDLE_T pHandle, RSI_GPDMA_CHA_CFG_T *pCfg);

  rsi_error_t (*gpdma_interrupt_clear)(RSI_GPDMA_HANDLE_T pHandle, RSI_GPDMA_CHA_CFG_T *pCfg);

  uint32_t (*gpdma_interrupt_status)(RSI_GPDMA_HANDLE_T pHandle, RSI_GPDMA_CHA_CFG_T *pCfg);

} ROM_RPDMA_API_T;
///////////////////////////GPDMA END/////////////////////

/** @brief TIMERS ROM indirect function structure */
typedef PRE_PACK struct POST_PACK {
  rsi_error_t (*timers_microsec_timer_config)(RSI_TIMERS_T *pTIMER,
                                              uint8_t timerNum,
                                              uint16_t integer,
                                              uint8_t fractional,
                                              uint8_t mode);
  uint32_t (*timers_read_timer)(RSI_TIMERS_T *pTIMER, uint8_t timerNum, boolean_t countDir);
} ROM_TIMERS_API_T;

////////////////////////////TIMERS END///////////////////////

//USART
typedef PRE_PACK struct POST_PACK {
  int32_t (*USART_Initialize)(ARM_USART_SignalEvent_t cb_event,
                              USART_RESOURCES *usart,
                              UDMA_RESOURCES *udma,
                              RSI_UDMA_DESC_T *UDMA_Table,
                              RSI_UDMA_HANDLE_T *udmaHandle,
                              uint32_t *mem);
  int32_t (*USART_Uninitialize)(USART_RESOURCES *usart, UDMA_RESOURCES *udma);
  int32_t (*USART_PowerControl)(ARM_POWER_STATE state,
                                USART_RESOURCES *usart,
                                UDMA_RESOURCES *udma,
                                RSI_UDMA_HANDLE_T udmaHandle);
  int32_t (*USART_SetBaudrate)(uint32_t baudrate, uint32_t baseClk, USART_RESOURCES *usart);
  int32_t (*USART_Send_Data)(const void *data,
                             uint32_t num,
                             USART_RESOURCES *usart,
                             UDMA_RESOURCES *udma,
                             UDMA_Channel_Info *chnl_info,
                             RSI_UDMA_HANDLE_T udmaHandle);
  int32_t (*USART_Receive_Data)(const void *data,
                                uint32_t num,
                                USART_RESOURCES *usart,
                                UDMA_RESOURCES *udma,
                                UDMA_Channel_Info *chnl_info,
                                RSI_UDMA_HANDLE_T udmaHandle);
  int32_t (*USART_Transfer)(const void *data_out,
                            void *data_in,
                            uint32_t num,
                            USART_RESOURCES *usart,
                            UDMA_RESOURCES *udma,
                            UDMA_Channel_Info *chnl_info,
                            RSI_UDMA_HANDLE_T udmaHandle);
  uint32_t (*USART_GetTxCount)(USART_RESOURCES *usart);
  uint32_t (*USART_GetRxCount)(USART_RESOURCES *usart);
  int32_t (*USART_Control)(uint32_t control,
                           uint32_t arg,
                           uint32_t baseClk,
                           USART_RESOURCES *usart,
                           UDMA_RESOURCES *udma,
                           RSI_UDMA_HANDLE_T udmaHandle);
  ARM_USART_STATUS (*USART_GetStatus)(USART_RESOURCES *usart);
  int32_t (*USART_SetModemControl)(ARM_USART_MODEM_CONTROL control, USART_RESOURCES *usart);
  ARM_USART_MODEM_STATUS (*USART_GetModemStatus)(USART_RESOURCES *usart);
  void (*UartIrqHandler)(USART_RESOURCES *usart);
  void (*USART_UDMA_Tx_Event)(uint32_t event, uint8_t dmaCh, USART_RESOURCES *usart);
  void (*USART_UDMA_Rx_Event)(uint32_t event, uint8_t dmaCh, USART_RESOURCES *usart);
} ROM_USART_API_T;
//////////////////////USART END////////////////////

//GPSI
typedef PRE_PACK struct POST_PACK {
  int32_t (*GSPI_Initialize)(ARM_SPI_SignalEvent_t cb_event,
                             const GSPI_RESOURCES *gspi,
                             UDMA_RESOURCES *udma,
                             RSI_UDMA_DESC_T *UDMA_Table,
                             RSI_UDMA_HANDLE_T *udmaHandle,
                             uint32_t *mem);
  int32_t (*GSPI_Uninitialize)(const GSPI_RESOURCES *gspi, UDMA_RESOURCES *udma);
  int32_t (*GSPI_PowerControl)(ARM_POWER_STATE state, const GSPI_RESOURCES *gspi);
#if defined(SLI_SI917B0)
  int32_t (*GSPI_Control)(uint32_t control,
                          uint32_t arg,
                          const GSPI_RESOURCES *gspi,
                          uint32_t base_clock,
                          uint8_t spi_slavenumber);
#else
  int32_t (*GSPI_Control)(uint32_t control, uint32_t arg, const GSPI_RESOURCES *gspi, uint32_t base_clock);
#endif
  int32_t (*GSPI_Send)(const void *data,
                       uint32_t num,
                       const GSPI_RESOURCES *gspi,
                       UDMA_RESOURCES *udma,
                       UDMA_Channel_Info *chnl_info,
                       RSI_UDMA_HANDLE_T udmaHandle);
  int32_t (*GSPI_Receive)(void *data,
                          uint32_t num,
                          const GSPI_RESOURCES *gspi,
                          UDMA_RESOURCES *udma,
                          UDMA_Channel_Info *chnl_info,
                          RSI_UDMA_HANDLE_T udmaHandle);
  int32_t (*GSPI_Transfer)(const void *data_out,
                           void *data_in,
                           uint32_t num,
                           const GSPI_RESOURCES *gspi,
                           UDMA_RESOURCES *udma,
                           UDMA_Channel_Info *chnl_info,
                           RSI_UDMA_HANDLE_T udmaHandle);
  uint32_t (*GSPI_GetDataCount)(const GSPI_RESOURCES *gspi);
  void (*GSPI_UDMA_Tx_Event)(uint32_t event, uint8_t dmaCh, GSPI_RESOURCES *gspi);
  void (*GSPI_UDMA_Rx_Event)(uint32_t event, uint8_t dmaCh, GSPI_RESOURCES *gspi);
  void (*GSPI_IRQHandler)(const GSPI_RESOURCES *gspi);
} ROM_GSPI_API_T;
////////////////////////////GSPI END//////////////////////////

//i2s
typedef PRE_PACK struct POST_PACK {
  int32_t (*I2S_Initialize)(ARM_SAI_SignalEvent_t cb_event,
                            I2S_RESOURCES *i2s,
                            UDMA_RESOURCES *udma,
                            RSI_UDMA_DESC_T *UDMA_Table,
                            RSI_UDMA_HANDLE_T *udmaHandle,
                            uint32_t *mem);
  int32_t (*I2S_Uninitialize)(I2S_RESOURCES *i2s, UDMA_RESOURCES *udma);
  int32_t (*I2S_PowerControl)(ARM_POWER_STATE state,
                              I2S_RESOURCES *i2s,
                              UDMA_RESOURCES *udma,
                              RSI_UDMA_HANDLE_T udmaHandle);
  int32_t (*I2S_Send)(const void *data,
                      uint32_t num,
                      I2S_RESOURCES *i2s,
                      UDMA_RESOURCES *udma,
                      UDMA_Channel_Info *chnl_info,
                      RSI_UDMA_HANDLE_T udmaHandle);
  int32_t (*I2S_Receive)(void *data,
                         uint32_t num,
                         I2S_RESOURCES *i2s,
                         UDMA_RESOURCES *udma,
                         UDMA_Channel_Info *chnl_info,
                         RSI_UDMA_HANDLE_T udmaHandle);
  uint32_t (*I2S_GetTxCount)(I2S_RESOURCES *i2s);
  uint32_t (*I2S_GetRxCount)(I2S_RESOURCES *i2s);
  int32_t (*I2S_Control)(uint32_t control,
                         uint32_t arg1,
                         uint32_t arg2,
                         I2S_RESOURCES *i2s,
                         UDMA_RESOURCES *udma,
                         RSI_UDMA_HANDLE_T udmaHandle);
  ARM_SAI_STATUS (*I2S_GetStatus)(I2S_RESOURCES *i2s);
  void (*I2S_IRQHandler)(I2S_RESOURCES *i2s);
  void (*I2S_UDMA_Tx_Event)(uint32_t event, uint8_t dmaCh, I2S_RESOURCES *i2s);
  void (*I2S_UDMA_Rx_Event)(uint32_t event, uint8_t dmaCh, I2S_RESOURCES *i2s);
} ROM_I2S_API_T;
////////////////////////////I2S END////////////////////////

//I2C
typedef PRE_PACK struct POST_PACK {
  int32_t (*I2Cx_Initialize)(ARM_I2C_SignalEvent_t cb_event, I2C_RESOURCES *i2c);
  int32_t (*I2Cx_Uninitialize)(I2C_RESOURCES *i2c);
  int32_t (*I2Cx_PowerControl)(ARM_POWER_STATE state, I2C_RESOURCES *i2c);
  int32_t (
    *I2Cx_MasterTransmit)(uint32_t addr, const uint8_t *data, uint32_t num, bool xfer_pending, I2C_RESOURCES *i2c);
  int32_t (*I2Cx_MasterReceive)(uint32_t addr, uint8_t *data, uint32_t num, bool xfer_pending, I2C_RESOURCES *i2c);
  int32_t (*I2Cx_SlaveTransmit)(const uint8_t *data, uint32_t num, I2C_RESOURCES *i2c);
  int32_t (*I2Cx_SlaveReceive)(uint8_t *data, uint32_t num, I2C_RESOURCES *i2c);
  int32_t (*I2Cx_GetDataCount)(I2C_RESOURCES *i2c);
  int32_t (*I2Cx_Control)(uint32_t control, uint32_t arg, I2C_RESOURCES *i2c, uint32_t clock);
  ARM_I2C_STATUS (*I2Cx_GetStatus)(I2C_RESOURCES *i2c);
  uint32_t (*I2Cx_MasterHandler)(I2C_RESOURCES *i2c);
  uint32_t (*I2Cx_SlaveHandler)(I2C_RESOURCES *i2c);
  void (*I2Cx_IRQHandler)(I2C_RESOURCES *i2c);
} ROM_I2C_API_T;
///////////////////I2C END/////////////////
#if 0
typedef PRE_PACK struct POST_PACK 
		{
	void  (* rsi_arm_offset_f32_opt)(int32_t *pSrc, int32_t scale, int32_t *pDst,uint32_t length,uint16_t inBank, uint16_t outBank );

	void  (* rsi_arm_offset_q7_opt)( q7_t *pSrc, q7_t scale, q7_t *pDst, uint32_t length,uint16_t inBank, uint16_t outBank);

	void  (* rsi_arm_offset_q15_opt)( q15_t *pSrc, q15_t scale, q15_t *pDst, uint32_t length,uint16_t inBank, uint16_t outBank);

	void  (* rsi_arm_offset_q31_opt)( q31_t *pSrc, q31_t scale, q31_t *pDst, uint32_t length,uint16_t inBank, uint16_t outBank);

	void  (* rsi_fim_scalar_sub_q7)(q7_t *pSrc,q7_t scale,q7_t *pDst, uint32_t blockSize,uint16_t inBank, uint16_t outBank);

	void  (* rsi_fim_scalar_sub_q31)(q31_t *pSrc,q31_t scale,q31_t *pDst, uint32_t blockSize,uint16_t inBank, uint16_t outBank);

	void  (* rsi_fim_scalar_sub_f32)( int32_t *pSrc, int32_t scale,int32_t *pDst, uint32_t length,uint16_t inBank, uint16_t outBank);

	void  (* rsi_arm_scale_f32_opt)( int32_t *pSrc,int32_t scale,int32_t *pDst, uint32_t length,uint16_t inBank, uint16_t outBank);

	void  (* rsi_arm_scale_q7_opt)( q7_t *pSrc,q7_t scaleFract,int8_t shift,q7_t *pDst,uint32_t blockSize,uint16_t inBank, uint16_t outBank);

	void  (* rsi_arm_scale_q15_opt)( q15_t *pSrc, q15_t scaleFract, int8_t shift, q15_t *pDst,uint32_t blockSize,uint16_t inBank, uint16_t outBank);

	void  (* rsi_arm_scale_q31_opt)( q31_t * pSrc,q31_t scaleFract, int8_t shift,q31_t * pDst,uint32_t blockSize,uint16_t inBank, uint16_t outBank);

	void  (* rsi_fim_scalar_mul_q15)(q15_t *pSrc,q15_t *scale,uint32_t blockSize, typ_data_t typ_data,uint16_t inBank, uint16_t outBank );

	void  (* rsi_arm_add_f32_opt)( int32_t *pSrcA,  int32_t *pSrcB,uint32_t blockSize,uint16_t inBank1, uint16_t inBank2, uint16_t outBank);

	void  (* rsi_arm_add_q7_opt)( q7_t *pSrcA, q7_t *pSrcB, q7_t *pDst,  uint32_t blockSize,uint16_t inBank1, uint16_t inBank2, uint16_t outBank);

	void  (* rsi_arm_add_q15_opt)( q15_t *pSrcA,   q15_t *pSrcB,  q15_t *pDst, uint32_t blockSize,uint16_t inBank1, uint16_t inBank2, uint16_t outBank);

	void  (* rsi_arm_add_q31_opt)( q31_t *pSrcA, q31_t *pSrcB, q31_t *pDst, uint32_t blockSize,uint16_t inBank1, uint16_t inBank2, uint16_t outBank);

	void  (* rsi_fim_vector_add_q15)(q15_t *pIn1, q15_t *pIn2,q15_t *pDst,uint32_t blockSize, typ_data_t typ_data,uint16_t inBank1, uint16_t inBank2, uint16_t outBank);

	void  (* rsi_arm_sub_f32_opt)(int32_t *pSrcA, int32_t *pSrcB,int32_t *pDst, uint32_t blockSize,uint16_t inBank1, uint16_t inBank2, uint16_t outBank);

	void  (* rsi_arm_sub_q7_opt)( q7_t *pSrcA,  q7_t *pSrcB, q7_t *pDst, uint32_t blockSize,uint16_t inBank1, uint16_t inBank2, uint16_t outBank);

	void  (* rsi_arm_sub_q15_opt)( q15_t *pSrcA, q15_t *pSrcB,  q15_t *pDst,uint32_t blockSize,uint16_t inBank1, uint16_t inBank2, uint16_t outBank);

	void  (*rsi_arm_sub_q31_opt)( q31_t *pSrcA,  q31_t *pSrcB,q31_t *pDst, uint32_t blockSize,uint16_t inBank1, uint16_t inBank2, uint16_t outBank);

	void  (* rsi_fim_read_data)( uint32_t bank,uint32_t length,volatile void  *pDst, uint8_t data_type,typ_data_t type_data);

	void  (* rsi_fim_vector_sub_q15)( q15_t *pIn1, q15_t *pIn2,q15_t *pDst,uint32_t blockSize,typ_data_t typ_data,uint16_t inBank1, uint16_t inBank2, uint16_t outBank);

	void  (* rsi_arm_mult_f32_opt)( int32_t *pIn1,int32_t *pIn2, uint32_t SatTruncRound,uint32_t length,uint16_t inBank1, uint16_t inBank2, uint16_t outBank);

	void  (* rsi_arm_mult_q7_opt)( q7_t *pSrcA, q7_t *pSrcB, q7_t *pDst, uint32_t blockSize,uint16_t inBank1, uint16_t inBank2, uint16_t outBank);

	void  (* rsi_arm_mult_q15_opt)( q15_t *pSrcA, q15_t *pSrcB, q15_t *pDst, uint32_t blockSize,uint16_t inBank1, uint16_t inBank2, uint16_t outBank);

	void  (* rsi_arm_mult_q31_opt)(q31_t *pSrcA,q31_t *pSrcB,q31_t *pDst,uint32_t blockSize,uint16_t inBank1, uint16_t inBank2, uint16_t outBank);

	void  (* rsi_fim_vector_mul_q15)( q15_t *pIn1, q15_t *pIn2,q15_t *pDst,uint32_t blockSize,uint16_t inBank1, uint16_t inBank2, uint16_t outBank);

	void  (* rsi_arm_cmplx_mult_real_q15_opt)(q15_t *pSrcCmplx,q15_t *pSrcReal,q15_t *pDst,uint32_t numSamples,uint16_t inBank1, uint16_t inBank2, uint16_t outBank);

	void  (* rsi_arm_cmplx_mult_cmplx_q15_opt)(q15_t * pSrcA,q15_t * pSrcB,q15_t *pDst,uint32_t numSamples,uint16_t inBank1, uint16_t inBank2, uint16_t outBank);

	void  (* rsi_arm_cmplx_mag_squared_q15_opt)(q15_t *pSrc,q15_t *pDst,uint32_t numSamples,uint16_t inBank, uint16_t outBank);

	void  (* rsi_fim_absSqr_q7)( q7_t *pSrc,uint32_t length,uint16_t inBank, uint16_t outBank);

	void  (* rsi_fim_absSqr_q15)( q15_t *pSrc,  uint32_t length,uint16_t inBank, uint16_t outBank);

	void  (* rsi_fim_absSqr_q31)( q31_t *pSrc,uint32_t length,uint16_t inBank, uint16_t outBank);

	void  (* rsi_fim_absSqr_f32)(int32_t *pSrc, uint32_t length,uint16_t inBank, uint16_t outBank);

	rsi_error_t (* rsi_arm_mat_mult_f32_opt)( const arm_matrix_instance_f32_opt *pSrcA,
			const arm_matrix_instance_f32_opt *pSrcB,arm_matrix_instance_f32_opt *pDst,uint16_t inBank1, uint16_t inBank2, uint16_t outBank);

	rsi_error_t (* rsi_arm_mat_mult_q31_opt)( const arm_matrix_instance_q31_opt * pSrcA,const arm_matrix_instance_q31_opt * pSrcB,
			arm_matrix_instance_q31_opt * pDst,uint16_t inBank1, uint16_t inBank2, uint16_t outBank);

	rsi_error_t (* rsi_arm_mat_mult_q15_opt)( const arm_matrix_instance_q15_opt * pSrcA,
			const arm_matrix_instance_q15_opt * pSrcB,arm_matrix_instance_q15_opt * pDst,q15_t * pState
			,uint16_t inBank1, uint16_t inBank2, uint16_t outBank);

	void  (* rsi_arm_fir_init_f32_opt)( arm_fir_instance_f32_opt *S,  uint16_t numTaps,int32_t *pCoeffs, int32_t *pState,
			uint32_t blockSize);

	void  (* rsi_arm_fir_f32_opt)( arm_fir_instance_f32_opt *S, int32_t *pSrc,  int32_t *pDst,
			uint32_t blockSize,uint16_t inBank1, uint16_t inBank2, uint16_t outBank);

	void  (* rsi_arm_fir_init_q31_opt)( arm_fir_instance_q31_opt *S,uint16_t numTaps,q31_t *pCoeffs,
			q31_t *pState, uint32_t blockSize);

	void  (* rsi_arm_fir_q31_opt)( arm_fir_instance_q31_opt *S, q31_t *pSrc,q31_t *pDst, uint32_t blockSize,uint16_t inBank1, uint16_t inBank2, uint16_t outBank);

	void  (* rsi_arm_fir_init_q15_opt)( arm_fir_instance_q15_opt *S,uint16_t numTaps,q15_t *pCoeffs,
			q15_t *pState, uint32_t blockSize);

	void  (* rsi_arm_fir_q15_opt)( arm_fir_instance_q15_opt *S, q15_t *pSrc,q15_t *pDst, uint32_t blockSize,uint16_t inBank1, uint16_t inBank2, uint16_t outBank);

	void  (* rsi_arm_fir_init_q7_opt)( arm_fir_instance_q7_opt *S,uint16_t numTaps,q7_t *pCoeffs,
			q7_t *pState, uint32_t blockSize);

	void  (* rsi_arm_fir_q7_opt)( arm_fir_instance_q7_opt *S, q7_t *pSrc,q7_t *pDst,
			uint32_t blockSize,uint16_t inBank1, uint16_t inBank2, uint16_t outBank);

	void  (* rsi_fim_interrupt_handler)(volatile FIM_Type *ptFim);

	void  (* rsi_arm_fir_interpolate_f32_opt)( const arm_fir_interpolate_instance_f32_opt * S, int32_t * pSrc,
			int32_t * pDst,uint32_t blockSize,uint16_t inBank1, uint16_t inBank2, uint16_t outBank);

	arm_status (* rsi_arm_fir_interpolate_init_f32_opt)( arm_fir_interpolate_instance_f32_opt * S,
			uint8_t L, uint16_t numTaps,  int32_t * pCoeffs,
			int32_t * pState,uint32_t blockSize );

	arm_status (* rsi_arm_fir_interpolate_init_q15_opt)( arm_fir_interpolate_instance_q15_opt * S,uint8_t L,
			uint16_t numTaps,q15_t * pCoeffs,q15_t * pState,
			uint32_t blockSize );

	arm_status (* rsi_arm_fir_interpolate_init_q31_opt)( arm_fir_interpolate_instance_q31_opt *S,  uint8_t L,
			uint16_t numTaps,q31_t * pCoeffs, q31_t * pState,
			uint32_t blockSize );

	void  (* rsi_arm_fir_interpolate_q15_opt)( arm_fir_interpolate_instance_q15_opt * S,q15_t * pSrc,q15_t *pDst,
			uint32_t blockSize,uint16_t inBank1, uint16_t inBank2, uint16_t outBank);


	void  (* rsi_arm_fir_interpolate_q31_opt)( const arm_fir_interpolate_instance_q31_opt * S, q31_t * pSrc,
			q31_t * pDst, uint32_t blockSize,uint16_t inBank1, uint16_t inBank2, uint16_t outBank);
		}ROM_FIM_API_T;    
    
/////////////////////FIM END//////////////////////
#endif
typedef PRE_PACK struct POST_PACK {
  void (*ct_ocu_high_Low_toggle_select)(RSI_CT_T *pCT, boolean_t lowHigh, boolean_t counterNum, uint8_t outputSel);
  rsi_error_t (*ct_wfg_control_config)(RSI_CT_T *pCT, WFG_PARAMS_T ctrlReg);
  rsi_error_t (*ct_ocu_control)(RSI_CT_T *pCT,
                                boolean_t counterNum,
                                boolean_t dmaEn,
                                OCU_PARAMS_T *pOCUparams,
                                RSI_CT_CALLBACK_T *pCB);

  rsi_error_t (*ct_wfg_comapre_value_set)(RSI_CT_T *pCT, boolean_t counterNum, OCU_PARAMS_T *pOCUparams);
} ROM_CT_API_T;

///////////////////////CT END///////////////////////////////////
/** @brief SPI master ROM indirect function structure */
typedef PRE_PACK struct POST_PACK {
  void (*ps_clr_wkp_up_status)(uint32_t wakeUpIntrClear);
  void (*ps_bg_ldo_config)(uint8_t ldo_0p6_ctrl, uint8_t ldo_0p6_lp_mode);
  void (*ps_configure_trim_values)(uint16_t lf_ro_trim,
                                   uint16_t lf_rc_trim,
                                   uint16_t hf_ro_trim,
                                   uint16_t hf_rc_trim,
                                   uint16_t bg_ptat_trim,
                                   uint16_t bg_trim);
  void (*ps_wireless_shutdown)(void);
  uint32_t (*RSI_SelectTaBootOption)(uint32_t option);
  void (*RSI_GotoSleepWithRetention)(uint32_t stack_address,
                                     uint32_t jump_cb_address,
                                     uint32_t vector_offset,
                                     uint32_t mode);
  void (*RSI_WakeupWirelessProcessor)(void);
} ROM_PWR_API_T;
///////////////////////POWERSAVE END///////////////////////////

typedef PRE_PACK struct POST_PACK {
  rsi_error_t (*ulpss_clock_config)(M4CLK_Type *pCLK, boolean_t clkEnable, uint16_t divFactor, boolean_t oddDivFactor);

  rsi_error_t (*ulpss_ulp_peri_clk_enable)(ULPCLK_Type *pULPCLK, uint32_t u32Flags);

  rsi_error_t (*ulpss_ulp_peri_clk_disable)(ULPCLK_Type *pULPCLK, uint32_t u32Flags);

  rsi_error_t (*ulpss_ulp_dyn_clk_enable)(ULPCLK_Type *pULPCLK, uint32_t u32Flags);

  rsi_error_t (*ulpss_ulp_dyn_clk_disable)(ULPCLK_Type *pULPCLK, uint32_t u32Flags);

  rsi_error_t (*ulpss_ulp_ssi_clk_config)(ULPCLK_Type *pULPCLK,
                                          CLK_ENABLE_T clkType,
                                          ULP_SSI_CLK_SELECT_T clkSource,
                                          uint16_t divFactor);

  rsi_error_t (*ulpss_ulp_i2s_clk_config)(ULPCLK_Type *pULPCLK, ULP_I2S_CLK_SELECT_T clkSource, uint16_t divFactor);

  rsi_error_t (*ulpss_ulp_uar_clk_config)(ULPCLK_Type *pULPCLK,
                                          CLK_ENABLE_T clkType,
                                          boolean_t bFrClkSel,
                                          ULP_UART_CLK_SELECT_T clkSource,
                                          uint16_t divFactor);

  rsi_error_t (*ulpss_time_clk_disable)(ULPCLK_Type *pULPCLK);
  rsi_error_t (*ulpss_time_clk_config)(ULPCLK_Type *pULPCLK,
                                       CLK_ENABLE_T clkType,
                                       boolean_t bTmrSync,
                                       ULP_TIMER_CLK_SELECT_T clkSource,
                                       uint8_t skipSwitchTime);

  rsi_error_t (*ulpss_aux_clk_config)(ULPCLK_Type *pULPCLK, CLK_ENABLE_T clkType, ULP_AUX_CLK_SELECT_T clkSource);

  rsi_error_t (*ulpss_vad_clk_config)(ULPCLK_Type *pULPCLK,
                                      ULP_VAD_CLK_SELECT_T clkSource,
                                      ULP_VAD_FCLK_SELECT_T FclkSource,
                                      uint16_t divFactor);

  rsi_error_t (*ulpss_touch_clk_config)(ULPCLK_Type *pULPCLK, ULP_TOUCH_CLK_SELECT_T clkSource, uint16_t divFactor);

  rsi_error_t (*ulpss_slp_sensor_clk_config)(ULPCLK_Type *pULPCLK, boolean_t clkEnable, uint32_t divFactor);

  rsi_error_t (*ulpss_peripheral_enable)(ULPCLK_Type *pULPCLK, ULPPERIPHERALS_CLK_T module, CLK_ENABLE_T clkType);

  rsi_error_t (*ulpss_peripheral_disable)(ULPCLK_Type *pULPCLK, ULPPERIPHERALS_CLK_T module);

} ROM_ULPSS_CLK_API_T;

////////////////////////////ULPSS CLOCKS END/////////////////////////////////

typedef PRE_PACK struct POST_PACK {
  boolean_t (*clk_check_pll_lock)(PLL_TYPE_T pllType);
  rsi_error_t (*clk_soc_pll_clk_enable)(boolean_t clkEnable);
  rsi_error_t (*clk_set_soc_pll_freq)(const M4CLK_Type *pCLK, uint32_t socPllFreq, uint32_t pllRefClk);
  rsi_error_t (*clk_soc_pll_set_freq_div)(const M4CLK_Type *pCLK,
                                          boolean_t clk_en,
                                          uint16_t divFactor,
                                          uint16_t nFactor,
                                          uint16_t mFactor,
                                          uint16_t fcwF,
                                          uint16_t dcoFixSel,
                                          uint16_t ldoProg);
  rsi_error_t (*clk_soc_pll_clk_set)(const M4CLK_Type *pCLK);
  rsi_error_t (*clk_soc_pll_clk_bypass_enable)(boolean_t clkEnable);
  rsi_error_t (*clk_soc_pll_clk_reset)(void);
  rsi_error_t (*clk_soc_pll_pd_enable)(boolean_t en);
  rsi_error_t (*clk_soc_pll_turn_off)(void);
  rsi_error_t (*clk_soc_pll_turn_on)(void);
  rsi_error_t (*clk_i2s_pll_clk_enable)(boolean_t clkEnable);
  rsi_error_t (*clk_i2s_pll_clk_bypass_enable)(boolean_t clkEnable);
  rsi_error_t (*clk_i2s_pll_pd_enable)(boolean_t en);
  rsi_error_t (*clk_i2s_pll_turn_off)(void);
  rsi_error_t (*clk_i2s_pll_turn_on)(void);
  rsi_error_t (*clk_set_i2s_pll_freq)(const M4CLK_Type *pCLK, uint32_t i2sPllFreq, uint32_t fXtal);
  rsi_error_t (*clk_i2s_pll_set_freq_div)(const M4CLK_Type *pCLK,
                                          uint16_t u16DivFactor1,
                                          uint16_t u16DivFactor2,
                                          uint16_t nFactor,
                                          uint16_t mFactor,
                                          uint16_t fcwF);
  rsi_error_t (*clk_i2s_pll_clk_set)(const M4CLK_Type *pCLK);
  rsi_error_t (*clk_i2s_pll_clk_reset)(void);
  rsi_error_t (*clk_intf_pll_clk_enable)(boolean_t clkEnable);
  rsi_error_t (*clk_intf_pll_pd_enable)(boolean_t en);
  rsi_error_t (*clk_intf_pll_turn_off)(void);
  rsi_error_t (*clk_set_intf_pll_freq)(const M4CLK_Type *pCLK, uint32_t intfPllFreq, uint32_t pllRefClk);
  rsi_error_t (*clk_intf_pll_set_freq_div)(const M4CLK_Type *pCLK,
                                           boolean_t clk_en,
                                           uint16_t divFactor,
                                           uint16_t nFactor,
                                           uint16_t mFactor,
                                           uint16_t fcwF,
                                           uint16_t dcoFixSel,
                                           uint16_t ldoProg);
  rsi_error_t (*clk_intf_pll_clk_bypass_enable)(boolean_t clkEnable);
  rsi_error_t (*clk_intf_pll_turn_on)(void);
  rsi_error_t (*clk_intf_pll_clk_reset)(void);
  rsi_error_t (*clk_intf_pll_clk_set)(const M4CLK_Type *pCLK);
  rsi_error_t (*clk_peripheral_clk_enable1)(M4CLK_Type *pCLK, uint32_t flags);
  rsi_error_t (*clk_peripheral_clk_disable1)(M4CLK_Type *pCLK, uint32_t flags);
  rsi_error_t (*clk_peripheral_clk_enable2)(M4CLK_Type *pCLK, uint32_t flags);
  rsi_error_t (*clk_peripheral_clk_disable2)(M4CLK_Type *pCLK, uint32_t flags);
  rsi_error_t (*clk_peripheral_clk_enable3)(M4CLK_Type *pCLK, uint32_t flags);
  rsi_error_t (*clk_peripheral_clk_disable3)(M4CLK_Type *pCLK, uint32_t flags);
  rsi_error_t (*clk_dynamic_clk_gate_disable)(M4CLK_Type *pCLK, uint32_t flags);
  rsi_error_t (*clk_dynamic_clk_gate_disable2)(M4CLK_Type *pCLK, uint32_t flags);
  rsi_error_t (*clk_dynamic_clk_gate_enable)(M4CLK_Type *pCLK, uint32_t flags);
  rsi_error_t (*clk_dynamic_clk_gate_enable2)(M4CLK_Type *pCLK, uint32_t flags);
  rsi_error_t (*ulpss_enable_ref_clks)(REF_CLK_ENABLE_T enable, SRC_TYPE_T srcType, cdDelay delayFn);
  rsi_error_t (*ulpss_disable_ref_clks)(REF_CLK_ENABLE_T clk_type);
  rsi_error_t (*clk_qspi_clk_config)(M4CLK_Type *pCLK,
                                     QSPI_CLK_SRC_SEL_T clkSource,
                                     boolean_t swalloEn,
                                     boolean_t OddDivEn,
                                     uint32_t divFactor);
  rsi_error_t (*clk_usart_clk_config)(M4CLK_Type *pCLK,
                                      CLK_ENABLE_T clkType,
                                      boolean_t FracDivEn,
                                      EN_USART_T EN_USART_T,
                                      USART_CLK_SRC_SEL_T clkSource,
                                      uint32_t divFactor);
  rsi_error_t (*clk_ssi_mst_clk_config)(M4CLK_Type *pCLK,
                                        CLK_ENABLE_T clkType,
                                        SSI_MST_CLK_SRC_SEL_T clkSource,
                                        uint32_t divFactor);
#if !defined(SLI_SI917)
  rsi_error_t (*clk_sd_mem_clk_config)(M4CLK_Type *pCLK,
                                       boolean_t swalloEn,
                                       SDMEM_CLK_SRC_SEL_T clkSource,
                                       uint32_t divFactor);
#endif
  rsi_error_t (*clk_ct_clk_config)(M4CLK_Type *pCLK,
                                   CT_CLK_SRC_SEL_T clkSource,
                                   uint32_t divFactor,
                                   CLK_ENABLE_T clkType);
#if !defined(SLI_SI917)
  rsi_error_t (*clk_cci_clk_config)(M4CLK_Type *pCLK,
                                    CCI_CLK_SRC_SEL_T clkSource,
                                    uint32_t divFactor,
                                    CLK_ENABLE_T clkType);
#endif
  rsi_error_t (*clk_i2s_clk_config)(M4CLK_Type *pCLK, I2S_CLK_SRC_SEL_T clkSource, uint32_t divFactor);
  rsi_error_t (*clk_mcu_clk_cut_config)(M4CLK_Type *pCLK, MCU_CLKOUT_SRC_SEL_T clkSource, uint32_t divFactor);
#if !defined(SLI_SI917)
  rsi_error_t (*clk_can_clk_config)(M4CLK_Type *pCLK, uint32_t divFactor, CLK_ENABLE_T clkType);
  rsi_error_t (*clk_ethernet_clk_config)(M4CLK_Type *pCLK,
                                         boolean_t swalloEn,
                                         ETHERNET_CLK_SRC_SEL_T clkSource,
                                         uint32_t divFactor);
#endif
  rsi_error_t (*clk_m4_soc_clk_div)(M4CLK_Type *pCLK, uint32_t divFactor);
  rsi_error_t (*clk_qspi_clk_div)(M4CLK_Type *pCLK, boolean_t u8SwallowEn, boolean_t u8OddDivEn, uint32_t divFactor);
  rsi_error_t (*clk_ct_clk_div)(M4CLK_Type *pCLK, uint32_t divFactor);
  rsi_error_t (*clk_ssi_mst_clk_div)(M4CLK_Type *pCLK, uint32_t divFactor);
#if !defined(SLI_SI917)
  rsi_error_t (*clk_cci_clk_div)(M4CLK_Type *pCLK, uint32_t divFactor);
#endif
  rsi_error_t (*clk_i2s_clk_div)(M4CLK_Type *pCLK, uint32_t divFactor);
#if !defined(SLI_SI917)
  rsi_error_t (*clk_sd_mem_clk_div)(M4CLK_Type *pCLK, boolean_t u8SwallowEn, uint32_t divFactor);
#endif
  rsi_error_t (*clk_usart_clk_div)(M4CLK_Type *pCLK, EN_USART_T EN_USART_T, uint8_t u8FracDivEn, uint32_t divFactor);
  uint32_t (*clk_slp_clk_calib_config)(M4CLK_Type *pCLK, uint8_t clkCycles);
  rsi_error_t (*clk_gspi_clk_config)(M4CLK_Type *pCLK, GSPI_CLK_SRC_SEL_T clkSel);
  rsi_error_t (*clk_slp_clk_config)(M4CLK_Type *pCLK, SLEEP_CLK_SRC_SEL_T clkSrc);
  rsi_error_t (*clk_i2c_clk_config)(M4CLK_Type *pCLK, boolean_t clkEnable, EN_I2C_T enI2C);
  rsi_error_t (*clk_xtal_clk_config)(uint8_t xtalPin);
#if !defined(SLI_SI917)
  rsi_error_t (*clk_usb_clk_config)(M4CLK_Type *pCLK, USB_CLK_SRC_SEL_T clkSource, uint16_t divFactor);
#endif
  rsi_error_t (*clk_peripheral_clk_enable)(M4CLK_Type *pCLK, PERIPHERALS_CLK_T module, CLK_ENABLE_T clkType);
  rsi_error_t (*clk_peripheral_clk_disable)(M4CLK_Type *pCLK, PERIPHERALS_CLK_T module);
  void (*clk_config_pll_lock)(boolean_t manual_lock, boolean_t bypass_manual_lock, uint8_t mm_count_limit);
  void (*clk_config_pll_ref_clk)(uint8_t ref_clk_src);
#if defined(SLI_SI917B0)
  rsi_error_t (*clk_qspi_2_clk_config)(M4CLK_Type *pCLK,
                                       QSPI_CLK_SRC_SEL_T clkSource,
                                       boolean_t swalloEn,
                                       boolean_t OddDivEn,
                                       uint32_t divFactor);
#endif
} ROM_M4SS_CLK_API_T;

//////////////////////////M4 CLOCKS END///////////////////////

///////////////SSI/////////////////////////////////
typedef PRE_PACK struct POST_PACK {
  int32_t (*SPI_Initialize)(ARM_SPI_SignalEvent_t cb_event,
                            const SPI_RESOURCES *spi,
                            UDMA_RESOURCES *udma,
                            RSI_UDMA_DESC_T *UDMA_Table,
                            RSI_UDMA_HANDLE_T *udmaHandle,
                            uint32_t *mem);
  int32_t (*SPI_Uninitialize)(const SPI_RESOURCES *spi, UDMA_RESOURCES *udma);
  int32_t (*SPI_PowerControl)(ARM_POWER_STATE state, const SPI_RESOURCES *spi);
  int32_t (*SPI_Send)(const void *data,
                      uint32_t num,
                      const SPI_RESOURCES *spi,
                      UDMA_RESOURCES *udma,
                      UDMA_Channel_Info *chnl_info,
                      RSI_UDMA_HANDLE_T udmaHandle);
  int32_t (*SPI_Receive)(void *data,
                         uint32_t num,
                         const SPI_RESOURCES *spi,
                         UDMA_RESOURCES *udma,
                         UDMA_Channel_Info *chnl_info,
                         RSI_UDMA_HANDLE_T udmaHandle);
  int32_t (*SPI_Transfer)(const void *data_out,
                          void *data_in,
                          uint32_t num,
                          const SPI_RESOURCES *spi,
                          UDMA_RESOURCES *udma,
                          UDMA_Channel_Info *chnl_info,
                          RSI_UDMA_HANDLE_T udmaHandle);
  uint32_t (*SPI_GetDataCount)(const SPI_RESOURCES *spi);
#if defined(SLI_SI917B0)
  int32_t (*SPI_Control)(uint32_t control,
                         uint32_t arg,
                         const SPI_RESOURCES *spi,
                         uint32_t base_clock,
                         uint8_t spi_slavenumber);
#else
  int32_t (*SPI_Control)(uint32_t control, uint32_t arg, const SPI_RESOURCES *spi, uint32_t base_clock);
#endif
  ARM_SPI_STATUS (*SPI_GetStatus)(const SPI_RESOURCES *spi);
  void (*SPI_UDMA_Tx_Event)(uint32_t event, uint8_t dmaCh, SPI_RESOURCES *spi);
  void (*SPI_UDMA_Rx_Event)(uint32_t event, uint8_t dmaCh, SPI_RESOURCES *spi);
  void (*SPI_IRQHandler)(const SPI_RESOURCES *spi);
} ROM_SSI_API_T;
//////////////////SSI END//////////////////////////

typedef struct {
  const ROM_EGPIO_API_T *pEGPIOROM;   /*!< EGPIO driver API function table base address */
  const ROM_TIMERS_API_T *pTIMERSROM; /*!< TIMERS driver API function table base address */
  const ROM_UDMA_API_T *pUDMAROM;     /*!< UDMA driver API function table base address */
  const ROM_UDMA_WRAPPER_API_T *pUDMAWRAPPERROM;
  const ROM_CT_API_T *pCTROM; /*!< SCT driver API function table base address */
  const ROM_RPDMA_API_T *pRPDMAROM;
  const ROM_PWR_API_T *pPWR;
  const ROM_M4SS_CLK_API_T *pM4SSCLK;
  const ROM_ULPSS_CLK_API_T *pULPSSCLK;
  const ROM_QSPI_API_T *pQSPIROM;
#if !defined(SLI_SI917B0)
  const ROM_EFUSE_API_T *pEFUSEROM;
#endif
  const ROM_CRC_API_T *pCRCROM;
  const ROM_RNG_API_T *pRNGROM;
#if !defined(SLI_SI917B0)
  const ROM_MCPWM_API_T *pMCPWMROM;
#endif
  const ROM_USART_API_T *pUSARTROM;
  const ROM_GSPI_API_T *pGSPIROM;
  const ROM_I2S_API_T *pI2SROM;
  const ROM_I2C_API_T *pI2CROM;
#ifdef CHIP_9118
  const struct ROM_WL_API_S *pWLROM;
#endif
#if defined(SLI_SI917)
  const ROM_SSI_API_T *pSSIROM;
#endif
} RSI_ROM_API_T;

/*ROM base address */
#define RSI_ROM_API_BASE_LOC (0x00300100)

#define RSI_ROM_API ((RSI_ROM_API_T *)RSI_ROM_API_BASE_LOC)
/* Pointer to EGPIO peripheral driver functions in ROM */
#define ROMAPI_EGPIO_API ((RSI_ROM_API)->pEGPIOROM)

/* Pointer to Timers peripheral driver functions in ROM */
#define ROMAPI_TIMER_API ((RSI_ROM_API)->pTIMERSROM)

/* Pointer to UDMA peripheral driver functions in ROM */
#define ROMAPI_UDMA_API ((RSI_ROM_API)->pUDMAROM)

#define ROMAPI_UDMA_WRAPPER_API ((RSI_ROM_API)->pUDMAWRAPPERROM)

/* Pointer to SCT peripheral driver functions in ROM */
#define ROMAPI_CT_API ((RSI_ROM_API)->pCTROM)

/* Pointer to RPDMA peripheral driver functions in ROM */
#define ROMAPI_GPDMA_API ((RSI_ROM_API)->pRPDMAROM)

/* Pointer to POWER driver functions in ROM */
#define ROMAPI_PWR_API ((RSI_ROM_API)->pPWR)

/* Pointer to M4SS CLOCKS  driver functions in ROM */
#define ROMAPI_M4SS_CLK_API ((RSI_ROM_API)->pM4SSCLK)

/*ULP clocks*/
#define ROMAPI_ULPSS_CLK_API ((RSI_ROM_API)->pULPSSCLK)

/* Pointer to QSPI peripheral driver functions in ROM */
#define ROMAPI_QSPI_API ((RSI_ROM_API)->pQSPIROM)

#if !defined(SLI_SI917B0)
/* Pointer to EFUSE driver functions in ROM */
#define ROMAPI_EFUSE_API ((RSI_ROM_API)->pEFUSEROM)
#endif

/* Pointer to CRC peripheral driver functions in ROM */
#define ROMAPI_CRC_API ((RSI_ROM_API)->pCRCROM)

/* Pointer to RNG peripheral driver functions in ROM */
#define ROMAPI_RNG_API ((RSI_ROM_API)->pRNGROM)

#if !defined(SLI_SI917B0)
/* Pointer to MCPWM peripheral driver functions in ROM */
#define ROMAPI_MCPWM_API ((RSI_ROM_API)->pMCPWMROM)
#endif

/* Pointer to USART peripheral driver functions in ROM */
#define ROMAPI_USART_API ((RSI_ROM_API)->pUSARTROM)

/* Pointer to GSPI peripheral driver functions in ROM */
#define ROMAPI_GSPI_API ((RSI_ROM_API)->pGSPIROM)

/* Pointer to I2S peripheral driver functions in ROM */
#define ROMAPI_I2S_API ((RSI_ROM_API)->pI2SROM)

/* Pointer to I2C peripheral driver functions in ROM */
#define ROMAPI_I2C_API ((RSI_ROM_API)->pI2CROM)

#ifdef CHIP_9118
/* Pointer to WIRELESS driver functions in ROM */
#define ROMAPI_WL ((RSI_ROM_API)->pWLROM)
#endif

#if defined(SLI_SI917)
/* Pointer to SSI driver functions in ROM */
#define ROMAPI_SSI_API ((RSI_ROM_API)->pSSIROM)
#endif

#ifdef __cplusplus
}
#endif

#endif

#endif /* __RSI_ROM_TABLE_RS9116_H__ */
