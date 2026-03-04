#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <cli/cli_command_min.h>

#ifndef COUNT_OF
#define COUNT_OF(x) (sizeof(x) / sizeof(x[0]))
#endif

typedef int32_t (*FuriThreadCallback)(void* context);

typedef enum {
    FlipperInternalApplicationFlagDefault = 0,
    FlipperInternalApplicationFlagInsomniaSafe = (1 << 0),
} FlipperInternalApplicationFlag;

typedef struct {
    const FuriThreadCallback app;
    const char* name;
    const char* appid;
    const size_t stack_size;
    const FlipperInternalApplicationFlag flags;
} FlipperInternalApplication;

typedef struct {
    const char* name;
    const char* path;
} FlipperExternalApplication;

typedef void (*FlipperInternalOnStartHook)(void);

typedef struct {
    const CliCommandExecuteCallback callback;
    const char* name;
    const size_t stack_size;
    const CliCommandFlag flags;
} FlipperInternalCommandApplication;

extern const char* FLIPPER_AUTORUN_APP_NAME;

/* Services list
 * Spawned on startup
 */
extern const FlipperInternalApplication FLIPPER_SERVICES[];
extern const size_t FLIPPER_SERVICES_COUNT;

/* Apps list
 * Spawned by loader
 */
extern const FlipperInternalApplication FLIPPER_APPS[];
extern const size_t FLIPPER_APPS_COUNT;

/* On system start hooks
 * Spawned by loader in separate threads right after OS initialization is complete
 */
extern const FlipperInternalOnStartHook FLIPPER_ON_SYSTEM_START[];
extern const size_t FLIPPER_ON_SYSTEM_START_COUNT;

/* System apps
 * Can only be spawned by loader by name
 */
extern const FlipperInternalApplication FLIPPER_SYSTEM_APPS[];
extern const size_t FLIPPER_SYSTEM_APPS_COUNT;

/* Debug apps 
 * Can only be spawned by loader by name
 */
extern const FlipperInternalApplication FLIPPER_DEBUG_APPS[];
extern const size_t FLIPPER_DEBUG_APPS_COUNT;

extern const FlipperInternalApplication FLIPPER_ARCHIVE;

/* Settings list
 * Spawned by loader
 */
extern const FlipperInternalApplication FLIPPER_SETTINGS_APPS[];
extern const size_t FLIPPER_SETTINGS_APPS_COUNT;

/* External Menu Apps list
 * Spawned by loader
 */
extern const FlipperExternalApplication FLIPPER_EXTERNAL_APPS[];
extern const size_t FLIPPER_EXTERNAL_APPS_COUNT;

/* Internal CLI commands
 * Added to main CLI registry by CLI startup hook
 */
extern const FlipperInternalCommandApplication FLIPPER_CLI_COMMANDS[];
extern const size_t FLIPPER_CLI_COMMANDS_COUNT;
