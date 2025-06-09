#include "front_display_i.h"

#include <stm32u5xx_ll_tim.h>
#include <stm32u5xx_ll_rcc.h>
#include <stm32u5xx_ll_spi.h>
#include <stm32u5xx_ll_dma.h>

#include <furi.h>

#define GCLK_COUNT      138
#define GCLK_PRESCALER  10
#define SCAN_PRESCALLER 16
#define SCAN_PERIOD     180
#define VSYNC_DELAY     10
#define LATCH_DELAY     4

#define SCAN_DISABLED (1 << 5)

// Display block select look-up table
static const uint8_t display_scan_table[DISPLAY_BLOCKS] = {
    (1 << 0) | (4 << 2), (1 << 0) | (2 << 2), (1 << 0) | (6 << 2), (1 << 0) | (1 << 2),
    (1 << 0) | (5 << 2), (1 << 0) | (3 << 2), (1 << 0) | (7 << 2),

    (1 << 1) | (0 << 2), (1 << 1) | (4 << 2), (1 << 1) | (2 << 2), (1 << 1) | (6 << 2),
    (1 << 1) | (1 << 2), (1 << 1) | (5 << 2), (1 << 1) | (3 << 2), (1 << 1) | (7 << 2),

    (1 << 6) | (0 << 2), (1 << 6) | (4 << 2), (1 << 6) | (2 << 2), (1 << 6) | (6 << 2),
    (1 << 6) | (1 << 2), (1 << 6) | (5 << 2), (1 << 6) | (3 << 2), (1 << 6) | (7 << 2),

    (1 << 0) | (0 << 2),
};

struct FrontDisplayScan {
    LL_DMA_LinkNodeTypeDef dma_link_node;
    uint32_t dma_channel;
    uint8_t scan_order_table[DISPLAY_BLOCKS];
};

static FrontDisplayScan* led_scan;

static void scan_dma_tc_irq(void* context);

static void gclk_tim_init(void) {
    furi_hal_bus_enable(FuriHalBusTIM8);

    LL_TIM_SetCounterMode(TIM8, LL_TIM_COUNTERMODE_UP);
    LL_TIM_SetClockDivision(TIM8, LL_TIM_CLOCKDIVISION_DIV1);
    LL_TIM_SetPrescaler(TIM8, GCLK_PRESCALER - 1);
    LL_TIM_SetAutoReload(TIM8, 1);
    LL_TIM_SetRepetitionCounter(TIM8, GCLK_COUNT - 1);

    LL_TIM_GenerateEvent_UPDATE(TIM8);

    LL_TIM_SetOnePulseMode(TIM8, LL_TIM_ONEPULSEMODE_SINGLE);
    LL_TIM_SetClockSource(TIM8, LL_TIM_CLOCKSOURCE_INTERNAL);
    LL_TIM_EnableARRPreload(TIM8);

    LL_TIM_SetTriggerInput(TIM8, LL_TIM_TS_ITR4);
    LL_TIM_SetSlaveMode(TIM8, LL_TIM_SLAVEMODE_TRIGGER);

    LL_TIM_CC_DisableChannel(TIM8, LL_TIM_CHANNEL_CH4);
    LL_TIM_CC_DisableChannel(TIM8, LL_TIM_CHANNEL_CH4N);

    LL_TIM_OC_SetMode(TIM8, LL_TIM_CHANNEL_CH4, LL_TIM_OCMODE_PWM1);
    LL_TIM_OC_SetPolarity(TIM8, LL_TIM_CHANNEL_CH4N, LL_TIM_OCPOLARITY_LOW);
    LL_TIM_OC_SetIdleState(TIM8, LL_TIM_CHANNEL_CH4N, LL_TIM_OCIDLESTATE_LOW);
    LL_TIM_OC_SetCompareCH4(TIM8, 1);

    LL_TIM_OC_EnablePreload(TIM8, LL_TIM_CHANNEL_CH4);

    LL_TIM_EnableAllOutputs(TIM8);
    LL_TIM_CC_EnableChannel(TIM8, LL_TIM_CHANNEL_CH4N);

    furi_hal_gpio_init_ex(
        &gpio_front_display_gclk,
        GpioModeAltFunctionPushPull,
        GpioPullNo,
        DISPLAY_GPIO_SPEED,
        GpioAltFn3TIM8);
}

