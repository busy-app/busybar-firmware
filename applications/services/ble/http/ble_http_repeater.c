#include "ble_http_repeater.h"
#include <mongoose.h>
#include <network/network.h>

#define TAG "BleHttp"

#define BLE_HTTP_HOST "http://127.0.0.1:80"

#define BLE_HTTP_SESSION_TIMEOUT_ON_TX_CONFIRM_FAIL (4000)

typedef struct {
    struct mg_mgr mgr;
    struct mg_connection* conn;
    FuriThread* thread;
    FuriSemaphore* wait;
    FuriSemaphore* uart_conn_sync;
    Ble* ble;
    Network* network;
    bool exit;
    FuriString* debug;

    FuriMutex* session_lock;
    uint32_t current_request_num;
    uint32_t previous_request_num;
} BleHttpRepeater;

static FuriMutex* ble_http_init_mutex;
static BleHttpRepeater* ble_http_repeater;

static void ble_event_handler(struct mg_connection* conn, int ev, void* ev_data);

static void ble_session_reset(BleHttpRepeater* instance) {
    furi_mutex_acquire(instance->session_lock, FuriWaitForever);
    instance->current_request_num = 0;
    instance->previous_request_num = 0;
    instance->conn->is_draining = true;
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
        if(session != 0) {
            FURI_LOG_W(TAG, "Session not 0, ignore");
            break;
        }

        FURI_LOG_W(TAG, "Session reset from remote");
        ble_session_reset(context);
    } while(false);
}

static void ble_session_update_on_open(BleHttpRepeater* instance) {
    furi_mutex_acquire(instance->session_lock, FuriWaitForever);
    if(instance->previous_request_num == instance->current_request_num)
        instance->current_request_num += 1;
    FURI_LOG_D(TAG, "Session: %ld", instance->current_request_num);
    furi_mutex_release(instance->session_lock);
}

static void ble_session_update_on_close(BleHttpRepeater* instance) {
    furi_mutex_acquire(instance->session_lock, FuriWaitForever);

    instance->conn =
        mg_connect(&instance->mgr, BLE_HTTP_HOST, ble_event_handler, ble_http_repeater);

    ble_uart_session_set_value(instance->ble, instance->current_request_num);
    instance->previous_request_num = instance->current_request_num;
    furi_mutex_release(instance->session_lock);
}

static void ble_uart_rx_callback(size_t data_size, void* data, void* context) {
    furi_assert(context);
    BleHttpRepeater* instance = context;
    furi_semaphore_acquire(ble_http_repeater->uart_conn_sync, FuriWaitForever);
    mg_wakeup(&instance->mgr, instance->conn->id, data, data_size);
}

static void ble_uart_tx_done_callback(void* context) {
    furi_assert(context);
    BleHttpRepeater* instance = context;
    furi_semaphore_release(instance->wait);
}

static void ble_event_handler(struct mg_connection* conn, int ev, void* ev_data) {
    BleHttpRepeater* ble_http = ble_http_repeater;

    if(ev == MG_EV_WAKEUP) {
        struct mg_str* data = (struct mg_str*)ev_data;
        mg_send(conn, data->buf, data->len);
        furi_semaphore_release(ble_http_repeater->uart_conn_sync);
    } else if(ev == MG_EV_CONNECT) {
        ble_session_update_on_open(ble_http);
        furi_semaphore_release(ble_http->uart_conn_sync);
    } else if(ev == MG_EV_READ) {
        size_t total_size = conn->recv.len;
        size_t index = 0;
        while(total_size) {
            size_t send_size = total_size > MAX_TX_CHUNK_SIZE ? MAX_TX_CHUNK_SIZE : total_size;
            ble_uart_tx_data(
                ble_http->ble, BleUartChannelNordic, &conn->recv.buf[index], send_size);

            if(furi_semaphore_acquire(
                   ble_http->wait, BLE_HTTP_SESSION_TIMEOUT_ON_TX_CONFIRM_FAIL) != FuriStatusOk) {
                FURI_LOG_W(TAG, "Session reset by timeout");
                ble_session_reset(ble_http);
                break;
            }

            index += send_size;
            total_size -= send_size;
        }
        conn->recv.len = 0;
    } else if(ev == MG_EV_CLOSE) {
        if(ble_http->exit) return;
        ble_session_update_on_close(ble_http);
    }
}

