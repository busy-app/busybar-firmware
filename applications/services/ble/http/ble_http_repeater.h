/**
 * @file ble_http_repeater.h
 * @brief API for controlling http over ble transmission
 *
 * Performs data exchange between Nordic Uart characteristics from ble service
 * and web server. Runs in a separate thread which starts on enabling ble. 
 * This thread stops on disabling ble. 
 *
 */
#pragma once

#include "../ble.h"

/**
 * @brief Inits repeater, called once during service startup
 */
void ble_http_repeater_init(void);

/**
 * @brief Start repeater
 *
 * Spawns repeater thread, which opens connection to web server
 * over loopback. Called when @ref ble_start is called.
 *
 * @param[in] ble pointer to the ble instance
 */
void ble_http_repeater_start(Ble* ble);

/**
 * @brief Stop repeater
 *
 * Stops repeater thread, blocks until repeater is actually stopped
 * and loopback connection is closed.  Called when @ref BleCommandDisable
 * is performed.
 * 
 */
void ble_http_repeater_stop(void);
