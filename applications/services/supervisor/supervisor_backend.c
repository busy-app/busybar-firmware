#include <furi.h>

#include <furi_hal_random.h>
#include <furi_hal_crypto.h>
#include <furi_hal_crypto_storage.h>

#include <wifi/wifi.h>

#define TAG "Supervisor"

#define SUPERVISOR_POLL_PERIOD_MS     (1000)
#define SUPERVISOR_COOLDOWN_PERIOD_MS (10000)

#define SUPERVISOR_ERROR_THRESHOLD (10)

#define SUPERVISOR_CRYPTO_KEY_ID       (0x11) // Device key
#define SUPERVISOR_CRYPTO_TEST_MSG_LEN (256)

typedef struct {
    FuriEventLoop* event_loop;
    FuriEventLoopTimer* poll_timer;
    FuriEventLoopTimer* cooldown_timer;
    uint32_t error_count;
} Supervisor;

typedef struct {
    const char* name;
    bool (*handler)(void);
} SupervisorPolledHealthCheck;

static bool supervisor_is_tls_crypto_healthy(void) {
    bool is_healthy = false;

    FuriHalCryptoKey* key = furi_hal_crypto_storage_alloc(FuriHalCryptoPartitionMain);

    do {
        const FuriHalCryptoStatus status = furi_hal_crypto_storage_read(
            key, FuriHalCryptoKeyTypeEcdsaPriv256, SUPERVISOR_CRYPTO_KEY_ID);

        if(status != FuriHalCryptoStatusOk) {
            // Special case: report good health if the key is not provisioned
            is_healthy = (status == FuriHalCryptoStatusNotFound);
            break;
        }

        // TODO: Incorporate key wrapping mode into FuriHalCrypto
        const FuriHalCryptoWrappingMode wrap_mode =
            (key->header.flags & FuriHalCryptoKeyFlagWrap) ? FuriHalCryptoWrappingModeOn :
                                                             FuriHalCryptoWrappingModeOff;

        FuriHalCryptoEcdsa* sign_ctx = furi_hal_crypto_ecdsa_sign_init(
            FuriHalCryptoEcdsaModeSha256,
            key->data,
            FURI_HAL_CRYPTO_ECDSA_PRIV_KEY_SIZE_256,
            wrap_mode);

        uint8_t message[SUPERVISOR_CRYPTO_TEST_MSG_LEN];
        furi_hal_random_fill_buf(message, sizeof(message));

        uint8_t signature[FURI_HAL_CRYPTO_ECDSA_MAX_SIGNATURE_SIZE];
        size_t signature_len = sizeof(signature);

        is_healthy = furi_hal_crypto_ecdsa_sign(
            sign_ctx, message, sizeof(message), signature, &signature_len);

        furi_hal_crypto_ecdsa_deinit(sign_ctx);

    } while(false);

    furi_hal_crypto_storage_free(key);

    return is_healthy;
}

static const SupervisorPolledHealthCheck supervisor_polled_health_checks[] = {
    {
        .name = "TlsCrypto",
        .handler = supervisor_is_tls_crypto_healthy,
    },
};

static void supervisor_poll_timer_callback(void* context) {
    furi_assert(context);
    Supervisor* instance = context;

    uint32_t error_count_increment = 0;

    for(uint32_t i = 0; i < COUNT_OF(supervisor_polled_health_checks); ++i) {
        const SupervisorPolledHealthCheck* check = &supervisor_polled_health_checks[i];

        if(!check->handler()) {
            ++error_count_increment;
            FURI_LOG_W(TAG, "Health check failed: %s", check->name);
        }
    }

    if(error_count_increment > 0) {
        instance->error_count += error_count_increment;
        furi_event_loop_timer_start(instance->cooldown_timer, SUPERVISOR_COOLDOWN_PERIOD_MS);
    }

    if(instance->error_count > SUPERVISOR_ERROR_THRESHOLD) {
        furi_crash("Error threshold exceeded");
    }
}

static void supervisor_cooldown_timer_callback(void* context) {
    furi_assert(context);
    Supervisor* instance = context;

    instance->error_count = 0;

    FURI_LOG_D(TAG, "Resetting error count");
}

static Supervisor* supervisor_alloc(void) {
    Supervisor* instance = malloc(sizeof(Supervisor));

    instance->event_loop = furi_event_loop_alloc();
    instance->poll_timer = furi_event_loop_timer_alloc(
        instance->event_loop,
        supervisor_poll_timer_callback,
        FuriEventLoopTimerTypePeriodic,
        instance);
    instance->cooldown_timer = furi_event_loop_timer_alloc(
        instance->event_loop,
        supervisor_cooldown_timer_callback,
        FuriEventLoopTimerTypeOnce,
        instance);

    furi_record_open(RECORD_WIFI);

    furi_event_loop_timer_start(instance->poll_timer, SUPERVISOR_POLL_PERIOD_MS);

    return instance;
}

int32_t supervisor_srv(void* arg) {
    UNUSED(arg);

    Supervisor* instance = supervisor_alloc();
    furi_event_loop_run(instance->event_loop);

    return 0;
}
