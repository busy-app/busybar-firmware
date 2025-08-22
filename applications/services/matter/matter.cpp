#include <furi.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

#include <platform/PlatformManager.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>

#include <app/server/Server.h>

#include <platform/bsb/BSBDeviceInfoProvider.hpp>
#include <platform/bsb/BSBCommissionableDataProvider.hpp>
#include <platform/bsb/BSBDeviceInstanceInfoProvider.hpp>

#include <network/network.h>
#include <wifi/wifi_common.h>

#define TAG "MatterSrv"

using namespace chip;
using namespace Credentials;
using namespace Platform;
using namespace DeviceLayer;

class MatterSrv {
public:
    CHIP_ERROR init(void);

private:
    CommonCaseDeviceServerInitParams m_server_init_params;
};

CHIP_ERROR MatterSrv::init(void) {
    auto* network = static_cast<Network*>(furi_record_open(RECORD_NETWORK));
    network_init_current_thread(network);

    // TODO: react to Wifi events
    auto* wifi_pubsub = static_cast<FuriPubSub*>(furi_record_open(RECORD_WIFI));
    UNUSED(wifi_pubsub);

    // FIXME: commissioning doesn't work if not connected to a network first
    FURI_LOG_I(TAG, "Waiting 10s for Wifi to connect...");
    furi_delay_ms(10000);

    CHIP_ERROR err;

    do {
        err = MemoryInit();
        if(err != CHIP_NO_ERROR) {
            break;
        }

        err = PlatformMgr().InitChipStack();
        if(err != CHIP_NO_ERROR) {
            break;
        }

        StackLock lock;

        SetDeviceInfoProvider(BSB::GetDeviceInfoProvider());
        SetDeviceInstanceInfoProvider(BSB::GetDeviceInstanceInfoProvider());
        SetCommissionableDataProvider(BSB::GetCommissionableDataProvider());
        SetDeviceAttestationCredentialsProvider(Examples::GetExampleDACProvider());

        err = m_server_init_params.InitializeStaticResourcesBeforeServerInit();
        if(err != CHIP_NO_ERROR) {
            break;
        }

        err = Server::GetInstance().Init(m_server_init_params);
        if(err != CHIP_NO_ERROR) {
            break;
        }

        BSB::GetDeviceInfoProvider()->SetStorageDelegate(
            &Server::GetInstance().GetPersistentStorage());

        PlatformMgr().ScheduleWork([](intptr_t arg) {
            Server::GetInstance().GetCommissioningWindowManager().OpenBasicCommissioningWindow();
        });

    } while(false);

    return err;
}

extern "C" {
int matter_srv(void* arg);
}

int matter_srv(void* arg) {
    UNUSED(arg);

    MatterSrv* instance = new MatterSrv;
    const CHIP_ERROR err = instance->init();

    if(err == CHIP_NO_ERROR) {
        PlatformMgr().RunEventLoop();

    } else {
        FURI_LOG_E(TAG, "Failed to start: 0x%lx", err.AsInteger());
        furi_thread_suspend(furi_thread_get_current_id());
    }

    return 0;
}

#pragma GCC diagnostic pop
