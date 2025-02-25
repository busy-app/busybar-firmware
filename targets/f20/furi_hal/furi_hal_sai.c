#include <stm32u5xx.h>
#include <stm32u5xx_ll_dma.h>
#include <stm32u5xx_ll_rcc.h>
#include <furi_hal_clock.h>
#include <furi_hal_dma.h>
#include <furi_hal_bus.h>
#include <furi_hal_resources.h>
#include <furi_hal_interrupt.h>
#include <furi_hal_sai.h>

#define FURI_HAL_SAI       SAI1
#define FURI_HAL_SAI_BLOCK SAI1_Block_A

#define TAG "FuriHalSAI"

#define SAI_DEFAULT_TIMEOUT 4U

#define FURI_HAL_SAI_DMA              GPDMA1
#define FURI_HAL_SAI_DMA_REQUEST      LL_GPDMA1_REQUEST_SAI1_A
#define FURI_HAL_SAI_DMA_PRIORITY     LL_DMA_HIGH_PRIORITY
#define FURI_HAL_SAI_DMA_SAMPLE_COUNT (256u * 10u)

typedef __PACKED_STRUCT {
    int16_t left;
    int16_t right;
}
SaiData;

static_assert(sizeof(SaiData) == 4);

typedef struct {
    FuriHalSaiCallback callback;
    void* callback_context;

    SaiData data[FURI_HAL_SAI_DMA_SAMPLE_COUNT];

    uint32_t dma_channel;
    uint32_t data_ptr;
    uint32_t data_size;
} FuriHalSai;

static FuriHalSai furi_hal_sai = {};

static void furi_hal_sai_refill(size_t start, size_t size) {
    for(size_t i = 0; i < size; i++) {
        int16_t sample = 0;

        if(furi_hal_sai.callback) {
            sample = furi_hal_sai.callback(furi_hal_sai.callback_context);
        }

        furi_hal_sai.data[start + i].left = sample;
        furi_hal_sai.data[start + i].right = sample;
    }
}

static void furi_hal_sai_isr(void*) {
    if(LL_DMA_IsActiveFlag_TC(GPDMA1, furi_hal_sai.dma_channel)) {
        LL_DMA_ClearFlag_TC(GPDMA1, furi_hal_sai.dma_channel);
        furi_hal_sai_refill(FURI_HAL_SAI_DMA_SAMPLE_COUNT / 2, FURI_HAL_SAI_DMA_SAMPLE_COUNT / 2);
    }
    if(LL_DMA_IsActiveFlag_HT(GPDMA1, furi_hal_sai.dma_channel)) {
        LL_DMA_ClearFlag_HT(GPDMA1, furi_hal_sai.dma_channel);
        furi_hal_sai_refill(0, FURI_HAL_SAI_DMA_SAMPLE_COUNT / 2);
    }

    if(LL_DMA_IsActiveFlag_TO(GPDMA1, furi_hal_sai.dma_channel)) {
        LL_DMA_ClearFlag_TO(GPDMA1, furi_hal_sai.dma_channel);
        furi_crash("SAI DMA trigger overrun");
    }
    if(LL_DMA_IsActiveFlag_SUSP(GPDMA1, furi_hal_sai.dma_channel)) {
        LL_DMA_ClearFlag_SUSP(GPDMA1, furi_hal_sai.dma_channel);
        furi_crash("SAI DMA suspension");
    }
    if(LL_DMA_IsActiveFlag_USE(GPDMA1, furi_hal_sai.dma_channel)) {
        LL_DMA_ClearFlag_USE(GPDMA1, furi_hal_sai.dma_channel);
        furi_crash("SAI DMA user setting error Hi");
    }
    if(LL_DMA_IsActiveFlag_ULE(GPDMA1, furi_hal_sai.dma_channel)) {
        LL_DMA_ClearFlag_ULE(GPDMA1, furi_hal_sai.dma_channel);
        furi_crash("SAI DMA user setting error Lo");
    }
    if(LL_DMA_IsActiveFlag_DTE(GPDMA1, furi_hal_sai.dma_channel)) {
        LL_DMA_ClearFlag_DTE(GPDMA1, furi_hal_sai.dma_channel);
        furi_crash("SAI DMA data transfer error");
    }
}

