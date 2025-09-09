#include <furi.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

#include <platform/PlatformManager.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>

#include <app/server/Server.h>
#include <app/clusters/on-off-server/on-off-server.h>
#include <app-common/zap-generated/attributes/Accessors.h>

#include <platform/bsb/BSBDeviceInfoProvider.hpp>
#include <platform/bsb/BSBCommissionableDataProvider.hpp>
#include <platform/bsb/BSBDeviceInstanceInfoProvider.hpp>

#include <network/network.h>
#include <wifi/wifi_common.h>
#include <intercom/intercom.h>

#include "matter_common_i.h"

#define TAG "MatterSrv"

using namespace chip;
using namespace Credentials;
using namespace Platform;
using namespace DeviceLayer;
using namespace chip::app::Clusters;

class MatterSrv {
public:
    MatterSrv(void);
    CHIP_ERROR init(void);

    Intercom* intercom;

private:
    CommonCaseDeviceServerInitParams m_server_init_params;
};

// sorry - the MatterPostAttributeChangeCallback can't accept any context
static MatterSrv* matter_global_srv;

// ======================
// Communication with f20
// ======================

/**
 * @brief Maps `MatterVirtualDevice` to `EndpointId`
 */
static const EndpointId matter_endpoint_ids[MatterVirtualDeviceMAX] = {
    [MatterVirtualDeviceSwitch1] = 1,
    [MatterVirtualDeviceSwitch2] = 2,
};

/**
 * @brief Maps `EndpointId` to `MatterVirtualDevice`
 */
static const MatterVirtualDevice matter_device_ids[] = {
    [0] = MatterVirtualDeviceMAX, // reserved
    [1] = MatterVirtualDeviceSwitch1,
    [2] = MatterVirtualDeviceSwitch2,
};

/**
 * @brief Updates cluster attributes in the Matter stack to match our
 * representation of the state
 */
static void matter_apply_new_device_state(MatterVirtualDeviceState* state) {
    furi_assert(state);

    switch(state->device) {
    case MatterVirtualDeviceSwitch1:
    case MatterVirtualDeviceSwitch2:
        OnOff::Attributes::OnOff::Set(matter_endpoint_ids[state->device], state->bool_val);
        break;

    case MatterVirtualDeviceMAX:
        furi_crash();
    }
}

/**
 * @brief Handles an intercom frame
 */
static void matter_handle_frame(const void* data, size_t data_size, void* context) {
    furi_check(data);
    furi_check(data_size == sizeof(MatterIntercomFrame));
    furi_check(context);
    const auto* frame = static_cast<const MatterIntercomFrame*>(data);
    auto* matter = static_cast<MatterSrv*>(context);

    if(frame->type == MatterIntercomFrameTypeRequest) {
        FURI_LOG_D(TAG, "Request frame");
        auto* dup_state = new MatterVirtualDeviceState;
        memcpy(dup_state, &frame->request.req_state, sizeof(*dup_state));

        PlatformMgr().ScheduleWork(
            [](intptr_t context) {
                auto* dup_state = (MatterVirtualDeviceState*)context;
                matter_apply_new_device_state(dup_state);
                delete dup_state;
            },
            (intptr_t)dup_state);

    } else if(frame->type == MatterIntercomFrameTypeReset) {
        FURI_LOG_D(TAG, "Reset frame");
        Server::GetInstance().ScheduleFactoryReset();

    } else {
        furi_crash();
    }
}

/**
 * @brief Notifies f20 about an updated state 
 */
static void matter_send_state_update(MatterSrv* matter, MatterVirtualDeviceState state) {
    MatterIntercomFrame frame = {
        .type = MatterIntercomFrameTypeUpdate,
        .update =
            {
                .new_state = state,
            },
    };
    furi_check(
        intercom_tx(
            matter->intercom, IntercomChannelMatter, &frame, sizeof(frame), FuriWaitForever) ==
        sizeof(frame));
}

/**
 * @brief Receives updates about cluster attribute changes in the Matter stack
 * @note Overrides an `__attribute__((weak))` stub callback in the Matter SDK
 */
void MatterPostAttributeChangeCallback(
    const chip::app::ConcreteAttributePath& attributePath,
    uint8_t type,
    uint16_t size,
    uint8_t* value) {
    EndpointId endpoint = attributePath.mEndpointId;
    ClusterId cluster = attributePath.mClusterId;
    AttributeId attribute = attributePath.mAttributeId;
    MatterVirtualDevice device = matter_device_ids[endpoint];

    switch(device) {
    case MatterVirtualDeviceMAX:
        return;

    case MatterVirtualDeviceSwitch1:
    case MatterVirtualDeviceSwitch2: {
        if(!(cluster == OnOff::Id && attribute == OnOff::Attributes::OnOff::Id)) return;
        matter_send_state_update(
            matter_global_srv,
            (MatterVirtualDeviceState){
                .device = device,
                .bool_val = (bool)(*value),
            });
        break;
    }
    }
}

/**
 * @brief Sends the current state to f20
 */
static void matter_send_current_state(MatterSrv* matter, MatterVirtualDevice device) {
    switch(device) {
    case MatterVirtualDeviceSwitch1:
    case MatterVirtualDeviceSwitch2: {
        MatterVirtualDeviceState state = {
            .device = device,
            .bool_val = false /* to be filled */,
        };
        OnOff::Attributes::OnOff::Get(matter_endpoint_ids[device], &state.bool_val);
        break;
    }

    default:
        furi_crash();
    }
}

// =============
// Service setup
// =============

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

    // TODO: Find out why it doesn't work if connecting right away
    furi_delay_ms(3000);
}

MatterSrv::MatterSrv(void) {
    this->intercom = static_cast<Intercom*>(furi_record_open(RECORD_INTERCOM));
}

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

        intercom_set_rx_callback(this->intercom, IntercomChannelMatter, matter_handle_frame, this);
        matter_send_current_state(this, MatterVirtualDeviceSwitch1);
        matter_send_current_state(this, MatterVirtualDeviceSwitch2);
    } while(false);

    return err;
}

extern "C" {
int matter_srv(void* arg);
}

int matter_srv(void* arg) {
    UNUSED(arg);

    MatterSrv* instance = new MatterSrv;
    matter_global_srv = instance;
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
