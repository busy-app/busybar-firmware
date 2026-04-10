#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <time.h>

typedef struct RateLimiterLimit {
    uint32_t period_ms;
    uint32_t max_packet_count;
} RateLimiterLimit;

#define RATE_LIMITER_UNLIMITED \
    (RateLimiterLimit) {       \
        0                      \
    }

typedef struct RateLimiter {
    RateLimiterLimit limit;
    time_t last_tick_ms;
    uint32_t sent_since_last_tick;
    bool paused;
} RateLimiter;

typedef bool (*RateLimiterSendCallback)(void* context, bool heartbeat);

RateLimiter rate_limiter_init(RateLimiterLimit limit);

void rate_limiter_set_limit(RateLimiter* instance, RateLimiterLimit limit);

/**
 * Pause or resume.
 *
 * @return previous paused state
 */
bool rate_limiter_set_paused(RateLimiter* instance, bool paused);

uint32_t rate_limiter_send_if_allowed(
    RateLimiter* instance,
    time_t now,
    RateLimiterSendCallback cb,
    void* cb_context,
    bool heartbeat);
