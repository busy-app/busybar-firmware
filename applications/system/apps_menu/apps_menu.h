/**
 * @file apps_menu.h
 * @brief Applications Menu public APIs.
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Request AppsMenu to forget the most recent application.
 *
 * Next time the AppsMenu is run, it will show the applications menu
 * instead of going directly into the most recent application.
 */
void apps_menu_forget_current_app(void);

#ifdef __cplusplus
}
#endif