static void scan_tim_init(void) {
    furi_hal_bus_enable(FuriHalBusTIM5);

    LL_TIM_SetCounterMode(TIM5, LL_TIM_COUNTERMODE_UP);
    LL_TIM_SetClockDivision(TIM5, LL_TIM_CLOCKDIVISION_DIV1);
    LL_TIM_SetPrescaler(TIM5, SCAN_PRESCALLER - 1);
    LL_TIM_SetAutoReload(TIM5, SCAN_PERIOD - 1);
    LL_TIM_SetRepetitionCounter(TIM5, 0);

    LL_TIM_GenerateEvent_UPDATE(TIM5);
    LL_TIM_EnableARRPreload(TIM5);

    LL_TIM_SetTriggerOutput(TIM5, LL_TIM_TRGO_UPDATE);

    // CC1 - VSYNC + Data load start
    LL_TIM_CC_DisableChannel(TIM5, LL_TIM_CHANNEL_CH1);
    LL_TIM_CC_DisableChannel(TIM5, LL_TIM_CHANNEL_CH1N);
    LL_TIM_OC_SetMode(TIM5, LL_TIM_CHANNEL_CH1, LL_TIM_OCMODE_TOGGLE);
    LL_TIM_OC_SetPolarity(TIM5, LL_TIM_CHANNEL_CH1, LL_TIM_OCPOLARITY_LOW);
    LL_TIM_OC_SetIdleState(TIM5, LL_TIM_CHANNEL_CH1, LL_TIM_OCIDLESTATE_LOW);
    LL_TIM_OC_SetCompareCH1(TIM5, SCAN_PERIOD - VSYNC_DELAY);

    // CC2 - Scan latch
    LL_TIM_CC_DisableChannel(TIM5, LL_TIM_CHANNEL_CH2);
    LL_TIM_CC_DisableChannel(TIM5, LL_TIM_CHANNEL_CH2N);
    LL_TIM_OC_SetMode(TIM5, LL_TIM_CHANNEL_CH2, LL_TIM_OCMODE_PWM1);
    LL_TIM_OC_SetPolarity(TIM5, LL_TIM_CHANNEL_CH2, LL_TIM_OCPOLARITY_LOW);
    LL_TIM_OC_SetIdleState(TIM5, LL_TIM_CHANNEL_CH2, LL_TIM_OCIDLESTATE_LOW);
    LL_TIM_OC_SetCompareCH2(TIM5, SCAN_PERIOD - LATCH_DELAY);

    LL_TIM_EnableAllOutputs(TIM5);
    LL_TIM_CC_EnableChannel(TIM5, LL_TIM_CHANNEL_CH1);
    LL_TIM_CC_EnableChannel(TIM5, LL_TIM_CHANNEL_CH2);

    LL_TIM_ClearFlag_CC1(TIM5);

    NVIC_SetPriority(TIM5_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0, 0));
    NVIC_EnableIRQ(TIM5_IRQn);

    furi_hal_gpio_init_ex(
        &gpio_front_display_scan_latch,
        GpioModeAltFunctionPushPull,
        GpioPullNo,
        DISPLAY_GPIO_SPEED,
        GpioAltFn2TIM5);
}

