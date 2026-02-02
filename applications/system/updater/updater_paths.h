#pragma once

#include <storage/storage.h>
#include <toolbox/update_lib/common_vals.h>

#define UPDATER_ROOT_PATH EXT_PATH("update")

#define UPDATER_DEFAULT_DOWNLOAD_PATH UPDATER_ROOT_PATH "/bundle.tar"
#define UPDATER_DEFAULT_STAGING_PATH  UPDATER_ROOT_PATH "/staging"
#define UPDATER_DEFAULT_MANIFEST_PATH UPDATER_DEFAULT_STAGING_PATH "/" UPDATE_CONFIG_FILENAME

#define UPDATER_CHECK_DIRECTORY_PATH UPDATER_ROOT_PATH "/directory.json"
