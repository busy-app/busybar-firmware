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

#include <nvm3_default.h>
#include <nvm3_generic.h>

#define TAG "MatterSrv"

using namespace chip;
using namespace Credentials;
using namespace Platform;
using namespace DeviceLayer;

static void matter_wait_for_network(void) {
    FURI_LOG_I(TAG, "Waiting for network...");

    auto* network = static_cast<Network*>(furi_record_open(RECORD_NETWORK));
    network_init_current_thread(network);

    auto* wifi_pubsub = static_cast<FuriPubSub*>(furi_record_open(RECORD_WIFI));

    FuriSemaphore* wifi_sem = furi_semaphore_alloc(1, 0);

    furi_pubsub_subscribe(
        wifi_pubsub,
        [](const void* message, void* context) {
            const auto state = *(static_cast<const WifiState*>(message));

            if(state == WifiStateUp) {
                auto* wifi_sem = static_cast<FuriSemaphore*>(context);
                furi_semaphore_release(wifi_sem);
            }
        },
        wifi_sem);

    furi_semaphore_acquire(wifi_sem, FuriWaitForever);
    furi_semaphore_free(wifi_sem);

    // TODO: Find out why it doesn't work if connecting right away
    furi_delay_ms(3000);
}

class MatterSrv {
public:
    CHIP_ERROR init(void);

private:
    CommonCaseDeviceServerInitParams m_server_init_params;
};

CHIP_ERROR MatterSrv::init(void) {
    // TODO: Implement proper network handling
    matter_wait_for_network();

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

        // TODO: Implement factory reset procedure & controls
#ifdef MATTER_FACTORY_RESET
        FURI_LOG_D(TAG, "Resetting configuration...");
        nvm3_eraseAll(nvm3_defaultHandle);
        FURI_LOG_D(TAG, "Configuration was reset");
#endif
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

        // TODO: Implement pairing controls
        // Always opening a commission window now
        PlatformMgr().ScheduleWork([](intptr_t arg) {
            UNUSED(arg);
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
