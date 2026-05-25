#include "ble_http_repeater.h"
#include <mongoose.h>
#include <network/network.h>

#define TAG "BleHttp"

#define THREAD_STACK_SIZE (1500)

#define BLE_HTTP_HOST "http://127.0.0.1:80"

#define BLE_HTTP_SESSION_TIMEOUT_ON_TX_CONFIRM_FAIL (4000)

struct BleHttpRepeater {
    FuriMutex* lock;
    struct mg_mgr mgr;
    struct mg_connection* conn;
    FuriThread* thread;
    FuriSemaphore* wait;
    FuriSemaphore* uart_conn_sync;
    Ble* ble;
    bool run;

    FuriMutex* session_lock;
    uint32_t session;
};

static void ble_event_handler(struct mg_connection* conn, int ev, void* ev_data);

static void ble_session_set(BleHttpRepeater* instance, const uint32_t session) {
    furi_mutex_acquire(instance->session_lock, FuriWaitForever);
    instance->session = session;

    FURI_LOG_D(TAG, "Session: %ld", instance->session);
    furi_mutex_release(instance->session_lock);
}

static void ble_session_check_reset(BleHttpRepeater* instance) {
    furi_mutex_acquire(instance->session_lock, FuriWaitForever);
    if(instance->session == 0) {
        FURI_LOG_D(TAG, "Session reset from remote");
        instance->session = 0;
        instance->conn->is_draining = true;
    }
    furi_mutex_release(instance->session_lock);
}

static void ble_session_callback(size_t data_size, void* data, void* context) {
    furi_assert(context);
    do {
        if(data_size != sizeof(uint32_t)) {
            FURI_LOG_W(TAG, "Wrong session data size");
            break;
        }

        const uint32_t session = *((uint32_t*)data);
        ble_session_set(context, session);
    } while(false);
}

static void ble_uart_rx_callback(size_t data_size, void* data, void* context) {
    furi_assert(context);
    BleHttpRepeater* instance = context;
    furi_semaphore_acquire(instance->uart_conn_sync, FuriWaitForever);
    mg_wakeup(&instance->mgr, instance->conn->id, data, data_size);
}

static void ble_uart_tx_done_callback(void* context) {
    furi_assert(context);
    BleHttpRepeater* instance = context;
    furi_semaphore_release(instance->wait);
}

static void ble_event_handler(struct mg_connection* conn, int ev, void* ev_data) {
    BleHttpRepeater* instance = conn->fn_data;

    if(ev == MG_EV_WAKEUP) {
        struct mg_str* data = (struct mg_str*)ev_data;
        mg_send(conn, data->buf, data->len);
        furi_semaphore_release(instance->uart_conn_sync);
    } else if(ev == MG_EV_CONNECT) {
        furi_semaphore_release(instance->uart_conn_sync);
    } else if(ev == MG_EV_READ) {
        size_t total_size = conn->recv.len;
        size_t index = 0;
        while(total_size) {
            size_t send_size = total_size > MAX_TX_CHUNK_SIZE ? MAX_TX_CHUNK_SIZE : total_size;
            ble_uart_tx_data(
                instance->ble, BleUartChannelNordic, &conn->recv.buf[index], send_size);

            if(furi_semaphore_acquire(
                   instance->wait, BLE_HTTP_SESSION_TIMEOUT_ON_TX_CONFIRM_FAIL) != FuriStatusOk) {
                if(!instance->run) break;
                FURI_LOG_W(TAG, "Disconnect due to timeout");
                ble_disconnect(instance->ble);
                break;
            }

            index += send_size;
            total_size -= send_size;
        }
        conn->recv.len = 0;
    } else if(ev == MG_EV_CLOSE) {
        if(!instance->run) return;
        instance->conn = mg_connect(&instance->mgr, BLE_HTTP_HOST, ble_event_handler, instance);
    } else if(ev == MG_EV_ERROR) {
        FURI_LOG_W(TAG, "Error occurred, disconnect from remote");
        if(!instance->run) return;
        ble_disconnect(instance->ble);
    } else if(ev == MG_EV_POLL) {
        ble_session_check_reset(instance);
    }
}

static int32_t ble_http_repeater_thread_handler(void* context) {
    BleHttpRepeater* instance = context;

    ble_uart_set_rx_callback(instance->ble, BleUartChannelNordic, ble_uart_rx_callback, instance);
    ble_uart_set_tx_done_callback(
        instance->ble, BleUartChannelNordic, ble_uart_tx_done_callback, instance);

    ble_uart_set_session_callback(instance->ble, ble_session_callback, instance);
    ble_session_set(instance, 1);

    Network* network = furi_record_open(RECORD_NETWORK);
    network_init_current_thread(network);
    mg_mgr_init(&instance->mgr);
    mg_wakeup_init(&instance->mgr);

    instance->conn = mg_connect(&instance->mgr, BLE_HTTP_HOST, ble_event_handler, instance);

    // Event loop
    while(instance->run) {
        mg_mgr_poll(&instance->mgr, 1000);
    }

    // Cleanup
    ble_uart_set_rx_callback(instance->ble, BleUartChannelNordic, NULL, NULL);
    ble_uart_set_tx_done_callback(instance->ble, BleUartChannelNordic, NULL, NULL);
    mg_mgr_free(&instance->mgr);
    network_deinit_current_thread(network);
    furi_record_close(RECORD_NETWORK);

    return 0;
}

BleHttpRepeater* ble_http_repeater_alloc(Ble* ble) {
    BleHttpRepeater* instance = malloc(sizeof(BleHttpRepeater));
    instance->lock = furi_mutex_alloc(FuriMutexTypeNormal);
    instance->wait = furi_semaphore_alloc(1, 0);
    instance->uart_conn_sync = furi_semaphore_alloc(1, 0);
    instance->ble = ble;

    instance->session_lock = furi_mutex_alloc(FuriMutexTypeNormal);

    instance->thread =
        furi_thread_alloc_ex(TAG, THREAD_STACK_SIZE, ble_http_repeater_thread_handler, instance);
    return instance;
}

static void ble_http_repeater_start(BleHttpRepeater* instance) {
    furi_mutex_acquire(instance->lock, FuriWaitForever);

    if(!instance->run) {
        FURI_LOG_D(TAG, "Http start");
        instance->run = true;
        furi_thread_start(instance->thread);
    }
    furi_mutex_release(instance->lock);
}

static void ble_http_repeater_stop(BleHttpRepeater* instance) {
    furi_mutex_acquire(instance->lock, FuriWaitForever);

    if(instance->run) {
        instance->run = false;
        furi_thread_join(instance->thread);
        FURI_LOG_D(TAG, "Http stopped");
    }
    furi_mutex_release(instance->lock);
}

void ble_http_repeater_free(BleHttpRepeater* instance) {
    furi_assert(instance);
    ble_http_repeater_stop(instance);
    furi_thread_free(instance->thread);
    furi_semaphore_free(instance->wait);
    furi_semaphore_free(instance->uart_conn_sync);
    furi_mutex_free(instance->session_lock);
    furi_mutex_free(instance->lock);
    free(instance);
}

void ble_http_repeater_update(BleHttpRepeater* instance, const BleServiceStatus status) {
    furi_assert(instance);
    furi_assert(status < BleServiceStatusCount);

    if(status == BleServiceStatusConnected)
        ble_http_repeater_start(instance);
    else
        ble_http_repeater_stop(instance);
}
