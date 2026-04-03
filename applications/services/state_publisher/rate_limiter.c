#include "rate_limiter.h"

RateLimiter rate_limiter_init(RateLimiterLimit limit) {
    return (RateLimiter){
        .limit = limit,
        .last_tick_ms = 0,
        .sent_since_last_tick = 0,
    };
}

void rate_limiter_set_limit(RateLimiter* instance, RateLimiterLimit limit) {
    instance->limit = limit;
}

uint32_t rate_limiter_send_if_allowed(
    RateLimiter* instance,
    time_t now,
    RateLimiterSendCallback cb,
    void* cb_context,
    bool heartbeat) {
    bool send = false;
    if(now - instance->last_tick_ms >= instance->limit.period_ms) {
        send = true;
        instance->last_tick_ms = now;
        instance->sent_since_last_tick = 0;
    } else if(instance->sent_since_last_tick < instance->limit.max_packet_count) {
        send = true;
    }

    uint32_t sleep_time_ms = UINT32_MAX;

    if(send) {
        if(cb(cb_context, heartbeat)) {
            instance->sent_since_last_tick += 1;
        }
    } else {
        // rate limited
        sleep_time_ms = instance->last_tick_ms + instance->limit.period_ms - now;
    }
    return sleep_time_ms;
}