static bool furi_hal_sai_disable() {
    FURI_HAL_SAI_BLOCK->CR1 &= ~SAI_xCR1_SAIEN;

    uint32_t count = SAI_DEFAULT_TIMEOUT * (SystemCoreClock / 7U / 1000U);
    do {
        if(count == 0U) {
            return false;
        }
        count--;
    } while((FURI_HAL_SAI_BLOCK->CR1 & SAI_xCR1_SAIEN) != 0U);

    return true;
}

static bool furi_hal_sai_enable() {
    FURI_HAL_SAI_BLOCK->CR1 |= SAI_xCR1_SAIEN;

    uint32_t count = SAI_DEFAULT_TIMEOUT * (SystemCoreClock / 7U / 1000U);
    do {
        if(count == 0U) {
            return false;
        }
        count--;
    } while((FURI_HAL_SAI_BLOCK->CR1 & SAI_xCR1_SAIEN) == 0U);

    return true;
}

static void furi_hal_sai_setup_dma() {
    furi_check(furi_hal_dma_allocate_gpdma_channel(&furi_hal_sai.dma_channel));

    LL_DMA_InitTypeDef dma_init_strust = {0};

    furi_hal_sai.data_ptr = (uint32_t)furi_hal_sai.data;
    furi_hal_sai.data_size = sizeof(furi_hal_sai.data);

    dma_init_strust.SrcAddress = furi_hal_sai.data_ptr;
    dma_init_strust.DestAddress = (uint32_t)(&FURI_HAL_SAI_BLOCK->DR);
    dma_init_strust.BlkDataLength = furi_hal_sai.data_size;

    dma_init_strust.Request = FURI_HAL_SAI_DMA_REQUEST;
    dma_init_strust.Direction = LL_DMA_DIRECTION_MEMORY_TO_PERIPH;

    dma_init_strust.SrcAllocatedPort = LL_DMA_SRC_ALLOCATED_PORT1;
    dma_init_strust.SrcBurstLength = 8;
    dma_init_strust.SrcIncMode = LL_DMA_SRC_INCREMENT;
    dma_init_strust.SrcDataWidth = LL_DMA_SRC_DATAWIDTH_HALFWORD;

    dma_init_strust.DestAllocatedPort = LL_DMA_DEST_ALLOCATED_PORT0;
    dma_init_strust.DestBurstLength = 1;
    dma_init_strust.DestIncMode = LL_DMA_DEST_FIXED;
    dma_init_strust.DestDataWidth = LL_DMA_DEST_DATAWIDTH_HALFWORD;

    dma_init_strust.Priority = FURI_HAL_SAI_DMA_PRIORITY;
    dma_init_strust.LinkAllocatedPort = LL_DMA_LINK_ALLOCATED_PORT1;
    dma_init_strust.LinkedListBaseAddr = (uint32_t)&furi_hal_sai.data_ptr;
    dma_init_strust.LinkedListAddrOffset = (uint32_t)&furi_hal_sai.data_ptr;

    LL_DMA_Init(GPDMA1, furi_hal_sai.dma_channel, &dma_init_strust);
    LL_DMA_EnableCSARUpdate(GPDMA1, furi_hal_sai.dma_channel);

    LL_DMA_EnableIT_TC(GPDMA1, furi_hal_sai.dma_channel);
    LL_DMA_EnableIT_HT(GPDMA1, furi_hal_sai.dma_channel);

    LL_DMA_EnableIT_TO(GPDMA1, furi_hal_sai.dma_channel);
    LL_DMA_EnableIT_SUSP(GPDMA1, furi_hal_sai.dma_channel);
    LL_DMA_EnableIT_USE(GPDMA1, furi_hal_sai.dma_channel);
    LL_DMA_EnableIT_ULE(GPDMA1, furi_hal_sai.dma_channel);
    LL_DMA_EnableIT_DTE(GPDMA1, furi_hal_sai.dma_channel);

    furi_hal_interrupt_set_isr(
        furi_hal_dma_get_gpdma_interrupt_id(furi_hal_sai.dma_channel), furi_hal_sai_isr, NULL);

    //Start DMA Channel
    LL_DMA_EnableChannel(GPDMA1, furi_hal_sai.dma_channel);
}

