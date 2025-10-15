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

static constexpr EndpointId onOffEndpointId = 1;

class BsbFabricTableDelegate : public FabricTable::Delegate {
    void OnFabricRemoved(const FabricTable& fabricTable, FabricIndex fabricIndex) override;
};

class MatterSrv {
public:
    MatterSrv(void);
    CHIP_ERROR init(void);

    Intercom* m_intercom;

private:
    CommonCaseDeviceServerInitParams m_server_init_params;
    BsbFabricTableDelegate m_fabric_delegate;
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
            matter->m_intercom, IntercomChannelMatter, frame, sizeof(*frame), FuriWaitForever) ==
        sizeof(*frame));
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

    if(frame->type == MatterIntercomFrameTypeSwitchState) {
        FURI_LOG_D(TAG, "SwitchState frame");

        const auto workFn =
            frame->switch_state.value ?
                [](intptr_t arg) { OnOffServer::Instance().setOnOffValue(onOffEndpointId, true, false); } :
                [](intptr_t arg) { OnOffServer::Instance().setOnOffValue(onOffEndpointId, false, false); };

        PlatformMgr().ScheduleWork(workFn, 0);

    } else if(frame->type == MatterIntercomFrameTypeReset) {
        FURI_LOG_D(TAG, "Reset frame");
        Server::GetInstance().ScheduleFactoryReset();

    } else if(frame->type == MatterIntercomFrameTypeCommission) {
        FURI_LOG_D(TAG, "Commission frame");

        PlatformMgr().ScheduleWork(
            [](intptr_t arg) {
                Server::GetInstance().GetCommissioningWindowManager().OpenBasicCommissioningWindow(
                    Seconds32(MATTER_COMMISSION_TIME_SECONDS));
            },
            0);

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
static void matter_send_state_update(MatterSrv* matter, bool state) {
    MatterIntercomFrame frame = {
        .switch_state =
            {
                .value = state,
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

    if(cluster == OnOff::Id && attribute == OnOff::Attributes::OnOff::Id) {
        matter_send_state_update(matter_global_srv, static_cast<bool>(*value));
    }
}

/**
 * @brief Sends the current state to u5
 */
static void matter_send_current_state(MatterSrv* matter) {
    bool state;

    if(OnOffServer::Instance().getOnOffValue(onOffEndpointId, &state) ==
       Protocols::InteractionModel::Status::Success) {
        matter_send_state_update(matter, state);
    }
}

// =============
// Service setup
// =============

void BsbFabricTableDelegate::OnFabricRemoved(
    const FabricTable& fabricTable,
    FabricIndex fabricIndex) {
    UNUSED(fabricTable);
    UNUSED(fabricIndex);
    matter_send_fabric_count_update(matter_global_srv);
}

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
    this->m_intercom = static_cast<Intercom*>(furi_record_open(RECORD_INTERCOM));
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
        Server::GetInstance().GetFabricTable().AddFabricDelegate(&m_fabric_delegate);

        intercom_set_rx_callback(m_intercom, IntercomChannelMatter, matter_handle_frame, this);

        matter_send_current_state(this);
        matter_send_fabric_count_update(this);

    } while(false);

    return err;
}

extern "C" {
int matter_srv(void* arg);
}

int matter_srv(void* arg) {
    UNUSED(arg);

    matter_global_srv = new MatterSrv;

    const auto err = matter_global_srv->init();

    if(err == CHIP_NO_ERROR) {
        PlatformMgr().RunEventLoop();

    } else {
        FURI_LOG_E(TAG, "Failed to start: 0x%lx", err.Format());
        furi_thread_suspend(furi_thread_get_current_id());
    }

    return 0;
}

#pragma GCC diagnostic pop
