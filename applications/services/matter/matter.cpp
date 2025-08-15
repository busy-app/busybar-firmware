#include <furi.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

#include <app/server/Server.h>

#define TAG "MatterSrv"

extern "C" {
int matter_srv(void* arg);
}

int matter_srv(void* arg) {
    UNUSED(arg);

    // CHIP_ERROR err;
    //
    // static chip::ServerInitParams init_params;
    //
    // err = chip::Server::GetInstance().Init(init_params);

    for(;;) {
        ChipLogDetail(Test, "Hello there!");
        furi_delay_ms(5000);
    }

    return 0;
}

#pragma GCC diagnostic pop
