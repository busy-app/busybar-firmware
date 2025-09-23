#pragma once

#include <stdbool.h>

typedef struct ParseUpdateJson ParseUpdateJson;

#ifdef __cplusplus
extern "C" {
#endif

ParseUpdateJson* parse_update_init(void);
bool parse_update_json(ParseUpdateJson* instance, const char* file_path, const char* branch_id);
void parse_update_free(ParseUpdateJson* instance);

const char* parse_update_get_url(ParseUpdateJson* instance);
const char* parse_update_get_id(ParseUpdateJson* instance);
const char* parse_update_get_version(ParseUpdateJson* instance);
const char* parse_update_get_sha256(ParseUpdateJson* instance);

#ifdef __cplusplus
}
#endif
