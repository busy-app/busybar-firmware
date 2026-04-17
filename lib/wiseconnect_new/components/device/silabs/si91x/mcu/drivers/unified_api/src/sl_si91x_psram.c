/******************************************************************************
* @file  sl_si91x_psram.c
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

#include "UDMA.h"
#include "rsi_pll.h"
#include "rsi_qspi.h"
#include "rsi_rom_qspi.h"
#include "rsi_rom_clks.h"
#include "rsi_rom_egpio.h"
#include "rsi_rom_udma_wrapper.h"
#include "rsi_rom_udma.h"
#include "sl_si91x_psram_handle.h"
#include "sl_si91x_psram.h"
/*******************************************************************************
 *******************************   DEFINES   ***********************************
 ******************************************************************************/

#define QSPI_DMA_SRC_CHANNEL_NUM 4

#define QSPI_DMA_DEST_CHANNEL_NUM 5

#define MAX_QSPI_READ_LENGTH 32764 /// ((32 * 1024) - 4)

#define tXHS_MAX_COUNT 1850

/*******************************************************************************
 ******************************   VARIABLES   **********************************
 ******************************************************************************/

extern struct PSRAMSecureSegmentType PSRAMSecureSegments[MAX_SEC_SEGMENTS];

static sl_psram_return_type_t autoModeState = PSRAM_UNKNOWN;

spi_config_t spi_psram_default_config = {

  .spi_config_1 = { .inst_mode         = SINGLE_MODE,
                    .addr_mode         = SINGLE_MODE,
                    .dummy_mode        = SINGLE_MODE,
                    .data_mode         = SINGLE_MODE,
                    .extra_byte_mode   = SINGLE_MODE,
                    .prefetch_en       = DIS_PREFETCH,
                    .dummy_W_or_R      = DUMMY_READS,
                    .d3d2_data         = 3,
                    .continuous        = DIS_CONTINUOUS,
                    .read_cmd          = 0x03,
                    .flash_type        = 0xf,
                    .no_of_dummy_bytes = 0,
                    .extra_byte_en     = 0 },

  .spi_config_2 = { .auto_mode                   = EN_MANUAL_MODE,
                    .cs_no                       = PSRAM_CHIP_SELECT,
                    .neg_edge_sampling           = NEG_EDGE_SAMPLING,
                    .qspi_clk_en                 = QSPI_FULL_TIME_CLK,
                    .protection                  = DNT_REM_WR_PROT,
                    .dma_mode                    = NO_DMA,
                    .swap_en                     = NO_SWAP,
                    .full_duplex                 = IGNORE_FULL_DUPLEX,
                    .wrap_len_in_bytes           = NO_WRAP,
                    .addr_width_valid            = 0,
                    .addr_width                  = _24BIT_ADDR,
                    .pinset_valid                = 0,
                    .dummy_cycles_for_controller = 0 },

  .spi_config_3 = { .xip_mode          = 0,
                    ._16bit_cmd_valid  = 0,
                    ._16bit_rd_cmd_msb = 0,
                    .reserved          = 0,
                    .wr_cmd            = 0x2,
                    .wr_inst_mode      = SINGLE_MODE,
                    .wr_addr_mode      = SINGLE_MODE,
                    .wr_data_mode      = SINGLE_MODE,
                    .dummys_4_jump     = 1 },

  .spi_config_4 = { ._16bit_wr_cmd_msb    = 0,
                    .dual_flash_mode      = 0,
                    .secondary_csn        = 1,
                    .polarity_mode        = 0,
                    .valid_prot_bits      = 4,
                    .no_of_ms_dummy_bytes = 0,
                    .continue_fetch_en    = 0 },

};

static struct PSRAMStatusType PSRAMStatus = {
  .state               = unknown,
  .interfaceMode       = SINGLE_MODE,
  .burstSize           = 0,
  .secureModeEnable    = false,
  .secureSegmentNumber = 0,
};

struct PSRAMPinConfigType PSRAMPinConfig[NUM_OF_PSRAM_PINS] = {
  { M4SS_PSRAM_CLK_PORT, M4SS_PSRAM_CLK_PIN, M4SS_PSRAM_CLK_MUX, M4SS_PSRAM_CLK_PAD },
  { M4SS_PSRAM_CSN_PORT, M4SS_PSRAM_CSN_PIN, M4SS_PSRAM_CSN_MUX, M4SS_PSRAM_CSN_PAD },
  { M4SS_PSRAM_D0_PORT, M4SS_PSRAM_D0_PIN, M4SS_PSRAM_D0_MUX, M4SS_PSRAM_D0_PAD },
  { M4SS_PSRAM_D1_PORT, M4SS_PSRAM_D1_PIN, M4SS_PSRAM_D1_MUX, M4SS_PSRAM_D1_PAD },
  { M4SS_PSRAM_D2_PORT, M4SS_PSRAM_D2_PIN, M4SS_PSRAM_D2_MUX, M4SS_PSRAM_D2_PAD },
  { M4SS_PSRAM_D3_PORT, M4SS_PSRAM_D3_PIN, M4SS_PSRAM_D3_MUX, M4SS_PSRAM_D3_PAD },
#if (PSRAM_GPIO_PIN_SET_SEL == PSRAM_GPIO_PIN_SET_46_TO_57_CS_0) \
  || (PSRAM_GPIO_PIN_SET_SEL == PSRAM_GPIO_PIN_SET_46_TO_57_CS_1)
  { M4SS_PSRAM_D4_PORT, M4SS_PSRAM_D4_PIN, M4SS_PSRAM_D4_MUX, M4SS_PSRAM_D4_PAD },
  { M4SS_PSRAM_D5_PORT, M4SS_PSRAM_D5_PIN, M4SS_PSRAM_D5_MUX, M4SS_PSRAM_D5_PAD },
  { M4SS_PSRAM_D6_PORT, M4SS_PSRAM_D6_PIN, M4SS_PSRAM_D6_MUX, M4SS_PSRAM_D6_PAD },
  { M4SS_PSRAM_D7_PORT, M4SS_PSRAM_D7_PIN, M4SS_PSRAM_D7_MUX, M4SS_PSRAM_D7_PAD },
#endif
};

/// DMA descriptors must be aligned to 16 bytes
#if defined(__CC_ARM)
extern RSI_UDMA_DESC_T UDMA0_Table[CONTROL_STRUCT0];
#endif /* defined (__CC_ARM) */

#if defined(__GNUC__)
extern RSI_UDMA_DESC_T __attribute__((section(".udma_addr0"))) UDMA0_Table[CONTROL_STRUCT0];
#endif /* defined (__GNUC__) */

extern UDMA_Channel_Info udma0_chnl_info[32];

/// UDMA0 Resources
extern UDMA_RESOURCES UDMA0_Resources;

extern RSI_UDMA_HANDLE_T udmaHandle0;

extern uint32_t dma_rom_buff0[30];

static volatile struct xferContextType ctx;
RSI_UDMA_CHA_CONFIG_DATA_T control;
RSI_UDMA_CHA_CFG_T config;

/*******************************************************************************
 *********************   LOCAL FUNCTION PROTOTYPES   ***************************
 ******************************************************************************/

/***************************************************************************/ /**
 * @brief       
 *   Enable PSRAM Auto Read-Write mode. The API waits until idle state of QSPI 
 *   controller and then enables the Auto mode.                       
 *
 * @return      
 *   Status Code of the operation
 ******************************************************************************/
static sl_psram_return_type_t psram_enable_auto_readwrite(void);

/***************************************************************************/ /**
 * @brief       
 *   Disable PSRAM Auto Read-Write mode. The API waits until idle state of QSPI 
 *   controller and then disable the Auto mode.                        
 *
 * @return      
 *   Status Code of the operation
 ******************************************************************************/
static sl_psram_return_type_t psram_disable_auto_readwrite(void);

/***************************************************************************/ /**
 * @brief       
 *   Read the Manufactoring ID, KGD (Known Good Die) & EID.                        
 *
 * @return      
 *   Read ID structure
 ******************************************************************************/
static sl_psram_id_type_t psram_read_id(void);

/***************************************************************************/ /**
 * @brief       
 *   Change interface mode to QPI (Quad IO).
 * 
 * @note 
 *   "Enter Quad/QPI Mode" command is only supported when current interface mode 
 *   is SPI (Serial IO).                       
 *
 * @return      
 *   Status Code of the operation
 ******************************************************************************/
static sl_psram_return_type_t psram_enter_qpi_mode(void);

/***************************************************************************/ /**
 * @brief       
 *   Change interface mode to SPI (Serial IO).
 * 
 * @note 
 *   "Exit Quad/QPI Mode" command is only supported when current interface mode 
 *   is QPI (Quad IO).                       
 *
 * @return      
 *   Status Code of the operation
 ******************************************************************************/
static sl_psram_return_type_t psram_exit_qpi_mode(void);

#if (PSRAM_MODEL_WRAP == 1 && PSRAM_BURST_LEN_TOGGLE_SUPPORTED)

/***************************************************************************/ /**
 * @brief       
 *   Toggles the device’s burst length wrap between PSRAM_DEFAULT_BURST_WRAP_SIZE 
 *   and PSRAM_TOGGLE_BURST_WRAP_SIZE                        
 *
 * @return      
 *   Status Code of the operation
 ******************************************************************************/
static sl_psram_return_type_t psram_toggle_burst_length(void);

/***************************************************************************/ /**
 * @brief       
 *   Writes wrap length to Mode Register
 * 
 * @param[in] PSRAMBurstSize
 *   Wrap burst size enum value                         
 *
 * @return      
 *   Status Code of the operation
 ******************************************************************************/
static sl_psram_return_type_t psram_set_wrap_size(sl_psram_burst_size_type_t PSRAMBurstSize);

#endif // PSRAM_BURST_LEN_TOGGLE_SUPPORTED

/***************************************************************************/ /**
 * @brief       
 *   Transmit data via QSPI interface
 * 
 * @param[in] qspi_reg
 *   QSPI register address
 * 
 * @param[in] hSize
 *   Size of each element 
 *  
 * @param[in] data
 *   Reference of the Source buffer
 * 
 * @param[in] length
 *   Number of data elements
 * 
 * @param[in] chipSelect
 *   Chip select number
 * 
 * @param[in] chipSelectContinue
 *   boolean to keep CS line asserted                       
 ******************************************************************************/
static void qspi_transmit(qspi_reg_t *qspi_reg,
                          uint8_t hSize,
                          void *data,
                          uint32_t length,
                          uint8_t chipSelect,
                          uint8_t chipSelectContinue);

/***************************************************************************/ /**
 * @brief       
 *   Receive data via QSPI interface
 * 
 * @param[in] qspi_reg
 *   QSPI register address
 * 
 * @param[in] hSize
 *   Size of each element 
 *  
 * @param[out] data
 *   Reference of the destination buffer
 * 
 * @param[in] length
 *   Number of data elements
 * 
 * @param[in] chipSelect
 *   Chip select number
 * 
 * @param[in] chipSelectContinue
 *   boolean to keep CS line asserted                       
 ******************************************************************************/
