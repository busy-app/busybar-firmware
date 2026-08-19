#include "front_display_i.h"

#include <furi.h>
#include <stm32u5xx_ll_dma.h>

#define OCTOSPI_PRESCALLER  8
#define START_REFRESH_COUNT 10
#define START_VSYNC_COUNT   START_REFRESH_COUNT

#define GAMMA_LUT_SIZE (UINT8_MAX + 1)

typedef enum {
    LedDriverCmdNone = 0, // Placeholder
    LedDriverCmdDataLatch = 1, // Latch 16bit data and send it to SRAM
    LedDriverCmdWriteCfg5Dbg = 2, // Write debug register (DBG_MODE, GROUP_SEL)
    LedDriverCmdVsync = 3, // Update display data
    LedDriverCmdWriteCfg1 = 4, // Write configuration register 1
    LedDriverCmdReadCfg1 = 5, // Read configuration register 1
    LedDriverCmdWriteCfg2 = 6, // Write configuration register 2
    LedDriverCmdReadCfg2 = 7, // Read configuration register 2
    LedDriverCmdWriteCfg3 = 8, // Write configuration register 3
    LedDriverCmdReadCfg3 = 9, // Read configuration register 3
    LedDriverCmdWriteCfg4 = 10, // Write configuration register 4
    LedDriverCmdReadCfg4 = 11, // Read configuration register 4
    LedDriverCmdEnOp = 12, // Enable all output channels
    LedDriverCmdDisOp = 13, // Disable all output channels
    LedDriverCmdPreactive = 14, // Write enable command (Send before register writes)
    LedDriverCmdMbist = 15, // Enable SRAM checksum read status
} LedDriverCommand;

typedef union {
    uint16_t value;
    struct {
        uint16_t test_0_2        : 3;
        // Bit 3: Cross-version color difference optimization: (Default: 1'h0)
        uint16_t pwm_c           : 1;
        // Bits 5:4: DATA_MAPPING (1: Enable, Other: Disable, Default: 2'h0)
        uint16_t data_mapping_en : 2;
        // Bits 7:6: Low ash uniformity (Default: 2'h0)
        uint16_t opt_lvl         : 2;
        // Bits 12:8: Number of scan lines, (Default: 5'h1F)
        uint16_t scan_line       : 5;
        uint16_t test_13         : 1;
        // Bit 14: Enable open circuit detection (0: Disable, 1: enable, Default: 1'h0)
        uint16_t open_det_en     : 1;
        uint16_t reserved        : 1;
    };
} LedDriverCfg1;
_Static_assert(sizeof(LedDriverCfg1) == sizeof(uint16_t), "LedDriverCfg1 size mismatch");

typedef union {
    uint16_t value;
    struct {
        // Bit 0: TEST (Text ghost optimization, 0=Open/Enable, 1=Close/Disable, Default: 1'h1)
        uint16_t text_ghost_opt_dis : 1;
        // Bits 8:1: IGAIN (Constant current gain, Range 64-255, Default: 8'hFF)
        uint16_t igain              : 8;
        // Bit 9: I_DIV4N (Current divisor select, 1=IOUT*=/256, 0=IOUT*=/1024, Default: 1'h1)
        uint16_t i_div4n            : 1;
        // Bits 14:10: ADJ Blanking level adjustment (Range 0-31, Default: 5'h1F)
        uint16_t blanking           : 5;
        // Bit 15: Reserved
        uint16_t reserved           : 1;
    };
} LedDriverCfg2;
_Static_assert(sizeof(LedDriverCfg2) == sizeof(uint16_t), "LedDriverCfg2 size mismatch");

