/***************************************************************************/ /**
* @file  rsi_qspi_proto.h
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

#include "rsi_ccp_common.h"
#include "base_types.h"

#ifndef QSPI_PROTO_H
#define QSPI_PROTO_H

#ifdef __cplusplus
extern "C" {
#endif

// GPIO register bit position related defines
#define GPIO_0_TO_5   0
#define GPIO_6_TO_11  1
#define GPIO_46_TO_51 2
#define GPIO_52_TO_57 3
#define GPIO_58_TO_63 4
// NWP OCTA/DUAL combinations
#define GPIO_0_AND_58  5
#define GPIO_6_AND_58  6
#define GPIO_46_AND_58 7
// M4 OCTA/DUAL combinations
#define GPIO_0_AND_52  8
#define GPIO_6_AND_52  9
#define GPIO_46_AND_52 10
#define NOT_USING      11 // Not applicable
#define GPIO_58_AND_52 12
#define GPIO_DDR_PADS  13

#define M4SS_PAD_CONFIG_REG(x) *(volatile uint32_t *)(0x46004000 + 4 * (x)) // REN enable bit(this should be enable)
#define M4_DDR_PAD_CONFIG(x)   *(volatile uint32_t *)(0x46006000 + ((x)*4))
// Pad config(P2 P1)
// 00 - Hi-Z, 01 - Pullup, 10 - PullDown, 11 - Repeater
#define PAD_CONFIG_P2 BIT(7)
#define PAD_CONFIG_P1 BIT(6)

#define PADSELECTION \
  (*(volatile uint32_t *)(0x41300000 + 0x610)) // PAD selection (0 t0 21) A value of 1 on this gives control to M4SS
// Note BIT(0) to BIT(7) are reserved for channel no
#define DEFAULT_DESC_MODE BIT(8)
#define USE_UDMA_MODE     BIT(9)

typedef enum qspi_mode_e { NONE = 0, READ_MODE, WRITE_MODE } qspi_mode_t;

// Structure to qspi standalone encrypt/decrypt configs
typedef struct qspi_standalone_config_s {
  uint8_t aes_mode;   // AES mode
  bool encrypt;       // 0 = Encrypt, 1 = Decrypt
  bool kh_enable;     // 1 = Pick the key from keyholder, 0 = pass the key
  uint32_t *iv;       // flash offset where data stored
  uint32_t *key1;     // Pass key1 if kh_enable = 0
  uint32_t *key2;     // Pass key1 if kh_enable = 0 and 32 byte key
  uint32_t key_len;   // Key len i.e 16 or 32 bytes
  uint32_t flip_data; // writing 1 to this Flips the 32-bit endian for data in standalone mode
} qspi_standalone_config_t;

extern qspi_mode_t qspi_mode_g;

typedef struct qspi_reg_s qspi_reg_t;
// This structure members are used to configure qspi
typedef struct spi_config_1_s {
  // QSPI operation modes, all modes are single, dual or quad

  unsigned int inst_mode : 2;       // instruction will be sent in this mode
  unsigned int addr_mode : 2;       // addr will be sent in this mode
  unsigned int data_mode : 2;       // data will be sent/received in this mode
  unsigned int dummy_mode : 2;      // dummy bytes will be sent/received in this mode
  unsigned int extra_byte_mode : 2; // extra bytes will be sent in this mode
                                    // SPI mode
#define SINGLE_MODE 0
  // dual mode
#define DUAL_MODE 1
  // quad mode
#define QUAD_MODE 2
#define OCTA_MODE 3

  unsigned int prefetch_en : 1; // prefetch enable
// prefetch will be enabled
#define EN_PREFETCH 1
// prefetch will be disabled
#define DIS_PREFETCH 0

  unsigned int dummy_W_or_R : 1; // dummy writes or read select
// dummy's are read
#define DUMMY_READS 0
// dummy's are written
#define DUMMY_WRITES 1

  unsigned int extra_byte_en : 1; // Enable extra byte
                                  // Extra byte will be enabled
#define EN_EXTRA_BYTE
  // Extra byte will be disabled
#define DIS_EXTRA_BYTE

  unsigned int d3d2_data : 2; // Data on D3 and D2 line in SPI or DUAL mode

  unsigned int continuous : 1; // continuous mode select
                               // continuous mode is selected
#define CONTINUOUS 1
  // discontinuous mode is selected
#define DIS_CONTINUOUS 0

  unsigned int read_cmd : 8; // read cmd to be used

  unsigned int flash_type : 4; // flash defines
                               // sst spi flash
#define FREAD_QUAD_O 0x6B

#define FREAD_QUAD_O_EB 0xEB

// WINBOND + MACRONIX specific cmds

// fast read dual IO
#define FREAD_DUAL_IO 0xBB
// fast read quad IO
#define FREAD_QUAD_IO 0xEB
#define SST_SPI_FLASH 0
  // sst dual flash
#define SST_DUAL_FLASH 1
  // sst quad flash
#define SST_QUAD_FLASH 2
  // Winbond quad flash
#define WBOND_QUAD_FLASH 3
  // Atmel quad flash
#define AT_QUAD_FLASH 4
  // macronix quad flash
#define MX_QUAD_FLASH 5
  // cFeon quad flash
#define EON_QUAD_FLASH 6
  // Micron quad flash
#define MICRON_QUAD_FLASH 7
  // Giga Device flash
#define GIGA_DEVICE_FLASH 8
  // macronix octa flash
#define MX_OCTA_FLASH 9
  // Adesto octa flash
#define ADESTO_OCTA_FLASH 10

#if defined(SLI_SI917)
  // Adesto quad flash
#define ADESTO_QUAD_FLASH 11
  //ISSI flash
#define ISSI_FLASH 12

  //XMC fash
#define XMC_FLASH 13

#endif
  unsigned int no_of_dummy_bytes : 4; // no_of_dummy_bytes to be used for read operations
} spi_config_1_t;

// This structure members are used to configure qspi
typedef struct spi_config_2_s {

  unsigned int auto_mode : 1; // mode select
// Auto mode selection
#define EN_AUTO_MODE 1
  // Manual mode selection
#define EN_MANUAL_MODE 0

  unsigned int cs_no : 2; //  QSPI chip_select
// cs-0
#define CHIP_ZERO 0
// cs-1
#define CHIP_ONE 1
// cs-2
#define CHIP_TWO 2
// cs-3
#define CHIP_THREE 3

  unsigned int reserved1 : 1; // Jump Enable
// Enables jump
#define EN_JUMP 1
// Disables jump
#define DIS_JUMP 0

  unsigned int neg_edge_sampling : 1; // For High speed mode, sample at neg edge
// enables neg edge sampling
#define NEG_EDGE_SAMPLING 1
// enables pos edge sampling
#define POS_EDGE_SAMPLING 0

  unsigned int qspi_clk_en : 1; // qspi clk select
// full time clk will be provided
#define QSPI_FULL_TIME_CLK 1
// dynamic clk gating will be enabled
#define QSPI_DYNAMIC_CLK 0

  unsigned int protection : 2; // flash protection select
// enable write protection
#define EN_WR_PROT 2
// remove write protection
#define REM_WR_PROT 1
// no change to wr protection
#define DNT_REM_WR_PROT 0

  unsigned int dma_mode : 1; // dma mode enable
// use dma only in manaul mode
#define DMA_MODE 1
// dma will not be used
#define NO_DMA 0

  unsigned int swap_en : 1; // swap enable for w/r
// swap will be enabled
#define SWAP 1
// swap will be disabled
#define NO_SWAP 0

  unsigned int full_duplex : 2; // full duplex mode select
// do nothing for full duplex
#define IGNORE_FULL_DUPLEX 2
// enable full duplex
#define EN_FULL_DUPLEX 1
// disable full duplex
#define DIS_FULL_DUPLEX 0

  unsigned int wrap_len_in_bytes : 3; // wrap len to be used
// wrap is diabled
#define NO_WRAP 7
// 8 byte wrap will be used
#define SST_8BYTE_WRAP 0
// 16 byte wrap will be used
#define SST_16BYTE_WRAP 1
// 32 byte wrap will be used
#define SST_32BYTE_WRAP 2
// 64 byte wrap will be used
#define SST_64BYTE_WRAP 3

// 16 byte wrap will be used
#define MICRON_16BYTE_WRAP 0
// 32 byte wrap will be used
#define MICRON_32BYTE_WRAP 1
// 64 byte wrap will be used
#define MICRON_64BYTE_WRAP 2

  unsigned int addr_width_valid : 1;
  // mode 3 clk will be used
  // mode 0 clk will be used

  unsigned int addr_width : 3; // addr width to used
// 32 bit addr is configured
#define _32BIT_ADDR 4
// 24 bit addr is configured
#define _24BIT_ADDR 3
// 16 bit addr is configured
#define _16BIT_ADDR 2
// 9 bit addr is configured
#define _9BIT_ADDR 1
// 8 bit addr is configured
#define _8BIT_ADDR 0

#define MANUAL_DUMMY_BYTE_OR_BIT_MODE BIT(25)
#define DUMMY_BYTE_OR_BIT_MODE        BIT(0)
  unsigned int dummy_cycles_for_controller : 2;

  unsigned int reserved2 : 6;
  //  uint32 jump_inst : 8;  // Instruction to be used in case of jump

  unsigned int pinset_valid : 1;

  unsigned int flash_pinset : 4; // width of memory protection reg for sst flashes

} spi_config_2_t;

// This structure members are used to configure qspi
typedef struct spi_config_3_s {
#define CONTINUE_FETCH_EN BIT(12)
#define WORD_SWAP_EN      20
  unsigned int en_word_swap : 1;
  unsigned int _16bit_cmd_valid : 1;
  unsigned int _16bit_rd_cmd_msb : 8;
  unsigned int xip_mode : 1;
  unsigned int no_of_dummy_bytes_wrap : 4; // no_of_dummy_bytes to be used for wrap operations
#ifdef CHIP_9118
  unsigned int ddr_mode_en : 1;
#else
  unsigned int reserved : 1;
#endif
  unsigned int wr_cmd : 8;
  unsigned int wr_inst_mode : 2;
  unsigned int wr_addr_mode : 2;
  unsigned int wr_data_mode : 2;
  unsigned int dummys_4_jump : 2; // no_of_dummy_bytes in case of jump instruction
} spi_config_3_t;

typedef struct spi_config_4_s {
  unsigned int _16bit_wr_cmd_msb : 8;
  unsigned int high_perf_mode_en : 1; //used for high performance mode not ddr
  unsigned int qspi_loop_back_mode_en : 1;
#ifdef CHIP_9118
  unsigned int qspi_manual_ddr_phasse : 1;
  unsigned int ddr_data_mode : 1;
  unsigned int ddr_inst_mode : 1;
  unsigned int ddr_addr_mode : 1;
  unsigned int ddr_dummy_mode : 1;
  unsigned int ddr_extra_byte : 1;
#else
  unsigned int reserved : 1;
  unsigned int reserved1 : 1;
  unsigned int reserved2 : 1;
  unsigned int reserved3 : 1;
  unsigned int reserved4 : 1;
  unsigned int reserved5 : 1;
#endif
  unsigned int dual_flash_mode : 1;
  unsigned int secondary_csn : 1;
  unsigned int polarity_mode : 1;
  unsigned int valid_prot_bits : 4;
  unsigned int no_of_ms_dummy_bytes : 4;
#ifdef CHIP_9118
  unsigned int ddr_dll_en : 1;
#else
  unsigned int reserved6 : 1;
#endif
  unsigned int continue_fetch_en : 1;
  unsigned int dma_write : 1;
  unsigned int prot_top_bottom : 1;
  unsigned int auto_csn_based_addr_en : 1;
} spi_config_4_t;
typedef struct spi_config_5_s {
  unsigned int block_erase_cmd : 16;
  unsigned int busy_bit_pos : 3;
  unsigned int d7_d4_data : 4;
  unsigned int dummy_bytes_for_rdsr : 4;
  unsigned int reset_type : 5;
} spi_config_5_t;

typedef struct spi_config_6_s {
  unsigned int chip_erase_cmd : 16;
  unsigned int sector_erase_cmd : 16;
} spi_config_6_t;

typedef struct spi_config_7_s {
  unsigned int status_reg_write_cmd : 16;
  unsigned int status_reg_read_cmd : 16;
} spi_config_7_t;

// This structure has two daughter structures to configure qspi
typedef struct spi_config_s {
  spi_config_1_t spi_config_1; // daughter structure 1
  spi_config_2_t spi_config_2; // daughter structure 2
  spi_config_3_t spi_config_3; // daughter structure 3
  spi_config_4_t spi_config_4; // daughter structure 4
  spi_config_5_t spi_config_5; // daughter structure 5
  spi_config_6_t spi_config_6; // daughter structure 5
  spi_config_7_t spi_config_7; // daughter structure 5
} spi_config_t;

typedef const struct qspi_func_s qspi_func_t;

struct qspi_func_s {

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
                           uint32_t dma_flags,
                           void *udmaHandle,
                           void *rpdmaHandle);

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
                        uint32_t RSI_QSPI_FlashInit_req,
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
                             uint32_t dma_flags,
                             void *udmaHandle,
                             void *rpdmaHandle);

  void (*qspi_spi_read)(qspi_reg_t *qspi_reg,
                        spi_config_t *spi_config,
                        uint32_t addr,
                        uint8_t *data,
                        uint32_t hsize,
                        uint32_t len_in_bytes,
                        uint32_t dma_flags,
                        void *udmaHandle,
                        void *rpdmaHandle);
  void (*RSI_QSPI_ConfigureQspiRead)(spi_config_t *spi_config, qspi_func_t *qspi_func);

  void (*RSI_QSPI_ConfigureQspiWrite)(spi_config_t *spi_config, qspi_func_t *qspi_func);
  void (*qspi_usleep)(uint32_t delay_us); // function ptr for halting processor for delay (us) specified

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
};

// SPI API LIST

uint32_t qspi_flash_reg_read(qspi_reg_t *qspi_reg, uint8_t reg_read_cmd, uint32_t cs_no, spi_config_t *spi_config);

void qspi_status_reg_write(qspi_reg_t *qspi_reg,
                           uint32_t write_value,
                           spi_config_t *spi_config,
                           uint32_t wr_reg_delay_ms);

void qspi_enable_status_reg_write(qspi_reg_t *qspi_reg, uint32_t flash_type, spi_config_t *spi_config, uint32_t cs_no);

uint32_t qspi_wait_flash_status_Idle(qspi_reg_t *qspi_reg, spi_config_t *spi_config, uint32_t wr_reg_delay_ms);

void qspi_spi_init(qspi_reg_t *qspi_reg,
                   spi_config_t *spi_config,
                   uint32_t flash_init_req,
                   uint32_t wr_reg_delay_ms,
                   uint8_t fifo_thrsld);
#define FLASH_INIT_REQ  1
#define SKIP_FLASH_INIT 0

void qspi_spi_erase(qspi_reg_t *qspi_reg,
                    spi_config_t *spi_config,
                    uint32_t erase_cmd,
                    uint32_t blk_sec_addr,
                    uint32_t dis_hw_ctrl,
                    uint32_t wr_reg_delay_ms);
// chip erase cmd
#define CHIP_ERASE 0xC7
// block erase cmd
#define BLOCK_ERASE 0xD8
// sector erase cmd
#define SECTOR_ERASE 0x20

// disable hw ctrl
#define DIS_HW_CTRL 1
// donot disable hw ctrl
#define DNT_DIS_HW_CTRL 0

// 32bit hsize
#define _32BIT 3
// 24bit hsize is not supported, so reserved
//      reserved                     2
// 16bit hsize
#define _16BIT 1
// 8bit hsize
#define _8BIT 0

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
                        void *rpdmaHandle);

void qspi_spi_read(qspi_reg_t *qspi_reg,
                   spi_config_t *spi_config,
                   uint32_t addr,
                   uint8_t *data,
                   uint32_t hsize,
                   uint32_t len_in_bytes,
                   uint32_t dma_flags,
                   void *udmaHandle,
                   void *rpdmaHandle);

uint32_t RSI_QSPI_Aes_Encrypt_Decrypt_Standalone(qspi_reg_t *qspi_reg,
                                                 qspi_standalone_config_t *configs,
                                                 uint32_t *in_data,
                                                 uint32_t *out_data,
                                                 uint32_t data_length);

// ROM API Fuctions

void qspi_write_block_protect(qspi_reg_t *qspi_reg,
                              uint32_t protect,
                              uint32_t cs_no,
                              uint32_t num_prot_bytes,
                              uint32_t wr_reg_delay_ms);
void qspi_usleep(uint32_t delay_us); // function ptr for halting processor for delay (us) specified
void qspi_auto_init(qspi_reg_t *qspi_reg, spi_config_t *spi_config);
void qspi_set_flash_mode(qspi_reg_t *qspi_reg,
                         uint32_t data_mode,
                         uint32_t cs_no,
                         uint32_t ddr_en,
                         uint32_t flash_type);

void qspi_write_to_flash(qspi_reg_t *qspi_reg, uint32_t len_in_bits, uint32_t cmd_addr_data, uint32_t cs_no);

void qspi_switch_qspi2(qspi_reg_t *qspi_reg, uint32_t mode, uint32_t cs_no);

void qspi_flash_reg_write(qspi_reg_t *qspi_reg,
                          uint32_t reg_write_cmd,
                          uint32_t reg_write_value,
                          uint32_t cs_no,
                          uint32_t wr_reg_delay_ms);

void qspi_config_qflash4_read(qspi_reg_t *qspi_reg, spi_config_t *spi_config, uint32_t addr);

void qspi_manual_read(qspi_reg_t *qspi_reg,
                      spi_config_t *spi_config,
                      uint32_t addr,
                      uint8_t *data,
                      uint32_t hsize,
                      uint32_t len_in_bytes,
                      uint32_t dma_flags,
                      void *udmaHandle,
                      void *rpdmaHandle);

void qspi_auto_read(uint32_t cs_no,
                    uint32_t addr,
                    uint8_t *data,
                    uint32_t hsize,
                    uint32_t len_in_bytes,
                    spi_config_t *spi_config,
                    uint32_t dma_flags);

void qspi_flash_init(qspi_reg_t *qspi_reg, spi_config_t *spi_config, uint32_t wr_reg_delay_ms);

#if defined(SLI_SI917)
#if defined(SLI_SI917B0)
void qspi_qspiload_key(qspi_reg_t *qspi_reg,
                       uint8_t mode,
                       uint32_t *key1,
                       uint32_t *key2,
                       uint32_t key_len,
                       uint32_t kh_enable);
#else
void qspi_qspiload_key(qspi_reg_t *qspi_reg, uint8_t mode, uint32_t *key, uint32_t kh_enable);
#endif
#else
void qspi_qspiload_key(qspi_reg_t *qspi_reg, uint32_t *key, uint32_t kh_enable);
#endif

void qspi_qspiload_nonce(qspi_reg_t *qspi_reg, uint32_t *nonce);

void qspi_seg_sec_en(qspi_reg_t *qspi_reg, uint32_t seg_no, uint32_t start_addr, uint32_t end_addr);
void qspi_status_control_reg_write(spi_config_t *spi_config,
                                   qspi_reg_t *qspi_reg,
                                   uint16_t write_command,
                                   uint32_t addr,
                                   uint16_t write_value,
                                   uint32_t cs_no,
                                   uint32_t wr_reg_delay_ms);
void qspi_flash_protection(spi_config_t *spi_config,
                           qspi_reg_t *qspi_reg,
                           uint32_t protection,
                           uint32_t wr_reg_delay_ms);

void RSI_QSPI_ResetFlash(qspi_reg_t *qspi_reg, uint32_t cs_no);

void RSI_QSPI_UpdateOperatingMode_and_ResetType(qspi_reg_t *qspi_reg, uint32_t operating_mode);

extern spi_config_t spi_default_config;

#ifdef __cplusplus
}
#endif

#endif // QSPI_PROTO_H