static void qspi_receive(qspi_reg_t *qspi_reg,
                         uint8_t hSize,
                         void *data,
                         uint32_t length,
                         uint8_t chipSelect,
                         uint8_t chipSelectContinue);

/***************************************************************************/ /**
 * @brief       
 *   Deassert CS line
 * 
 * @param[in] qspi_reg
 *   QSPI register address                     
 ******************************************************************************/
static void qspi_deassert_csn(qspi_reg_t *qspi_reg);

/***************************************************************************/ /**
 * @brief       
 *   Initialize SRAM auto mode
 * 
 * @param[in] qspi_reg
 *   QSPI register address
 *  
 * @param[in] spi_config
 *   SPI configs for QSPI interface                    
 ******************************************************************************/
static void qspi_sram_auto_init(qspi_reg_t *qspi_reg, spi_config_t *spi_config);

#if (PSRAM_MODEL_WRAP == 1)
/***************************************************************************/ /**
 * @brief       
 *   Initialize SRAM Wrap mode
 * 
 * @param[in] qspi_reg
 *   QSPI register address
 *  
 * @param[in] spi_config
 *   SPI configs for QSPI interface                    
 ******************************************************************************/
static void qspi_sram_wrap_init(qspi_reg_t *qspi_reg, spi_config_t *spi_config);
#endif

/***************************************************************************/ /**
 * @brief       
 *   Set SWAP mode based on SPI config
 * 
 * @param[in] qspi_reg
 *   QSPI register address
 *  
 * @param[in] spi_config
 *   SPI configs for QSPI interface                    
 ******************************************************************************/
static void qspi_set_swap_mode(qspi_reg_t *qspi_reg, spi_config_t *spi_config);

/***************************************************************************/ /**
 * @brief       
 *   This API used to enable auto mode in QSPI
 * 
 * @param[in] qspi_reg
 *   QSPI register address                  
 ******************************************************************************/
static void qspi_auto_mode_en(qspi_reg_t *qspi_reg);

static void qspi_qspiload_key_ext(qspi_reg_t *qspi_reg, uint16_t key_size);

#ifdef SECURITY_KEY_CONFIG
static void qspi_qspiunload_key_ext(qspi_reg_t *qspi_reg);
#endif

/*******************************************************************************
 ***************************   LOCAL FUNCTIONS   *******************************
 ******************************************************************************/

/* Wait for QSPI to become idle */
__STATIC_INLINE void wait_state_manual()
{
  qspi_reg_t *qspi_reg = (qspi_reg_t *)M4_QSPI_2_BASE_ADDRESS;
  // wait till QSPI becomes idle
  while (qspi_reg->QSPI_STATUS_REG & 1)
    ;
}

/* UDMA controller transfer descriptor chain complete callback */
static void udma_transfer_complete(uint32_t event, uint32_t ch)
{
  if (event == UDMA_EVENT_XFER_DONE) {
    if (ch == 5 || ch == 4) {

      wait_state_manual();
      qspi_deassert_csn((qspi_reg_t *)M4_QSPI_2_BASE_ADDRESS);

      RSI_UDMA_DeInit(udmaHandle0, &config);
      if ((ctx.xferRemLength > 0) && (ctx.xferStatus == TX_RUNNING)) {
        if (sl_si91x_psram_manual_write_in_dma_mode(ctx.xferNextAddress,
                                                    ctx.xferNextSourceAddress,
                                                    ctx.xferHsize,
                                                    ctx.xferRemLength,
                                                    ctx.done)
            != PSRAM_SUCCESS) {

#if (PSRAM_MODEL_WRAP == 1 && PSRAM_BURST_LEN_TOGGLE_SUPPORTED)
          psram_set_wrap_size(PSRAM_Device.toggleBurstWrapSize);
          PSRAMStatus.burstSize = PSRAM_Device.toggleBurstWrapSize;
#endif
          if (PSRAM_MANUAL_MODE != autoModeState) {
            psram_enable_auto_readwrite();
          }
          ctx.xferStatus = FAILED;
          /*Notify the Failure*/
          *ctx.done = DMA_FAIL;
        }
      } else if ((ctx.xferRemLength > 0) && (ctx.xferStatus == RX_RUNNING)) {
        if (sl_si91x_psram_manual_read_in_dma_mode(ctx.xferNextAddress,
                                                   ctx.xferNextSourceAddress,
                                                   ctx.xferHsize,
                                                   ctx.xferRemLength,
                                                   ctx.done)
            != PSRAM_SUCCESS) {
#if (PSRAM_MODEL_WRAP == 1 && PSRAM_BURST_LEN_TOGGLE_SUPPORTED)
          psram_set_wrap_size(PSRAM_Device.toggleBurstWrapSize);
          PSRAMStatus.burstSize = PSRAM_Device.toggleBurstWrapSize;
#endif
          if (PSRAM_MANUAL_MODE != autoModeState) {
            psram_enable_auto_readwrite();
          }

          ctx.xferStatus = FAILED;
          /*Notify the Failure*/
          *ctx.done = DMA_FAIL;
        }
      } else {

        /*Clear the xfer Context*/
        ctx.xferStatus            = IDLE;
        ctx.xferHsize             = 0;
        ctx.xferNextAddress       = 0;
        ctx.xferNextSourceAddress = 0;
        ctx.xferRemLength         = 0;

#if (PSRAM_MODEL_WRAP == 1 && PSRAM_BURST_LEN_TOGGLE_SUPPORTED)
        psram_set_wrap_size(PSRAM_Device.toggleBurstWrapSize);
        PSRAMStatus.burstSize = PSRAM_Device.toggleBurstWrapSize;
#endif
        if (PSRAM_MANUAL_MODE != autoModeState) {
          psram_enable_auto_readwrite();
        }

        /*Notify the completion*/
        *ctx.done = DMA_DONE;
      }
    }
  }
}

/* This API used to enable auto mode in QSPI */
void qspi_auto_mode_en(qspi_reg_t *qspi_reg)
{
  if (!(qspi_reg->QSPI_STATUS_REG & HW_CTRLD_QSPI_MODE_CTRL_SCLK)) {
    while (!(qspi_reg->QSPI_MANUAL_CONFIG_REG & CSN_ACTIVE)) {
      //wait till manual becomes idle
    }
    qspi_reg->QSPI_BUS_MODE_REG |= AUTO_MODE;
  }
}

/* Transmit data via QSPI interface */
void qspi_transmit(qspi_reg_t *qspi_reg,
                   uint8_t hSize,
                   void *data,
                   uint32_t length,
                   uint8_t chipSelect,
                   uint8_t chipSelectContinue)
{

  uint8_t noOfElelments;

  hSize = hSize - 1;

  while (length) {

    if (length >= 16) {
      noOfElelments = 16;
    } else {
      noOfElelments = (length % 16);
    }

    for (uint32_t fifIndex = 0; fifIndex < noOfElelments; fifIndex++) {

      if (_32BIT == hSize) {
        // length of the word to be programmed
        qspi_reg->QSPI_MANUAL_WRITE_DATA_2_REG = 32;
        // write the data
        qspi_reg->QSPI_MANUAL_RD_WR_DATA_REG = *((uint32_t *)data);
        data                                 = ((uint32_t *)data) + 1;
      } else if (_16BIT == hSize) {
        // length of the word to be programmed
        qspi_reg->QSPI_MANUAL_WRITE_DATA_2_REG = 16;
        // write the data
        qspi_reg->QSPI_MANUAL_RD_WR_DATA_REG = *((uint16_t *)data);
        data                                 = ((uint16_t *)data) + 1;
      } else if (_8BIT == hSize) {
        // length of the word to be programmed
        qspi_reg->QSPI_MANUAL_WRITE_DATA_2_REG = 8;
        // write the data
        qspi_reg->QSPI_MANUAL_RD_WR_DATA_REG = *((uint8_t *)data);
        data                                 = ((uint8_t *)data) + 1;
      }

      length -= 1;
    }
    qspi_reg->QSPI_MANUAL_CONFIG_REG =
      (((qspi_reg->QSPI_MANUAL_CONFIG_REG & ~0x6007) & ~TAKE_LEN_FRM_REG) | WRITE_TRIGGER | (chipSelect << 13));

    // wait till QSPI becomes idle
    while (qspi_reg->QSPI_STATUS_REG & 1)
      ;
  }

  if (!chipSelectContinue) {
    DEASSERT_CSN;
  }
}

/* Receive data via QSPI interface */
void qspi_receive(qspi_reg_t *qspi_reg,
                  uint8_t hSize,
                  void *data,
                  uint32_t length,
                  uint8_t chipSelect,
                  uint8_t chipSelectContinue)
{

  uint32_t qspi_manual_config_reg;
  uint32_t readCount;
  uint32_t lengthInBytes;
  uint8_t *data_8bit;
  uint16_t *data_16bit;
  uint32_t *data_32bit;

  data_8bit  = (uint8_t *)data;
  data_16bit = (uint16_t *)data;
  data_32bit = (uint32_t *)data;

  lengthInBytes = length * hSize;

  hSize = hSize - 1;

  while (lengthInBytes) {

    if (lengthInBytes > MAX_QSPI_READ_LENGTH) // 32kbytes is a max allowed size in QSPI read length
    {
      readCount = MAX_QSPI_READ_LENGTH;
    } else {
      readCount = lengthInBytes;
    }

    lengthInBytes = lengthInBytes - readCount;

    // configure qspi for read
    qspi_manual_config_reg = (qspi_reg->QSPI_MANUAL_CONFIG_REG & ~0xF8387FFF);
    qspi_manual_config_reg |= (chipSelect << 13) | ((hSize) << 19) | TAKE_LEN_FRM_REG;

    // trigger qspi to read loop_count bytes from flash
    qspi_reg->QSPI_MANUAL_CONFIG_REG = qspi_manual_config_reg | READ_TRIGGER | ((readCount & 0x3FF) << 3)
                                       | (((readCount >> 10) & 0x1F) << 27);

    if (hSize == _32BIT) {
      do {
        while (qspi_reg->QSPI_STATUS_REG & QSPI_FIFO_EMPTY_RFIFO_S) {
          // wait till the fifo empty is deasserted
        }
        *(data_32bit) = (uint32_t)qspi_reg->QSPI_MANUAL_RD_WR_DATA_REG;
        data_32bit++;
        readCount -= 4;
      } while (readCount);
    } else if (hSize == _16BIT) {
      do {
        while (qspi_reg->QSPI_STATUS_REG & QSPI_FIFO_EMPTY_RFIFO_S) {
          // wait till the fifo empty is deasserted
        }
        *(data_16bit) = (uint16_t)qspi_reg->QSPI_MANUAL_RD_WR_DATA_REG;
        data_16bit++;
        readCount -= 2;
      } while (readCount);
    } else if (hSize == _8BIT) {
      do {
        while (qspi_reg->QSPI_STATUS_REG & QSPI_FIFO_EMPTY_RFIFO_S) {
          // wait till the fifo empty is deasserted
        }
        *(data_8bit) = (uint8_t)qspi_reg->QSPI_MANUAL_RD_WR_DATA_REG;
        data_8bit++;
        readCount -= 1;
      } while (readCount);
    }
  }

  if (!chipSelectContinue) {
    DEASSERT_CSN;
  }
}