typedef union {
    uint16_t value;
    struct {
        uint16_t test_cfg    : 2;
        // Bit 2: UP_SEL (Blanking level select, Default: 1'b1)
        uint16_t up_sel      : 1;
        uint16_t test_3      : 1;
        // Bits 7:4: PWM_ADD (Low gray color cast compensation level, Range 0-15, Default: 4'h0)
        uint16_t pwm_add     : 4;
        // Bit 8: Reg_EN (Register 5 write enable, 0: Disable, 1: Enable, Default: 1'h0)
        uint16_t reg_en      : 1;
        // Bit 9: (Register map select, 0: Write Reg1-4, 1: Write Debug Reg5, Default: 1'h1)
        uint16_t reg_map_sel : 1;
        uint16_t test_10_11  : 2;
        uint16_t test_12_14  : 3;
        uint16_t reserved_15 : 1;
    };
} LedDriverCfg3;
_Static_assert(sizeof(LedDriverCfg3) == sizeof(uint16_t), "LedDriverCfg3 size mismatch");

typedef union {
    uint16_t value;
    struct {
        // Bit 0: Mapping_EN (Default: 1'h0)
        uint16_t mapping_en  : 1;
        // Bits 2:1: TRIM_ADJ (Constant current trimming value, Default: 2'h0)
        uint16_t trim_adj    : 2;
        // Bit 3: TRIM_ADD_EN (Trimming sign, 0: Subtract, 1: Add, Default: 1'h0)
        uint16_t trim_add_en : 1;
        // Bits 5:4: DN_SEL (First row dark compensation level, Range 0-3, Default: 2'h0)
        uint16_t dn_sel      : 2;
        // Bit 6: DN (First row dark compensation enable, Default: 1'h1)
        uint16_t dn_en       : 1;
        // Bit 7: OPEN_SCAN (Open circuit detection scan, 0: Off, 1: Reset & On, Default: 1'h0)
        uint16_t open_scan   : 1;
        uint16_t test_8_9    : 2;
        uint16_t test_10_11  : 2;
        uint16_t test_12     : 1;
        uint16_t test_13     : 1;
        // Bit 14: PWM_ADD_EN (Low gray compensation enable, Default: 1'h0)
        uint16_t pwm_add_en  : 1;
        uint16_t reserved    : 1;
    };
} LedDriverCfg4;

_Static_assert(sizeof(LedDriverCfg4) == sizeof(uint16_t), "LedDriverCfg4 size mismatch");

// VSYNC command: LE high during 3x DCLK periods + 1 dummy clock
#define VSYNC_CMD ((1 << 3) | (1 << 5) | (1 << 7))

struct FrontDisplayDriver {
    uint8_t spi_buf[FRONT_DISPLAY_W * FRONT_DISPLAY_H * PIXEL_BUF_LEN];
    uint16_t gamma_lut[GAMMA_LUT_SIZE];
    uint16_t index_lut[FRONT_DISPLAY_W * FRONT_DISPLAY_H];
    uint32_t dma_channel;
    uint32_t refresh_count;
    uint32_t vsync_count;
    FrontDisplayCallback load_done_callback;
    void* callback_context;
};

static FrontDisplayDriver led_driver = {0};

// Send VSYNC command the fastest possible way
void front_display_driver_vsync_trig(void) {
    if(led_driver.vsync_count < START_VSYNC_COUNT) {
        *(uint8_t*)&OCTOSPI1->DR = (uint8_t)VSYNC_CMD;
        led_driver.vsync_count++;
    }
}

// Start OCTOSPI transfer
inline void front_display_driver_send_buf_start(void) {
    LL_DMA_EnableChannel(GPDMA1, led_driver.dma_channel);
}

// Prepare OCTOSPI transfer
static void octospi_send_buf_prepare(FrontDisplayDriver* driver, uint8_t* buf, size_t len) {
    LL_DMA_DisableChannel(GPDMA1, driver->dma_channel);
    LL_DMA_SetSrcAddress(GPDMA1, driver->dma_channel, (uint32_t)(buf));
    LL_DMA_SetDestAddress(GPDMA1, driver->dma_channel, (uint32_t)&OCTOSPI1->DR);
    LL_DMA_SetBlkDataLength(GPDMA1, driver->dma_channel, len);
}

