#pragma once

#include <mongoose.h>

void mg_init_early(void);

struct mg_fs* http_fs_get(void);