/* Deassert CS line */
void qspi_deassert_csn(qspi_reg_t *qspi_reg)
{
  DEASSERT_CSN;
}

/* Initialize SRAM auto mode */
void qspi_sram_auto_init(qspi_reg_t *qspi_reg, spi_config_t *spi_config)
{
  uint32_t *sram_ctrl_csn_ptr;

  switch (spi_config->spi_config_2.cs_no) {
    case CHIP_ONE:
      sram_ctrl_csn_ptr = ((uint32_t *)&qspi_reg->QSPI_SRAM_CTRL_CSN1_REG);
      break;
    case CHIP_TWO:
      sram_ctrl_csn_ptr = ((uint32_t *)&qspi_reg->QSPI_SRAM_CTRL_CSN2_REG);
      break;
    case CHIP_THREE:
      sram_ctrl_csn_ptr = ((uint32_t *)&qspi_reg->QSPI_SRAM_CTRL_CSN3_REG);
      break;
    default:
      sram_ctrl_csn_ptr = ((uint32_t *)&qspi_reg->QSPI_SRAM_CTRL_CSN0_REG);
      break;
  }

  *sram_ctrl_csn_ptr = ((spi_config->spi_config_3.wr_inst_mode << 8)    //< cmd mode
                        | (spi_config->spi_config_3.wr_addr_mode << 10) //< addr mode
                        | (spi_config->spi_config_3.wr_data_mode << 12) //< data mode
                        | (spi_config->spi_config_3.wr_cmd << 16));

  // enable write data swapping for auto mode
  if (spi_config->spi_config_2.swap_en == SWAP) {
    *sram_ctrl_csn_ptr |= (SWAP << 14);
  }
}

#if (PSRAM_MODEL_WRAP == 1)
/* Initialize SRAM Wrap mode */
void qspi_sram_wrap_init(qspi_reg_t *qspi_reg, spi_config_t *spi_config)
{
  uint32_t offset;
  uint32_t *auto_1_ptr;
  uint32_t *auto_2_ptr;

  // address difference between auto config reg for csn 0 and 1 is 12regs
  offset     = 12 * spi_config->spi_config_2.cs_no;
  auto_1_ptr = ((uint32_t *)&qspi_reg->QSPI_AUTO_CTRL_CONFIG_1_REG + offset);
  auto_2_ptr = ((uint32_t *)&qspi_reg->QSPI_AUTO_CTRL_CONFIG_2_REG + offset);

  // Enable burst mode read in qspi
  qspi_reg->QSPI_BUS_MODE_REG |= QSPI_WRAP_EN;

  if (spi_config->spi_config_1.inst_mode == SINGLE_MODE) {
    // no. of dummy bytes for wrap
    *auto_1_ptr |= 1 << 28;

  } else if (spi_config->spi_config_1.inst_mode == QUAD_MODE) {
    // no. of dummy bytes for wrap
    *auto_1_ptr |= 3 << 28;
  }

  // read cmd for wrap mode
  *auto_2_ptr |= 0x8B << 16;
}
#endif

/* Set SWAP mode based on SPI config */
void qspi_set_swap_mode(qspi_reg_t *qspi_reg, spi_config_t *spi_config)
{
  /*Set Swap for Manual transactions */
  if (spi_config->spi_config_2.swap_en) {
    // Manual Write Swap
    qspi_reg->QSPI_MANUAL_CONFIG_2_REG |= (SWAP << spi_config->spi_config_2.cs_no);
    // Manual Read Swap
    qspi_reg->QSPI_MANUAL_CONFIG_2_REG |= (SWAP << (4 + spi_config->spi_config_2.cs_no));
  } else {
    // Manual Write Swap
    qspi_reg->QSPI_MANUAL_CONFIG_2_REG &= (~(SWAP << spi_config->spi_config_2.cs_no));
    // Manual Read Swap
    qspi_reg->QSPI_MANUAL_CONFIG_2_REG &= (~(SWAP << (4 + spi_config->spi_config_2.cs_no)));
  }
}

void qspi_qspiload_key_ext(qspi_reg_t *qspi_reg, uint16_t key_size)
{
  uint8_t segIndex;
  uint32_t aes_config;
  uint32_t aes_octa_spi_controller_2;

  aes_config                = qspi_reg->QSPI_AES_CONFIG;
  aes_octa_spi_controller_2 = qspi_reg->OCTA_SPI_BUS_CONTROLLER2;
  aes_config                = CTR_MODE | KEY_FLIP_FOR_KH_INTF;

  if (256 == key_size) {
    aes_config |= KEY_SIZE_256;
    qspi_reg->QSPI_AES_KEY_IV_VALID = ((0xF << 8) | (0xF << 4) | 0xF);
  } else {
    aes_config &= (~KEY_SIZE_256);
    qspi_reg->QSPI_AES_KEY_IV_VALID = ((0xF << 8) | (0xF << 4));
  }

  qspi_reg->QSPI_AES_CONFIG = aes_config;
  qspi_reg->QSPI_AES_SEC_KEY_FRM_KH |= LOAD_SEC_KEY_FRM_KH;
  while (qspi_reg->QSPI_AES_SEC_KEY_FRM_KH & LOAD_SEC_KEY_FRM_KH)
    ;

  for (segIndex = 0; segIndex < MAX_SEC_SEGMENTS; segIndex++) {
    if (PSRAMSecureSegments[segIndex].segmentEnable) {
      qspi_reg->QSPI_AES_SEC_SEG_ADDR[segIndex * 2]     = PSRAMSecureSegments[segIndex].lowerBoundary;
      qspi_reg->QSPI_AES_SEC_SEG_ADDR[segIndex * 2 + 1] = PSRAMSecureSegments[segIndex].higherBoundary;
      aes_octa_spi_controller_2 |= (BIT(segIndex) << EN_SEG_SEC); // enabling security for segment
    }
  }

  aes_octa_spi_controller_2 |= (EN_KH_KEY | EN_SECURITY);

  qspi_reg->OCTA_SPI_BUS_CONTROLLER2 = aes_octa_spi_controller_2; // enabling security ;
}

#ifdef SECURITY_KEY_CONFIG
void qspi_qspiunload_key_ext(qspi_reg_t *qspi_reg)
{

  qspi_reg->OCTA_SPI_BUS_CONTROLLER2 &= ~(EN_KH_KEY | EN_SECURITY);

  qspi_reg->QSPI_AES_CONFIG = DEFAULT_AES_CONFIG;

  qspi_reg->QSPI_AES_KEY_IV_VALID = DEFAULT_AES_KEY_IV_VALID;

  qspi_reg->QSPI_AES_SEC_KEY_FRM_KH = DEFAULT_AES_SEC_KEY_FRM_KH;
}
#endif

/* Enters QPI Mode, can only be called in SPI mode */
sl_psram_return_type_t psram_enter_qpi_mode(void)
{

  uint8_t command[2];

  // Set the PSRAM device to QPI mode
  command[0] = PSRAM_ENTER_QPI;

  qspi_transmit((qspi_reg_t *)M4_QSPI_2_BASE_ADDRESS,
                sizeof(uint8_t),
                command,
                1,
                PSRAM_Device.spi_config.spi_config_2.cs_no,
                0);

  return PSRAM_SUCCESS;
}

/* Exits QPI interface Mode, can only be called in QPI mode */
sl_psram_return_type_t psram_exit_qpi_mode(void)
{

  uint8_t command[2];

  // Set the PSRAM device to SPI mode
  command[0] = PSRAM_EXIT_QPI;

  qspi_transmit((qspi_reg_t *)M4_QSPI_2_BASE_ADDRESS,
                sizeof(uint8_t),
                command,
                1,
                PSRAM_Device.spi_config.spi_config_2.cs_no,
                0);

  return PSRAM_SUCCESS;
}

/* Enables Auto Read-Write mode */
sl_psram_return_type_t psram_enable_auto_readwrite(void)
{

  qspi_reg_t *qspi_reg = (qspi_reg_t *)M4_QSPI_2_BASE_ADDRESS;

  while (!(qspi_reg->QSPI_STATUS_REG & AUTO_MODE_FSM_IDLE_SCLK)) {
    // wait till auto mode becomes idle
  }

  // Check if already in Auto mode
  if ((qspi_reg->QSPI_STATUS_REG & HW_CTRLD_QSPI_MODE_CTRL_SCLK)) {
    return PSRAM_SUCCESS;
  }

  if (qspi_reg->QSPI_BUS_MODE_REG & AUTO_MODE) {
    return PSRAM_AUTO_MODE;
  }

  qspi_auto_mode_en(qspi_reg);

  return PSRAM_SUCCESS;
}

/* Disables Auto Read-Write mode */
sl_psram_return_type_t psram_disable_auto_readwrite(void)
{

  qspi_reg_t *qspi_reg = (qspi_reg_t *)M4_QSPI_2_BASE_ADDRESS;

  while (!(qspi_reg->QSPI_STATUS_REG & AUTO_MODE_FSM_IDLE_SCLK)) {
    // wait till auto mode becomes idle
  }

  // Check if already not in auto mode

  if ((qspi_reg->QSPI_STATUS_REG & HW_CTRLD_QSPI_MODE_CTRL_SCLK)) {
    return PSRAM_SUCCESS;
  }

  if (!(qspi_reg->QSPI_BUS_MODE_REG & AUTO_MODE)) {
    return PSRAM_MANUAL_MODE;
  }

  qspi_reg->QSPI_BUS_MODE_REG &= ~AUTO_MODE;
  while (qspi_reg->QSPI_STATUS_REG & AUTO_MODE_ENABLED)
    ;

  return PSRAM_SUCCESS;
}

/* Reads Manufactoring ID, KGD (Known Good Die) & EID */
sl_psram_id_type_t psram_read_id()
{
  sl_psram_id_type_t psram_id;
  uint8_t command[4];

  spi_config_t *spi_config = &spi_psram_default_config;

  //TODO: To check the start up time after boot exit, should be >150us or else implement wait here

  /*Read ID*/
  command[0] = PSRAM_READ_ID;
  command[1] = 0x00;
  command[2] = 0x00;
  command[3] = 0x00;

  qspi_transmit((qspi_reg_t *)M4_QSPI_2_BASE_ADDRESS,
                sizeof(uint8_t),
                &command[0],
                sizeof(command),
                spi_config->spi_config_2.cs_no,
                1); // do not de-assert the CS

  qspi_receive((qspi_reg_t *)M4_QSPI_2_BASE_ADDRESS,
               sizeof(uint8_t),
               ((uint8_t *)&psram_id),
               sizeof(psram_id),
               spi_config->spi_config_2.cs_no,
               0); // de-assert the CS

  return psram_id;
}

