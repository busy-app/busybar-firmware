/**
 * @file device_name_common.h
 * @brief DeviceName common defines.
 */
#pragma once

/**
 * @brief Default device name.
 */
#define DEVICE_NAME_DEFAULT "BUSY Bar"

/**
 * @brief Maximum device name length.
 */
#define DEVICE_NAME_MAX_LENGTH (20U)

/**
 * @brief Maximum size necessary to store the device name (including zero terminator).
 */
#define DEVICE_NAME_MAX_SIZE (DEVICE_NAME_MAX_LENGTH + 1U)
