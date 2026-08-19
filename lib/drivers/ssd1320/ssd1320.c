#include "ssd1320.h"

#include <furi.h>
#include <furi_hal_spi.h>
#include <furi_hal_resources.h>

#define TAG "SSD1320"

#define CMD_LEN_MAX (16)
#define CMD_DELAY   (0xFE)
#define CMD_END     (0xFF)

typedef enum {
    Cmd1320_MemoryMode = 0x20,
    Cmd1320_ColumnAddress = 0x21,
    Cmd1320_PageAddress = 0x22,
    Cmd1320_PortraitMode = 0x25,
    Cmd1320_ContrastControl = 0x81,
    Cmd1320_SegmentRemap0 = 0xA0,
    Cmd1320_SegmentRemap1 = 0xA1,
    Cmd1320_DisplayStartLine = 0xA2,
    Cmd1320_DisplayModeNormal = 0xA4,
    Cmd1320_DisplayModeAllOn = 0xA5,
    Cmd1320_DisplayModeAllOff = 0xA6,
    Cmd1320_DisplayModeInvert = 0xA7,
    Cmd1320_MultiplexRatio = 0xA8,
    Cmd1320_IREFSelect = 0xAD,
    Cmd1320_DisplayOff = 0xAE,
    Cmd1320_DisplayOn = 0xAF,
    Cmd1320_PrechargeVoltage = 0xBC,
    Cmd1320_GrayScaleTable = 0xBE,
    Cmd1320_GrayScaleTableLinear = 0xBF,
    Cmd1320_ComOutputScanDirNormal = 0xC0,
    Cmd1320_ComOutputScanDirRemap = 0xC8,
    Cmd1320_DisplayOffset = 0xD3,
    Cmd1320_DisplayClockDiv = 0xD5,
    Cmd1320_DisplayEnhancement = 0xD8,
    Cmd1320_PhaseLength = 0xD9,
    Cmd1320_SegPinsHardwareConfig = 0xDA,
    Cmd1320_VCOMHDeselectLevel = 0xDB,
    Cmd1320_CommandLock = 0xFD,
} Cmd1320;

static const uint8_t display_init_table_ssd1320[] = {
    /* clang-format off */
    0,  Cmd1320_DisplayOff,
    1,  Cmd1320_DisplayClockDiv, 0x22,
    1,  Cmd1320_MultiplexRatio, 0x4F,
    1,  Cmd1320_DisplayOffset, 0x78,
    1,  Cmd1320_DisplayStartLine, 0x00,
    0,  Cmd1320_SegmentRemap1,
    0,  Cmd1320_ComOutputScanDirNormal,
    1,  Cmd1320_SegPinsHardwareConfig, 0x32,
    1,  Cmd1320_ContrastControl, 0x27,
    1,  Cmd1320_PrechargeVoltage, 0x0C,
    1,  Cmd1320_PhaseLength, 0x62,
    1,  Cmd1320_VCOMHDeselectLevel, 0x30,
    1,  Cmd1320_IREFSelect, 0x10,
    3,  Cmd1320_DisplayEnhancement, 0xD5, 0xF0, 0x21,
    15, Cmd1320_GrayScaleTable, 0x02, 0x04, 0x07, 0x0A, 0x0E, 0x12, 0x16, 0x1B, 0x1F, 0x24, 0x29, 0x2F, 0x34, 0x3A, 0x3F,
    1,  Cmd1320_MemoryMode, 0x00,
    0,  Cmd1320_DisplayModeNormal,
    0,  Cmd1320_DisplayModeAllOff,
    CMD_END,
    /* clang-format on */
};

static void ssd1320_send_command(FuriHalSpiBusHandle* handle, uint8_t* command, size_t len) {
    furi_hal_gpio_write(&gpio_back_display_dc, false);

    furi_hal_spi_acquire(handle);
    furi_hal_spi_bus_tx(handle, command, len, 100);
    furi_hal_spi_release(handle);
}