// Wait for OCTOSPI transfer end TODO: wait for thread flag, set in TC IRQ
static void octospi_wait_end(FrontDisplayDriver* driver) {
    while(LL_DMA_IsActiveFlag_IDLE(GPDMA1, driver->dma_channel) == 0) {
    }
    LL_DMA_ClearFlag_TC(GPDMA1, driver->dma_channel);
}

static void octospi_dma_tc_irq(void* context) {
    FrontDisplayDriver* driver = context;

    if(LL_DMA_IsEnabledIT_TC(GPDMA1, driver->dma_channel) &&
       LL_DMA_IsActiveFlag_TC(GPDMA1, driver->dma_channel)) {
        LL_DMA_DisableIT_TC(GPDMA1, driver->dma_channel);
        if(driver->refresh_count < START_REFRESH_COUNT) {
            driver->refresh_count++;
        } else if(driver->refresh_count == START_REFRESH_COUNT) {
            front_display_scan_output_enable(true);
            driver->refresh_count++;
        } else if(driver->load_done_callback) {
            driver->load_done_callback(driver->callback_context);
        }
    }
}

static void octospi_dma_init(FrontDisplayDriver* driver) {
    furi_hal_dma_allocate_gpdma_channel(&driver->dma_channel);

    LL_DMA_InitTypeDef tx_dma_cfg = {0};
    tx_dma_cfg.SrcAddress = 0;
    tx_dma_cfg.DestAddress = (uint32_t)&OCTOSPI1->DR;
    tx_dma_cfg.BlkDataLength = 0;
    tx_dma_cfg.Request = LL_GPDMA1_REQUEST_OCTOSPI1;

    tx_dma_cfg.Direction = LL_DMA_DIRECTION_MEMORY_TO_PERIPH;
    tx_dma_cfg.BlkHWRequest = LL_DMA_HWREQUEST_BLK;
    tx_dma_cfg.DataAlignment = LL_DMA_DATA_ALIGN_ZEROPADD;

    tx_dma_cfg.SrcAllocatedPort = LL_DMA_SRC_ALLOCATED_PORT1;
    tx_dma_cfg.SrcBurstLength = 64; // DMA burst len = OCTOSPI tx FIFO size (64)
    tx_dma_cfg.SrcIncMode = LL_DMA_SRC_INCREMENT;
    tx_dma_cfg.SrcDataWidth = LL_DMA_SRC_DATAWIDTH_BYTE; // TODO: word + burst

    tx_dma_cfg.DestAllocatedPort = LL_DMA_DEST_ALLOCATED_PORT0;
    tx_dma_cfg.DestBurstLength = 64;
    tx_dma_cfg.DestIncMode = LL_DMA_DEST_FIXED;
    tx_dma_cfg.DestDataWidth = LL_DMA_DEST_DATAWIDTH_BYTE;

    tx_dma_cfg.TriggerMode = LL_DMA_TRIGM_BLK_TRANSFER;
    tx_dma_cfg.TriggerPolarity = LL_DMA_TRIG_POLARITY_MASKED;
    tx_dma_cfg.TriggerSelection = 0;

    tx_dma_cfg.TransferEventMode = LL_DMA_TCEM_BLK_TRANSFER;
    tx_dma_cfg.Priority = LL_DMA_HIGH_PRIORITY;
    tx_dma_cfg.LinkAllocatedPort = LL_DMA_LINK_ALLOCATED_PORT1;
    tx_dma_cfg.LinkStepMode = LL_DMA_LSM_FULL_EXECUTION;
    tx_dma_cfg.LinkedListBaseAddr = 0;
    tx_dma_cfg.LinkedListAddrOffset = 0;
    LL_DMA_Init(GPDMA1, driver->dma_channel, &tx_dma_cfg);
    LL_DMA_EnableCDARUpdate(GPDMA1, driver->dma_channel);
    LL_DMA_DisableChannel(GPDMA1, driver->dma_channel);

    furi_hal_interrupt_set_isr_ex(
        furi_hal_dma_get_gpdma_interrupt_id(driver->dma_channel),
        FuriHalInterruptPriorityHighest,
        octospi_dma_tc_irq,
        driver);
}