#if (PSRAM_MODEL_WRAP == 1 && PSRAM_BURST_LEN_TOGGLE_SUPPORTED)

/* Toggles the device’s burst length wrap between 
   PSRAM_DEFAULT_BURST_WRAP_SIZE and PSRAM_TOGGLE_BURST_WRAP_SIZE */
sl_psram_return_type_t psram_toggle_burst_length(void)
{

  if (PSRAMStatus.state != initialised) {
    return PSRAM_NOT_INITIALIZED;
  }

  // switch QSPI to command mode
  RSI_QSPI_SwitchQspi2((qspi_reg_t *)M4_QSPI_2_BASE_ADDRESS,
                       PSRAM_Device.spi_config.spi_config_1.inst_mode,
                       PSRAM_Device.spi_config.spi_config_2.cs_no);

  qspi_transmit((qspi_reg_t *)M4_QSPI_2_BASE_ADDRESS,
                sizeof(uint8_t),
                (uint8_t *)PSRAM_BURST_LEN,
                1,
                PSRAM_Device.spi_config.spi_config_2.cs_no,
                0);

  if (PSRAMStatus.burstSize == PSRAM_Device.defaultBurstWrapSize) {
    PSRAMStatus.burstSize = PSRAM_Device.toggleBurstWrapSize;
  } else {
    PSRAMStatus.burstSize = PSRAM_Device.defaultBurstWrapSize;
  }

  return PSRAM_SUCCESS;
}

/* Writes wrap length to Mode Register */
sl_psram_return_type_t psram_set_wrap_size(sl_psram_burst_size_type_t PSRAMBurstSize)
{

  uint8_t command[8];
  uint8_t Response;

  if (PSRAMStatus.state != initialised) {
    return PSRAM_NOT_INITIALIZED;
  }

  // switch QSPI to command mode
  RSI_QSPI_SwitchQspi2((qspi_reg_t *)M4_QSPI_2_BASE_ADDRESS,
                       PSRAM_Device.spi_config.spi_config_1.inst_mode,
                       PSRAM_Device.spi_config.spi_config_2.cs_no);

  /*Read Mode Resgister*/
  command[0] = PSRAM_MODE_REG_READ;
  command[1] = 0x00;
  command[2] = 0x00;
  command[3] = 0x00;
  command[4] = 0x00;
  command[5] = 0x00;
  command[6] = 0x00;

  qspi_transmit((qspi_reg_t *)M4_QSPI_2_BASE_ADDRESS,
                sizeof(uint8_t),
                &command[0],
                7,
                PSRAM_Device.spi_config.spi_config_2.cs_no,
                1); // Do not de-assert the CS

  qspi_receive((qspi_reg_t *)M4_QSPI_2_BASE_ADDRESS,
               sizeof(uint8_t),
               ((uint8_t *)&Response),
               1,
               PSRAM_Device.spi_config.spi_config_2.cs_no,
               0); // de-assert the CS

  Response &= 0x9F;
  Response |= (PSRAMBurstSize << 5);

  /*Write Mode Resgister*/
  command[0] = PSRAM_MODE_REG_WRITE;
  command[1] = 0x00;
  command[2] = 0x00;
  command[3] = 0x00;
  command[4] = Response;

  qspi_transmit((qspi_reg_t *)M4_QSPI_2_BASE_ADDRESS,
                sizeof(uint8_t),
                &command[0],
                5,
                PSRAM_Device.spi_config.spi_config_2.cs_no,
                0);

  return PSRAM_SUCCESS;
}
#endif // PSRAM_BURST_LEN_TOGGLE_SUPPORTED

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/

/***************************************************************************/ /**
 * Initialize the PSRAM Device           
 ******************************************************************************/
sl_psram_return_type_t sl_si91x_psram_init()
{
  sl_psram_return_type_t PSRAM_Status = PSRAM_UNKNOWN;
  rsi_error_t clkStatus               = RSI_FAIL;
  uint8_t pinIndex                    = 0;
  uint16_t clkDivFactor               = 0;
#if (SL_SI91X_D_CACHE_ENABLE == 1)
  uint32_t dCacheInitStatus = 0;
#endif

  /*Clock Initialization*/
  uint32_t system_clock_value = system_clocks.ulpss_ref_clk;

  /* read ID command supports max frequency of 33MHz */
  if (system_clock_value > 33000000) {
    clkDivFactor = (system_clock_value / 33000000);
  }

  /* Set qspi clk with value <= 33MHz for read ID call*/
  clkStatus = RSI_CLK_Qspi2ClkConfig(M4CLK, QSPI_ULPREFCLK, 0, 0, clkDivFactor);

  if (RSI_OK != clkStatus) {
    return PSRAM_CLOCK_INIT_FAILURE;
  }

  /*PinMux Initialization*/
  for (pinIndex = 0; pinIndex < NUM_OF_PSRAM_PINS; pinIndex++) {
    RSI_EGPIO_PadSelectionEnable(PSRAMPinConfig[pinIndex].pad);
    RSI_EGPIO_PadReceiverEnable((PSRAMPinConfig[pinIndex].port * 16) + PSRAMPinConfig[pinIndex].pin);

    /*Set the GPIO pin MUX */
    RSI_EGPIO_SetPinMux(EGPIO,
                        PSRAMPinConfig[pinIndex].port,
                        PSRAMPinConfig[pinIndex].pin,
                        PSRAMPinConfig[pinIndex].mux);
  }

  /*QSPI Initialization*/

  /*Initialize the QSPI controller to PSRAM default mode configuration*/
  RSI_QSPI_SpiInit((qspi_reg_t *)M4_QSPI_2_BASE_ADDRESS, (spi_config_t *)&spi_psram_default_config, 0, 0, 0);

  sl_psram_id_type_t psram_id = psram_read_id();

  if ((psram_id.MFID == PSRAM_Device.deviceID.MFID) && (psram_id.KGD == PSRAM_Device.deviceID.KGD)) {

    if (PSRAM_INTERFACE_MODE == QUAD_MODE) {
      /*Set the PSRAM device to QPI mode*/
      psram_enter_qpi_mode();

      PSRAMStatus.interfaceMode = QUAD_MODE;
    }

    /*Initialize the QSPI controller as per PSRAM device configuration*/
    RSI_QSPI_SpiInit((qspi_reg_t *)M4_QSPI_2_BASE_ADDRESS, (spi_config_t *)&PSRAM_Device.spi_config, 0, 0, 0);

#if (PSRAM_MODEL_WRAP == 1)
    qspi_sram_wrap_init((qspi_reg_t *)M4_QSPI_2_BASE_ADDRESS, (spi_config_t *)&PSRAM_Device.spi_config);
#endif

    qspi_sram_auto_init((qspi_reg_t *)M4_QSPI_2_BASE_ADDRESS, (spi_config_t *)&PSRAM_Device.spi_config);

    qspi_set_swap_mode((qspi_reg_t *)M4_QSPI_2_BASE_ADDRESS, (spi_config_t *)&PSRAM_Device.spi_config);

    PSRAMStatus.state = initialised;

    //TODO: Remove the following two vars

    PSRAMStatus.secureSegmentNumber = 0;

    PSRAMStatus.secureModeEnable = false;

#if (PSRAM_MODEL_WRAP == 1 && PSRAM_BURST_LEN_TOGGLE_SUPPORTED)
    psram_set_wrap_size(_WRAP32);

    PSRAMStatus.burstSize = _WRAP32;
#endif

    /*Clear the xfer Context*/
    ctx.xferStatus            = IDLE;
    ctx.xferHsize             = 0;
    ctx.xferNextAddress       = 0;
    ctx.xferNextSourceAddress = 0;
    ctx.xferRemLength         = 0;

    /*Return success*/
    PSRAM_Status = PSRAM_SUCCESS;

  } else {

    /*UnInitialize QSPI Controller*/

    /*UnInitialize PinMux*/
    for (pinIndex = 0; pinIndex < NUM_OF_PSRAM_PINS; pinIndex++) {

      /*Set the GPIO pin MUX */
      RSI_EGPIO_SetPinMux(EGPIO, PSRAMPinConfig[pinIndex].port, PSRAMPinConfig[pinIndex].pin, EGPIO_PIN_MUX_MODE0);

      /* Set output direction */
      RSI_EGPIO_SetDir(EGPIO, PSRAMPinConfig[pinIndex].port, PSRAMPinConfig[pinIndex].pin, EGPIO_CONFIG_DIR_INPUT);
    }

    /*UnInitialize clock*/
    RSI_CLK_PeripheralClkDisable(M4CLK, QSPI_2_CLK);

    PSRAMStatus.state = uninitialised;

    PSRAMStatus.burstSize = 0;

    PSRAMStatus.secureSegmentNumber = 0;

    PSRAMStatus.secureModeEnable = false;

    /*Return success*/
    PSRAM_Status = PSRAM_FAILURE;

    return PSRAM_Status;
  }

  /*D-cache Initialization*/
#if (SL_SI91X_D_CACHE_ENABLE == 1)
  DCACHE_REG_CTRL |= 0x3;
  dCacheInitStatus = DCACHE_REG_MAINT_STATUS;
  while (dCacheInitStatus != 0x101) {
    dCacheInitStatus = DCACHE_REG_MAINT_STATUS;
  }
  /*Disable HPORT allocation signal*/
  DCACHE_CTRL_AND_STATUS &= ~(HPORT_ALLOCATE_SIGNAL);
#endif

  return PSRAM_Status;
}

/***************************************************************************/ /**
 * Uninitialize the PSRAM Device
 ******************************************************************************/
sl_psram_return_type_t sl_si91x_psram_uninit(void)
{
  uint8_t pinIndex = 0;

  /*Exits PSRAM device from QPI mode*/
  psram_exit_qpi_mode();

  PSRAMStatus.interfaceMode = SINGLE_MODE;

  /*Initialize the QSPI controller to PSRAM default mode configuration*/
  RSI_QSPI_SpiInit((qspi_reg_t *)M4_QSPI_2_BASE_ADDRESS, (spi_config_t *)&spi_psram_default_config, 0, 0, 0);

  /*disable cache*/
#if (SL_SI91X_D_CACHE_ENABLE == 1)
  DCACHE_REG_CTRL &= 0xFFFFFFFE;
  while ((DCACHE_REG_MAINT_STATUS & 0x3) != 0x0)
    ;
#endif

  /*UnInitialize PinMux*/
  for (pinIndex = 0; pinIndex < NUM_OF_PSRAM_PINS; pinIndex++) {

    /*Set the GPIO pin MUX */
    RSI_EGPIO_SetPinMux(EGPIO, PSRAMPinConfig[pinIndex].port, PSRAMPinConfig[pinIndex].pin, EGPIO_PIN_MUX_MODE0);

    /* Set output direction */
    RSI_EGPIO_SetDir(EGPIO, PSRAMPinConfig[pinIndex].port, PSRAMPinConfig[pinIndex].pin, EGPIO_CONFIG_DIR_INPUT);
  }

  RSI_CLK_PeripheralClkDisable(M4CLK, QSPI_2_CLK);

  PSRAMStatus.state = uninitialised;

  PSRAMStatus.burstSize = 0;

  PSRAMStatus.secureSegmentNumber = 0;

  PSRAMStatus.secureModeEnable = false;

  /*Return success*/
  return PSRAM_SUCCESS;
}

