#include "intercom_i.h"

#include <furi_hal_power.h>

size_t intercom_build_frame(
    IntercomFrame* frame,
    IntercomChannelId channel_id,
    const void* data,
    size_t data_size) {
    const size_t actual_data_size = MIN(data_size, sizeof(frame->data));

    memcpy(frame->data, data, actual_data_size);
    frame->data_size = actual_data_size;
    frame->channel_id = channel_id;
    frame->check = intercom_frame_get_checksum(frame);

    return actual_data_size;
}

void intercom_dump_frame(const IntercomFrame* frame) {
    FuriString* tmp = furi_string_alloc();

    furi_string_printf(
        tmp,
        "chan : %hhu\r\n"
        "size : %hu\r\n"
        "data : \r\n",
        frame->channel_id,
        frame->data_size);

    for(uint32_t i = 0; i < sizeof(frame->data); ++i) {
        if(i && i % 32 == 0) furi_string_cat(tmp, "\r\n");
        furi_string_cat_printf(tmp, "%02X ", frame->data[i]);
    }

    furi_string_cat_printf(tmp, "\r\ncheck: 0x%04X\r\n", frame->check);

    furi_log_puts(furi_string_get_cstr(tmp));

    furi_string_free(tmp);
}

#if defined(BSB_MCU_U5)
void intercom_reset_other_side(void) {
    furi_hal_power_reset_917(false);
}
#endif //BSB_MCU_U5
