/***************************************************************************//**
 * @file
 * @brief UI interface for Zigbee apps (MemLCD)
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#ifndef SL_ZIGBEE_UI_H
#define SL_ZIGBEE_UI_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @defgroup zigbee-ui Zigbee UI
 * @ingroup component
 * @brief API and Callbacks for the Zigbee UI Component
 *
 * Zigbee UI uses the underlying DMD interface and DMD/GLIB and exposes several
 * wrapper functions to an application. These functions draw the Silicon Labs
 * banner, Zigbee logo, a light on/off glyph, and short network status text.
 */

/**
 * @addtogroup zigbee-ui
 * @{
 */

/******************************************************************************/
/**
 *
 ******************************************************************************/

#define UI_PJOIN_EVENT_DURATION           (250)

/*******************************************************************************
 ********************************   ENUMS   ************************************
 ******************************************************************************/

/**
 * @brief
 *   Logical light state used by the UI.
 */
typedef enum {
  UI_LIGHT_OFF,
  UI_LIGHT_ON,
  UI_LIGHT_UNCHANGED
} sl_zigbee_ui_light_state_t;

/**
 * @brief
 *   Short Zigbee network states rendered near the Zigbee logo.
 */
typedef enum {
  UI_NO_NETWORK,
  UI_SCANNING,
  UI_JOINING,
  UI_FORMING,
  UI_NETWORK_UP,
  UI_LOST_NETWORK,
  UI_DISCOVERING,
  UI_STATE_UNKNOWN
} sl_zigbee_ui_zigbee_network_state_t;

/*******************************************************************************
 ******************************   PROTOTYPES   *********************************
 ******************************************************************************/

/**
 * @name API
 * @{
 */

/**
 * @brief
 *   Initializes the UI component across application init levels.
 *
 *   Typical usage by init level:
 *   - SL_ZIGBEE_INIT_LEVEL_EVENT: initializes internal UI event(s).
 *   - SL_ZIGBEE_INIT_LEVEL_LOCAL_DATA: resets local UI state.
 *   - SL_ZIGBEE_INIT_LEVEL_DONE: initializes DMD/GLIB and draws the first page.
 *
 * @param[in] init_level
 *   SL_ZIGBEE_INIT_LEVEL_EVENT or SL_ZIGBEE_INIT_LEVEL_LOCAL_DATA
 *   or SL_ZIGBEE_INIT_LEVEL_DONE.
 *
 * @return
 *   void
 */
void sl_zigbee_ui_init(uint8_t init_level);

/**
 * @brief
 *   Updates the display to light off bitmap.
 *
 *   This function clears the display area and re-renders the LCD with the
 *   light-off bitmap along with the banner and Zigbee logo.
 *
 * @return
 *   void
 */
void sl_zigbee_ui_light_off(void);

/**
 * @brief
 *   Updates the display to light on bitmap.
 *
 *   This function clears the display area and re-renders the LCD with the
 *   light-on bitmap along with the banner and Zigbee logo.
 *
 * @return
 *   void
 */
void sl_zigbee_ui_light_on(void);

/**
 * @brief
 *   Updates the status of both the lights on LCD.
 *
 *   If there is no status change for a light, pass UI_LIGHT_UNCHANGED for that
 *   argument to keep its previous state.
 *
 * @param[in] light1
 *   New state for light #1 (or UI_LIGHT_UNCHANGED).
 *
 * @param[in] light2
 *   New state for light #2 (or UI_LIGHT_UNCHANGED).
 *
 * @return
 *   void
 */
void sl_zigbee_ui_light_update(sl_zigbee_ui_light_state_t light1,
                               sl_zigbee_ui_light_state_t light2);

/**
 * @brief
 *   Displays a flashing status to indicate that permit-join is active.
 *
 *   A short periodic event toggles the status text while permit-join is
 *   enabled. The flashing sequence stops automatically once permit-join is
 *   disabled.
 *
 * @param[in] enable
 *   true to enable the effect, false to disable it.
 *
 * @return
 *   void
 */
void sl_zigbee_ui_zigbee_permit_join(bool enable);

/**
 * @brief
 *   Displays the provided Zigbee network state text.
 *
 *   If UI_STATE_UNKNOWN is passed, the component determines the current state
 *   internally (e.g., shows PAN:XXXX when joined).
 *
 * @param[in] state
 *   Value from sl_zigbee_ui_zigbee_network_state_t to render.
 *
 * @return
 *   void
 */
void sl_zigbee_ui_display_zigbee_state(sl_zigbee_ui_zigbee_network_state_t state);

/**
 * @brief
 *   Updates the display with a simple help menu.
 *
 *   Renders a short help page describing push-button actions and network hints.
 *
 * @return
 *   void
 */
void sl_zigbee_ui_display_help(void);

/** @} */ // end of name API
/** @} */ // end of zigbee-ui

#endif // SL_ZIGBEE_UI_H