/***************************************************************************/ /**     
 * Write data to PSRAM in manual mode
 ******************************************************************************/
sl_psram_return_type_t sl_si91x_psram_manual_write_in_blocking_mode(uint32_t addr,
                                                                    void *SourceBuf,
                                                                    uint8_t hSize,
                                                                    uint32_t num_of_elements)
{
  uint32_t prev_bus_mode;
  uint16_t writeLength;
  uint8_t psramXferBuf[4];
  uint32_t xferAddr;
  uint32_t lengthInBytes = 0;

  if (PSRAMStatus.state != initialised) {
    return PSRAM_NOT_INITIALIZED;
  }

  if (((hSize != sizeof(uint8_t)) && (hSize != sizeof(uint16_t)) && (hSize != sizeof(uint32_t)))) {
    return PSRAM_INVALID_HSIZE;
  }

  if (NULL == SourceBuf) {
    return PSRAM_NULL_ADDRESS;
  }

  if ((!(addr >= PSRAM_BASE_ADDRESS && addr < (PSRAM_BASE_ADDRESS + (PSRAM_Device.devDensity / 8))))
      || ((addr + (num_of_elements * hSize)) > (PSRAM_BASE_ADDRESS + (PSRAM_Device.devDensity / 8)))) {
    return PSRAM_INVALID_ADDRESS_LENGTH;
  }
#if PSRAM_ROW_BOUNDARY_CROSSING_SUPPORTED
  uint32_t rbxOffset;
  rbxOffset = addr % PSRAM_Device.defaultBurstWrapSize;
#endif

  autoModeState = psram_disable_auto_readwrite();

#if (PSRAM_MODEL_WRAP == 1 && PSRAM_BURST_LEN_TOGGLE_SUPPORTED)
  psram_set_wrap_size(PSRAM_Device.defaultBurstWrapSize);
  PSRAMStatus.burstSize = PSRAM_Device.defaultBurstWrapSize;
#endif

  while (num_of_elements) {
    lengthInBytes = (num_of_elements * hSize);

    if (lengthInBytes >= PSRAM_READ_WRITE_PAGE_SIZE) {
      writeLength = PSRAM_READ_WRITE_PAGE_SIZE;
    } else {
      writeLength = lengthInBytes;
    }

#if PSRAM_ROW_BOUNDARY_CROSSING_SUPPORTED
    if (0 != rbxOffset) {
      rbxOffset += writeLength;
      if (rbxOffset > PSRAM_Device.defaultBurstWrapSize) {
        writeLength = writeLength - (rbxOffset - PSRAM_Device.defaultBurstWrapSize);
        rbxOffset   = 0;
      }
    }
#endif

    if (PSRAM_Device.spi_config.spi_config_1.inst_mode != PSRAM_Device.spi_config.spi_config_1.addr_mode) {

      if (PSRAM_Device.spi_config.spi_config_2.swap_en == SWAP) {
        xferAddr = ((addr >> 16) & 0xff);
        xferAddr |= (addr & 0xff00);
        xferAddr |= ((addr & 0xff) << 16);
        addr = xferAddr;
      }

      // switch QSPI to command mode
      RSI_QSPI_SwitchQspi2((qspi_reg_t *)M4_QSPI_2_BASE_ADDRESS,
                           PSRAM_Device.spi_config.spi_config_1.inst_mode,
                           PSRAM_Device.spi_config.spi_config_2.cs_no);

      prev_bus_mode = PSRAM_Device.spi_config.spi_config_1.inst_mode;

      /*Send the write command to PSRAM*/
      if (PSRAM_Device.spi_config.spi_config_3._16bit_cmd_valid) {
        RSI_QSPI_WriteToFlash(
          (qspi_reg_t *)M4_QSPI_2_BASE_ADDRESS,
          QSPI_16BIT_LEN,
          ((PSRAM_Device.spi_config.spi_config_3.wr_cmd << 8) | PSRAM_Device.spi_config.spi_config_4._16bit_wr_cmd_msb),
          PSRAM_Device.spi_config.spi_config_2.cs_no);
      } else {
        RSI_QSPI_WriteToFlash(
          (qspi_reg_t *)M4_QSPI_2_BASE_ADDRESS,
          QSPI_8BIT_LEN,
          (PSRAM_Device.spi_config.spi_config_3.wr_cmd
           | ((PSRAM_Device.spi_config.spi_config_2.addr_width == _9BIT_ADDR) ? ((addr & BIT(8)) << 3) : 0)),
          PSRAM_Device.spi_config.spi_config_2.cs_no);
      }

      // if addr mode is not same as prev bus mode, switch qspi to addr mode
      if (PSRAM_Device.spi_config.spi_config_1.addr_mode != prev_bus_mode) {
        RSI_QSPI_SwitchQspi2((qspi_reg_t *)M4_QSPI_2_BASE_ADDRESS,
                             PSRAM_Device.spi_config.spi_config_1.addr_mode,
                             PSRAM_Device.spi_config.spi_config_2.cs_no);
        prev_bus_mode = PSRAM_Device.spi_config.spi_config_1.addr_mode;
      }

      /* Send write Address to PSRAM  */
      RSI_QSPI_WriteToFlash(
        (qspi_reg_t *)M4_QSPI_2_BASE_ADDRESS,
        (PSRAM_Device.spi_config.spi_config_2.addr_width ? (PSRAM_Device.spi_config.spi_config_2.addr_width * 8) : 8),
        addr,
        PSRAM_Device.spi_config.spi_config_2.cs_no);

      if (PSRAM_Device.spi_config.spi_config_1.data_mode != prev_bus_mode) {
        RSI_QSPI_SwitchQspi2((qspi_reg_t *)M4_QSPI_2_BASE_ADDRESS,
                             PSRAM_Device.spi_config.spi_config_1.data_mode,
                             PSRAM_Device.spi_config.spi_config_2.cs_no);
      }
    } else {
      addr            = addr & 0xffffff;
      psramXferBuf[0] = PSRAM_Device.spi_config.spi_config_3.wr_cmd;
      psramXferBuf[1] = (addr >> 16) & 0xff;
      psramXferBuf[2] = (addr >> 8) & 0xff;
      psramXferBuf[3] = addr & 0xff;

      qspi_transmit((qspi_reg_t *)M4_QSPI_2_BASE_ADDRESS,
                    sizeof(uint8_t),
                    psramXferBuf,
                    4,
                    PSRAM_Device.spi_config.spi_config_2.cs_no,
                    1); // do not deassert the chip
    }

    qspi_transmit((qspi_reg_t *)M4_QSPI_2_BASE_ADDRESS,
                  hSize,
                  SourceBuf,
                  (writeLength / hSize),
                  PSRAM_Device.spi_config.spi_config_2.cs_no,
                  0); // deassert the chip select at end of data.

    num_of_elements = num_of_elements - (writeLength / hSize);

    if (num_of_elements) {
      SourceBuf = (uint8_t *)(((uint8_t *)SourceBuf) + writeLength);
      addr      = addr + writeLength;
    }
  }

#if (PSRAM_MODEL_WRAP == 1 && PSRAM_BURST_LEN_TOGGLE_SUPPORTED)
  psram_set_wrap_size(PSRAM_Device.toggleBurstWrapSize);
  PSRAMStatus.burstSize = PSRAM_Device.toggleBurstWrapSize;
#endif
  if (PSRAM_MANUAL_MODE != autoModeState) {
    psram_enable_auto_readwrite();
  }

  return PSRAM_SUCCESS;
}

/***************************************************************************/ /**
 * Read data from PSRAM in manual mode
 ******************************************************************************/
