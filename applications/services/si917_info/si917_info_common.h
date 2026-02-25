#pragma once

#include <furi.h>
#include "si917_info_client.h"

typedef enum {
    Si917InfoMessageRequest,
    Si917InfoMessageResponse,
} Si917InfoMessage;

typedef struct {
    Si917InfoMessage type;
} Si917InfoRequestMessage;

typedef struct {
    Si917InfoMessage type;
    Si917InfoData data;
} Si917InfoResponseMessage;
