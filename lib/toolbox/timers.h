/**
 * @file timers.h
 * @brief Low-level timers API.
 *
 * @see @ref simple-timers for more information.
 */
#pragma once

#include <stdint.h>
#include <stdbool.h>

#include <core/common_defines.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup simple-timers Simple timers
 *
 * Simple timers can be used for conveniently
 * storing points in time and then checking whether
 * a certain amount of time (timeout) has passed.
 *
 * @ref coarse-timers use the OS tick counter, so they
 * can only be used when an OS is running, and the precision
 * is limited to that of the OS ticks.
 *
 * @ref precise-timers use the CPU cycle counter and
 * have microsecond resolution, however, the maximum timeout
 * is limited to several seconds, depending on the underlying
 * implementation and CPU frequency.
 * Unlike @ref coarse-timers, they can be used in critical sections and/or
 * when an OS is not running.
 *
 * Both timer types are only useful for measuring time and waiting.
 * If a callback function needs to be called periodically,
 * consider instead using FuriEventLoopTimer if there is
 * a FuriEventLoop available, or a regular FuriTimer.
 *
 * @{
 */

/**
 * @brief Condition test function type.
 *
 * @param[in,out] context pointer to a user-specific object (may be @c NULL)
 * @returns @c true if the condition has been fulfilled, @c false otherwise
 */
typedef bool (*TimerConditionCallback)(void* context);

/**
 * @defgroup coarse-timers Coarse timers
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
 * @note Passing 0 as @p timeout_ms will cause the timer to expire immediately
 *
 * @param[in] timeout_ms timeout value, in milliseconds
 * @returns initialised CoarseTimer object
 */
CoarseTimer FURI_WARN_UNUSED coarse_timer_create(uint32_t timeout_ms);

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

/** @} coarse-timers */

/**
 * @defgroup precise-timers Precise timers
 *
 * Timers with microsecond (&mu;s) precision.
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
 * @note Passing 0 as @p timeout_ms will cause the timer to expire immediately
 *
 * @param[in] timeout_us timeout value, in microseconds
 * @returns initialised PreciseTimer object
 */
PreciseTimer FURI_WARN_UNUSED precise_timer_create(uint32_t timeout_us);

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
 * @note If the timer has expired, this function will always
 * return @c false even if the callback returns @c true.
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

/** @} precise-timers */

/** @} simple-timers */

#ifdef __cplusplus
}
#endif
