#include "cli_command_light_sensor.h"

#include <furi.h>

#include <furi_hal_i2c_config.h>
#include <drivers/bh1730/bh1730.h>

void cli_command_light_sensor(Cli* cli, FuriString* args, void* context) {
    UNUSED(cli);
    UNUSED(args);
    UNUSED(context);

    uint16_t data0 = 0;
    uint16_t data1 = 0;
    float lux = 0.0f;

    bool read_success = false;
    do {
        if(!bh1730_read_raw_data0(&furi_hal_i2c_handle_1, &data0)) break;
        if(!bh1730_read_raw_data1(&furi_hal_i2c_handle_1, &data1)) break;
        if(!bh1730_read_lux(&furi_hal_i2c_handle_1, &lux)) break;

        read_success = true;
    } while(false);

    if(read_success) {
        printf("data0: %d\r\n", data0);
        printf("data1: %d\r\n", data1);
        printf("lux  : %.2f\r\n", lux);
    } else {
        printf("Error: Can't reach sensor via I2C.\r\n");
    }
}
