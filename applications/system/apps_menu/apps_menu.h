/**
 * @file apps_menu.h
 * @brief Applications Menu public APIs.
 */
#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Enumeration of AppsMenu operation modes.
 */
typedef enum {
    AppsMenuModeResume, /**< Resume the currently active app (if present) */
    AppsMenuModeShowMenu, /**< Show the applications list and reset the active app */
} AppsMenuMode;

/**
 * @brief Replace the currently running application with AppsMenu.
 *
 * @param[in] mode value from @ref AppsMenuMode to select the operation model
 * @returns @c true if AppsMenu could be started, @c false otherwise
 */
bool apps_menu_start(AppsMenuMode mode);

#ifdef __cplusplus
}
#endif
