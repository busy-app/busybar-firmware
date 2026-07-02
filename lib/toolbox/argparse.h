/**
 * @file argparse.h
 * @brief Command line argument parsing library.
 */
#pragma once

#include <core/string.h>

typedef void (*OptionCallback)(char opt, const char* optarg, void* context);

bool parse_args(FuriString* args, const char* opts, OptionCallback callback, void* context);
