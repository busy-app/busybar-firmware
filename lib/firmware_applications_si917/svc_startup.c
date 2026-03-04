#include "applications.h"

#include <flipper.h>
#include <furi.h>

#define TAG "SvcStartup"

static int32_t startup_hook_thread_callback(void* context) {
    const FlipperInternalOnStartHook* hook = context;
    hook->callback();
    return 0;
}

static void
    startup_hook_thread_state_callback(FuriThread* thread, FuriThreadState state, void* context) {
    furi_assert(thread);
    UNUSED(context);

    if(state == FuriThreadStateStopped) {
        furi_thread_free(thread);
    }
}

static void run_service(const FlipperInternalApplication* service) {
    FURI_LOG_D(TAG, "Starting service %s", service->name);

    FuriThread* thread =
        furi_thread_alloc_service(service->name, service->stack_size, service->app, NULL);
    furi_thread_set_appid(thread, service->appid);
    furi_thread_start(thread);
}

static void run_startup_hook(const FlipperInternalOnStartHook* hook) {
    FuriThread* hook_thread = furi_thread_alloc_ex(
        "Hook thread", hook->stack_size, startup_hook_thread_callback, (void*)hook);
    furi_thread_set_state_callback(hook_thread, startup_hook_thread_state_callback);
    furi_thread_start(hook_thread);
}

static void run_all_services(void) {
    FURI_LOG_I(TAG, "Starting %d services", FLIPPER_SERVICES_COUNT);

    for(size_t i = 0; i < FLIPPER_SERVICES_COUNT; i++) {
        run_service(&FLIPPER_SERVICES[i]);
    }
}

static void run_all_startup_hooks(void) {
    FURI_LOG_I(TAG, "Running %d startup hooks", FLIPPER_ON_SYSTEM_START_COUNT);

    for(size_t i = 0; i < FLIPPER_ON_SYSTEM_START_COUNT; i++) {
        run_startup_hook(&FLIPPER_ON_SYSTEM_START[i]);
    }
}

// Redefinition of a global weak symbol in flipper.c
void flipper_init_services(void) {
    run_all_services();
    run_all_startup_hooks();
}
