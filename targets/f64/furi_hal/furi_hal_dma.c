#include <furi_hal_dma.h>

#include <furi_hal_bus.h>
#include <furi_hal_interrupt.h>

#include <furi.h>

#include <si91x_device.h>

#define UDMA_CH_COUNT (32U)

#define UDMA_CFG_EN_SET (1U)

// TODO: Convert to a struct and put to a separate file
#define M4SS_INTR_SEL_BASE        (0x46110000UL)
#define M4SS_UDMA_INTR_SEL_OFFSET (0x0CUL)
#define M4SS_UDMA_INTR_SEL \
    (*((uint32_t volatile*)(M4SS_INTR_SEL_BASE + M4SS_UDMA_INTR_SEL_OFFSET)))

#define PERIPHERAL_UDMA_DMA_SEL (*((uint32_t volatile*)(0x46008000UL + 0x58)))

typedef struct {
    uint32_t transfer_type  : 3;
    uint32_t next_burst     : 1;
    uint32_t transfer_count : 10;
    uint32_t arbitr_size    : 4;
    uint32_t src_protect    : 3;
    uint32_t dst_protect    : 3;
    uint32_t src_width      : 2;
    uint32_t src_increment  : 2;
    uint32_t dst_width      : 2;
    uint32_t dst_increment  : 2;
} FuriHalDmaChannelConfig;

static_assert(sizeof(FuriHalDmaChannelConfig) == 4);

typedef struct {
    volatile uint32_t src_end_addr;
    volatile uint32_t dst_end_addr;
    volatile FuriHalDmaChannelConfig config;
    volatile uint32_t unused;
} FuriHalDmaDescriptor;

static_assert(sizeof(FuriHalDmaDescriptor) == 16);

typedef struct {
    FuriHalDmaCallback callback;
    void* context;
} FuriHalDmaChannelData;

static PLACE_IN_SECTION(".udma_addr0")
    FuriHalDmaDescriptor furi_hal_dma_descriptor[UDMA_CH_COUNT * 2];

static FuriHalDmaChannelData furi_hal_dma_channel[UDMA_CH_COUNT];

static void furi_hal_dma_udma_init(void) {
    memset(furi_hal_dma_descriptor, 0, sizeof(furi_hal_dma_descriptor));

    UDMA0->CTRL_BASE_PTR = (uint32_t)furi_hal_dma_descriptor;
    UDMA0->UDMA_CONFIG_CTRL_REG = 1UL; // Enable single requests
    UDMA0->DMA_CFG = UDMA_CFG_EN_SET;

    NVIC_SetPriority(UDMA0_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 10, 0));
    NVIC_EnableIRQ(UDMA0_IRQn);
}

void furi_hal_dma_init_early(void) {
    furi_hal_bus_enable(FuriHalBusUDMA_HCLK);
    furi_hal_dma_udma_init();
}

void furi_hal_dma_deinit_early(void) {
    furi_hal_bus_disable(FuriHalBusUDMA_HCLK);
}

void furi_hal_dma_set_callback(
    FuriHalDmaChannel channel,
    FuriHalDmaCallback callback,
    void* context) {
    furi_check(channel < FuriHalDmaChannelMax);

    FuriHalDmaChannelData* channel_data = &furi_hal_dma_channel[channel];

    channel_data->callback = callback;
    channel_data->context = context;
}

void furi_hal_dma_init_channel(FuriHalDmaChannel channel, const FuriHalDmaTransfer* transfer) {
    furi_check(channel < FuriHalDmaChannelMax);
    furi_check(transfer);

    const uint32_t channel_mask = 1UL << channel;

    UDMA0->CHNL_ENABLE_CLR = channel_mask;
    UDMA0->CHNL_PRI_ALT_CLR = channel_mask;
    UDMA0->UDMA_SKIP_DESC_FETCH_REG |= channel_mask;

    FuriHalDmaDescriptor* desc = &furi_hal_dma_descriptor[channel];

    const FuriHalDmaChannelConfig cfg = {
        .next_burst = 0,
        .transfer_type = transfer->type,
        .transfer_count = transfer->count - 1,
        .arbitr_size = 0,
        .src_width = transfer->src_width,
        .src_increment = transfer->src_increment,
        .dst_width = transfer->dst_width,
        .dst_increment = transfer->dst_increment,
    };

    desc->config = cfg;

    if(transfer->src_increment != FuriHalDmaAddressIncrementNone) {
        const uint32_t src_offset = (transfer->count - 1) << transfer->src_width;
        desc->src_end_addr = transfer->src_address + src_offset;
    } else {
        desc->src_end_addr = transfer->src_address;
    }

    if(transfer->dst_increment != FuriHalDmaAddressIncrementNone) {
        const uint32_t dst_offset = (transfer->count - 1) << transfer->dst_width;
        desc->dst_end_addr = transfer->dst_address + dst_offset;
    } else {
        desc->dst_end_addr = transfer->dst_address;
    }

    // TODO: This should be enabled by a separate function
    PERIPHERAL_UDMA_DMA_SEL |= 1UL;

    UDMA0->CHNL_ENABLE_SET = channel_mask;
    M4SS_UDMA_INTR_SEL |= channel_mask;
}

void furi_hal_dma_deinit_channel(FuriHalDmaChannel channel) {
    FURI_CRITICAL_ENTER();

    const uint32_t channel_mask = 1UL << channel;

    M4SS_UDMA_INTR_SEL &= ~channel_mask;

    UDMA0->CHNL_ENABLE_CLR = channel_mask;
    UDMA0->UDMA_DONE_STATUS_REG = channel_mask;

    FuriHalDmaDescriptor* desc = &furi_hal_dma_descriptor[channel];
    desc->config.transfer_type = FuriHalDmaTransferTypeStop;

    FURI_CRITICAL_EXIT();
}

// UDMA1
void IRQ010_Handler(void) {
    furi_crash();
}

// GPDMA
void IRQ031_Handler(void) {
    furi_crash();
}

// UDMA0
void IRQ033_Handler(void) {
    /* Save the register outside of the loop to avoid repeated loads */
    const uint32_t done_status = UDMA0->UDMA_DONE_STATUS_REG;

    for(uint32_t i = 0; i < UDMA_CH_COUNT; ++i) {
        const uint32_t channel_mask = 1UL << i;
        if(done_status & channel_mask) {
            FuriHalDmaChannelData* channel_data = &furi_hal_dma_channel[i];
            /* Execute the transfer finished callback */
            if(channel_data->callback) {
                channel_data->callback(channel_data->context);
            }
            /* Clear channel done flag */
            UDMA0->UDMA_DONE_STATUS_REG = channel_mask;
        }
    }
}
