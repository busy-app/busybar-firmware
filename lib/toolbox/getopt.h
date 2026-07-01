#pragma once

#include <core/string.h>

typedef void (*OptionCallback)(char opt, const char* optval, void* context);

bool getopts(FuriString* args, const char* opts, OptionCallback callback, void* context);
