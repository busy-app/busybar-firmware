#pragma once

#include <stdbool.h>

#define RECORD_DESKTOP "desktop"

typedef struct Desktop Desktop;

bool desktop_replace_current_app(Desktop* instance, const char* name);
