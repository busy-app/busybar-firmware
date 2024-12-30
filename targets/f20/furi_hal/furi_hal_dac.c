#include "furi_hal_bus.h"
#include "furi_hal_interrupt.h"
#include <furi_hal_dac.h>
#include <furi_hal_resources.h>
#include <furi_hal_gpio.h>
#include <furi_hal_dma.h>

#include <stm32u5xx_ll_dac.h>
#include <stm32u5xx_ll_rcc.h>
#include <stm32u5xx_ll_lptim.h>
#include <stm32u5xx_ll_dma.h>

static inline void* memset16(void* m, uint16_t val, size_t count) {
    uint16_t* buf = m;
    while(count--)
        *buf++ = val;
    return m;
}

#define FURI_HAL_DAC_TIMER              LPTIM3
#define FURI_HAL_DAC_TIMER_BUS          FuriHalBusLPTIM3
#define FURI_HAL_DAC_TIMER_CLOCK_SOURCE LL_RCC_LPTIM34_CLKSOURCE_MSIK

#define FURI_HAL_DAC_DMA             GPDMA1
#define FURI_HAL_DAC_DMA_REQUEST     LL_GPDMA1_REQUEST_DAC1_CH1
#define FURI_HAL_DAC_DMA_PRIORITY    LL_DMA_HIGH_PRIORITY
#define FURI_HAL_DAC_DMA_BUFFER_SIZE (256u)

typedef struct {
    FuriHalDacCallback callback;
    void* callback_context;

    uint8_t data[FURI_HAL_DAC_DMA_BUFFER_SIZE];

    uint32_t dma_channel;
    uint32_t data_ptr;
    uint32_t data_size;
} FuriHalDac;

static FuriHalDac furi_hal_dac = {};

static inline void furi_hal_dac_refill(uint8_t* buffer, size_t buffer_size) {
    size_t ret = furi_hal_dac.callback(buffer, buffer_size, furi_hal_dac.callback_context);
    if(ret < buffer_size) {
        memset16(buffer + ret, 0xfff / 2, (buffer_size - ret) / 2);
    }
}

static void furi_hal_dac_isr(void*) {
    if(LL_DMA_IsActiveFlag_TC(GPDMA1, furi_hal_dac.dma_channel)) {
        LL_DMA_ClearFlag_TC(GPDMA1, furi_hal_dac.dma_channel);
        furi_hal_dac_refill(
            furi_hal_dac.data + FURI_HAL_DAC_DMA_BUFFER_SIZE / 2,
            FURI_HAL_DAC_DMA_BUFFER_SIZE / 2);
    }
    if(LL_DMA_IsActiveFlag_HT(GPDMA1, furi_hal_dac.dma_channel)) {
        LL_DMA_ClearFlag_HT(GPDMA1, furi_hal_dac.dma_channel);
        furi_hal_dac_refill(furi_hal_dac.data, FURI_HAL_DAC_DMA_BUFFER_SIZE / 2);
    }
}

static void furi_hal_dac_setup_dma() {
    furi_check(furi_hal_dma_allocate_gpdma_channel(&furi_hal_dac.dma_channel));

    LL_DMA_InitTypeDef dma_init_strust = {0};

    furi_hal_dac.data_ptr = (uint32_t)furi_hal_dac.data;
    furi_hal_dac.data_size = sizeof(furi_hal_dac.data);
    memset16(furi_hal_dac.data, 0xfff / 2, furi_hal_dac.data_size / 2);

    dma_init_strust.SrcAddress = furi_hal_dac.data_ptr;
    dma_init_strust.DestAddress =
        LL_DAC_DMA_GetRegAddr(DAC1, LL_DAC_CHANNEL_1, LL_DAC_DMA_REG_DATA_12BITS_RIGHT_ALIGNED);
    dma_init_strust.BlkDataLength = furi_hal_dac.data_size;

    dma_init_strust.Request = FURI_HAL_DAC_DMA_REQUEST;
    dma_init_strust.Direction = LL_DMA_DIRECTION_MEMORY_TO_PERIPH;

    dma_init_strust.SrcAllocatedPort = LL_DMA_SRC_ALLOCATED_PORT1;
    dma_init_strust.SrcBurstLength = 8;
    dma_init_strust.SrcIncMode = LL_DMA_SRC_INCREMENT;
    dma_init_strust.SrcDataWidth = LL_DMA_SRC_DATAWIDTH_WORD;

    dma_init_strust.DestAllocatedPort = LL_DMA_DEST_ALLOCATED_PORT0;
    dma_init_strust.DestBurstLength = 1;
    dma_init_strust.DestIncMode = LL_DMA_DEST_FIXED;
    dma_init_strust.DestDataWidth = LL_DMA_DEST_DATAWIDTH_WORD;

    dma_init_strust.Priority = FURI_HAL_DAC_DMA_PRIORITY;
    dma_init_strust.LinkAllocatedPort = LL_DMA_LINK_ALLOCATED_PORT1;
    dma_init_strust.LinkedListBaseAddr = (uint32_t)&furi_hal_dac.data_ptr;
    dma_init_strust.LinkedListAddrOffset = (uint32_t)&furi_hal_dac.data_ptr;

    LL_DMA_Init(GPDMA1, furi_hal_dac.dma_channel, &dma_init_strust);
    LL_DMA_EnableCSARUpdate(GPDMA1, furi_hal_dac.dma_channel);

    LL_DMA_EnableIT_TC(GPDMA1, furi_hal_dac.dma_channel);
    LL_DMA_EnableIT_HT(GPDMA1, furi_hal_dac.dma_channel);

    furi_hal_interrupt_set_isr(
        furi_hal_dma_get_gpdma_interrupt_id(furi_hal_dac.dma_channel), furi_hal_dac_isr, NULL);

    //Start DMA Channel
    LL_DMA_EnableChannel(GPDMA1, furi_hal_dac.dma_channel);
}

