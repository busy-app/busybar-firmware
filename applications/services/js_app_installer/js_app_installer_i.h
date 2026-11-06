#pragma once

#include "js_app_installer.h"
#include "js_app_installer_operation.h"

#include <toolbox/api_lock.h>

#include <js_app/js_app.h>

typedef enum JsAppInstallerCmd {
    JsAppInstallerCmdInvalid = 0xFF,
    JsAppInstallerCmdStage = 0,
    JsAppInstallerCmdInstall,
    JsAppInstallerCmdInstallStaged,
    JsAppInstallerCmdRemove,
    JsAppInstallerCmdCancel,
    JsAppInstallerCmdGetQueue,
    JsAppInstallerCmdMax,
} JsAppInstallerCmd;

typedef struct {
    uint32_t install_key;
    JsApp* staged_app;
    JsApp* installed_app;
} JsAppInstallerStageInfo;

typedef struct {
    JsAppInstallerStageInfo* stage_info;
} JsAppInstallerMsgStage;

typedef struct {
    const char* app_id;
    const JsAppInstallerSourceInfo* source_info;
    uint32_t* item_id;
} JsAppInstallerMsgInstall;

typedef struct {
    uint32_t install_key;
} JsAppInstallerMsgInstallStaged;

typedef struct {
    const char* app_id;
} JsAppInstallerMsgRemove;

typedef struct {
    uint32_t operation_id;
    bool is_clear_all;
} JsAppInstallerMsgCancel;

typedef struct {
    JsAppInstallerOperationInfoCallback callback;
    void* callback_context;
} JsAppInstallerMsgGetQueue;

typedef struct {
    JsAppInstallerCmd cmd;
    JsAppInstallerError* error;
    FuriApiLock api_lock;
    union {
        JsAppInstallerMsgStage stage;
        JsAppInstallerMsgInstall install;
        JsAppInstallerMsgInstallStaged install_staged;
        JsAppInstallerMsgRemove remove;
        JsAppInstallerMsgCancel cancel;
        JsAppInstallerMsgGetQueue get_queue;
    };
} JsAppInstallerMsg;

struct JsAppInstaller {
    FuriEventLoop* event_loop;
    FuriMessageQueue* msg_queue;
    uint32_t staged_install_key;
    FuriString* staged_app_path;
    uint32_t operation_id_counter;
    JsAppInstallerOperationQueue_t operation_queue;
};
