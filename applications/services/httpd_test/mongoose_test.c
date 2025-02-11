#include <furi.h>
#include <furi_hal_gpio.h>
#include <furi_hal_resources.h>
#include <version.h>
#include "mongoose.h"
#include "web_storage.h"
#include <usb_network/usb_network.h>

#define TAG "HTTPD"

#define WEB_DIR "/ext/web"

typedef struct {
    bool led_state;
} WebServerState;

const GpioPin* led_pin = NULL;

static void
    httpd_handle_led(struct mg_connection* c, struct mg_http_message* hm, WebServerState* state) {
    bool success = false;

    if(mg_match(hm->method, mg_str("GET"), NULL)) {
        if(hm->query.len == 0) {
            // Get current value
            success = true;
        } else {
            // Set by query string
            char led_state_str[2];
            do {
                int var_len =
                    mg_http_get_var(&hm->query, "state", led_state_str, sizeof(led_state_str));
                if(var_len != 1) {
                    break;
                }
                if(led_state_str[0] == '0') {
                    state->led_state = false;
                } else if(led_state_str[0] == '1') {
                    state->led_state = true;
                } else {
                    break;
                }
                if(led_pin) {
                    furi_hal_gpio_write(led_pin, state->led_state);
                }
                success = true;
            } while(0);
        }
    } else {
        // Set by JSON post
        do {
            struct mg_str led_state_str = mg_json_get_tok(hm->body, "$.state");
            if(led_state_str.len != 1) {
                break;
            }
            if(led_state_str.buf[0] == '0') {
                state->led_state = false;
            } else if(led_state_str.buf[0] == '1') {
                state->led_state = true;
            } else {
                break;
            }
            if(led_pin) {
                furi_hal_gpio_write(led_pin, state->led_state);
            }
            success = true;
        } while(0);
    }

    if(success) {
        mg_http_reply(
            c,
            200,
            "Content-Type: application/json\r\n",
            "{\"result\":\"OK\",\"state\":%u}\n",
            state->led_state);
    } else {
        mg_http_reply(c, 400, "", "Bad Request");
    }
}

static void httpd_handle_version(struct mg_connection* c, struct mg_http_message* hm) {
    if(mg_match(hm->method, mg_str("GET"), NULL)) {
        FuriString* ver_str = furi_string_alloc();
        const Version* firmware_version = version_get();
        furi_string_printf(
            ver_str,
            "\"branch\":\"%s\",\"version\":\"%s\",\"build_date\":\"%s\",",
            version_get_gitbranch(firmware_version),
            version_get_version(firmware_version),
            version_get_builddate(firmware_version));
        furi_string_cat_printf(
            ver_str,
            "\"commit_hash\":\"%s%s\"",
            version_get_githash(firmware_version),
            version_get_dirty_flag(firmware_version) ? "-dirty" : "");

        mg_http_reply(
            c, 200, "Content-Type: application/json\r\n", "{%s}\n", furi_string_get_cstr(ver_str));
        furi_string_free(ver_str);
    } else {
        mg_http_reply(c, 400, "", "Bad Request");
    }
}

static void httpd_handler(struct mg_connection* c, int ev, void* ev_data) {
    if(ev == MG_EV_HTTP_MSG) {
        WebServerState* state = c->fn_data;

        struct mg_http_message* hm = (struct mg_http_message*)ev_data;
        FURI_LOG_I(TAG, "%.*s %.*s", hm->method.len, hm->method.buf, hm->uri.len, hm->uri.buf);
        if(hm->query.len > 0) {
            FURI_LOG_I(TAG, "Query %.*s", hm->query.len, hm->query.buf);
        }
        if(mg_match(hm->uri, mg_str("/api/led"), NULL)) {
            httpd_handle_led(c, hm, state);
        } else if(mg_match(hm->uri, mg_str("/api/version"), NULL)) {
            httpd_handle_version(c, hm);
        } else {
            struct mg_http_serve_opts opts = {
                .root_dir = WEB_DIR,
                .page404 = WEB_DIR "/404.html",
                .fs = web_storage_get(),
            };
            mg_http_serve_dir(c, ev_data, &opts);
        }
    }
}

int32_t httpd_start(void* p) {
    UNUSED(p);
    UsbNetwork* usb_network = furi_record_open(RECORD_USB_NETWORK);
    usb_network_thread_init(usb_network);

    if(led_pin) {
        furi_hal_gpio_init_simple(led_pin, GpioModeOutputPushPull);
        furi_hal_gpio_write(led_pin, 0);
    }

    // mg_log_set(MG_LL_VERBOSE);
    mg_log_set(MG_LL_INFO);

    WebServerState* state = malloc(sizeof(WebServerState));

    struct mg_mgr mgr; // Event manager
    mg_mgr_init(&mgr); // Inititialise event manager

    // Setup listener
    mg_http_listen(&mgr, "http://0.0.0.0", httpd_handler, state);

    // Event loop
    for(;;) {
        mg_mgr_poll(&mgr, 1000);
    }

    // Cleanup
    mg_mgr_free(&mgr);

    usb_network_thread_cleanup(usb_network);
    furi_record_close(RECORD_USB_NETWORK);

    return 0;
}

uint64_t mg_millis(void) {
    return furi_get_tick();
}

void mg_log_prefix(int level, const char* file, int line, const char* fname) {
    UNUSED(file);
    FuriString* string = furi_string_alloc();

    const char* color = _FURI_LOG_CLR_RESET;
    const char* log_letter = " ";
    switch(level) {
    case MG_LL_ERROR:
        color = _FURI_LOG_CLR_E;
        log_letter = "E";
        break;
    case MG_LL_INFO:
        color = _FURI_LOG_CLR_I;
        log_letter = "I";
        break;
    case MG_LL_DEBUG:
        color = _FURI_LOG_CLR_D;
        log_letter = "D";
        break;
    case MG_LL_VERBOSE:
        color = _FURI_LOG_CLR_T;
        log_letter = "T";
        break;
    default:
        break;
    }

    furi_string_printf(
        string,
        "%lu %s[%s][%s] " _FURI_LOG_CLR_RESET,
        furi_get_tick(),
        color,
        log_letter,
        "Mongoose");

    furi_string_cat_printf(string, "%s:%u ", fname, line);
    furi_log_puts(furi_string_get_cstr(string));

    furi_string_free(string);
}

void mg_log(const char* fmt, ...) {
    FuriString* string = furi_string_alloc();

    va_list args;
    va_start(args, fmt);
    furi_string_vprintf(string, fmt, args);
    va_end(args);

    furi_string_cat_str(string, "\r\n");
    furi_log_puts(furi_string_get_cstr(string));
    furi_string_free(string);
}

int _gettimeofday(struct timeval* tv, void* tz) {
    uint64_t now = mg_now();
    (void)tz;
    tv->tv_sec = (time_t)(now / 1000);
    tv->tv_usec = (unsigned long)((now % 1000) * 1000);
    return 0;
}