static void octospi_dma_deinit(FrontDisplayDriver* driver) {
    LL_DMA_DisableChannel(GPDMA1, driver->dma_channel);
    LL_DMA_DeInit(GPDMA1, driver->dma_channel);

    furi_hal_dma_free_gpdma_channel(driver->dma_channel);
}

static void octospi_init(void) {
    furi_hal_bus_enable(FuriHalBusOCTOSPI1);
    furi_hal_bus_enable(FuriHalBusOCTOSPIM);

    OCTOSPI1->DCR1 = (2 << OCTOSPI_DCR1_MTYP_Pos) | (0x1F << OCTOSPI_DCR1_DEVSIZE_Pos);
    OCTOSPI1->DCR2 = ((OCTOSPI_PRESCALLER - 1) << OCTOSPI_DCR2_PRESCALER_Pos);
    OCTOSPI1->DCR3 = 0;
    OCTOSPI1->DCR4 = 0;

    OCTOSPI1->DLR = 0xFFFFFFFF; // Bypass memory size limit

    OCTOSPI1->CCR = (2 << OCTOSPI_CCR_DMODE_Pos);
    OCTOSPI1->WCCR = (2 << OCTOSPI_CCR_DMODE_Pos);
    OCTOSPI1->TCR = 0;
    OCTOSPI1->ABR = 0;

    OCTOSPI1->CR = OCTOSPI_CR_DMAEN;

    OCTOSPI1->CR |= OCTOSPI_CR_EN;

    OCTOSPIM->PCR[0] |= (OCTOSPIM_PCR_DQSEN | OCTOSPIM_PCR_CLKEN);

    furi_hal_gpio_init_ex(
        &gpio_front_display_sdi_ospi_d0,
        GpioModeAltFunctionPushPull,
        GpioPullNo,
        DISPLAY_GPIO_SPEED,
        GpioAltFn10OCTOSPI1);
    furi_hal_gpio_init_ex(
        &gpio_front_display_le_ospi_d1,
        GpioModeAltFunctionPushPull,
        GpioPullNo,
        DISPLAY_GPIO_SPEED,
        GpioAltFn10OCTOSPI1);
    furi_hal_gpio_init_ex(
        &gpio_front_display_dclk_ospi_clk,
        GpioModeAltFunctionPushPull,
        GpioPullNo,
        DISPLAY_GPIO_SPEED,
        GpioAltFn10OCTOSPI1);
}

static void octospi_deinit(void) {
    furi_hal_bus_reset(FuriHalBusOCTOSPI1);
    furi_hal_bus_reset(FuriHalBusOCTOSPIM);
    furi_hal_bus_disable(FuriHalBusOCTOSPI1);
    furi_hal_bus_disable(FuriHalBusOCTOSPIM);
    furi_hal_gpio_init(&gpio_front_display_sdi_ospi_d0, GpioModeInput, GpioPullDown, GpioSpeedLow);
    furi_hal_gpio_init(&gpio_front_display_le_ospi_d1, GpioModeInput, GpioPullDown, GpioSpeedLow);
    furi_hal_gpio_init(
        &gpio_front_display_dclk_ospi_clk, GpioModeInput, GpioPullDown, GpioSpeedLow);
}

static FURI_ALWAYS_INLINE void led_driver_add_le_cmd(uint8_t* tx_data, LedDriverCommand cmd) {
    uint32_t cmd_mask = 0;

    uint8_t bitcnt = 0;
    while(bitcnt < cmd) {
        cmd_mask |= (1 << (bitcnt * 2));
        bitcnt++;
    }

    cmd_mask <<= 1;
    tx_data[0] |= (cmd_mask >> 24);
    tx_data[1] |= (cmd_mask >> 16);
    tx_data[2] |= (cmd_mask >> 8);
    tx_data[3] |= cmd_mask;
}

