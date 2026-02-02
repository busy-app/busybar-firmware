#pragma once

#include <furi.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    FuriString* url;
    FuriString* id;
    FuriString* version;
    FuriString* sha256;
    FuriString* changelog;
} UpdateMetadata;

bool update_parser_metadata_parse(
    const char* file_path,
    const char* branch_id,
    UpdateMetadata* metadata);

#ifdef __cplusplus
}
#endif
