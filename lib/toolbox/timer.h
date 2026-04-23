/**
 * @file timer.h
 * @brief TBD
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
 * @returns @c true if the condition has been fulfilled, @c false otherwise
 */
typedef bool (*TimerConditionCallback)(void);

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
 * @brief Create a coarse timer that is synced to the previous one.
 *
 * The created timer will expire exactly @c tiemout_ms milliseconds after the
 * @c previous timer expiration time.
 *
 * @param[in] previous previous coarse timer object to use as the starting point
 * @param[in] timeout_ms timeout value, in milliseconds
 * @returns initialised CoarseTimer object
 */
CoarseTimer coarse_timer_create_synced(const CoarseTimer previous, uint32_t timeout_ms);

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
    const uint32_t data[2];
} PreciseTimer;

/**
 * @brief Create a precise timer with the designated timeout.
 *
 * @param[in] timeout_us timeout value, in microseconds
 * @returns initialised PreciseTimer object
 */
PreciseTimer precise_timer_create(uint32_t timeout_us);

/**
 * @brief Create a precise timer that is synced to the previous one.
 *
 * The created timer will expire exactly @c tiemout_us microseconds after the
 * @c previous timer expiration time.
 *
 * @param[in] previous previous precise timer object to use as the starting point
 * @param[in] timeout_ms timeout value, in microseconds
 * @returns initialised PreciseTimer object
 */
PreciseTimer precise_timer_create_synced(const PreciseTimer previous, uint32_t timeout_us);

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
 * @returns @c true if the condition has been reached before the timeout, @c false otherwise
 */
bool precise_timer_wait_for(const PreciseTimer timer, TimerConditionCallback condition_cb);

/** @} */

#ifdef __cplusplus
}
#endif
