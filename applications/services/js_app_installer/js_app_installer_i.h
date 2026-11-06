#pragma once

#include "js_app_installer.h"
#include "js_app_installer_operation.h"

#include <toolbox/api_lock.h>

#include <js_app/js_app.h>

typedef enum JsAppInstallerCmd {
    JsAppInstallerCmdInvalid = 0,
    JsAppInstallerCmdStage,
    JsAppInstallerCmdInstall,
    JsAppInstallerCmdMax,
} JsAppInstallerCmd;

typedef struct {
    JsAppInstallerStageResult* result;
} JsAppInstallerMsgStage;

typedef struct {
    uint32_t install_key;
    JsAppInstallerError* error;
} JsAppInstallerMsgInstall;

typedef struct {
    JsAppInstallerCmd cmd;
    FuriApiLock api_lock;
    union {
        JsAppInstallerMsgStage stage;
        JsAppInstallerMsgInstall install;
    };
} JsAppInstallerMsg;

struct JsAppInstaller {
    FuriEventLoop* event_loop;
    FuriMessageQueue* msg_queue;
    uint32_t staged_install_key;
    FuriString* staged_app_path;
    uint32_t operation_id_counter;
    FuriMutex* queue_mutex;
    JsAppInstallerOperationQueue_t operation_queue;
};
