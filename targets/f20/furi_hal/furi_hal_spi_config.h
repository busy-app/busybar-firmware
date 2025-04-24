#pragma once

#include <furi_hal_spi_types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint32_t mode;
    uint32_t transfer_dir;
    uint32_t data_width;
    uint32_t clk_polarity;
    uint32_t clk_phase;
    uint32_t nss_mode;
    uint32_t baud_prescaller;
    uint32_t bit_order;
    uint32_t crc_mode;
    uint32_t crc_poly;
} FuriHalSpiBusCfg;

/** Furi Hal Spi Bus 1 (OLED) */
extern FuriHalSpiBus furi_hal_spi_bus_1;

/** OLED on `furi_hal_spi_bus_1` */
extern FuriHalSpiBusHandle furi_hal_spi_bus_handle_back_display;

#ifdef __cplusplus
}
#endif
