#pragma once

#include <stdbool.h>

bool http_method_is_valid(const char* method_str);

void http_method_normalize(char* method_str);