static void spi_dma_init(void) {
    furi_hal_dma_allocate_gpdma_channel(&led_scan->dma_channel);

    LL_DMA_InitNodeTypeDef dma_node_cfg = {0};

    dma_node_cfg.NodeType = LL_DMA_GPDMA_LINEAR_NODE;

    dma_node_cfg.BlkHWRequest = LL_DMA_HWREQUEST_SINGLEBURST;
    dma_node_cfg.Direction = LL_DMA_DIRECTION_MEMORY_TO_PERIPH;

    dma_node_cfg.DestAllocatedPort = LL_DMA_DEST_ALLOCATED_PORT0;
    dma_node_cfg.DestBurstLength = 1;
    dma_node_cfg.DestIncMode = LL_DMA_DEST_FIXED;
    dma_node_cfg.DestDataWidth = LL_DMA_DEST_DATAWIDTH_BYTE;

    dma_node_cfg.SrcAllocatedPort = LL_DMA_SRC_ALLOCATED_PORT1;
    dma_node_cfg.SrcBurstLength = 1;
    dma_node_cfg.SrcIncMode = LL_DMA_SRC_INCREMENT;
    dma_node_cfg.SrcDataWidth = LL_DMA_SRC_DATAWIDTH_BYTE;

    dma_node_cfg.TransferEventMode = LL_DMA_TCEM_BLK_TRANSFER;

    dma_node_cfg.DataAlignment = LL_DMA_DATA_ALIGN_ZEROPADD;
    dma_node_cfg.DestHWordExchange = LL_DMA_DEST_HALFWORD_PRESERVE;
    dma_node_cfg.DestByteExchange = LL_DMA_DEST_BYTE_PRESERVE;
    dma_node_cfg.SrcByteExchange = LL_DMA_SRC_BYTE_PRESERVE;

    dma_node_cfg.TriggerSelection = LL_GPDMA1_TRIGGER_TIM5_TRGO;
    dma_node_cfg.TriggerMode = LL_DMA_TRIGM_SINGLBURST_TRANSFER;
    dma_node_cfg.TriggerPolarity = LL_DMA_TRIG_POLARITY_RISING;

    dma_node_cfg.BlkRptDestAddrUpdateMode = LL_DMA_BLKRPT_DEST_ADDR_INCREMENT;
    dma_node_cfg.BlkRptSrcAddrUpdateMode = LL_DMA_BLKRPT_SRC_ADDR_INCREMENT;
    dma_node_cfg.DestAddrUpdateMode = LL_DMA_BURST_DEST_ADDR_INCREMENT;
    dma_node_cfg.SrcAddrUpdateMode = LL_DMA_BURST_SRC_ADDR_INCREMENT;
    dma_node_cfg.BlkRptCount = 0;
    dma_node_cfg.DestAddrOffset = 0;
    dma_node_cfg.SrcAddrOffset = 0;
    dma_node_cfg.BlkRptDestAddrOffset = 0;
    dma_node_cfg.BlkRptSrcAddrOffset = 0;

    dma_node_cfg.UpdateRegisters =
        (LL_DMA_UPDATE_CTR1 | LL_DMA_UPDATE_CTR2 | LL_DMA_UPDATE_CBR1 | LL_DMA_UPDATE_CSAR |
         LL_DMA_UPDATE_CDAR | LL_DMA_UPDATE_CLLR);

    dma_node_cfg.SrcAddress = (uint32_t)led_scan->scan_order_table;
    dma_node_cfg.DestAddress = LL_SPI_DMA_GetTxRegAddr(SPI2);
    dma_node_cfg.BlkDataLength = sizeof(led_scan->scan_order_table);

    dma_node_cfg.Request = LL_GPDMA1_REQUEST_SPI2_TX;

    LL_DMA_CreateLinkNode(&dma_node_cfg, &led_scan->dma_link_node);

    LL_DMA_ConnectLinkNode(
        &led_scan->dma_link_node,
        LL_DMA_CLLR_OFFSET5,
        &led_scan->dma_link_node,
        LL_DMA_CLLR_OFFSET5);

    LL_DMA_InitLinkedListTypeDef dma_ll_cfg = {0};
    dma_ll_cfg.Priority = LL_DMA_HIGH_PRIORITY;
    dma_ll_cfg.TransferEventMode = LL_DMA_TCEM_LAST_LLITEM_TRANSFER;
    dma_ll_cfg.LinkStepMode = LL_DMA_LSM_FULL_EXECUTION;
    dma_ll_cfg.LinkAllocatedPort = LL_DMA_LINK_ALLOCATED_PORT0;

    LL_DMA_List_Init(GPDMA1, led_scan->dma_channel, &dma_ll_cfg);

    LL_DMA_SetLinkedListBaseAddr(
        GPDMA1, led_scan->dma_channel, (uint32_t)&led_scan->dma_link_node);
    LL_DMA_ConfigLinkUpdate(
        GPDMA1,
        led_scan->dma_channel,
        (LL_DMA_UPDATE_CTR1 | LL_DMA_UPDATE_CTR2 | LL_DMA_UPDATE_CBR1 | LL_DMA_UPDATE_CSAR |
         LL_DMA_UPDATE_CDAR | LL_DMA_UPDATE_CTR3 | LL_DMA_UPDATE_CBR2 | LL_DMA_UPDATE_CLLR),
        (uint32_t)&led_scan->dma_link_node);

    furi_hal_interrupt_set_isr_ex(
        furi_hal_dma_get_gpdma_interrupt_id(led_scan->dma_channel),
        FuriHalInterruptPriorityKamiSama,
        scan_dma_tc_irq,
        NULL);

    LL_DMA_EnableChannel(GPDMA1, led_scan->dma_channel);
}

