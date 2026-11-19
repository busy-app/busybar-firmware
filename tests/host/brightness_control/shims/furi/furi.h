#pragma once
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <furi/core/state.h>
#define malloc(size)                   calloc(1, size)
#define furi_assert                    assert
#define furi_check                     assert
#define UNUSED(x)                      ((void)(x))
#define FURI_LOG_D(...)                ((void)0)
#define FURI_LOG_I(...)                ((void)0)
#define FuriWaitForever                UINT32_MAX
#define FuriStatusOk                   0
#define FuriEventLoopEventIn           1
#define FuriEventLoopTimerTypePeriodic 1
#define FuriEventLoopTimerTypeOnce 0
typedef struct FuriEventLoop FuriEventLoop;
typedef struct FuriEventLoopObject FuriEventLoopObject;
typedef struct FuriMessageQueue FuriMessageQueue;
typedef struct FuriEventLoopTimer FuriEventLoopTimer;
FuriEventLoop* furi_event_loop_alloc(void);
FuriMessageQueue* furi_message_queue_alloc(size_t, size_t);
void furi_event_loop_subscribe_message_queue(
    FuriEventLoop*,
    FuriMessageQueue*,
    int,
    void (*)(FuriEventLoopObject*, void*),
    void*);
int furi_message_queue_put(FuriMessageQueue*, const void*, uint32_t);
int furi_message_queue_get(FuriMessageQueue*, void*, uint32_t);
void furi_event_loop_run(FuriEventLoop*);
void* furi_record_open(const char*);
void furi_record_create(const char*, void*);
FuriEventLoopTimer* furi_event_loop_timer_alloc(FuriEventLoop*, void (*)(void*), int, void*);
void furi_event_loop_timer_start(FuriEventLoopTimer*, uint32_t);
void furi_event_loop_timer_stop(FuriEventLoopTimer*);
bool furi_event_loop_timer_is_running(const FuriEventLoopTimer*);
uint32_t furi_get_tick(void);
uint32_t furi_ms_to_ticks(uint32_t);
