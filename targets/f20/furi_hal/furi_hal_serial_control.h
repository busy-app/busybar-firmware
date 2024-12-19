#pragma once

#include <stdint.h>
#include <stdbool.h>

#include <furi_hal_serial_types.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Initialize Serial Control
 *
 * @note    this method should be called in the early initialization stage
 */
void furi_hal_serial_control_init(void);

/** Deinitialize Serial Control
 *
 * @note    this method should be called in the early de-initialization stage
 */
void furi_hal_serial_control_deinit(void);

/** Suspend All Serial Interfaces
 *
 * @warning    this is internal method, can only be used in suppress tick
 *             callback
 */
void furi_hal_serial_control_suspend(void);

/** Resume All Serial Interfaces
 *
 * @warning    this is internal method, can only be used in suppress tick
 *             callback
 */
void furi_hal_serial_control_resume(void);

/** Acquire Serial Interface Handler
 *
 * @param[in]  serial_id  The serial transceiver identifier
 *
 * @return     The Serial Interface Handle or null if interfaces is in use
 */
FuriHalSerialHandle* furi_hal_serial_control_acquire(FuriHalSerialId serial_id);

/** Release Serial Interface Handler
 *
 * @param      handle  The handle
 */
void furi_hal_serial_control_release(FuriHalSerialHandle* handle);

/** Acquire Serial Interface Handler
 *
 * @param[in]  serial_id  The serial transceiver identifier
 *
 * @return     true if handle is acquired by someone
 */
bool furi_hal_serial_control_is_busy(FuriHalSerialId serial_id);

#ifdef __cplusplus
}
#endif