static void spi_595_init(void) {
    furi_hal_bus_enable(FuriHalBusSPI2);
    LL_RCC_SetSPIClockSource(LL_RCC_SPI2_CLKSOURCE_SYSCLK);

    LL_SPI_Disable(SPI2);
    LL_SPI_SetBaudRatePrescaler(SPI2, LL_SPI_BAUDRATEPRESCALER_DIV8);
    LL_SPI_SetDataWidth(SPI2, LL_SPI_DATAWIDTH_8BIT);
    LL_SPI_SetInternalSSLevel(SPI2, LL_SPI_SS_LEVEL_HIGH);
    LL_SPI_DisableCRC(SPI2);
    LL_SPI_SetClockPolarity(SPI2, LL_SPI_POLARITY_LOW);
    LL_SPI_SetClockPhase(SPI2, LL_SPI_PHASE_1EDGE);
    LL_SPI_SetTransferBitOrder(SPI2, LL_SPI_MSB_FIRST);
    LL_SPI_SetTransferDirection(SPI2, LL_SPI_SIMPLEX_TX);
    LL_SPI_SetNSSMode(SPI2, LL_SPI_NSS_SOFT);
    LL_SPI_SetMode(SPI2, LL_SPI_MODE_MASTER);

    LL_SPI_DisableNSSPulseMgt(SPI2);
    LL_SPI_EnableDMAReq_TX(SPI2);

    LL_SPI_Enable(SPI2);
    LL_SPI_StartMasterTransfer(SPI2);

    furi_hal_gpio_init_ex(
        &gpio_front_display_scan_clk,
        GpioModeAltFunctionPushPull,
        GpioPullNo,
        DISPLAY_GPIO_SPEED,
        GpioAltFn5SPI2);
    furi_hal_gpio_init_ex(
        &gpio_front_display_scan_sdi,
        GpioModeAltFunctionPushPull,
        GpioPullNo,
        DISPLAY_GPIO_SPEED,
        GpioAltFn3SPI2);

    LL_SPI_TransmitData8(SPI2, SCAN_DISABLED);
}

static void scan_dma_tc_irq(void* context) {
    UNUSED(context);
    if((LL_DMA_IsEnabledIT_TC(GPDMA1, led_scan->dma_channel)) &&
       (LL_DMA_IsActiveFlag_TC(GPDMA1, led_scan->dma_channel))) {
        LL_DMA_ClearFlag_TC(GPDMA1, led_scan->dma_channel);
        LL_DMA_DisableIT_TC(GPDMA1, led_scan->dma_channel);
        LL_TIM_ClearFlag_CC1(TIM5);
        LL_TIM_EnableIT_CC1(TIM5);
    }
}

void TIM5_IRQHandler(void) {
    if((LL_TIM_IsEnabledIT_CC1(TIM5)) && (LL_TIM_IsActiveFlag_CC1(TIM5))) {
        front_display_driver_vsync_trig();
        front_display_driver_send_buf_start();
        LL_TIM_DisableIT_CC1(TIM5);
    }
}

inline void front_display_scan_data_sync_enable(void) {
    LL_DMA_ClearFlag_TC(GPDMA1, led_scan->dma_channel);
    LL_DMA_EnableIT_TC(GPDMA1, led_scan->dma_channel);
}

void front_display_scan_init(void) {
    led_scan = malloc(sizeof(FrontDisplayScan));
    memset(led_scan->scan_order_table, SCAN_DISABLED, DISPLAY_BLOCKS);

    scan_tim_init();
    gclk_tim_init();
    spi_595_init();
    spi_dma_init();
}

void front_display_scan_start(void) {
    LL_TIM_EnableCounter(TIM5);
}

void front_display_scan_output_enable(bool enable) {
    if(enable) {
        memcpy(led_scan->scan_order_table, display_scan_table, DISPLAY_BLOCKS);
    } else {
        memset(led_scan->scan_order_table, SCAN_DISABLED, DISPLAY_BLOCKS);
    }
}
