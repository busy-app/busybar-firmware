#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/** Early initialization */
void furi_hal_dma_init_early(void);

/** Early de-initialization */
void furi_hal_dma_deinit_early(void);

/** Allocate GPDMA channel */
bool furi_hal_dma_allocate_gpdma_channel(uint32_t* gpdma_channel);

/** Allocate GPDMA 2D channel */
bool furi_hal_dma_allocate_gpdma_2d_channel(uint32_t* gpdma_channel);

/** Allocate LPDMA channel */
bool furi_hal_dma_allocate_lpdma_channel(uint32_t* lpdma_channel);

/** Free GPDMA channel */
void furi_hal_dma_free_gpdma_channel(uint32_t gpdma_channel);

/** Free LPDMA channel */
void furi_hal_dma_free_lpdma_channel(uint32_t lpdma_channel);

/** Get GPDMA interrupt ID */
uint32_t furi_hal_dma_get_gpdma_interrupt_id(uint32_t gpdma_channel);

/** Get LPDMA interrupt ID */
uint32_t furi_hal_dma_get_lpdma_interrupt_id(uint32_t lpdma_channel);

#ifdef __cplusplus
}
#endif
