/**
 * @file timer.h
 * @brief Low-level timers API.
 *
 * The timers found in this file can be used
 * as a convenient means for storing points in time
 * and then checking whether some amount of time
 * (timeout) has passed.
 *
 * CoarseTimer class uses the OS tick counter, so it
 * can only be used when an OS is running, and its precision
 * is limited to that of the OS ticks.
 *
 * PreciseTimer class uses the CPU cycle counter and
 * has microsecond resolution, however, its maximum timeout
 * is limited to several seconds, depending on the underlying
 * implementation and CPU frequency.
 * Additionally, it can be used in critical sections and/or
 * when an OS is not running.
 *
 * Both timer types are only useful for measuring time
 * and waiting, if a function needs to be called periodically,
 * consider using FuriEventLoopTimer if there is a FuriEventLoop
 * available, or a regular FuriTimer instead.
 */
#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup GenericTimer
 */

/**
 * @brief Condition test function type.
 *
 * @param[in,out] context pointer to a user-specific object (may be @c NULL)
 * @returns @c true if the condition has been fulfilled, @c false otherwise
 */
typedef bool (*TimerConditionCallback)(void* context);

/** @} */

/**
 * @defgroup CoarseTimer
 *
 * Timers with a single OS tick precision.
 * Can only be used when the scheduler is running.
 *
 * @{
 */

/**
 * @brief CoarseTimer structure definition.
 */
typedef struct {
    /** Opaque data. The calling code should NOT modify it directly. */
    uint32_t data[2];
} CoarseTimer;

/**
 * @brief Create a coarse timer with the designated timeout.
 *
 * @param[in] timeout_ms timeout value, in milliseconds
 * @returns initialised CoarseTimer object
 */
CoarseTimer coarse_timer_create(uint32_t timeout_ms);

/**
 * @brief Get the time elapsed since the timer creation, in milliseconds.
 *
 * @param[in] timer coarse timer object to query
 * @returns time elapsed since the timer creation, in milliseconds
 */
uint32_t coarse_timer_get_elapsed(const CoarseTimer timer);

/**
 * @brief Check whether a coarse timer is past its timeout.
 *
 * @param[in] timer coarse timer object to check
 * @returns @c true if the timeout has passed, @c false otherwise
 */
bool coarse_timer_is_expired(const CoarseTimer timer);

/** @} */

/**
 * @defgroup PreciseTimer
 *
 * Timers with MICROsecond precision.
 *
 * Can be used anytime anywhere (including critical sections),
 * but the maximum timeout is limited usually to
 * several seconds, depending on the system configuration.
 *
 * Typical applications include (but not limited to):
 * - Delays/waiting in critical sections
 * - Bit-banging input/output
 * - Performance profiling
 *
 * @{
 */

/**
 * @brief PreciseTimer structure definition.
 */
typedef struct {
    /** Opaque data. The calling code should NOT modify it directly. */
    uint32_t data[2];
} PreciseTimer;

/**
 * @brief Create a precise timer with the designated timeout.
 *
 * @param[in] timeout_us timeout value, in microseconds
 * @returns initialised PreciseTimer object
 */
PreciseTimer precise_timer_create(uint32_t timeout_us);

/**
 * @brief Get the time elapsed since the timer creation, in microseconds.
 *
 * @param[in] timer precise timer object to query
 * @returns time elapsed since the timer creation, in microseconds
 */
uint32_t precise_timer_get_elapsed(const PreciseTimer timer);

/**
 * @brief Check whether a precise timer is past its timeout.
 *
 * @param[in] timer precise timer object to check
 * @returns @c true if the timeout has passed, @c false otherwise
 */
bool precise_timer_is_expired(const PreciseTimer timer);

/**
 * @brief Block the current execution context until the timer is expired.
 *
 * @param[in] timer precise timer object to use
 */
void precise_timer_wait(const PreciseTimer timer);

/**
 * @brief Block the caller until the timer is expired OR a condition is reached.
 *
 * @note This function may still report true even if the timer has expired,
 *       e.g. if the function gets interrupted for a long time after calling condition_cb().
 *       This is the intended behaviour, as the calling code usually only cares about
 *       the condition being reached at all, not necessarily within a precise time frame.
 *
 * @param[in] timer precise timer object to use
 * @param[in] condition_cb pointer to the condition check (predicate) function
 * @param[in,out] context pointer to the user-specific object (will be passed to the callback)
 * @returns @c true if the condition has been reached before the timeout, @c false otherwise
 */
bool precise_timer_wait_for(
    const PreciseTimer timer,
    TimerConditionCallback condition_cb,
    void* context);

/** @} */

#ifdef __cplusplus
}
#endif
