#include <furi.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

#include <platform/PlatformManager.h>
#include <app/server/Server.h>

#define TAG "MatterSrv"

extern "C" {
int matter_srv(void* arg);
}

int matter_srv(void* arg) {
    UNUSED(arg);

    CHIP_ERROR err;

    do {
        err = chip::Platform::MemoryInit();
        if(err != CHIP_NO_ERROR) {
            break;
        }

        err = chip::DeviceLayer::PlatformMgr().InitChipStack();
        if(err != CHIP_NO_ERROR) {
            break;
        }

        chip::CommonCaseDeviceServerInitParams init_params;
        err = init_params.InitializeStaticResourcesBeforeServerInit();
        if(err != CHIP_NO_ERROR) {
            break;
        }

        err = chip::Server::GetInstance().Init(init_params);
        if(err != CHIP_NO_ERROR) {
            break;
        }

    } while(false);

    if(err == CHIP_NO_ERROR) {
        chip::DeviceLayer::PlatformMgr().RunEventLoop();
    } else {
        FURI_LOG_E(TAG, "Failed to start: 0x%lx", err.AsInteger());
        furi_thread_suspend(furi_thread_get_current_id());
    }

    return 0;
}

#pragma GCC diagnostic pop