sl_psram_return_type_t sl_si91x_psram_manual_read_in_blocking_mode(uint32_t addr,
                                                                   void *DestBuf,
                                                                   uint8_t hSize,
                                                                   uint32_t num_of_elements)
{
  uint32_t prev_bus_mode;
  uint16_t readLength;
  uint8_t psramXferBuf[7];
  uint32_t xferAddr;
  uint32_t lengthInBytes = 0;

  if (PSRAMStatus.state != initialised) {
    return PSRAM_NOT_INITIALIZED;
  }

  if (((hSize != sizeof(uint8_t)) && (hSize != sizeof(uint16_t)) && (hSize != sizeof(uint32_t)))) {
    return PSRAM_INVALID_HSIZE;
  }

  if (NULL == DestBuf) {
    return PSRAM_NULL_ADDRESS;
  }

  if ((!(addr >= PSRAM_BASE_ADDRESS && addr < (PSRAM_BASE_ADDRESS + (PSRAM_Device.devDensity / 8))))
      || ((addr + (num_of_elements * hSize)) > (PSRAM_BASE_ADDRESS + (PSRAM_Device.devDensity / 8)))) {
    return PSRAM_INVALID_ADDRESS_LENGTH;
  }

#if PSRAM_ROW_BOUNDARY_CROSSING_SUPPORTED
  uint32_t rbxOffset;
  rbxOffset = addr % PSRAM_Device.defaultBurstWrapSize;
#endif

  autoModeState = psram_disable_auto_readwrite();

#if (PSRAM_MODEL_WRAP == 1 && PSRAM_BURST_LEN_TOGGLE_SUPPORTED)
  psram_set_wrap_size(PSRAM_Device.defaultBurstWrapSize);
  PSRAMStatus.burstSize = PSRAM_Device.defaultBurstWrapSize;
#endif

  while (num_of_elements) {

    lengthInBytes = (num_of_elements * hSize);

    if (lengthInBytes >= PSRAM_READ_WRITE_PAGE_SIZE) {
      readLength = PSRAM_READ_WRITE_PAGE_SIZE;
    } else {
      readLength = lengthInBytes;
    }

#if PSRAM_ROW_BOUNDARY_CROSSING_SUPPORTED
    if (0 != rbxOffset) {
      rbxOffset += readLength;
      if (rbxOffset > PSRAM_Device.defaultBurstWrapSize) {
        readLength = readLength - (rbxOffset - PSRAM_Device.defaultBurstWrapSize);
        rbxOffset  = 0;
      }
    }
#endif

    if (PSRAM_Device.spi_config.spi_config_1.inst_mode != PSRAM_Device.spi_config.spi_config_1.addr_mode) {

      if (PSRAM_Device.spi_config.spi_config_2.swap_en == SWAP) {
        xferAddr = ((addr >> 16) & 0xff);
        xferAddr |= (addr & 0xff00);
        xferAddr |= ((addr & 0xff) << 16);
        addr = xferAddr;
      }
      // switch QSPI to command mode
      RSI_QSPI_SwitchQspi2((qspi_reg_t *)M4_QSPI_2_BASE_ADDRESS,
                           PSRAM_Device.spi_config.spi_config_1.inst_mode,
                           PSRAM_Device.spi_config.spi_config_2.cs_no);

      prev_bus_mode = PSRAM_Device.spi_config.spi_config_1.inst_mode;

      /*Send the Read command to PSRAM*/
      if (PSRAM_Device.spi_config.spi_config_3._16bit_cmd_valid) {
        RSI_QSPI_WriteToFlash((qspi_reg_t *)M4_QSPI_2_BASE_ADDRESS,
                              QSPI_16BIT_LEN,
                              ((PSRAM_Device.spi_config.spi_config_1.read_cmd << 8)
                               | PSRAM_Device.spi_config.spi_config_3._16bit_rd_cmd_msb),
                              PSRAM_Device.spi_config.spi_config_2.cs_no);
      } else {
        RSI_QSPI_WriteToFlash(
          (qspi_reg_t *)M4_QSPI_2_BASE_ADDRESS,
          QSPI_8BIT_LEN,
          (PSRAM_Device.spi_config.spi_config_1.read_cmd
           | ((PSRAM_Device.spi_config.spi_config_2.addr_width == _9BIT_ADDR) ? ((addr & BIT(8)) << 3) : 0)),
          PSRAM_Device.spi_config.spi_config_2.cs_no);
      }

      // if addr mode is not same as prev bus mode, switch qspi to addr mode
      if (PSRAM_Device.spi_config.spi_config_1.addr_mode != prev_bus_mode) {
        RSI_QSPI_SwitchQspi2((qspi_reg_t *)M4_QSPI_2_BASE_ADDRESS,
                             PSRAM_Device.spi_config.spi_config_1.addr_mode,
                             PSRAM_Device.spi_config.spi_config_2.cs_no);
        prev_bus_mode = PSRAM_Device.spi_config.spi_config_1.addr_mode;
      }

      /* Send Read Address to PSRAM  */
      RSI_QSPI_WriteToFlash(
        (qspi_reg_t *)M4_QSPI_2_BASE_ADDRESS,
        (PSRAM_Device.spi_config.spi_config_2.addr_width ? (PSRAM_Device.spi_config.spi_config_2.addr_width * 8) : 8),
        addr,
        PSRAM_Device.spi_config.spi_config_2.cs_no);

      if ((PSRAM_Device.spi_config.spi_config_1.dummy_W_or_R == DUMMY_READS) && (PSRAM_RD_DUMMY_BITS > 0)) {

        RSI_QSPI_WriteToFlash((qspi_reg_t *)M4_QSPI_2_BASE_ADDRESS,
                              (PSRAM_RD_DUMMY_BITS),
                              0x00,
                              PSRAM_Device.spi_config.spi_config_2.cs_no);
      }

      if (PSRAM_Device.spi_config.spi_config_1.data_mode != prev_bus_mode) {
        RSI_QSPI_SwitchQspi2((qspi_reg_t *)M4_QSPI_2_BASE_ADDRESS,
                             PSRAM_Device.spi_config.spi_config_1.data_mode,
                             PSRAM_Device.spi_config.spi_config_2.cs_no);
      }
    } else {

      uint8_t transmit_length = 4;
      // includes only cmd and addr by default, dummy byte count added later based on requirement

      addr            = addr & 0xffffff;
      psramXferBuf[0] = PSRAM_Device.spi_config.spi_config_1.read_cmd;
      psramXferBuf[1] = (addr >> 16) & 0xff;
      psramXferBuf[2] = (addr >> 8) & 0xff;
      psramXferBuf[3] = addr & 0xff;
      psramXferBuf[4] = 0x00; // Following bytes are for dummy reads, if required
      psramXferBuf[5] = 0x00;
      psramXferBuf[6] = 0x00;

      transmit_length += (PSRAM_RD_DUMMY_BITS / 8); // Dummy bits in this case are always multiple of 8

      qspi_transmit((qspi_reg_t *)M4_QSPI_2_BASE_ADDRESS,
                    sizeof(uint8_t),
                    psramXferBuf,
                    transmit_length,
                    PSRAM_Device.spi_config.spi_config_2.cs_no,
                    1); // do not deassert the chip
    }

    qspi_receive((qspi_reg_t *)M4_QSPI_2_BASE_ADDRESS,
                 hSize,
                 DestBuf,
                 (readLength / hSize),
                 PSRAM_Device.spi_config.spi_config_2.cs_no,
                 0);

    num_of_elements = num_of_elements - (readLength / hSize);

    if (num_of_elements) {
      DestBuf = (uint8_t *)(((uint8_t *)DestBuf) + readLength);
      addr    = addr + readLength;
    }
  }

#if (PSRAM_MODEL_WRAP == 1 && PSRAM_BURST_LEN_TOGGLE_SUPPORTED)
  psram_set_wrap_size(PSRAM_Device.toggleBurstWrapSize);
  PSRAMStatus.burstSize = PSRAM_Device.toggleBurstWrapSize;
#endif

  if (PSRAM_MANUAL_MODE != autoModeState) {
    psram_enable_auto_readwrite();
  }

  return PSRAM_SUCCESS;
}

/***************************************************************************/ /**
 * Write data to PSRAM in manual mode using DMA
 ******************************************************************************/
