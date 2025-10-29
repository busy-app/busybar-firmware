#include "wifi_poller.h"

#define TAG              "WifiPoller"
#define THREAD_NAME      "SettingsWifiPoller"
#define THREAD_STACK     (2048)
#define POLLING_INTERVAL (furi_ms_to_ticks(2000))
#define MUTEX_TIMEOUT    (furi_ms_to_ticks(1000))

struct WifiPoller {
    FuriThread* thread;

    FuriMutex* mutex;
    WifiPollerCallback callback;
    void* cb_context;
    WifiPollerState state;
};

typedef enum {
    WifiPollerFlagStop = (1 << 0),
} WifiPollerFlag;

#define WifiPollerFlagALL (WifiPollerFlagStop)

static WifiPollerState wifi_poller_info_to_state(WifiInfo* info) {
    WifiPollerState state = 0;
    if(info->state == WifiStateUp) state |= WifiPollerStateLinkUp;
    return state;
}

static int32_t wifi_poller_thread(void* context) {
    furi_assert(context);
    WifiPoller* poller = context;
    Wifi* wifi = furi_record_open(RECORD_WIFI);

    while(1) {
        WifiInfo info;
        WifiStatus status = wifi_get_info(wifi, &info);

        if(status == WifiStatusOk) {
            furi_check(furi_mutex_acquire(poller->mutex, MUTEX_TIMEOUT) == FuriStatusOk);

            WifiPollerState new_state = wifi_poller_info_to_state(&info);
            WifiPollerState old_state = poller->state;
            poller->state = new_state;

            if((new_state != old_state) && poller->callback) {
                poller->callback(poller->cb_context, new_state);
            }

            furi_check(furi_mutex_release(poller->mutex) == FuriStatusOk);

        } else {
            FURI_LOG_E(TAG, "wifi_get_info_failed: %d", status);
        }

        WifiPollerFlag flags =
            furi_thread_flags_wait(WifiPollerFlagALL, FuriFlagWaitAny, POLLING_INTERVAL);
        furi_check(!(flags & FuriFlagError));

        if(flags & WifiPollerFlagStop) {
            break;
        }
    }

    return 0;
}

static void wifi_poller_do_free(WifiPoller* poller) {
    furi_assert(poller);
    furi_thread_free(poller->thread);
    furi_mutex_free(poller->mutex);
}

static void wifi_poller_thread_state(FuriThread* thread, FuriThreadState state, void* context) {
    furi_assert(thread);
    furi_assert(context);
    WifiPoller* poller = context;

    if(state == FuriThreadStateStopped) {
        wifi_poller_do_free(poller);
    }
}

WifiPoller* wifi_poller_alloc(void) {
    WifiPoller* poller = malloc(sizeof(WifiPoller));

    poller->mutex = furi_mutex_alloc(FuriMutexTypeNormal);

    poller->thread = furi_thread_alloc_ex(THREAD_NAME, THREAD_STACK, wifi_poller_thread, poller);
    furi_thread_set_state_callback(poller->thread, wifi_poller_thread_state);
    furi_thread_set_state_context(poller->thread, poller);
    furi_thread_start(poller->thread);

    return poller;
}

void wifi_poller_free(WifiPoller* poller) {
    furi_check(poller);
    furi_thread_flags_set(furi_thread_get_id(poller->thread), WifiPollerFlagStop);
    // Actual freeing is done when the thread is `FuriThreadStateStopped`.
    // We don't want to force the user to wait several seconds if they happen to
    // quit Settings when Wifi hasn't done its thing yet.
}

void wifi_poller_set_callback(WifiPoller* poller, WifiPollerCallback callback, void* context) {
    furi_check(poller);
    if(context) furi_check(callback);

    furi_check(furi_mutex_acquire(poller->mutex, MUTEX_TIMEOUT) == FuriStatusOk);
    poller->callback = callback;
    poller->cb_context = context;
    furi_check(furi_mutex_release(poller->mutex) == FuriStatusOk);
}

WifiPollerState wifi_poller_get_state(WifiPoller* poller) {
    furi_check(poller);
    furi_check(furi_mutex_acquire(poller->mutex, MUTEX_TIMEOUT) == FuriStatusOk);
    WifiPollerState result = poller->state;
    furi_check(furi_mutex_release(poller->mutex) == FuriStatusOk);
    return result;
}