static void ssd1320_clear(void) {
    furi_hal_spi_acquire(&furi_hal_spi_bus_handle_back_display);

    const uint8_t addr_cmd[6] = {0x21, 0, SSD1320_H - 1, 0x22, 0, SSD1320_W - 1};
    furi_hal_gpio_write(&gpio_back_display_dc, false);
    furi_hal_spi_bus_tx(&furi_hal_spi_bus_handle_back_display, addr_cmd, sizeof(addr_cmd), 100);

    furi_hal_gpio_write(&gpio_back_display_dc, true);

    uint8_t tx_buf[32] = {0};
    for(size_t i = 0; i < SSD1320_BUF_SIZE; i += sizeof(tx_buf)) {
        furi_hal_spi_bus_tx(&furi_hal_spi_bus_handle_back_display, tx_buf, sizeof(tx_buf), 100);
    }

    furi_hal_spi_release(&furi_hal_spi_bus_handle_back_display);
}

static void ssd1320_send_init_sequence(const uint8_t* init_table, size_t table_len) {
    size_t cmd_offset = 0;
    uint8_t cmd_buf[CMD_LEN_MAX];
    while(1) {
        furi_assert(cmd_offset < table_len);
        uint8_t len_byte = init_table[cmd_offset];
        if(len_byte == CMD_END) {
            break;
        } else if(len_byte == CMD_DELAY) {
            furi_delay_ms(init_table[cmd_offset + 1]);
            cmd_offset += 2;
        } else {
            furi_assert(len_byte < CMD_LEN_MAX);
            memcpy(cmd_buf, &init_table[cmd_offset + 1], len_byte + 1);
            ssd1320_send_command(&furi_hal_spi_bus_handle_back_display, cmd_buf, len_byte + 1);
            cmd_offset += len_byte + 2;
        }
    }
}

void ssd1320_sleep_mode(bool sleep) {
    furi_hal_gpio_write(&gpio_back_display_vcc_en, !sleep);
    uint8_t power_cmd = sleep ? Cmd1320_DisplayOff : Cmd1320_DisplayOn;
    ssd1320_send_command(&furi_hal_spi_bus_handle_back_display, &power_cmd, 1);
}

void ssd1320_set_contrast(uint8_t contrast) {
    uint8_t contrast_cmd[2] = {Cmd1320_ContrastControl, contrast};
    ssd1320_send_command(
        &furi_hal_spi_bus_handle_back_display, contrast_cmd, sizeof(contrast_cmd));
}

void ssd1320_set_grayscale_table(const SSD1320GrayscaleTable* grayscale_table) {
    uint8_t grayscale_table_cmd[sizeof(grayscale_table->data) + 1];

    grayscale_table_cmd[0] = Cmd1320_GrayScaleTable;
    memcpy(&grayscale_table_cmd[1], grayscale_table->data, sizeof(grayscale_table->data));

    ssd1320_send_command(
        &furi_hal_spi_bus_handle_back_display, grayscale_table_cmd, sizeof(grayscale_table_cmd));
}

void ssd1320_draw(const uint8_t* buf) {
    furi_hal_spi_acquire(&furi_hal_spi_bus_handle_back_display);

    const uint8_t addr_cmd[6] = {
        Cmd1320_ColumnAddress,
        0,
        SSD1320_H - 1,
        Cmd1320_PageAddress,
        0,
        SSD1320_W - 1,
    };
    furi_hal_gpio_write(&gpio_back_display_dc, false);
    furi_hal_spi_bus_tx(&furi_hal_spi_bus_handle_back_display, addr_cmd, sizeof(addr_cmd), 100);

    furi_hal_gpio_write(&gpio_back_display_dc, true);

    // TODO: DMA transfer
    furi_hal_spi_bus_tx(&furi_hal_spi_bus_handle_back_display, buf, SSD1320_BUF_SIZE, 100);

    furi_hal_spi_release(&furi_hal_spi_bus_handle_back_display);
}

void ssd1320_init(void) {
    furi_hal_gpio_init(
        &gpio_back_display_vcc_en, GpioModeOutputPushPull, GpioPullNo, GpioSpeedMedium);
    furi_hal_gpio_write(&gpio_back_display_vcc_en, false);

    furi_hal_gpio_write(&gpio_back_display_dc, true);
    furi_hal_gpio_init(&gpio_back_display_dc, GpioModeOutputPushPull, GpioPullUp, GpioSpeedMedium);
    furi_delay_ms(1);

    ssd1320_send_init_sequence(display_init_table_ssd1320, sizeof(display_init_table_ssd1320));
    ssd1320_clear();

    furi_hal_gpio_write(&gpio_back_display_vcc_en, true);
    furi_delay_ms(5);

    ssd1320_sleep_mode(false);
    furi_delay_ms(10);
}