static const uint8_t interleave_lut[16] =
    {0x00, 0x01, 0x04, 0x05, 0x10, 0x11, 0x14, 0x15, 0x40, 0x41, 0x44, 0x45, 0x50, 0x51, 0x54, 0x55};

static inline void led_driver_encode_byte(uint8_t* tx_data, uint8_t data) {
    tx_data[0] = interleave_lut[data >> 4];
    tx_data[1] = interleave_lut[data & 0x0f];
}

static FURI_ALWAYS_INLINE uint16_t
    front_display_gamma_apply(const uint16_t* gamma_lut, uint8_t in_val) {
    return (gamma_lut[in_val]);
}

static float front_display_gamma_adjust_for_brightness(float gamma, uint8_t brightness) {
    return DISPLAY_GAMMA_MIN + (brightness * ((gamma - DISPLAY_GAMMA_MIN) / BRIGHTNESS_VAL_MAX));
}

static void
    front_display_gamma_lut_generate(uint16_t* gamma_lut, float gamma_val, uint8_t brightness) {
    if(brightness == 0) {
        // Everything is black
        memset(gamma_lut, 0, sizeof(uint16_t) * GAMMA_LUT_SIZE);
        return;
    }

    if(brightness > BRIGHTNESS_VAL_MAX) {
        brightness = BRIGHTNESS_VAL_MAX;
    }

    const float adjusted_gamma = front_display_gamma_adjust_for_brightness(gamma_val, brightness);
    const float brightness_coeff = (float)brightness * UINT16_MAX / BRIGHTNESS_VAL_MAX;

    for(uint32_t i = 0; i < GAMMA_LUT_SIZE; i++) {
        const float val_in = (float)i / UINT8_MAX;
        const float val_out = powf(val_in, adjusted_gamma);
        gamma_lut[i] = ceilf(val_out * brightness_coeff);
    }
}

static void front_display_index_lut_generate(FrontDisplayDriver* driver) {
    for(uint32_t tx_idx = 0; tx_idx < (FRONT_DISPLAY_W * FRONT_DISPLAY_H); tx_idx++) {
        uint32_t block_n = LED_DRIVER_CHAIN - (tx_idx % LED_DRIVER_CHAIN) - 1;
        uint32_t x = block_n * DISPLAY_BLOCKS + (tx_idx / LED_DRIVER_CHAIN) / FRONT_DISPLAY_H;
        uint32_t y = (tx_idx / LED_DRIVER_CHAIN) % FRONT_DISPLAY_H;

        if(y < 8) {
            y = 15 - y;
        } else {
            y -= 8;
        }

        driver->index_lut[tx_idx] = x + y * FRONT_DISPLAY_W;
    }
}

static void
    led_driver_encode_pixel(uint8_t* tx_data, const uint8_t* pix_data, const uint16_t* gamma) {
    // Fast path for empty (black) pixels
    if(pix_data[0] == 0 && pix_data[1] == 0 && pix_data[2] == 0) {
        // Clear all bytes at once for empty pixels
        memset(tx_data, 0, 12);
        return;
    }

    uint16_t led_data = front_display_gamma_apply(gamma, pix_data[0]);
    led_driver_encode_byte(&tx_data[0], (led_data >> 8));
    led_driver_encode_byte(&tx_data[2], (led_data & 0xFF));

    led_data = front_display_gamma_apply(gamma, pix_data[1]);
    led_driver_encode_byte(&tx_data[4], (led_data >> 8));
    led_driver_encode_byte(&tx_data[6], (led_data & 0xFF));

    led_data = front_display_gamma_apply(gamma, pix_data[2]);
    led_driver_encode_byte(&tx_data[8], (led_data >> 8));
    led_driver_encode_byte(&tx_data[10], (led_data & 0xFF));
}