sl_psram_return_type_t sl_si91x_psram_manual_write_in_dma_mode(uint32_t addr,
                                                               void *SourceBuf,
                                                               uint8_t hSize,
                                                               uint32_t length,
                                                               sl_psram_dma_status_type_t *dmastatus)
{
  uint32_t qspi_manual_config_reg;
  uint32_t prev_bus_mode;
  int32_t status       = RSI_OK;
  qspi_reg_t *qspi_reg = (qspi_reg_t *)M4_QSPI_2_BASE_ADDRESS;
  uint16_t xferLength  = 0;
  uint32_t splitLength = PSRAM_READ_WRITE_PAGE_SIZE;
  uint32_t lengthInBytes;
  uint8_t psramXferBuf[4];
  uint32_t xferAddr;

  if (PSRAMStatus.state != initialised) {
    return PSRAM_NOT_INITIALIZED;
  }

  if (((hSize != sizeof(uint8_t)) && (hSize != sizeof(uint16_t)) && (hSize != sizeof(uint32_t)))) {
    return PSRAM_INVALID_HSIZE;
  }

  if (NULL == SourceBuf) {
    return PSRAM_NULL_ADDRESS;
  }

  if ((!(addr >= PSRAM_BASE_ADDRESS && addr < (PSRAM_BASE_ADDRESS + (PSRAM_Device.devDensity / 8))))
      || ((addr + (length * hSize)) > (PSRAM_BASE_ADDRESS + (PSRAM_Device.devDensity / 8)))) {
    return PSRAM_INVALID_ADDRESS_LENGTH;
  }

  lengthInBytes = (length * hSize);

  if (lengthInBytes < splitLength) {
    splitLength = lengthInBytes;
  }

  if ((splitLength / hSize) >= 1024) {
    xferLength = 1024;
  } else {
    xferLength = splitLength / hSize;
  }

#if PSRAM_ROW_BOUNDARY_CROSSING_SUPPORTED
  static uint32_t rbxOffset;

  if (PSRAM_Device.defaultBurstWrapSize && TX_RUNNING != ctx.xferStatus) {
    rbxOffset = addr % PSRAM_Device.defaultBurstWrapSize;
  }

  if (0 != rbxOffset) {
    rbxOffset += splitLength;
    if (rbxOffset > PSRAM_Device.defaultBurstWrapSize) {
      splitLength = splitLength - (rbxOffset - PSRAM_Device.defaultBurstWrapSize);
      xferLength  = splitLength / hSize;
      rbxOffset   = 0;
    }
  }
#endif

  ctx.xferHsize             = hSize;
  ctx.xferNextAddress       = addr + splitLength;
  ctx.xferNextSourceAddress = ((uint8_t *)SourceBuf + splitLength);
  ctx.xferRemLength         = length - xferLength;
  ctx.xferStatus            = TX_RUNNING;
  ctx.done                  = dmastatus;

  memset(&control, 0, sizeof(RSI_UDMA_CHA_CONFIG_DATA_T));
  memset(&config, 0, sizeof(RSI_UDMA_CHA_CFG_T));

  config.altStruct       = 0;
  config.burstReq        = 1;
  config.channelPrioHigh = 0;
  config.dmaCh           = QSPI_DMA_DEST_CHANNEL_NUM;
  config.periAck         = 0;
  config.periphReq       = 0;
  config.reqMask         = 0;

  control.transferType       = UDMA_MODE_BASIC;
  control.nextBurst          = 0;
  control.totalNumOfDMATrans = (xferLength - 1);
  control.rPower             = ARBSIZE_1024;
  control.srcProtCtrl        = 0x000;
  control.dstProtCtrl        = 0x000;

  switch (hSize) {
    case sizeof(uint32_t):
      control.srcSize = SRC_SIZE_32;
      control.srcInc  = SRC_INC_32;
      control.dstSize = DST_SIZE_32;
      break;
    case sizeof(uint16_t):
      control.srcSize = SRC_SIZE_16;
      control.srcInc  = SRC_INC_16;
      control.dstSize = DST_SIZE_16;
      break;
    default:
      control.srcSize = SRC_SIZE_8;
      control.srcInc  = SRC_INC_8;
      control.dstSize = DST_SIZE_8;
  }

  control.dstInc = DST_INC_NONE;

  /* Initialize dma */
  udmaHandle0 = UDMAx_Initialize(&UDMA0_Resources, UDMA0_Table, udmaHandle0, dma_rom_buff0);
  if (udmaHandle0 == NULL) {
    return PSRAM_FAILURE;
  }

  /* Configure dma channel */
  status = UDMAx_ChannelConfigure(&UDMA0_Resources,
                                  config.dmaCh,
                                  (uint32_t)SourceBuf,
                                  (uint32_t)(M4_QSPI_2_BASE_ADDRESS + 0x40),
                                  splitLength / hSize,
                                  control,
                                  &config,
                                  (UDMA_SignalEvent_t)udma_transfer_complete,
                                  udma0_chnl_info,
                                  udmaHandle0);
  if (status != RSI_OK) {
    return PSRAM_FAILURE;
  }

  /* Enable dma channel */
  status = UDMAx_ChannelEnable(QSPI_DMA_DEST_CHANNEL_NUM, &UDMA0_Resources, udmaHandle0);
  if (status != RSI_OK) {
    RSI_UDMA_DeInit(udmaHandle0, &config);
    return PSRAM_FAILURE;
  }

  autoModeState = psram_disable_auto_readwrite();

#if (PSRAM_MODEL_WRAP == 1 && PSRAM_BURST_LEN_TOGGLE_SUPPORTED)
  psram_set_wrap_size(PSRAM_Device.defaultBurstWrapSize);
  PSRAMStatus.burstSize = PSRAM_Device.defaultBurstWrapSize;
#endif

  if (PSRAM_Device.spi_config.spi_config_1.inst_mode != PSRAM_Device.spi_config.spi_config_1.addr_mode) {

    if (PSRAM_Device.spi_config.spi_config_2.swap_en == SWAP) {
      xferAddr = ((addr >> 16) & 0xff);
      xferAddr |= (addr & 0xff00);
      xferAddr |= ((addr & 0xff) << 16);
      addr = xferAddr;
    }

    // switch QSPI to command mode
    RSI_QSPI_SwitchQspi2((qspi_reg_t *)M4_QSPI_2_BASE_ADDRESS,
                         PSRAM_Device.spi_config.spi_config_1.inst_mode,
                         PSRAM_Device.spi_config.spi_config_2.cs_no);

    prev_bus_mode = PSRAM_Device.spi_config.spi_config_1.inst_mode;

    /*Send the write command to PSRAM*/
    if (PSRAM_Device.spi_config.spi_config_3._16bit_cmd_valid) {
      RSI_QSPI_WriteToFlash(
        (qspi_reg_t *)M4_QSPI_2_BASE_ADDRESS,
        QSPI_16BIT_LEN,
        ((PSRAM_Device.spi_config.spi_config_3.wr_cmd << 8) | PSRAM_Device.spi_config.spi_config_3._16bit_rd_cmd_msb),
        PSRAM_Device.spi_config.spi_config_2.cs_no);
    } else {
      RSI_QSPI_WriteToFlash(
        (qspi_reg_t *)M4_QSPI_2_BASE_ADDRESS,
        QSPI_8BIT_LEN,
        (PSRAM_Device.spi_config.spi_config_3.wr_cmd
         | ((PSRAM_Device.spi_config.spi_config_2.addr_width == _9BIT_ADDR) ? ((addr & BIT(8)) << 3) : 0)),
        PSRAM_Device.spi_config.spi_config_2.cs_no);
    }

    // if addr mode is not same as prev bus mode, switch qspi to addr mode
    if (PSRAM_Device.spi_config.spi_config_1.addr_mode != prev_bus_mode) {
      RSI_QSPI_SwitchQspi2((qspi_reg_t *)M4_QSPI_2_BASE_ADDRESS,
                           PSRAM_Device.spi_config.spi_config_1.addr_mode,
                           PSRAM_Device.spi_config.spi_config_2.cs_no);
      prev_bus_mode = PSRAM_Device.spi_config.spi_config_1.addr_mode;
    }

    /* Send write Address to PSRAM  */
    RSI_QSPI_WriteToFlash(
      (qspi_reg_t *)M4_QSPI_2_BASE_ADDRESS,
      (PSRAM_Device.spi_config.spi_config_2.addr_width ? (PSRAM_Device.spi_config.spi_config_2.addr_width * 8) : 8),
      addr,
      PSRAM_Device.spi_config.spi_config_2.cs_no);

    // if extra byte is enabled send that to PSRAM
    if (PSRAM_Device.spi_config.spi_config_1.extra_byte_en) {

      if (PSRAM_Device.spi_config.spi_config_1.extra_byte_mode != prev_bus_mode) {
        RSI_QSPI_SwitchQspi2((qspi_reg_t *)M4_QSPI_2_BASE_ADDRESS,
                             PSRAM_Device.spi_config.spi_config_1.extra_byte_mode,
                             PSRAM_Device.spi_config.spi_config_2.cs_no);
        prev_bus_mode = PSRAM_Device.spi_config.spi_config_1.extra_byte_mode;
      }

      RSI_QSPI_WriteToFlash((qspi_reg_t *)M4_QSPI_2_BASE_ADDRESS,
                            QSPI_8BIT_LEN,
                            0x00,
                            PSRAM_Device.spi_config.spi_config_2.cs_no);
    }

    if (PSRAM_Device.spi_config.spi_config_1.data_mode != prev_bus_mode) {
      RSI_QSPI_SwitchQspi2((qspi_reg_t *)M4_QSPI_2_BASE_ADDRESS,
                           PSRAM_Device.spi_config.spi_config_1.data_mode,
                           PSRAM_Device.spi_config.spi_config_2.cs_no);
    }

  } else {
    addr            = addr & 0xffffff;
    psramXferBuf[0] = PSRAM_Device.spi_config.spi_config_3.wr_cmd;
    psramXferBuf[1] = (addr >> 16) & 0xff;
    psramXferBuf[2] = (addr >> 8) & 0xff;
    psramXferBuf[3] = addr & 0xff;

    qspi_transmit((qspi_reg_t *)M4_QSPI_2_BASE_ADDRESS,
                  sizeof(uint8_t),
                  psramXferBuf,
                  4,
                  PSRAM_Device.spi_config.spi_config_2.cs_no,
                  1); // do not deassert the chip
  }

  // configure qspi for read

  qspi_manual_config_reg = (qspi_reg->QSPI_MANUAL_CONFIG_REG & ~0xF8387FFF);
  qspi_manual_config_reg |= (PSRAM_Device.spi_config.spi_config_2.cs_no << 13) | ((hSize - 1) << 19) | TAKE_LEN_FRM_REG;

  // trigger write
  qspi_reg->QSPI_MANUAL_CONFIG_REG = qspi_manual_config_reg | WRITE_TRIGGER | ((splitLength & 0x3FF) << 3)
                                     | (((splitLength >> 10) & 0x1F) << 27);

  /* Enable dma controller */
  status = UDMAx_DMAEnable(&UDMA0_Resources, udmaHandle0);
  if (status != RSI_OK) {
    return PSRAM_FAILURE;
  }

  status = RSI_UDMA_ChannelSoftwareTrigger(udmaHandle0, QSPI_DMA_DEST_CHANNEL_NUM);
  if (status != RSI_OK) {
    return PSRAM_FAILURE;
  }
  return PSRAM_SUCCESS;
}

/***************************************************************************/ /**
 * Read data from PSRAM in manual mode using DMA
 ******************************************************************************/
