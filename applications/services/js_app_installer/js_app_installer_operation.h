#pragma once

#include "js_app_installer_common.h"

typedef void (*JsAppInstallerOperationStepHandler)(void);

typedef struct {
    JsAppInstallerOperationStepHandler handler;
} JsAppInstallerOperationStep;

typedef struct {
    const JsAppInstallerOperationStep* steps;
} JsAppInstallerOperation;
