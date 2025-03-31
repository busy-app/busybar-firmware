#include <furi_hal_spi_config.h>
#include <furi_hal_resources.h>
#include <furi_hal_spi.h>
#include <furi_hal_bus.h>
#include <stm32u5xx_ll_spi.h>
#include <furi.h>

#define TAG "FuriHalSpiConfig"

#define LL_SPI_CRCCALCULATION_DISABLE (0x00000000UL)
#define LL_SPI_CRCCALCULATION_ENABLE  (SPI_CFG1_CRCEN)

/* SPI Presets */

const FuriHalSpiBusCfg furi_hal_spi_preset_oled = {
    .mode = LL_SPI_MODE_MASTER,
    .transfer_dir = LL_SPI_SIMPLEX_TX,
    .data_width = LL_SPI_DATAWIDTH_8BIT,
    .clk_polarity = LL_SPI_POLARITY_HIGH,
    .clk_phase = LL_SPI_PHASE_2EDGE,
    .nss_mode = LL_SPI_NSS_SOFT,
    .baud_prescaller = LL_SPI_BAUDRATEPRESCALER_DIV2,
    .bit_order = LL_SPI_MSB_FIRST,
    .crc_mode = LL_SPI_CRCCALCULATION_DISABLE,
    .crc_poly = 7,
};

/* SPI Buses */

void furi_hal_spi_bus_set_config(SPI_TypeDef* spi, const FuriHalSpiBusCfg* bus_cfg) {
    if(LL_SPI_IsEnabled(spi) == 0) {
        MODIFY_REG(
            spi->CFG1,
            SPI_CFG1_BPASS | SPI_CFG1_MBR | SPI_CFG1_CRCEN | SPI_CFG1_DSIZE,
            bus_cfg->baud_prescaller | bus_cfg->crc_mode | bus_cfg->data_width);

        uint32_t tmp_mode = bus_cfg->mode;
        uint32_t tmp_nss_polarity = LL_SPI_GetNSSPolarity(spi);

        if((bus_cfg->nss_mode == LL_SPI_NSS_SOFT) &&
           (((tmp_nss_polarity == LL_SPI_NSS_POLARITY_LOW) && (tmp_mode == LL_SPI_MODE_MASTER)) ||
            ((tmp_nss_polarity == LL_SPI_NSS_POLARITY_HIGH) && (tmp_mode == LL_SPI_MODE_SLAVE)))) {
            LL_SPI_SetInternalSSLevel(spi, LL_SPI_SS_LEVEL_HIGH);
        }

        MODIFY_REG(
            spi->CFG2,
            SPI_CFG2_SSM | SPI_CFG2_SSOE | SPI_CFG2_CPOL | SPI_CFG2_CPHA | SPI_CFG2_LSBFRST |
                SPI_CFG2_MASTER | SPI_CFG2_COMM,
            bus_cfg->nss_mode | bus_cfg->clk_polarity | bus_cfg->clk_phase | bus_cfg->bit_order |
                bus_cfg->mode | (bus_cfg->transfer_dir & SPI_CFG2_COMM));

        MODIFY_REG(spi->CR1, SPI_CR1_HDDIR, bus_cfg->transfer_dir & SPI_CR1_HDDIR);

        if(bus_cfg->crc_mode == LL_SPI_CRCCALCULATION_ENABLE) {
            LL_SPI_SetCRCPolynomial(spi, bus_cfg->crc_poly);
        }
    }
}

void furi_hal_spi_config_init_early(void) {
    furi_hal_spi_bus_init(&furi_hal_spi_bus_1);
    furi_hal_spi_bus_handle_init(&furi_hal_spi_bus_handle_oled);
}

void furi_hal_spi_config_deinit_early(void) {
    furi_hal_spi_bus_handle_deinit(&furi_hal_spi_bus_handle_oled);
    furi_hal_spi_bus_deinit(&furi_hal_spi_bus_1);
}