static void led_driver_encode_cmd_16(uint8_t* tx_buf, LedDriverCommand cmd, uint16_t data) {
    led_driver_encode_byte(&tx_buf[0], (data >> 8));
    led_driver_encode_byte(&tx_buf[2], (data & 0xFF));

    led_driver_add_le_cmd(tx_buf, cmd);
}

static void led_driver_encode_buffer(FrontDisplayDriver* driver, const uint8_t* frame_buf) {
    size_t tx_idx_offset = 0;
    size_t buf_offset = 0;

    for(size_t transfer_n = 0; transfer_n < TRANSFER_COUNT; transfer_n++) {
        for(size_t pixel_n = 0; pixel_n < LED_DRIVER_CHAIN; pixel_n++) {
            uint32_t fb_offset = driver->index_lut[tx_idx_offset++];
            led_driver_encode_pixel(
                &(driver->spi_buf)[buf_offset], &frame_buf[fb_offset * 3], driver->gamma_lut);
            buf_offset += PIXEL_BUF_LEN;
        }

        led_driver_add_le_cmd(&(driver->spi_buf)[buf_offset - 4], LedDriverCmdDataLatch);
    }
}

static void led_driver_encode_empty_buffer(FrontDisplayDriver* driver) {
    const uint8_t empty_pixel[3] = {0};

    for(size_t transfer_n = 0, buf_offset = 0; transfer_n < TRANSFER_COUNT; transfer_n++) {
        for(size_t pixel_n = 0; pixel_n < LED_DRIVER_CHAIN; pixel_n++) {
            led_driver_encode_pixel(driver->spi_buf + buf_offset, empty_pixel, driver->gamma_lut);
            buf_offset += PIXEL_BUF_LEN;
        }

        led_driver_add_le_cmd(&(driver->spi_buf)[buf_offset - 4], LedDriverCmdDataLatch);
    }
}

static void
    led_driver_write_reg(FrontDisplayDriver* driver, LedDriverCommand cmd, const uint16_t data[]) {
    size_t tx_len = 4 * (1 + 3 * LED_DRIVER_CHAIN);
    memset(driver->spi_buf, 0, tx_len);
    size_t ptr = 0;

    led_driver_encode_cmd_16(&driver->spi_buf[4 * ptr++], LedDriverCmdPreactive, 0);
    for(size_t i = 0; i < LED_DRIVER_CHAIN; i++) {
        if(i != LED_DRIVER_CHAIN - 1) {
            led_driver_encode_cmd_16(&driver->spi_buf[4 * ptr++], LedDriverCmdNone, data[2]);
            led_driver_encode_cmd_16(&driver->spi_buf[4 * ptr++], LedDriverCmdNone, data[1]);
            led_driver_encode_cmd_16(&driver->spi_buf[4 * ptr++], LedDriverCmdNone, data[0]);
        } else {
            int8_t cmd_len_remain = cmd;
            LedDriverCommand cmd_len_cur = LedDriverCmdNone;
            if(cmd_len_remain > 32) {
                cmd_len_cur = cmd_len_remain - 32;
                cmd_len_remain = 32;
            } else {
                cmd_len_cur = LedDriverCmdNone;
            }
            led_driver_encode_cmd_16(&driver->spi_buf[4 * ptr++], cmd_len_cur, data[2]);
            if(cmd_len_remain > 16) {
                cmd_len_cur = cmd_len_remain - 16;
                cmd_len_remain = 16;
            } else {
                cmd_len_cur = LedDriverCmdNone;
            }
            led_driver_encode_cmd_16(&driver->spi_buf[4 * ptr++], cmd_len_cur, data[1]);
            led_driver_encode_cmd_16(&driver->spi_buf[4 * ptr++], cmd_len_remain, data[0]);
        }
    }

    octospi_send_buf_prepare(driver, driver->spi_buf, tx_len);
    front_display_driver_send_buf_start();
    octospi_wait_end(driver);
}

