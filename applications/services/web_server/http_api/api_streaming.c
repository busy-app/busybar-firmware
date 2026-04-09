#include "http_api.h"
#include <gui/gui.h>
#include <front_display/front_display.h>
#include <back_display/back_display.h>
#include <toolbox/rle_encode.h>
#include <toolbox/color.h>

#define TAG "Stream"

bool http_api_streaming_single_frame_callback(
    FuriString* path,
    HttpMethod method,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(method);
    UNUSED(msg);
    UNUSED(ctx);

    if(!IS_HTTP_ENDPOINT(path)) return false;
    if(method != HttpMethodGet) {
        http_reply_405_method_not_allowed(conn, HttpMethodGet);
        return true;
    }

    char display_str[2];
    int var_len = mg_http_get_var(&msg->query, "display", display_str, sizeof(display_str));

    if(var_len == 1 && (display_str[0] == '0' || display_str[0] == '1')) {
        GuiDisplayId display_id = display_str[0] == '0' ? GuiDisplayIdFront : GuiDisplayIdBack;

        const size_t frame_size =
            display_id == GuiDisplayIdFront ?
                FRONT_DISPLAY_BUF_SIZE :
                (BACK_DISPLAY_BUF_SIZE /
                 2); // We do 8bpp to 4bpp conversion for the back display, so the size is halved

        Gui* gui = furi_record_open(RECORD_GUI);
        uint8_t* frame = malloc(frame_size);

        with_gui(gui, {
            const uint8_t* buf = gui_display_get_frame_buffer(gui, display_id);
            if(display_id == GuiDisplayIdFront)
                memcpy(frame, buf, frame_size);
            else {
                color_buf_l8_to_l4(frame, buf, BACK_DISPLAY_BUF_SIZE);
            }
        });
        furi_record_close(RECORD_GUI);

        MG_REPLY_IMAGE(conn, frame, frame_size);
        free(frame);
    } else
        MG_REPLY_ERROR(conn, 400, "Wrong display");
    return true;
}
