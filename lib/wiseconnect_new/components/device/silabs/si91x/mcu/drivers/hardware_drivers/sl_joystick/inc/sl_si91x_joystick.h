/***************************************************************************/ /**
 * @file sl_si91x_joystick.h
 * @brief JOYSTICK API implementation
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/

#ifndef SL_SI91X_JOYSTICK_H_
#define SL_SI91X_JOYSTICK_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "sl_status.h"
#include "sl_si91x_adc.h"
#include "sl_si91x_joystick_config.h"

/***************************************************************************/
/**
 * @addtogroup JOYSTICK Joystick
 * @ingroup SI91X_HARDWARE_DRIVER_APIS
 * @{
 ******************************************************************************/

/*******************************************************************************
 ******************************   DEFINES   ************************************
 ******************************************************************************/
/// @cond DO_NOT_INCLUDE_WITH_DOXYGEN
#define JOYSTICK_MV_ERROR JOYSTICK_MV_ERR_CARDINAL_ONLY // tolerance in mV for cardinal
/// @endcond

/*******************************************************************************
 *****************************   DATA TYPES   *********************************
 ******************************************************************************/
/**
 * @brief Enumeration for identifying the position of the joystick.
 */
typedef enum {
  SL_JOYSTICK_NONE, ///< Not pressed
  SL_JOYSTICK_C,    ///< Center
  SL_JOYSTICK_N,    ///< North
  SL_JOYSTICK_E,    ///< East
  SL_JOYSTICK_S,    ///< South
  SL_JOYSTICK_W,    ///< West
} sl_joystick_position_t;

/**
 * @brief Enumeration for the joystick state (ENABLE/DISABLE).
 */
typedef enum {
  SL_JOYSTICK_DISABLED, ///< Joystick data acquisition is disabled.
  SL_JOYSTICK_ENABLED,  ///< Joystick data acquisition is enabled.
} sl_joystick_state_t;

// -----------------------------------------------------------------------------
// Prototypes

/***************************************************************************/
/**
 * @brief To configure and initialize the joystick.
 * 
 * @details This API initializes the hoystick by configuring the ADC to read different voltages based on the Joystick's position.
 * 
 * @return Status code indicating the result:
 *         - SL_STATUS_OK  - Operation is successful.
 *         - SL_STATUS_FAIL  - Function failed.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_joystick_init(void);

/***************************************************************************/
/**
 * @brief To get the direction/position of the joystick.
 *
 * @details This API retrieves the current direction or position of the joystick.
 *
 * @pre Pre-conditions:
 * - \ref sl_si91x_joystick_init 
 * - \ref sl_si91x_joystick_start 
 *
 * @param[in]  state  Joystick enable/disable state, see \ref sl_joystick_state_t.
 * @param[out] pos    Position of the joystick, see \ref sl_joystick_position_t.
 *
 * @return Status code indicating the result:
 *         - SL_STATUS_OK  - Operation is successful.
 *         - SL_STATUS_NOT_READY - Module is not ready for the requested operation (Joystick is in disabled state).
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_joystick_get_position(sl_joystick_state_t state, sl_joystick_position_t *pos);

/***************************************************************************/
/**
 * @brief To start or enable the joystick.
 *
 * @details This API starts or enables the joystick by setting its state to enabled.
 *
 * @pre Pre-conditions:
 * - \ref sl_si91x_joystick_init must be called before this function.
 *
 * @param[in] state Joystick enable/disable state, see \ref sl_joystick_state_t.
 *
 * @return Status code indicating the result:
 *         - SL_STATUS_OK  - Operation is successful.
 *         - SL_STATUS_ABORT - Operation is aborted (Joystick is in disabled state).
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_joystick_start(sl_joystick_state_t state);

/***************************************************************************/
/**
 * @brief To stop or disable the joystick.
 *
 * @details This API stops or disables the joystick by setting its state to disabled.
 *
 * @pre Pre-conditions:
 * - \ref sl_si91x_joystick_init must be called before this function.
 * - \ref sl_si91x_joystick_start must be called before this function.
 *
 * @param[in] state Joystick enable/disable state, see \ref sl_joystick_state_t.
 *
 * @return Status code indicating the result:
 *         - SL_STATUS_OK  - Operation is successful.
 *         - SL_STATUS_BUSY  - Module is busy (Joystick is in enabled state).
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_joystick_stop(sl_joystick_state_t state);

/// @} end group JOYSTICK ********************************************************/

/******** THE REST OF THE FILE IS DOCUMENTATION ONLY !****************************/
/** @addtogroup JOYSTICK Joystick
 *
 * @{
 *
 * @details
 *
 * @n @section joystick_intro Introduction
 *
 * The SI91x joystick driver is a platform-level software module responsible for managing joystick controls.
 * It provides an interface for initializing, starting, stopping, and reading the joystick's position.
 * The joystick module reads different voltage levels based on the joystick's position to determine its orientation.
 *
 * The @ref sl_joystick_state_t enumeration defines the states for enabling or disabling joystick data acquisition.
 * The @ref sl_joystick_position_t enumeration defines the possible positions of the joystick, including not pressed,
 * center, north, east, south, and west. These positions correspond to the physical orientation of the joystick.
 *
 * @li For more information on configuring available parameters, see the respective peripheral example readme document.
 *
 * @section joystick_usage Usage
 * After defining the joystick configuration structures and passing an instance of @ref sl_joystick_state_t and @ref sl_joystick_position_t, the following functions can be used to initiate and configure the joystick module. The typical flow for implementation is as follows:
 * 1. Initialize the joystick: @ref sl_si91x_joystick_init
 * 2. Start joystick data acquisition: @ref sl_si91x_joystick_start
 * 3. Retrieve the joystick position: @ref sl_si91x_joystick_get_position
 * 4. Stop joystick data acquisition: @ref sl_si91x_joystick_stop
 *
 * These functions enable easy integration of joystick controls into applications, providing a consistent interface for reading and managing joystick input.
 * 
 * @} (end addtogroup JOYSTICK)
 */

#ifdef __cplusplus
}
#endif

#endif /* SL_SI91X_JOYSTICK_H_ */
