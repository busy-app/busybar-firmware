#include "ble_http_repeater.h"
#include <mongoose.h>
#include <network/network.h>

#define TAG "BleHttp"

#define BLE_HTTP_HOST "http://127.0.0.1:80"

#define MAX_TX_CHUNK_SIZE (237)

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
} BleHttpRepeater;

static FuriMutex* ble_http_init_mutex;
static BleHttpRepeater* ble_http_repeater;

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
        FURI_LOG_D(TAG, "Connected");
        furi_semaphore_release(ble_http_repeater->uart_conn_sync);
    } else if(ev == MG_EV_READ) {
        size_t total_size = conn->recv.len;
        size_t index = 0;
        while(total_size) {
            size_t send_size = total_size > MAX_TX_CHUNK_SIZE ? MAX_TX_CHUNK_SIZE : total_size;
            ble_uart_tx_data(
                ble_http->ble, BleUartChannelNordic, &conn->recv.buf[index], send_size);

            if(furi_semaphore_acquire(ble_http->wait, 2000) != FuriStatusOk) {
                FURI_LOG_W(TAG, "Error during send process");
                break;
            }

            index += send_size;
            total_size -= send_size;
        }
        conn->recv.len = 0;
    } else if(ev == MG_EV_CLOSE) {
        if(ble_http->exit) return;
        ble_http->conn =
            mg_connect(&ble_http->mgr, BLE_HTTP_HOST, ble_event_handler, ble_http_repeater);
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

    instance->thread = furi_thread_alloc_ex(TAG, 1024 * 8, ble_http_repeater_thread_handler, NULL);
    return instance;
}

static void ble_http_repeater_free(BleHttpRepeater* instance) {
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
