#include "matter_app.h"

//#include <sl_mbedtls.h>
#include <MatterConfig.h>
#include <BaseApplication.h>
#include <app/clusters/switch-server/switch-server.h>
#include <app-common/zap-generated/attributes/Accessors.h>

constexpr chip::EndpointId kLightSwitchEndpoint = 1;
constexpr chip::EndpointId kGenericSwitchEndpoint = 2;

void matter_app_init(void) {
    //sl_mbedtls_init();
    SilabsMatterConfig::AppInit();
}

void matter_factory_reset(void) {
    BaseApplication::StartFactoryResetSequence();
}

void matter_button_press(void) {
    // chip::app::Clusters::SwitchServer::Instance().OnInitialPress(kGenericSwitchEndpoint, 1);

    uint8_t currentPosition = 1;

    // Set new attribute value
    chip::app::Clusters::Switch::Attributes::CurrentPosition::Set(kGenericSwitchEndpoint, currentPosition);

    // Trigger event
    chip::app::Clusters::SwitchServer::Instance().OnInitialPress(kGenericSwitchEndpoint, currentPosition);
}

void matter_button_release(void) {
    // chip::app::Clusters::SwitchServer::Instance().OnShortRelease(kGenericSwitchEndpoint, 1);

    uint8_t previousPosition = 1;
    uint8_t currentPosition  = 0;

    // Set new attribute value
    chip::app::Clusters::Switch::Attributes::CurrentPosition::Set(kGenericSwitchEndpoint, currentPosition);

    // Trigger event
    chip::app::Clusters::SwitchServer::Instance().OnShortRelease(kGenericSwitchEndpoint, previousPosition);
}

void matter_basic_commissioning_window(void) {
    BaseApplication::DoProvisioningBasicCommissioningWindow();
}
