#include <furi_hal_memory.h>
#include <furi.h>

#include <stm32u5_linker.h>

void furi_hal_memory_init(void) {
    // TODO: implement
}

void* furi_hal_memory_alloc(size_t size) {
    UNUSED(size);
    // TODO: implement
    return NULL;
}

size_t furi_hal_memory_get_free(void) {
    // TODO: implement
    return 0;
}

size_t furi_hal_memory_max_pool_block(void) {
    // TODO: implement
    return 0;
}

static const FuriHalMemoryRegion memory_regions[] = {
    /* clang-format off */
    [FuriHalMemoryRegionIdHeap] = {
        .start = (void*)&__heap_start__,
        .size_bytes = (size_t)&__heap_size__,
    },
    /* clang-format on */
};

uint32_t furi_hal_memory_get_region_count(void) {
    return COUNT_OF(memory_regions);
}

const FuriHalMemoryRegion* furi_hal_memory_get_region(uint32_t index) {
    furi_check(index < COUNT_OF(memory_regions));
    return &memory_regions[index];
}

void furi_hal_memory_set_heap_track_mode(FuriHalMemoryHeapTrackMode mode) {
    UNUSED(mode);
    // TODO: implement
}

FuriHalMemoryHeapTrackMode furi_hal_memory_get_heap_track_mode(void) {
    // TODO: implement
    return FuriHalMemoryHeapTrackModeNone;
}
