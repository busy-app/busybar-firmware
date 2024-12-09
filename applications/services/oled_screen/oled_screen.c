#include <furi.h>
#include <gui/gui.h>
#include <gui/gui_i.h>
#include "furi_hal_spi.h"
#include "furi_hal_resources.h"

#define TAG "OLED"

#define TURN_180 0

#define CMD_LEN_MAX 16

#define CMD_DELAY 0xFE
#define CMD_END   0xFF

static const uint8_t oled_init_table_ssd1320[] = {
    /* clang-format off */
    0, 0xAE, // Display off
    1, 0xD5, 0x22,
    1, 0xA8, 0x4F,
    1, 0xD3, 0x78,
    1, 0xA2, 0x00,
    0, 0xA1,
    0, 0xC0,
    1, 0xDA, 0x32,
    1, 0x81, 0x27,
    1, 0xBC, 0x10,
    1, 0xD9, 0x42,
    1, 0xDB, 0x30,
    1, 0xAD, 0x10,
    3, 0xD8, 0xD5, 0xF0, 0x21,
    15, 0xBE, // Gray scale table values
        0x00, 0x02, 0x04, 0x06, 0x09, 0x0D, 0x11, 0x15, 
        0x19, 0x1E, 0x24, 0x2A, 0x31, 0x38, 0x3F,
    1, 0x20, 0x00,
    0, 0xA4,
    0, 0xA6,
    CMD_END,
    /* clang-format on */
};

static void oled_send_command(FuriHalSpiBusHandle* handle, uint8_t* command, size_t len) {
    furi_hal_gpio_write(&gpio_oled_dc, false);

    furi_hal_spi_acquire(handle);
    furi_hal_spi_bus_tx(handle, command, len, 100);
    furi_hal_spi_release(handle);
}

static void oled_write_buf(const uint8_t* buf) {
    furi_hal_spi_acquire(&furi_hal_spi_bus_handle_oled);

    uint8_t addr_cmd[6] = {0x21, 0, 79, 0x22, 0, 159};
    furi_hal_gpio_write(&gpio_oled_dc, false);
    furi_hal_spi_bus_tx(&furi_hal_spi_bus_handle_oled, addr_cmd, sizeof(addr_cmd), 100);

    furi_hal_gpio_write(&gpio_oled_dc, true);
    furi_hal_spi_bus_tx(&furi_hal_spi_bus_handle_oled, buf, 160 * 80 / 2, 100);

    furi_hal_spi_release(&furi_hal_spi_bus_handle_oled);
}

static void oled_clear(void) {
    furi_hal_spi_acquire(&furi_hal_spi_bus_handle_oled);

    uint8_t addr_cmd[6] = {0x21, 0, 79, 0x22, 0, 159};
    furi_hal_gpio_write(&gpio_oled_dc, false);
    furi_hal_spi_bus_tx(&furi_hal_spi_bus_handle_oled, addr_cmd, sizeof(addr_cmd), 100);

    furi_hal_gpio_write(&gpio_oled_dc, true);

    uint8_t tx_buf[32] = {0};
    for(size_t i = 0; i < (160 * 80 / 2); i += sizeof(tx_buf)) {
        furi_hal_spi_bus_tx(&furi_hal_spi_bus_handle_oled, tx_buf, sizeof(tx_buf), 100);
    }

    furi_hal_spi_release(&furi_hal_spi_bus_handle_oled);
}

static void oled_send_init_sequence(const uint8_t* init_table, size_t table_len) {
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
            oled_send_command(&furi_hal_spi_bus_handle_oled, cmd_buf, len_byte + 1);
            cmd_offset += len_byte + 2;
        }
    }
}

static void oled_sleep_mode(bool sleep) {
    uint8_t power_cmd = sleep ? 0xAE : 0xAF;
    oled_send_command(&furi_hal_spi_bus_handle_oled, &power_cmd, 1);
}

static void oled_init(void) {
    furi_hal_gpio_init(&gpio_oled_vcc_en, GpioModeOutputPushPull, GpioPullNo, GpioSpeedMedium);
    furi_hal_gpio_write(&gpio_oled_vcc_en, false);

    furi_hal_gpio_write(&gpio_oled_dc, true);
    furi_hal_gpio_init(&gpio_oled_dc, GpioModeOutputPushPull, GpioPullUp, GpioSpeedMedium);

    furi_delay_ms(1);

    oled_send_init_sequence(oled_init_table_ssd1320, sizeof(oled_init_table_ssd1320));

    oled_clear();

    furi_hal_gpio_write(&gpio_oled_vcc_en, true);
    furi_delay_ms(10);
    oled_sleep_mode(false);
    furi_delay_ms(100);
}

static void oled_commit(uint8_t* data, size_t size, CanvasOrientation orientation, void* context) {
    UNUSED(orientation);
    uint32_t* frame_count = (uint32_t*)context;

    UNUSED(size);
    oled_write_buf(data);

    *frame_count += 1;
}

// TODO: move to startup hooks?
int32_t oled_screen(void* p) {
    UNUSED(p);

    uint32_t frame_count = 0;

    oled_init();

    FURI_LOG_I(TAG, "Started");

    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_framebuffer_callback(gui, oled_commit, &frame_count);

    // for(uint8_t i = 0; i < 4; i++) {
    //     furi_delay_ms(100);
    //     input_key_press(InputKeyDown);
    //     furi_delay_ms(100);
    //     input_key_release(InputKeyDown);
    // }
    // furi_delay_ms(100);
    // input_key_press(InputKeyOk);
    // furi_delay_ms(100);
    // input_key_release(InputKeyOk);

    while(true) {
        // gui_update(gui);
        furi_delay_ms(1000);
    }

    gui_remove_framebuffer_callback(gui, oled_commit, &frame_count);
    furi_record_close(RECORD_GUI);
    return 0;
}
