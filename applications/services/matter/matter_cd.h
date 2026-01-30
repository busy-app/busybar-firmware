/**
 * @file matter_cd.h
 * @brief Matter CD (Certification Declaration) certificate management. Runs on u5.
 */

#pragma once

#include <storage/storage.h>
#include "matter_common_i.h"

/**
 * @warning These fields are strictly for use by `matter_cd.c` only.
 */
typedef struct {
    Storage* storage;
    char wanted_selection[64];
    const char* de_facto_selection;
} MatterCd;

void matter_cd_init(MatterCd* cd);

/**
 * @returns true on success
 */
bool matter_cd_prepare_initialization_frame(MatterCd* cd, MatterIntercomFrame* frame);

const char* matter_cd_get_wanted_selection(MatterCd* cd);

bool matter_cd_set_wanted_selection(MatterCd* cd, const char* selection);

const char* matter_cd_get_de_facto_selection(MatterCd* cd);