static void front_display_driver_send_init(FrontDisplayDriver* driver) {
    const LedDriverCfg1 cfg1 = {
        .scan_line = DISPLAY_BLOCKS - 1,
        .data_mapping_en = 3, // Disable data mapping
    };
    led_driver_write_reg(
        driver, LedDriverCmdWriteCfg1, (const uint16_t[]){cfg1.value, cfg1.value, cfg1.value});

    const LedDriverCfg2 cfg2_r = {
        .blanking = 31,
        .i_div4n = 1,
        .igain = CUR_GAIN_R,
        .text_ghost_opt_dis = 1,
    };
    const LedDriverCfg2 cfg2_g = {
        .blanking = 28,
        .i_div4n = 1,
        .igain = CUR_GAIN_G,
        .text_ghost_opt_dis = 1,
    };
    const LedDriverCfg2 cfg2_b = {
        .blanking = 23,
        .i_div4n = 1,
        .igain = CUR_GAIN_B,
        .text_ghost_opt_dis = 1,
    };
    led_driver_write_reg(
        driver,
        LedDriverCmdWriteCfg2,
        (const uint16_t[]){cfg2_r.value, cfg2_g.value, cfg2_b.value});

    const LedDriverCfg3 cfg3 = {
        .test_12_14 = 4,
        .reg_en = 1,
        .pwm_add = 15,
        .up_sel = 1,
        .test_cfg = 3,
    };
    led_driver_write_reg(
        driver, LedDriverCmdWriteCfg3, (const uint16_t[]){cfg3.value, cfg3.value, cfg3.value});

    const LedDriverCfg4 cfg4 = {
        .pwm_add_en = 1,
        .mapping_en = 1,
    };
    led_driver_write_reg(
        driver, LedDriverCmdWriteCfg4, (const uint16_t[]){cfg4.value, cfg4.value, cfg4.value});

    // Enable all output channels
    led_driver_write_reg(driver, LedDriverCmdEnOp, (const uint16_t[]){0, 0, 0});
}

static void front_display_driver_send_buffer(FrontDisplayDriver* driver) {
    LL_DMA_ClearFlag_TC(GPDMA1, driver->dma_channel);
    LL_DMA_EnableIT_TC(GPDMA1, driver->dma_channel);
    octospi_send_buf_prepare(driver, driver->spi_buf, sizeof(driver->spi_buf));
    front_display_scan_data_sync_enable();
}

void front_display_driver_send_frame(const uint8_t* frame_buf) {
    led_driver_encode_buffer(&led_driver, frame_buf);
    front_display_driver_send_buffer(&led_driver);
}

void front_display_driver_init(uint8_t initial_brightness) {
    memset(&led_driver, 0, sizeof(FrontDisplayDriver));

    front_display_index_lut_generate(&led_driver);
    front_display_gamma_lut_generate(led_driver.gamma_lut, DISPLAY_GAMMA, initial_brightness);

    octospi_init();
    octospi_dma_init(&led_driver);

    front_display_driver_send_init(&led_driver);

    led_driver_encode_empty_buffer(&led_driver);
    octospi_send_buf_prepare(&led_driver, led_driver.spi_buf, sizeof(led_driver.spi_buf));
    front_display_driver_send_buf_start();
    octospi_wait_end(&led_driver);
}

void front_display_driver_deinit(void) {
    octospi_dma_deinit(&led_driver);
    octospi_deinit();
}

void front_display_driver_set_update_callback(FrontDisplayCallback callback, void* context) {
    led_driver.load_done_callback = callback;
    led_driver.callback_context = context;
}

void front_display_driver_start(void) {
    led_driver_encode_empty_buffer(&led_driver);

    while(led_driver.refresh_count < START_REFRESH_COUNT) {
        // TODO: Replace delay with proper synchronisation
        furi_delay_ms(5);
        front_display_driver_send_buffer(&led_driver);
    }
}

void front_display_driver_set_brightness(uint8_t brightness) {
    front_display_gamma_lut_generate(led_driver.gamma_lut, DISPLAY_GAMMA, brightness);
}
