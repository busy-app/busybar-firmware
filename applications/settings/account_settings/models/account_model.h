/**
 * @brief
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <furi.h>

#define ACCOUNT_MODEL_LINK_PIN_LEN 4

typedef struct AccountModel AccountModel;

typedef enum {
    AccountModelStateNotConnected, // Not connected to MQTT broker
    AccountModelStateConnectedNotLinked, // Connected to MQTT broker, not linked
    AccountModelStateConnectedLinked, // Connected to MQTT broker, linked
} AccountModelState;

typedef enum {
    AccountModelEventStateChange,
    AccountModelEventPinGot,
    AccountModelEventPinTimeout,
    AccountModelEventLinkDone,
    AccountModelEventUnlinked,
} AccountModelEvent;

typedef void (*AccountModelEventCallback)(AccountModelEvent event, const char* pin, void* context);

AccountModel* account_model_alloc(void);

void account_model_free(AccountModel* model);

AccountModelState account_model_get_state(AccountModel* model);

bool account_model_is_linked(AccountModel* model);

void account_model_get_email(AccountModel* model, FuriString* email);

void account_model_set_event_callback(
    AccountModel* model,
    AccountModelEventCallback callback,
    void* context);

void account_model_unlink(AccountModel* model);

void account_model_request_link_pin(AccountModel* model);

#ifdef __cplusplus
}
#endif