void furi_hal_spi_config_init(void) {
    FURI_LOG_I(TAG, "Init OK");
}

FuriMutex* furi_hal_spi_bus_1_mutex = NULL;

static void furi_hal_spi_bus_1_event_callback(FuriHalSpiBus* bus, FuriHalSpiBusEvent event) {
    if(event == FuriHalSpiBusEventInit) {
        furi_hal_spi_bus_1_mutex = furi_mutex_alloc(FuriMutexTypeNormal);
        bus->current_handle = NULL;
    } else if(event == FuriHalSpiBusEventDeinit) {
        furi_mutex_free(furi_hal_spi_bus_1_mutex);
    } else if(event == FuriHalSpiBusEventLock) {
        furi_check(furi_mutex_acquire(furi_hal_spi_bus_1_mutex, FuriWaitForever) == FuriStatusOk);
    } else if(event == FuriHalSpiBusEventUnlock) {
        furi_check(furi_mutex_release(furi_hal_spi_bus_1_mutex) == FuriStatusOk);
    } else if(event == FuriHalSpiBusEventActivate) {
        furi_hal_bus_enable(FuriHalBusSPI1);
    } else if(event == FuriHalSpiBusEventDeactivate) {
        furi_hal_bus_disable(FuriHalBusSPI1);
    }
}

FuriHalSpiBus furi_hal_spi_bus_1 = {
    .spi = SPI1,
    .callback = furi_hal_spi_bus_1_event_callback,
};

/* SPI Bus Handles */

inline static void furi_hal_spi_bus_1_handle_event_callback(
    FuriHalSpiBusHandle* handle,
    FuriHalSpiBusHandleEvent event,
    const FuriHalSpiBusCfg* preset) {
    if(event == FuriHalSpiBusHandleEventInit) {
        furi_hal_gpio_write(handle->cs, true);
        furi_hal_gpio_init(handle->cs, GpioModeOutputPushPull, GpioPullUp, GpioSpeedMedium);

        furi_hal_gpio_init_ex(
            handle->mosi, GpioModeAltFunctionPushPull, GpioPullNo, GpioSpeedMedium, GpioAltFn5SPI1);
        furi_hal_gpio_init_ex(
            handle->sck, GpioModeAltFunctionPushPull, GpioPullNo, GpioSpeedMedium, GpioAltFn5SPI1);

    } else if(event == FuriHalSpiBusHandleEventDeinit) {
        furi_hal_gpio_write(handle->cs, true);
        furi_hal_gpio_init(handle->cs, GpioModeAnalog, GpioPullUp, GpioSpeedLow);
    } else if(event == FuriHalSpiBusHandleEventActivate) {
        furi_hal_spi_bus_set_config(handle->bus->spi, preset);
        LL_SPI_Enable(handle->bus->spi);
        if(preset->mode == LL_SPI_MODE_MASTER) {
            LL_SPI_StartMasterTransfer(handle->bus->spi);
        }
        furi_hal_gpio_write(handle->cs, false);
    } else if(event == FuriHalSpiBusHandleEventDeactivate) {
        furi_hal_gpio_write(handle->cs, true);
        LL_SPI_Disable(handle->bus->spi);
    }
}

static void furi_hal_spi_bus_handle_oled_event_callback(
    FuriHalSpiBusHandle* handle,
    FuriHalSpiBusHandleEvent event) {
    furi_hal_spi_bus_1_handle_event_callback(handle, event, &furi_hal_spi_preset_oled);
}

FuriHalSpiBusHandle furi_hal_spi_bus_handle_oled = {
    .bus = &furi_hal_spi_bus_1,
    .callback = furi_hal_spi_bus_handle_oled_event_callback,
    .miso = NULL,
    .mosi = &gpio_oled_spi_sdin,
    .sck = &gpio_oled_spi_sclk,
    .cs = &gpio_oled_cs,
};