static int32_t ble_http_repeater_thread_handler(void* p) {
    UNUSED(p);
    network_init_current_thread(ble_http_repeater->network);

    mg_mgr_init(&ble_http_repeater->mgr);
    mg_wakeup_init(&ble_http_repeater->mgr);
    ble_http_repeater->debug = furi_string_alloc();

    ble_http_repeater->conn =
        mg_connect(&ble_http_repeater->mgr, BLE_HTTP_HOST, ble_event_handler, ble_http_repeater);

    // Event loop
    while(!ble_http_repeater->exit) {
        mg_mgr_poll(&ble_http_repeater->mgr, 1000);
    }

    // Cleanup
    furi_string_free(ble_http_repeater->debug);
    FURI_LOG_D(TAG, "Ble repeater stopped");
    mg_mgr_free(&ble_http_repeater->mgr);
    network_deinit_current_thread(ble_http_repeater->network);

    return 0;
}

static BleHttpRepeater* ble_http_repeater_alloc(Ble* ble) {
    BleHttpRepeater* instance = malloc(sizeof(BleHttpRepeater));
    instance->wait = furi_semaphore_alloc(1, 0);
    instance->uart_conn_sync = furi_semaphore_alloc(1, 0);
    instance->ble = ble;

    ble_uart_set_rx_callback(ble, BleUartChannelNordic, ble_uart_rx_callback, instance);
    ble_uart_set_tx_done_callback(ble, BleUartChannelNordic, ble_uart_tx_done_callback, instance);

    instance->session_lock = furi_mutex_alloc(FuriMutexTypeNormal);
    furi_mutex_acquire(instance->session_lock, FuriWaitForever);
    ble_uart_set_session_callback(ble, ble_session_callback, instance);
    instance->current_request_num = 0;
    instance->previous_request_num = 0;
    furi_mutex_release(instance->session_lock);

    instance->thread = furi_thread_alloc_ex(TAG, 1024 * 8, ble_http_repeater_thread_handler, NULL);
    return instance;
}

static void ble_http_repeater_free(BleHttpRepeater* instance) {
    ble_uart_set_rx_callback(instance->ble, BleUartChannelNordic, NULL, NULL);
    ble_uart_set_tx_done_callback(instance->ble, BleUartChannelNordic, NULL, NULL);
    furi_thread_free(instance->thread);
    furi_semaphore_free(instance->wait);
    furi_record_close(RECORD_NETWORK);
    free(instance);
}

void ble_http_repeater_init() {
    furi_assert(ble_http_init_mutex == NULL);
    ble_http_init_mutex = furi_mutex_alloc(FuriMutexTypeNormal);

    Network* network = furi_record_open(RECORD_NETWORK);
    network_init_current_thread(network);
}

void ble_http_repeater_start(Ble* ble) {
    if(furi_mutex_acquire(ble_http_init_mutex, 100) == FuriStatusOk) {
        if(ble_http_repeater == NULL) {
            FURI_LOG_D(TAG, "Start ble repeater");
            ble_http_repeater = ble_http_repeater_alloc(ble);
            furi_thread_start(ble_http_repeater->thread);
        }
        furi_mutex_release(ble_http_init_mutex);
    }
}

void ble_http_repeater_stop() {
    if(furi_mutex_acquire(ble_http_init_mutex, 100) == FuriStatusOk) {
        if(ble_http_repeater != NULL) {
            FURI_LOG_D(TAG, "Stop ble repeater");
            ble_http_repeater->exit = true;
            furi_thread_join(ble_http_repeater->thread);
            ble_http_repeater_free(ble_http_repeater);
            ble_http_repeater = NULL;
        }
        furi_mutex_release(ble_http_init_mutex);
    }
}
