#pragma once

#include "interface.h"

#ifdef __cplusplus
extern "C" {
#endif

SETTING_VALIDATE_DECLARATION(type_struct, setting, value);
SETTING_SAVE_DECLARATION(type_struct, json_node, setting, value);
SETTING_LOAD_DECLARATION(type_struct, json_node, setting, value);
SETTING_RESET_DECLARATION(type_struct, json_node, setting, value);

#ifdef __cplusplus
}
#endif
