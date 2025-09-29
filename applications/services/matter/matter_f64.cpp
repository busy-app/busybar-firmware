#include <furi.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

#include <platform/PlatformManager.h>

#include <app/server/Server.h>
#include <app/server/OnboardingCodesUtil.h>
#include <app/clusters/on-off-server/on-off-server.h>
#include <app-common/zap-generated/attributes/Accessors.h>

#include <platform/bsb/BSBDeviceInfoProvider.hpp>
#include <platform/bsb/BSBCommissionableDataProvider.hpp>
#include <platform/bsb/BSBDeviceInstanceInfoProvider.hpp>
#include <platform/bsb/BSBDeviceAttestationCredsProvider.hpp>

#include <network/network.h>
#include <wifi/wifi_common.h>
#include <intercom/intercom.h>

#include "matter_common_i.h"

#define TAG "MatterSrv"

#define RENDEZVOUS_FLAGS (RendezvousInformationFlags(chip::RendezvousInformationFlag::kOnNetwork))

using namespace chip;
using namespace Credentials;
using namespace Platform;
using namespace DeviceLayer;
using namespace Transport;
using namespace chip::app::Clusters;
using namespace chip::System::Clock;

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

// =========
// Utilities
// =========

static void matter_hyphenate_manual_code(char* buffer, size_t buf_size) {
    furi_check(buf_size >= (MATTER_MAX_MAN_CODE_LEN + 1));

    static const size_t pattern[2] = {4, 3};
    const size_t original_len = strlen(buffer);
    const size_t orig_len_with_terminator = original_len + 1;
    furi_check((original_len == 11) || (original_len == 21));

    size_t pattern_step = 0;
    size_t i = 0;

    while(1) {
        i += pattern[pattern_step];
        pattern_step = (pattern_step + 1) % COUNT_OF(pattern);

        if(buffer[i] == '\0') break;

        memmove(buffer + i + 1, buffer + i, orig_len_with_terminator - i);
        buffer[i] = '-';

        i++;
    }
}

// ======================
// Communication with u5
// ======================

static void matter_send_frame(MatterSrv* matter, const MatterIntercomFrame* frame) {
    furi_check(
        intercom_tx(
            matter->intercom, IntercomChannelMatter, frame, sizeof(*frame), FuriWaitForever) ==
        sizeof(*frame));
}

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

    } else if(frame->type == MatterIntercomFrameTypeCommission) {
        FURI_LOG_D(TAG, "Commission frame");

        size_t duration = MATTER_COMMISSION_TIME_SECONDS;
        PlatformMgr().ScheduleWork(
            [](intptr_t arg) {
                auto duration = static_cast<size_t>(arg);
                Server::GetInstance().GetCommissioningWindowManager().OpenBasicCommissioningWindow(
                    Seconds32(duration));
            },
            static_cast<intptr_t>(duration));

        MatterIntercomFrame frame = {
            .type = MatterIntercomFrameTypePairingCodes,
        };
        auto qr_code = MutableCharSpan(frame.codes.qr_code, sizeof(frame.codes.qr_code));
        auto manual_code =
            MutableCharSpan(frame.codes.manual_code, sizeof(frame.codes.manual_code));

        StackLock lock;
        GetQRCode(qr_code, RENDEZVOUS_FLAGS);
        GetManualPairingCode(manual_code, RENDEZVOUS_FLAGS);

        matter_hyphenate_manual_code(frame.codes.manual_code, sizeof(frame.codes.manual_code));
        matter_send_frame(matter, &frame);

    } else {
        furi_crash();
    }
}

/**
 * @brief Notifies u5 about an updated state 
 */
static void matter_send_state_update(MatterSrv* matter, MatterVirtualDeviceState state) {
    MatterIntercomFrame frame = {
        .type = MatterIntercomFrameTypeUpdate,
        .update =
            {
                .new_state = state,
            },
    };
    matter_send_frame(matter, &frame);
}

/**
 * @brief Sends current count of commissioned fabrics to u5
 * @warning Requires Matter stack to be locked
 */
static void matter_send_fabric_count_update(MatterSrv* matter) {
    MatterIntercomFrame frame = {
        .type = MatterIntercomFrameTypeFabricCountUpdate,
        .fabric_count =
            {
                .fabric_count = Server::GetInstance().GetFabricTable().FabricCount(),
            },
    };
    matter_send_frame(matter, &frame);
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
 * @brief Sends the current state to u5
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

static void matter_device_event(const ChipDeviceEvent* event, intptr_t arg) {
    auto matter = (MatterSrv*)arg;

    if(event->Type == DeviceEventType::kSecureSessionEstablished) {
        if(event->SecureSessionEstablished.SecureSessionType ==
           (uint8_t)SecureSession::Type::kPASE) {
            FURI_LOG_D(TAG, "PASE established");
            // Matter doesn't provide a "Commissioning started" event,
            // but the earliest commissioning step that we can detect is the
            // establishment of a passcode-authenticated session. So we do that.
            MatterIntercomFrame frame = {
                .type = MatterIntercomFrameTypeCommissionStatus,
                .commission_status =
                    {
                        .status = MatterCommissioningStatusStarted,
                    },
            };
            matter_send_frame(matter, &frame);
        }

    } else if(event->Type == DeviceEventType::kFailSafeTimerExpired) {
        FURI_LOG_D(TAG, "Fail-safe expired");
        MatterIntercomFrame frame = {
            .type = MatterIntercomFrameTypeCommissionStatus,
            .commission_status =
                {
                    .status = MatterCommissioningStatusFailed,
                },
        };
        matter_send_frame(matter, &frame);

    } else if(event->Type == DeviceEventType::kCommissioningComplete) {
        FURI_LOG_D(TAG, "Commissioning complete");
        MatterIntercomFrame frame = {
            .type = MatterIntercomFrameTypeCommissionStatus,
            .commission_status =
                {
                    .status = MatterCommissioningStatusComplete,
                },
        };
        matter_send_frame(matter, &frame);
        matter_send_fabric_count_update(matter);
    }
}

MatterSrv::MatterSrv(void) {
    this->intercom = static_cast<Intercom*>(furi_record_open(RECORD_INTERCOM));
}

CHIP_ERROR MatterSrv::init(void) {
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

        SetDeviceInfoProvider(DeviceLayer::BSB::GetDeviceInfoProvider());
        SetDeviceInstanceInfoProvider(DeviceLayer::BSB::GetDeviceInstanceInfoProvider());
        SetCommissionableDataProvider(DeviceLayer::BSB::GetCommissionableDataProvider());
        SetDeviceAttestationCredentialsProvider(
            Credentials::BSB::GetDeviceAttestationCredentialsProvider());

        err = m_server_init_params.InitializeStaticResourcesBeforeServerInit();
        if(err != CHIP_NO_ERROR) {
            break;
        }

        err = Server::GetInstance().Init(m_server_init_params);
        if(err != CHIP_NO_ERROR) {
            break;
        }

        DeviceLayer::BSB::GetDeviceInfoProvider()->SetStorageDelegate(
            &Server::GetInstance().GetPersistentStorage());

        PlatformMgr().AddEventHandler(matter_device_event, (intptr_t)this);

        intercom_set_rx_callback(this->intercom, IntercomChannelMatter, matter_handle_frame, this);
        matter_send_current_state(this, MatterVirtualDeviceSwitch1);
        matter_send_current_state(this, MatterVirtualDeviceSwitch2);
        matter_send_fabric_count_update(this);
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
