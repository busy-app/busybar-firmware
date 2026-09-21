/**
 * @file js_app_launcher.h
 * @brief JS Application Launcher public API.
 */
#pragma once

/**
 * @brief Application ID to start JsAppLauncher via Loader or Desktop.
 */
#define JS_APP_LAUNCHER_APP_ID "js_app_launcher"

/**
 * @brief Special JS Application ID to skip the start menu.
 *
 * Append this string to the end of the JS application ID (argument string)
 * to skip the start menu and go directly to the application.
 */
#define JS_APP_LAUNCHER_FLAG_SKIP_MENU "+"
