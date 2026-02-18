/**
 * @file state_publisher.h
 * @brief State publisher - publish state changes across various channels.
 */
#pragma once

#include <stdint.h>
#include <ble/ble.h>
#include <wifi/wifi_common.h>
#include <power/power_service/power.h>
#include <matter/matter.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RECORD_STATE_PUBLISHER "state_publisher"

typedef struct StatePublisher StatePublisher;

#ifdef __cplusplus
}
#endif
