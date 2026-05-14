/**
 * @file netstat.h
 * Finds out whether the network stack is overloaded
 */

#pragma once

#include <furi.h>

#define NETSTAT_RECOMMENDED_ERROR \
    "Network stack overloaded. Close some connections (HTTP, CLI) and try again later."

typedef enum {
    NetstatLogNever,
    NetstatLogOnOverload,
    NetstatLogMAX,
} NetstatLog;

bool netstat_is_overloaded(NetstatLog log);
