#pragma once

#include "js_app_installer.h"

#include <furi.h>
#include <toolbox/api_lock.h>

#include <js_app/js_app.h>

typedef enum JsAppInstallerCmd {
    JsAppInstallerCmdInvalid = 0,
    JsAppInstallerCmdStage,
    JsAppInstallerCmdInstall,

    JsAppInstallerCmdMax
} JsAppInstallerCmd;

typedef struct JsAppInstallerMsgStage {
    JsAppInstallerStageResult* result;
} JsAppInstallerMsgStage;

typedef struct JsAppInstallerMsgInstall {
    uint32_t install_id;

    JsAppInstallerError* error;
} JsAppInstallerMsgInstall;

typedef struct JsAppInstallerMsg {
    JsAppInstallerCmd cmd;

    FuriApiLock api_lock;

    union {
        JsAppInstallerMsgStage stage;
        JsAppInstallerMsgInstall install;
    };
} JsAppInstallerMsg;

typedef struct JsAppInstaller {
    FuriEventLoop* event_loop;
    FuriMessageQueue* msg_queue;

    uint32_t staged_install_id;
} JsAppInstaller;
