#include <furi.h>

#include <back_display/back_display.h>

static bool soft_off_signal_callback(uint32_t signal, void* arg, void* context) {
    UNUSED(arg);
    furi_check(context);
    FuriSemaphore* exit_semaphore = context;

    if(signal == FuriSignalExit) {
        // return value ignored in case we receive multiple exit signals before
        // the thread processes them
        furi_semaphore_release(exit_semaphore);
        return true;
    }

    return false;
}

int32_t soft_off_app(void* arg) {
    UNUSED(arg);

    BackDisplaySrv* back_display = furi_record_open(RECORD_BACK_DISPLAY);

    back_display_sleep_mode(back_display, true);

    FuriSemaphore* exit_semaphore = furi_semaphore_alloc(1, 0);
    furi_thread_set_signal_callback(furi_thread_get_current(), soft_off_signal_callback, exit_semaphore);
    furi_check(furi_semaphore_acquire(exit_semaphore, FuriWaitForever) == FuriStatusOk);

    back_display_sleep_mode(back_display, false);

    furi_record_close(RECORD_BACK_DISPLAY);

    return 0;
}
