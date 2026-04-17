/***************************************************************************/ /**
 * @file
 * @brief CPC GPIO Endpoint of the Secondary
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

#ifndef SL_CPC_GPIO_EXPANDER_H
#define SL_CPC_GPIO_EXPANDER_H

#include <stdint.h>
#include "sl_status.h"
#include "em_gpio.h"

/***************************************************************************//**
 * @addtogroup cpc_gpio_secondary CPC GPIO Secondary
 * @brief CPC GPIO Expander Secondary Device API
 * @details
 * The CPC GPIO Secondary API provides functions to initialize and manage
 * the GPIO endpoint on the secondary device (Silicon Labs microcontroller).
 * This allows a CPC host (primary device) to discover and control GPIO pins
 * through the CPC protocol.
 *
 * ## Overview
 *
 * The CPC GPIO Expander enables a CPC host to access GPIO pins on a
 * Silicon Labs microcontroller (secondary device) through the Co-Processor
 * Communication (CPC) protocol. The secondary device exposes GPIO pins as
 * a CPC endpoint that can be discovered and controlled by the host.
 *
 * ## Initialization
 *
 * The component automatically registers event handlers that integrate with
 * the Silicon Labs platform's service initialization system (`sl_main`):
 *
 * - **`service_init` event**: Automatically calls `sl_cpc_gpio_expander_init()`
 *   during system initialization via `sl_main_init()`
 * - **`service_process_action` event**: Automatically calls
 *   `sl_cpc_gpio_expander_process_action()` during the main loop via
 *   `sl_main_process_action()`
 *
 * You do **not** need to manually call these functions - they are handled
 * automatically by the platform's service system.
 *
 * ## Processing Commands
 *
 * The method for processing incoming GPIO commands depends on whether a
 * kernel/RTOS is present:
 *
 * **Bare-metal Applications:**
 * The `sl_main_process_action()` function must be called in the main loop,
 * which automatically calls `sl_cpc_gpio_expander_process_action()` through
 * the event handler system.
 *
 * **Kernel/RTOS Applications:**
 * When using a kernel or RTOS, processing is handled automatically by a
 * background task created during initialization. The `service_process_action`
 * event handler is not used in RTOS applications.
 *
 * This function handles all GPIO operations including:
 * - Reading GPIO values
 * - Setting GPIO values
 * - Configuring GPIO pins (bias, drive mode)
 * - Setting GPIO direction (input/output)
 *
 * ## Example Usage
 *
 * **Bare-metal Application:**
 *
 * ```c
 * #include "sl_main_init.h"
 * #include "sl_main_process_action.h"
 *
 * int main(void)
 * {
 *   // Automatically calls sl_cpc_gpio_expander_init() via service_init event
 *   sl_main_init();
 *
 *   app_init();
 *
 *   while (1) {
 *     // Automatically calls sl_cpc_gpio_expander_process_action() 
 *     // via service_process_action event
 *     sl_main_process_action();
 *
 *     app_process_action();
 *   }
 * }
 * ```
 *
 * **Kernel/RTOS Application:**
 *
 * ```c
 * #include "sl_main_init.h"
 *
 * int main(void)
 * {
 *   // Automatically calls sl_cpc_gpio_expander_init() via service_init event
 *   // Background task is created automatically for processing
 *   sl_main_init();
 *
 *   app_init();
 *
 *   while (1) {
 *     app_process_action();
 *   }
 * }
 * ```
 *
 * > **Note:** You do not need to include `sl_cpc_gpio_expander.h` or manually
 * > call the initialization/process functions. These are handled automatically
 * > by the platform's service system.
 *
 * @{
 ******************************************************************************/

/*******************************************************************************
 **************************   STRUCTS   ****************************************
 ******************************************************************************/

/// @brief Struct representing a CPC GPIO expander handle.
/// @details
/// This structure contains the configuration for a GPIO pin that will be
/// exposed to the CPC host (primary device) through the CPC GPIO endpoint.
typedef struct {
  const GPIO_Port_TypeDef port;  ///< GPIO port (e.g., gpioPortA, gpioPortB)
  const uint32_t pin;            ///< GPIO pin number (0-15)
  const char *name;              ///< Human-readable name for the GPIO (e.g., "LED1", "BUTTON1")
  uint32_t config;               ///< GPIO configuration flags (bias, drive mode, etc.)
} sl_cpc_gpio_expander_handle_t;

#ifdef __cplusplus
extern "C"
{
#endif

/***************************************************************************//**
 * @brief Initialize the CPC GPIO endpoint on the secondary device.
 *
 * @details
 * This function initializes the CPC GPIO endpoint, allowing the CPC host
 * (primary device) to discover and control GPIO pins on the secondary device.
 * The function performs the following operations:
 *
 * - Opens the CPC GPIO endpoint
 * - Registers command handlers for GPIO operations
 * - Prepares the GPIO hardware for remote control
 * - Initializes internal state and buffers
 *
 * @note
 * This function is automatically called by the platform's service initialization
 * system (`sl_main`) through the `service_init` event handler. You do **not**
 * need to call this function manually.
 *
 * @note
 * The GPIO pins that will be exposed to the host are configured through
 * the component configuration system. See `sl_cpc_gpio_expander_gpio_inst_config.h`
 * for GPIO pin definitions.
 *
 * @return
 * - SL_STATUS_OK on success
 * - SL_STATUS_FAIL if initialization fails
 * - SL_STATUS_NOT_READY if CPC is not initialized
 * - Other error codes if endpoint cannot be opened
 *
 * @note
 * This function is thread-safe and can be called from any context.
 *
 * @warning
 * Do not call this function multiple times without first deinitializing
 * the endpoint (if deinitialization is supported in future versions).
 ******************************************************************************/
sl_status_t sl_cpc_gpio_expander_init(void);

/***************************************************************************//**
 * @brief Process incoming GPIO commands from the host.
 *
 * @details
 * This function processes incoming GPIO commands from the CPC host (primary
 * device) through the CPC endpoint.
 *
 * **Bare-metal Applications:**
 * This function is automatically called by `sl_main_process_action()` through
 * the `service_process_action` event handler. You do **not** need to call
 * this function manually - just ensure `sl_main_process_action()` is called
 * in your main loop.
 *
 * **Kernel/RTOS Applications:**
 * When using a kernel or RTOS, this function is called automatically by a
 * background task created during initialization. You do not need to call
 * this function manually in kernel/RTOS applications.
 *
 * The function handles the following command types:
 * - Get GPIO value (read pin state)
 * - Set GPIO value (write pin state)
 * - Set GPIO configuration (bias, drive mode)
 * - Set GPIO direction (input/output)
 * - Get GPIO information (name, count, etc.)
 *
 * @note
 * This function is non-blocking and returns immediately if there are no
 * pending commands. In bare-metal applications, it is called automatically
 * when `sl_main_process_action()` is invoked in the main loop.
 *
 * @note
 * The function processes one command per call. If multiple commands are
 * queued, multiple calls may be needed to process all commands.
 *
 * @note
 * This function is thread-safe and can be called from any context, but
 * in bare-metal applications it is called from the main application context
 * through the event handler system.
 ******************************************************************************/
void sl_cpc_gpio_expander_process_action(void);

#ifdef __cplusplus
}
#endif

/** @} (end addtogroup cpc_gpio_secondary) */

#endif /* SL_CPC_GPIO_EXPANDER_H */
