/*******************************************************************************
 * @file zw_shutdown_manager.h
 * @brief This file contains application specific power manager IDs and abstraction
 *******************************************************************************
 * # License
 * <b> Copyright 2025 Silicon Laboratories Inc. www.silabs.com </b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of the Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * https://www.silabs.com/about-us/legal/master-software-license-agreement
 * By installing, copying or otherwise using this software, you agree to the
 * terms of the MSLA.
 *
 ******************************************************************************/
#ifndef ZW_SHUTDOWN_MANAGER_H
#define ZW_SHUTDOWN_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Function Declarations
// -----------------------------------------------------------------------------

/**
 * @brief Initializes the Z-Wave shutdown manager.
 *
 * This function sets up any necessary resources or state required for the
 * shutdown manager to operate. It should be called during system startup
 * before using any shutdown-related functionality.
 */
void zw_shutdown_manager_init(void);

/**
 * @brief Adds a lock to the Z-Wave shutdown manager.
 *
 * This function increments the internal lock count, preventing the shutdown
 * process from proceeding until all locks have been released. It is typically
 * used to ensure that critical operations are not interrupted by a system shutdown.
 *
 * @note Each call to this function should be paired with a corresponding call
 *       to zw_shutdown_manager_release_lock() to release the lock.
 */
void zw_shutdown_manager_add_lock(void);

/**
 * @brief Takes a temporary lock in the Z-Wave shutdown manager.
 *
 * This function acquires a temporary lock, preventing the shutdown process
 * for a short duration or until the temporary lock is explicitly released.
 * It is useful for short-lived operations that require protection from shutdown.
 *
 * @param duration The duration (in milliseconds) for which the temporary lock should be held.
 *
 * @note This API doesn't support concurrent calls. Each new call will alter
 * the previous request, if timeout is not already elapsed.
 *
 */
void zw_shutdown_manager_take_temporary_lock(uint32_t duration);

/**
 * @brief Removes a previously set shutdown lock from the Z-Wave shutdown manager.
 *
 * This function releases a lock that was set to prevent the Z-Wave shutdown process.
 * After calling this function, the shutdown manager may proceed with the shutdown
 * sequence if no other locks are present.
 *
 * @note Ensure that a corresponding lock was previously set before calling this function.
 */
void zw_shutdown_manager_release_lock(void);

/**
 * @brief Resets the Z-Wave shutdown manager state.
 *
 * This function reinitializes or clears any internal state or resources
 * managed by the Z-Wave shutdown manager, preparing it for a fresh start.
 * It should be called when a full reset of the shutdown management logic
 * is required.
 */
void zw_shutdown_manager_reset(void);

#ifdef __cplusplus
}
#endif

#endif // ZW_SHUTDOWN_MANAGER_H
