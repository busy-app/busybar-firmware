#include <furi.h>
#include <furi_hal_dma.h>
#include <furi_hal_bus.h>
#include <furi_hal_interrupt.h>
#include "stm32u5xx_ll_dma.h"

#define FURI_HAL_GPDMA_CHANNEL_COUNT    16
#define FURI_HAL_GPDMA_2D_CHANNEL_COUNT 4
#define FURI_HAL_LPDMA_CHANNEL_COUNT    4

static uint32_t furi_hal_dma_invalid_argument_crash() {
    furi_crash("Invalid argument");
    return 0;
}

#define FURI_HAL_GPDMA_INTERRUPT_ID(index, prefix) \
    (((index) == (0))  ? prefix##0 :               \
     ((index) == (1))  ? prefix##1 :               \
     ((index) == (2))  ? prefix##2 :               \
     ((index) == (3))  ? prefix##3 :               \
     ((index) == (4))  ? prefix##4 :               \
     ((index) == (5))  ? prefix##5 :               \
     ((index) == (6))  ? prefix##6 :               \
     ((index) == (7))  ? prefix##7 :               \
     ((index) == (8))  ? prefix##8 :               \
     ((index) == (9))  ? prefix##9 :               \
     ((index) == (10)) ? prefix##10 :              \
     ((index) == (11)) ? prefix##11 :              \
     ((index) == (12)) ? prefix##12 :              \
     ((index) == (13)) ? prefix##13 :              \
     ((index) == (14)) ? prefix##14 :              \
     ((index) == (15)) ? prefix##15 :              \
                         furi_hal_dma_invalid_argument_crash())

#define FURI_HAL_LPDMA_INTERRUPT_ID(index, prefix) \
    (((index) == (0)) ? prefix##0 :                \
     ((index) == (1)) ? prefix##1 :                \
     ((index) == (2)) ? prefix##2 :                \
     ((index) == (3)) ? prefix##3 :                \
                        furi_hal_dma_invalid_argument_crash())

typedef struct {
    bool allocated;
    uint32_t interrupt_id;
} FuriHalGpdmaChannel;

typedef struct {
    bool allocated;
    uint32_t interrupt_id;
} FuriHalLpdmaChannel;

static FuriHalGpdmaChannel furi_hal_gpdma_channel[FURI_HAL_GPDMA_CHANNEL_COUNT] = {0};
static FuriHalLpdmaChannel furi_hal_lpdma_channel[FURI_HAL_LPDMA_CHANNEL_COUNT] = {0};

void furi_hal_dma_init_early(void) {
    furi_hal_bus_enable(FuriHalBusGPDMA1);
    furi_hal_bus_enable(FuriHalBusLPDMA1);
    //furi_hal_bus_enable(FuriHalBusDMA2D);
}

void furi_hal_dma_deinit_early(void) {
    furi_hal_bus_disable(FuriHalBusGPDMA1);
    furi_hal_bus_disable(FuriHalBusLPDMA1);
    //furi_hal_bus_disable(FuriHalBusDMA2D);
}

bool furi_hal_dma_allocate_gpdma_channel(uint32_t* gpdma_channel) {
    for(int i = FURI_HAL_GPDMA_CHANNEL_COUNT - 1 - FURI_HAL_GPDMA_2D_CHANNEL_COUNT; i >= 0; i--) {
        if(!furi_hal_gpdma_channel[i].allocated) {
            furi_hal_gpdma_channel[i].allocated = true;
            furi_hal_gpdma_channel[i].interrupt_id =
                FURI_HAL_GPDMA_INTERRUPT_ID(i, FuriHalInterruptIdGPDMA1Channel);
            furi_hal_interrupt_set_isr(furi_hal_gpdma_channel[i].interrupt_id, NULL, NULL);
            LL_DMA_DeInit(GPDMA1, i);
            *gpdma_channel = i;
            return true;
        }
    }

    return false;
}

bool furi_hal_dma_allocate_gpdma_2d_channel(uint32_t* gpdma_channel) {
    for(int i = FURI_HAL_GPDMA_CHANNEL_COUNT - 1;
        i >= FURI_HAL_GPDMA_CHANNEL_COUNT - FURI_HAL_GPDMA_2D_CHANNEL_COUNT;
        i--) {
        if(!furi_hal_gpdma_channel[i].allocated) {
            furi_hal_gpdma_channel[i].allocated = true;
            furi_hal_gpdma_channel[i].interrupt_id =
                FURI_HAL_GPDMA_INTERRUPT_ID(i, FuriHalInterruptIdGPDMA1Channel);
            furi_hal_interrupt_set_isr(furi_hal_gpdma_channel[i].interrupt_id, NULL, NULL);
            LL_DMA_DeInit(GPDMA1, i);
            *gpdma_channel = i;
            return true;
        }
    }

    return false;
}

bool furi_hal_dma_allocate_lpdma_channel(uint32_t* lpdma_channel) {
    for(int i = FURI_HAL_LPDMA_CHANNEL_COUNT - 1; i >= 0; i--) {
        if(!furi_hal_lpdma_channel[i].allocated) {
            furi_hal_lpdma_channel[i].allocated = true;
            furi_hal_lpdma_channel[i].interrupt_id =
                FURI_HAL_LPDMA_INTERRUPT_ID(i, FuriHalInterruptIdLPDMA1Channel);
            furi_hal_interrupt_set_isr(furi_hal_lpdma_channel[i].interrupt_id, NULL, NULL);
            LL_DMA_DeInit(LPDMA1, i);
            *lpdma_channel = i;
            return true;
        }
    }

    return false;
}

void furi_hal_dma_free_gpdma_channel(uint32_t gpdma_channel) {
    furi_hal_gpdma_channel[gpdma_channel].allocated = false;
    LL_DMA_DeInit(GPDMA1, gpdma_channel);
    furi_hal_interrupt_set_isr(furi_hal_gpdma_channel[gpdma_channel].interrupt_id, NULL, NULL);
}

void furi_hal_dma_free_lpdma_channel(uint32_t lpdma_channel) {
    furi_hal_lpdma_channel[lpdma_channel].allocated = false;
    LL_DMA_DeInit(LPDMA1, lpdma_channel);
    furi_hal_interrupt_set_isr(furi_hal_lpdma_channel[lpdma_channel].interrupt_id, NULL, NULL);
}

uint32_t furi_hal_dma_get_gpdma_interrupt_id(uint32_t gpdma_channel) {
    furi_check(gpdma_channel < FURI_HAL_GPDMA_CHANNEL_COUNT, "Invalid GPDMA channel");
    furi_check(furi_hal_gpdma_channel[gpdma_channel].allocated, "GPDMA channel is not allocated");
    return furi_hal_gpdma_channel[gpdma_channel].interrupt_id;
}

uint32_t furi_hal_dma_get_lpdma_interrupt_id(uint32_t lpdma_channel) {
    furi_check(lpdma_channel < FURI_HAL_LPDMA_CHANNEL_COUNT, "Invalid LPDMA channel");
    furi_check(furi_hal_lpdma_channel[lpdma_channel].allocated, "LPDMA channel is not allocated");
    return furi_hal_lpdma_channel[lpdma_channel].interrupt_id;
}
