#include "matter_app.h"

//#include <sl_mbedtls.h>
#include <furi.h>
#include <MatterConfig.h>
#include <BaseApplication.h>
// #include <app/clusters/occupancy-sensor-server/occupancy-sensor-server.h>
#include <app/clusters/occupancy-sensor-server/occupancy-hal.h>
#include <app-common/zap-generated/attributes/Accessors.h>
// #include <app/PluginApplicationCallbacks.h>

#define TAG "MatterApp"

using namespace chip;
using namespace chip::app::Clusters;

constexpr EndpointId kModeEndpoint = 1;
constexpr EndpointId kOccupancyEndpoint = 2;

HalOccupancySensorType halOccupancyGetSensorType(EndpointId endpoint) {
    FURI_LOG_D(TAG, "Initializing occupancy HAL");
    furi_check(endpoint == kOccupancyEndpoint);
    return HAL_OCCUPANCY_SENSOR_TYPE_PHYSICAL;
}

void matter_app_init(void) {
    //sl_mbedtls_init();
    SilabsMatterConfig::AppInit();
}

void matter_factory_reset(void) {
    BaseApplication::StartFactoryResetSequence();
}

void matter_button_press(void) {
    FURI_LOG_D(TAG, "BUSY mode");

    DeviceLayer::PlatformMgr().LockChipStack();
    halOccupancyStateChangedCallback(kOccupancyEndpoint, HAL_OCCUPANCY_STATE_OCCUPIED);
    DeviceLayer::PlatformMgr().UnlockChipStack();

    // using namespace OccupancySensing;
    // Attributes::Occupancy::Set(kOccupancyEndpoint, OccupancyBitmap::kOccupied);
}

void matter_button_release(void) {
    FURI_LOG_D(TAG, "REST mode");

    DeviceLayer::PlatformMgr().LockChipStack();
    halOccupancyStateChangedCallback(kOccupancyEndpoint, HAL_OCCUPANCY_STATE_UNOCCUPIED);
    DeviceLayer::PlatformMgr().UnlockChipStack();

    // using namespace OccupancySensing;
    // Attributes::Occupancy::Set(kOccupancyEndpoint, 0);
}

void matter_basic_commissioning_window(void) {
    BaseApplication::DoProvisioningBasicCommissioningWindow();
}