static void furi_hal_dac_setup_dac() {
    LL_RCC_SetADCDACClockSource(LL_RCC_ADCDAC_CLKSOURCE_HSE); // kernel
    LL_RCC_SetDAC1ClockSource(LL_RCC_DAC1_CLKSOURCE_LSE); // sample and hold

    furi_hal_bus_enable(FuriHalBusDAC1);

    LL_DAC_DisableAutonomousMode(DAC1);
    LL_DAC_SetHighFrequencyMode(DAC1, LL_DAC_HIGH_FREQ_MODE_DISABLE);
    LL_DAC_SetSignedFormat(DAC1, LL_DAC_CHANNEL_1, LL_DAC_SIGNED_FORMAT_DISABLE);
    LL_DAC_SetWaveAutoGeneration(DAC1, LL_DAC_CHANNEL_1, LL_DAC_WAVE_AUTO_GENERATION_NONE);
    LL_DAC_SetOutputConnection(DAC1, LL_DAC_CHANNEL_1, LL_DAC_OUTPUT_CONNECT_GPIO);
    LL_DAC_SetOutputMode(DAC1, LL_DAC_CHANNEL_1, LL_DAC_OUTPUT_MODE_NORMAL);
    LL_DAC_SetMode(DAC1, LL_DAC_CHANNEL_1, LL_DAC_MODE_NORMAL_OPERATION);
    LL_DAC_SetOutputBuffer(DAC1, LL_DAC_CHANNEL_1, LL_DAC_OUTPUT_BUFFER_DISABLE);

    LL_DAC_SetTriggerSource(DAC1, LL_DAC_CHANNEL_1, LL_DAC_TRIG_EXT_LPTIM3_CH1);
    LL_DAC_EnableTrigger(DAC1, LL_DAC_CHANNEL_1);

    LL_DAC_EnableDMAReq(DAC1, LL_DAC_CHANNEL_1);

    LL_DAC_EnableDMADoubleDataMode(DAC1, LL_DAC_CHANNEL_1);

    LL_DAC_Enable(DAC1, LL_DAC_CHANNEL_1);
    furi_delay_us(15);
}

static void furi_hal_dac_setup_timer() {
    // furi_hal_gpio_init_ex(
    //     &gpio_led_le_ospi_d1,
    //     GpioModeAltFunctionPushPull,
    //     GpioPullNo,
    //     GpioSpeedVeryHigh,
    //     GpioAltFn4LPTIM3);

    LL_RCC_SetLPTIMClockSource(FURI_HAL_DAC_TIMER_CLOCK_SOURCE);
    furi_hal_bus_enable(FURI_HAL_DAC_TIMER_BUS);

    LL_LPTIM_SetClockSource(FURI_HAL_DAC_TIMER, LL_LPTIM_CLK_SOURCE_INTERNAL);
    LL_LPTIM_SetPrescaler(FURI_HAL_DAC_TIMER, LL_LPTIM_PRESCALER_DIV1);

    LL_LPTIM_OC_SetCompareCH1(FURI_HAL_DAC_TIMER, 0);
    LL_LPTIM_OC_SetPolarity(
        FURI_HAL_DAC_TIMER, LL_LPTIM_CHANNEL_CH1, LL_LPTIM_OUTPUT_POLARITY_INVERSE);
    LL_LPTIM_SetCounterMode(FURI_HAL_DAC_TIMER, LL_LPTIM_COUNTER_MODE_INTERNAL);
    LL_LPTIM_CC_SetChannelMode(
        FURI_HAL_DAC_TIMER, LL_LPTIM_CHANNEL_CH1, LL_LPTIM_CCMODE_OUTPUT_PWM);

    LL_LPTIM_Enable(FURI_HAL_DAC_TIMER);
    furi_delay_us(1000000 / 133000 * 4);

    LL_LPTIM_CC_EnableChannel(FURI_HAL_DAC_TIMER, LL_LPTIM_CHANNEL_CH1);

    LL_LPTIM_SetAutoReload(FURI_HAL_DAC_TIMER, 0x7);
    LL_LPTIM_StartCounter(FURI_HAL_DAC_TIMER, LL_LPTIM_OPERATING_MODE_CONTINUOUS);
}

void furi_hal_dac_init(void) {
    // furi_hal_gpio_init(&gpio_audio_dac, GpioModeAnalog, GpioPullNo, GpioSpeedLow);
}

void furi_hal_dac_pa_enable(void) {
    // Nothing!
}

void furi_hal_dac_pa_disable(void) {
    // Nothing!
}

void furi_hal_dac_start(FuriHalDacCallback callback, void* callback_context, uint32_t samplerate) {
    UNUSED(samplerate);
    furi_check(callback);

    furi_hal_dac.callback = callback;
    furi_hal_dac.callback_context = callback_context;

    furi_hal_dac_setup_dma();
    furi_hal_dac_setup_dac();
    furi_hal_dac_setup_timer();
}

void furi_hal_dac_stop(void) {
    furi_hal_bus_disable(FuriHalBusLPTIM3);
    furi_hal_bus_disable(FuriHalBusDAC1);
    furi_hal_dma_free_gpdma_channel(furi_hal_dac.dma_channel);
}
