#pragma once

#include <storage/storage.h>

#define CUSTOM_ASSETS_PATH(path) EXT_PATH("apps_assets/custom") "/" path
#define CUSTOM_ANIM_PATH(path)   CUSTOM_ASSETS_PATH("animations") "/" path
#define CUSTOM_IMG_PATH(path)    CUSTOM_ASSETS_PATH("images") "/" path
#define CUSTOM_SOUND_PATH(path)  CUSTOM_ASSETS_PATH("sounds") "/" path
