#include "cli_command_device_info.h"
#include <device_info/device_info.h>

static void
    cli_command_device_info_callback(const char* key, const char* value, bool last, void* context) {
    UNUSED(last);
    UNUSED(context);
    printf("%-30s: %s\r\n", key, value);
}

void cli_command_device_info(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    UNUSED(args);
    UNUSED(context);

    DeviceInfo* dev_info = furi_record_open(RECORD_DEVICE_INFO);
    device_info_query(dev_info, cli_command_device_info_callback, '_', NULL);
    furi_record_close(RECORD_DEVICE_INFO);
}