sl_psram_return_type_t sl_si91x_psram_manual_read_in_dma_mode(uint32_t addr,
                                                              void *DestBuf,
                                                              uint8_t hSize,
                                                              uint32_t length,
                                                              sl_psram_dma_status_type_t *dmastatus)
{
  uint32_t qspi_manual_config_reg;
  uint32_t prev_bus_mode;
  int32_t status       = RSI_OK;
  qspi_reg_t *qspi_reg = (qspi_reg_t *)M4_QSPI_2_BASE_ADDRESS;
  uint16_t xferLength  = 0;
  uint32_t splitLength = PSRAM_READ_WRITE_PAGE_SIZE;
  uint32_t lengthInBytes;
  uint8_t psramXferBuf[7];
  uint32_t xferAddr;

  if (PSRAMStatus.state != initialised) {
    return PSRAM_NOT_INITIALIZED;
  }

  if (((hSize != sizeof(uint8_t)) && (hSize != sizeof(uint16_t)) && (hSize != sizeof(uint32_t)))) {
    return PSRAM_INVALID_HSIZE;
  }

  if (NULL == DestBuf) {
    return PSRAM_NULL_ADDRESS;
  }

  if ((!(addr >= PSRAM_BASE_ADDRESS && addr < (PSRAM_BASE_ADDRESS + (PSRAM_Device.devDensity / 8))))
      || ((addr + (length * hSize)) > (PSRAM_BASE_ADDRESS + (PSRAM_Device.devDensity / 8)))) {
    return PSRAM_INVALID_ADDRESS_LENGTH;
  }

  lengthInBytes = (length * hSize);

  if (lengthInBytes < splitLength) {
    splitLength = lengthInBytes;
  }

  if ((splitLength / hSize) >= 1024) {
    xferLength = 1024;
  } else {
    xferLength = splitLength / hSize;
  }

#if PSRAM_ROW_BOUNDARY_CROSSING_SUPPORTED
  static uint32_t rbxOffset;

  if (PSRAM_Device.defaultBurstWrapSize && RX_RUNNING != ctx.xferStatus) {
    rbxOffset = addr % PSRAM_Device.defaultBurstWrapSize;
  }

  if (0 != rbxOffset) {
    rbxOffset += splitLength;
    if (rbxOffset > PSRAM_Device.defaultBurstWrapSize) {
      splitLength = splitLength - (rbxOffset - PSRAM_Device.defaultBurstWrapSize);
      xferLength  = splitLength / hSize;
      rbxOffset   = 0;
    }
  }
#endif

  ctx.xferHsize             = hSize;
  ctx.xferNextAddress       = addr + splitLength;
  ctx.xferNextSourceAddress = ((uint8_t *)DestBuf + splitLength);
  ctx.xferRemLength         = length - xferLength;
  ctx.xferStatus            = RX_RUNNING;
  ctx.done                  = dmastatus;

  memset(&control, 0, sizeof(RSI_UDMA_CHA_CONFIG_DATA_T));
  memset(&config, 0, sizeof(RSI_UDMA_CHA_CFG_T));

  config.altStruct       = 0;
  config.burstReq        = 1;
  config.channelPrioHigh = 0;
  config.dmaCh           = QSPI_DMA_SRC_CHANNEL_NUM;
  config.periAck         = 0;
  config.periphReq       = 0;
  config.reqMask         = 0;

  control.transferType       = UDMA_MODE_BASIC;
  control.nextBurst          = 0;
  control.totalNumOfDMATrans = (xferLength - 1);
  control.rPower             = ARBSIZE_1024;
  control.srcProtCtrl        = 0x000;
  control.dstProtCtrl        = 0x000;

  switch (hSize) {
    case sizeof(uint32_t):
      control.srcSize = SRC_SIZE_32;
      control.dstInc  = SRC_INC_32;
      control.dstSize = DST_SIZE_32;
      break;
    case sizeof(uint16_t):
      control.srcSize = SRC_SIZE_16;
      control.dstInc  = SRC_INC_16;
      control.dstSize = DST_SIZE_16;
      break;
    default:
      control.srcSize = SRC_SIZE_8;
      control.dstInc  = SRC_INC_8;
      control.dstSize = DST_SIZE_8;
  }

  control.srcInc = SRC_INC_NONE;

  /* Initialize dma */
  udmaHandle0 = UDMAx_Initialize(&UDMA0_Resources, UDMA0_Table, udmaHandle0, dma_rom_buff0);
  if (udmaHandle0 == NULL) {
    return PSRAM_FAILURE;
  }

  /* Configure dma channel */
  status = UDMAx_ChannelConfigure(&UDMA0_Resources,
                                  config.dmaCh,
                                  (uint32_t)(M4_QSPI_2_BASE_ADDRESS + 0x40),
                                  (uint32_t)DestBuf,
                                  splitLength / hSize,
                                  control,
                                  &config,
                                  (UDMA_SignalEvent_t)udma_transfer_complete,
                                  udma0_chnl_info,
                                  udmaHandle0);
  if (status != RSI_OK) {
    return PSRAM_FAILURE;
  }

  /* Enable dma channel */
  status = UDMAx_ChannelEnable(QSPI_DMA_SRC_CHANNEL_NUM, &UDMA0_Resources, udmaHandle0);
  if (status != RSI_OK) {
    RSI_UDMA_DeInit(udmaHandle0, &config);
    return PSRAM_FAILURE;
  }

  autoModeState = psram_disable_auto_readwrite();

#if (PSRAM_MODEL_WRAP == 1 && PSRAM_BURST_LEN_TOGGLE_SUPPORTED)
  psram_set_wrap_size(PSRAM_Device.defaultBurstWrapSize);
  PSRAMStatus.burstSize = PSRAM_Device.defaultBurstWrapSize;
#endif

  if (PSRAM_Device.spi_config.spi_config_1.inst_mode != PSRAM_Device.spi_config.spi_config_1.addr_mode) {

    if (PSRAM_Device.spi_config.spi_config_2.swap_en == SWAP) {
      xferAddr = ((addr >> 16) & 0xff);
      xferAddr |= (addr & 0xff00);
      xferAddr |= ((addr & 0xff) << 16);
      addr = xferAddr;
    }

    // switch QSPI to command mode
    RSI_QSPI_SwitchQspi2((qspi_reg_t *)M4_QSPI_2_BASE_ADDRESS,
                         PSRAM_Device.spi_config.spi_config_1.inst_mode,
                         PSRAM_Device.spi_config.spi_config_2.cs_no);

    prev_bus_mode = PSRAM_Device.spi_config.spi_config_1.inst_mode;

    /*Send the Read command to PSRAM*/
    if (PSRAM_Device.spi_config.spi_config_3._16bit_cmd_valid) {
      RSI_QSPI_WriteToFlash(
        (qspi_reg_t *)M4_QSPI_2_BASE_ADDRESS,
        QSPI_16BIT_LEN,
        ((PSRAM_Device.spi_config.spi_config_1.read_cmd << 8) | PSRAM_Device.spi_config.spi_config_3._16bit_rd_cmd_msb),
        PSRAM_Device.spi_config.spi_config_2.cs_no);
    } else {
      RSI_QSPI_WriteToFlash(
        (qspi_reg_t *)M4_QSPI_2_BASE_ADDRESS,
        QSPI_8BIT_LEN,
        (PSRAM_Device.spi_config.spi_config_1.read_cmd
         | ((PSRAM_Device.spi_config.spi_config_2.addr_width == _9BIT_ADDR) ? ((addr & BIT(8)) << 3) : 0)),
        PSRAM_Device.spi_config.spi_config_2.cs_no);
    }

    // if addr mode is not same as prev bus mode, switch qspi to addr mode
    if (PSRAM_Device.spi_config.spi_config_1.addr_mode != prev_bus_mode) {
      RSI_QSPI_SwitchQspi2((qspi_reg_t *)M4_QSPI_2_BASE_ADDRESS,
                           PSRAM_Device.spi_config.spi_config_1.addr_mode,
                           PSRAM_Device.spi_config.spi_config_2.cs_no);
      prev_bus_mode = PSRAM_Device.spi_config.spi_config_1.addr_mode;
    }

    /* Send Read Address to PSRAM  */
    RSI_QSPI_WriteToFlash(
      (qspi_reg_t *)M4_QSPI_2_BASE_ADDRESS,
      (PSRAM_Device.spi_config.spi_config_2.addr_width ? (PSRAM_Device.spi_config.spi_config_2.addr_width * 8) : 8),
      addr,
      PSRAM_Device.spi_config.spi_config_2.cs_no);

    // if extra byte is enabled send that to PSRAM
    if (PSRAM_Device.spi_config.spi_config_1.extra_byte_en) {

      if (PSRAM_Device.spi_config.spi_config_1.extra_byte_mode != prev_bus_mode) {
        RSI_QSPI_SwitchQspi2((qspi_reg_t *)M4_QSPI_2_BASE_ADDRESS,
                             PSRAM_Device.spi_config.spi_config_1.extra_byte_mode,
                             PSRAM_Device.spi_config.spi_config_2.cs_no);
        prev_bus_mode = PSRAM_Device.spi_config.spi_config_1.extra_byte_mode;
      }

      RSI_QSPI_WriteToFlash((qspi_reg_t *)M4_QSPI_2_BASE_ADDRESS,
                            QSPI_8BIT_LEN,
                            0x00,
                            PSRAM_Device.spi_config.spi_config_2.cs_no);
    }

    if ((PSRAM_Device.spi_config.spi_config_1.dummy_W_or_R == DUMMY_READS) && (PSRAM_RD_DUMMY_BITS > 0)) {

      RSI_QSPI_WriteToFlash((qspi_reg_t *)M4_QSPI_2_BASE_ADDRESS,
                            (PSRAM_RD_DUMMY_BITS),
                            0x00,
                            PSRAM_Device.spi_config.spi_config_2.cs_no);
    }
    if (PSRAM_Device.spi_config.spi_config_1.data_mode != prev_bus_mode) {
      RSI_QSPI_SwitchQspi2((qspi_reg_t *)M4_QSPI_2_BASE_ADDRESS,
                           PSRAM_Device.spi_config.spi_config_1.data_mode,
                           PSRAM_Device.spi_config.spi_config_2.cs_no);
    }
  } else {

    uint8_t transmit_length = 4;
    // includes only cmd and addr by default, dummy byte count added later based on requirement

    addr            = addr & 0xffffff;
    psramXferBuf[0] = PSRAM_Device.spi_config.spi_config_1.read_cmd;
    psramXferBuf[1] = (addr >> 16) & 0xff;
    psramXferBuf[2] = (addr >> 8) & 0xff;
    psramXferBuf[3] = addr & 0xff;
    psramXferBuf[4] = 0x00; // Following bytes are for dummy reads, if required
    psramXferBuf[5] = 0x00;
    psramXferBuf[6] = 0x00;

    transmit_length += (PSRAM_RD_DUMMY_BITS / 8); // Dummy bits in this case are always multiple of 8

    qspi_transmit((qspi_reg_t *)M4_QSPI_2_BASE_ADDRESS,
                  sizeof(uint8_t),
                  psramXferBuf,
                  transmit_length,
                  PSRAM_Device.spi_config.spi_config_2.cs_no,
                  1); // do not deassert the chip
  }

  // configure qspi for read
  qspi_manual_config_reg = (qspi_reg->QSPI_MANUAL_CONFIG_REG & ~0xF8387FFF);
  qspi_manual_config_reg |= (PSRAM_Device.spi_config.spi_config_2.cs_no << 13) | ((hSize - 1) << 19) | TAKE_LEN_FRM_REG;

  // trigger write
  qspi_reg->QSPI_MANUAL_CONFIG_REG = qspi_manual_config_reg | READ_TRIGGER | ((splitLength & 0x3FF) << 3)
                                     | (((splitLength >> 10) & 0x1F) << 27);

  /* Enable dma controller */
  status = UDMAx_DMAEnable(&UDMA0_Resources, udmaHandle0);
  if (status != RSI_OK) {
    return PSRAM_FAILURE;
  }

  status = RSI_UDMA_ChannelSoftwareTrigger(udmaHandle0, QSPI_DMA_SRC_CHANNEL_NUM);
  if (status != RSI_OK) {
    return PSRAM_FAILURE;
  }
  return PSRAM_SUCCESS;
}

/***************************************************************************/ /**
 * Reset the PSRAM Device
 ******************************************************************************/
sl_psram_return_type_t sl_si91x_psram_reset(void)
{
  uint8_t command;

  /*Send the RESET Enable command first*/
  command = PSRAM_RESET_EN;
  // switch QSPI to command mode
  RSI_QSPI_SwitchQspi2((qspi_reg_t *)M4_QSPI_2_BASE_ADDRESS,
                       PSRAM_Device.spi_config.spi_config_1.inst_mode,
                       PSRAM_Device.spi_config.spi_config_2.cs_no);

  qspi_transmit((qspi_reg_t *)M4_QSPI_2_BASE_ADDRESS,
                sizeof(uint8_t),
                &command,
                1,
                PSRAM_Device.spi_config.spi_config_2.cs_no,
                0);

  /*Send the RESET command*/
  command = PSRAM_RESET;
  qspi_transmit((qspi_reg_t *)M4_QSPI_2_BASE_ADDRESS,
                sizeof(uint8_t),
                &command,
                1,
                PSRAM_Device.spi_config.spi_config_2.cs_no,
                0);

  return PSRAM_SUCCESS;
}

#if PSRAM_HALF_SLEEP_SUPPORTED

/***************************************************************************/ /**
 * Put PSRAM Device in sleep 
 ******************************************************************************/
sl_psram_return_type_t sl_si91x_psram_sleep(void)
{
  uint8_t command;

  /*Exits PSRAM device from QPI mode*/
  psram_exit_qpi_mode();

  /*Initialize the QSPI controller to PSRAM default mode configuration*/
  RSI_QSPI_SpiInit((qspi_reg_t *)M4_QSPI_2_BASE_ADDRESS, (spi_config_t *)&spi_psram_default_config, 0, 0, 0);

  command = PSRAM_HALF_SLEEP;
  qspi_transmit((qspi_reg_t *)M4_QSPI_2_BASE_ADDRESS,
                sizeof(uint8_t),
                &command,
                1,
                PSRAM_Device.spi_config.spi_config_2.cs_no,
                0);

  /*Make sure tHS is not violated*/

  __asm("nop");

  return PSRAM_SUCCESS;
}

/***************************************************************************/ /**
 * Exit PSRAM device from sleep
 ******************************************************************************/
sl_psram_return_type_t sl_si91x_psram_wakeup(void)
{

  qspi_reg_t *qspi_reg = (qspi_reg_t *)M4_QSPI_2_BASE_ADDRESS;

  qspi_reg->QSPI_MANUAL_CONFIG_REG = (qspi_reg->QSPI_MANUAL_CONFIG_REG & 0xFFFFFFFE);

  /*Make sure tXPHS is not violated*/

  __asm("nop");
  __asm("nop");
  __asm("nop");

  qspi_reg->QSPI_MANUAL_CONFIG_REG = (qspi_reg->QSPI_MANUAL_CONFIG_REG | 0x1);

  /*Make sure tXHS is not violated*/

  for (int i = 0; i < tXHS_MAX_COUNT; i++) {
    __asm("nop");
  }

  return sl_si91x_psram_init();
}

#endif // PSRAM_HALF_SLEEP_SUPPORTED

/***************************************************************************/ /**
 * Enable CTR encryption-decryption on PSRAM
 ******************************************************************************/
sl_psram_return_type_t sl_si91x_psram_enable_encry_decry(uint16_t keySize)
{

  if (PSRAMStatus.state != initialised) {
    return PSRAM_NOT_INITIALIZED;
  }

  qspi_qspiload_key_ext((qspi_reg_t *)M4_QSPI_2_BASE_ADDRESS, keySize);

  PSRAMStatus.secureModeEnable = true;

  return PSRAM_SUCCESS;
}
