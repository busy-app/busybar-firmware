#pragma once

#include <storage/storage.h>

#define BUSY_ASSETS_PATH(path) EXT_PATH("apps_assets/busy") "/" path
#define BUSY_ANIM_PATH(path)   BUSY_ASSETS_PATH("animations") "/" path
#define BUSY_IMG_PATH(path)    BUSY_ASSETS_PATH("images") "/" path
#define BUSY_SOUND_PATH(path)  BUSY_ASSETS_PATH("sounds") "/" path
#define BUSY_LOTTIE_PATH(path) BUSY_ASSETS_PATH("lottie") "/" path
