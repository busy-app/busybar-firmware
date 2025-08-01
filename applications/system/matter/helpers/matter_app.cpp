#include "matter_app.h"

//#include <sl_mbedtls.h>
#include <furi.h>
#include <MatterConfig.h>
#include <BaseApplication.h>
#include <app-common/zap-generated/attributes/Accessors.h>

#define TAG "MatterApp"

using namespace chip;
using namespace chip::app::Clusters;

constexpr EndpointId kOnOffEndpoint = 1;
void matter_app_init(void) {
    //sl_mbedtls_init();
    SilabsMatterConfig::AppInit();
}

void matter_factory_reset(void) {
    BaseApplication::StartFactoryResetSequence();
}

void matter_button_press(void) {
    FURI_LOG_D(TAG, "BUSY mode");

    // DeviceLayer::PlatformMgr().LockChipStack();
    // halOccupancyStateChangedCallback(kOnOffEndpoint, HAL_OCCUPANCY_STATE_OCCUPIED);
    // DeviceLayer::PlatformMgr().UnlockChipStack();
}

void matter_button_release(void) {
    FURI_LOG_D(TAG, "REST mode");

    // DeviceLayer::PlatformMgr().LockChipStack();
    // halOccupancyStateChangedCallback(kOnOffEndpoint, HAL_OCCUPANCY_STATE_UNOCCUPIED);
    // DeviceLayer::PlatformMgr().UnlockChipStack();
}

void matter_basic_commissioning_window(void) {
    BaseApplication::DoProvisioningBasicCommissioningWindow();
}
