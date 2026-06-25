#include "about.h"

static const AboutLibInfo about_libs_info[] = {
    {.name = "cJSON", .license = "MIT", .url = "https://github.com/DaveGamble/cJSON"},
    {.name = "FatFS", .license = "Custom, BSD-like", .url = "https://elm-chan.org/fsw/ff"},
    {.name = "FreeRTOS", .license = "MIT", .url = "https://www.freertos.org/"},
    {.name = "Heatshrink", .license = "ISC", .url = "https://github.com/atomicobject/heatshrink"},
    {.name = "LVGL", .license = "MIT", .url = "https://lvgl.io/"},
    {.name = "lwIP", .license = "BSD", .url = "https://www.nongnu.org/lwip/"},
    {.name = "Mbed TLS",
     .license = "Apache 2.0 or GPLv2+",
     .url = "https://www.trustedfirmware.org/projects/mbed-tls/"},
    {.name = "MicroTar", .license = "MIT", .url = "https://github.com/amachronic/microtar"},
    {.name = "M*LIB", .license = "MIT", .url = "https://github.com/P-p-H-d/mlib"},
    {.name = "Mongoose", .license = "GPLv2+", .url = "https://mongoose.ws/"},
    {.name = "Nanopb", .license = "ZLib", .url = "https://jpa.kapsi.fi/nanopb/"},
    {.name = "TinyUSB", .license = "MIT", .url = "https://www.tinyusb.org/"},
    {.name = "utz", .license = "MIT", .url = "https://github.com/evq/utz"},
    {.name = "ZLib", .license = "ZLib", .url = "https://zlib.net/"},
};

size_t about_get_libs_count(void) {
    return COUNT_OF(about_libs_info);
}

const AboutLibInfo* about_get_lib_info(size_t index) {
    furi_check(index < COUNT_OF(about_libs_info));
    return &about_libs_info[index];
}