void furi_hal_sai_transmit_start() {
}

bool furi_hal_sai_init() {
    // Init the low level hardware : GPIO, CLOCK, NVIC and DMA
    LL_RCC_SetSAIClockSource(LL_RCC_SAI1_CLKSOURCE_PLL1);

    // Enable SAI peripheral clock
    furi_hal_bus_enable(FuriHalBusSAI1);

    furi_hal_gpio_init_ex(
        &gpio_i2s_sd, GpioModeAltFunctionPushPull, GpioPullNo, GpioSpeedLow, GpioAltFn13SAI1);
    furi_hal_gpio_init_ex(
        &gpio_i2s_fs, GpioModeAltFunctionPushPull, GpioPullNo, GpioSpeedLow, GpioAltFn13SAI1);
#ifndef FURI_HAL_CLOCK_MCO
    furi_hal_gpio_init_ex(
        &gpio_i2s_sck, GpioModeAltFunctionPushPull, GpioPullNo, GpioSpeedLow, GpioAltFn13SAI1);
#endif
    furi_hal_gpio_init(
        &gpio_audio_en_and_917_swo, GpioModeOutputPushPull, GpioPullNo, GpioSpeedLow);

    furi_hal_gpio_write(&gpio_audio_en_and_917_swo, true);

    // Disable the selected SAI peripheral
    if(!furi_hal_sai_disable()) {
        FURI_LOG_E(TAG, "furi_hal_sai_disable failed");
        return false;
    }

    // No SYNCEXT, ASYNCHRONOUS
    uint32_t syncen_bits = 0;
    SAI1->GCR = 0;

    uint32_t mckdiv = 0;

    {
        uint32_t freq = furi_hal_clock_get_freq(FuriHalClockHwSAI1);
        // NODIV = 0
        uint32_t tmposr = 1U;
        // (freq x 10) to keep Significant digits
        uint32_t tmpval = (freq * 10U) / ((44100U) * tmposr * 256U);

        mckdiv = tmpval / 10U;

        // Round result to the nearest integer
        if((tmpval % 10U) > 8U) {
            mckdiv += 1U;
        }

        uint32_t real_freq = (freq * 10U) / (tmposr * mckdiv * 256U);
        FURI_LOG_I(TAG, "mckdiv: %lu, real_freq: %lu", mckdiv, real_freq);
        float error = (real_freq - 44100.0f) / 44100.0f;
        FURI_LOG_I(TAG, "error: %.2f%%", error);
    }

    furi_check(mckdiv <= 63U);

    // Transmit clock strobing
    uint32_t ckstr_bits = SAI_xCR1_CKSTR; // SAI_xCR1_CKSTR - falling edge, 0 - rising edge

#define SAI_MODEMASTER_TX            0x00000000U
#define SAI_FREE_PROTOCOL            0x00000000U
#define SAI_DATASIZE_16              SAI_xCR1_DS_2
#define SAI_FIRSTBIT_MSB             0x00000000U
#define SAI_STEREOMODE               0x00000000U
#define SAI_OUTPUTDRIVE_DISABLE      0x00000000U
#define SAI_MASTERDIVIDER_ENABLE     0x00000000U
#define SAI_MCK_OUTPUT_DISABLE       0x00000000U
#define SAI_MCK_OVERSAMPLING_DISABLE 0x00000000U

#define SAI_FIFOTHRESHOLD_EMPTY 0x00000000U
#define SAI_NOCOMPANDING        0x00000000U
#define SAI_OUTPUT_NOTRELEASED  0x00000000U

// Frame: I2S, datasize 16
#define SAI_SLOTSIZE_16B SAI_xSLOTR_SLOTSZ_0
    uint32_t nbslot = 2;
    uint32_t frame_length = 32U * (nbslot / 2U);
    uint32_t active_frame_length = 16U * (nbslot / 2U);

#define SAI_FS_BEFOREFIRSTBIT         SAI_xFRCR_FSOFF
#define SAI_FS_ACTIVE_LOW             0x00000000U
#define SAI_FS_CHANNEL_IDENTIFICATION SAI_xFRCR_FSDEF

#define SAI_FIRSTBIT_OFFSET_0 0x00000000U
#define SAI_SLOTACTIVE_ALL    0x0000FFFFU

    // SAI CR1 Configuration
    FURI_HAL_SAI_BLOCK->CR1 &=
        ~(SAI_xCR1_MODE | SAI_xCR1_PRTCFG | SAI_xCR1_DS | SAI_xCR1_LSBFIRST | SAI_xCR1_CKSTR |
          SAI_xCR1_SYNCEN | SAI_xCR1_MONO | SAI_xCR1_OUTDRIV | SAI_xCR1_DMAEN | SAI_xCR1_NODIV |
          SAI_xCR1_MCKDIV | SAI_xCR1_OSR | SAI_xCR1_MCKEN);

    FURI_HAL_SAI_BLOCK->CR1 |=
        (SAI_MODEMASTER_TX | SAI_FREE_PROTOCOL | SAI_DATASIZE_16 | SAI_FIRSTBIT_MSB | ckstr_bits |
         syncen_bits | SAI_STEREOMODE | SAI_OUTPUTDRIVE_DISABLE | SAI_MASTERDIVIDER_ENABLE |
         (mckdiv << 20) | SAI_MCK_OVERSAMPLING_DISABLE | SAI_MCK_OUTPUT_DISABLE);

    // SAI CR2 Configuration
    FURI_HAL_SAI_BLOCK->CR2 &= ~(SAI_xCR2_FTH | SAI_xCR2_FFLUSH | SAI_xCR2_COMP | SAI_xCR2_CPL);
    FURI_HAL_SAI_BLOCK->CR2 |=
        (SAI_FIFOTHRESHOLD_EMPTY | SAI_NOCOMPANDING | SAI_OUTPUT_NOTRELEASED);

    // SAI Frame Configuration
    FURI_HAL_SAI_BLOCK->FRCR &=
        (~(SAI_xFRCR_FRL | SAI_xFRCR_FSALL | SAI_xFRCR_FSDEF | SAI_xFRCR_FSPOL | SAI_xFRCR_FSOFF));
    FURI_HAL_SAI_BLOCK->FRCR |=
        ((frame_length - 1U) | SAI_FS_BEFOREFIRSTBIT | SAI_FS_CHANNEL_IDENTIFICATION |
         SAI_FS_ACTIVE_LOW | ((active_frame_length - 1U) << 8));

    // SAI Block_x SLOT Configuration
    // This register has no meaning in AC 97 and SPDIF audio protocol
    FURI_HAL_SAI_BLOCK->SLOTR &=
        (~(SAI_xSLOTR_FBOFF | SAI_xSLOTR_SLOTSZ | SAI_xSLOTR_NBSLOT | SAI_xSLOTR_SLOTEN));

    FURI_HAL_SAI_BLOCK->SLOTR |= SAI_FIRSTBIT_OFFSET_0 | SAI_SLOTSIZE_16B |
                                 (SAI_SLOTACTIVE_ALL << 16) | ((nbslot - 1U) << 8);

    // Disable PDM
    FURI_HAL_SAI->PDMCR &= ~(SAI_PDMCR_PDMEN);

    FURI_LOG_I(TAG, "furi_hal_sai_init done");

    furi_hal_sai_setup_dma();

    // Enable DMA request
    FURI_HAL_SAI_BLOCK->CR1 |= SAI_xCR1_DMAEN;

    return true;
}

void furi_hal_sai_start(FuriHalSaiCallback callback, void* callback_context) {
    furi_hal_sai.callback = callback;
    furi_hal_sai.callback_context = callback_context;

    furi_hal_sai_refill(0, FURI_HAL_SAI_DMA_SAMPLE_COUNT);
    furi_check(furi_hal_sai_enable());
}

void furi_hal_sai_stop(void) {
    furi_check(furi_hal_sai_disable());

    furi_hal_sai.callback = NULL;
    furi_hal_sai.callback_context = NULL;
}
