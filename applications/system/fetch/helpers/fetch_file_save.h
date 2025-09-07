#pragma once

#include <furi.h>

typedef struct FetchFileSave FetchFileSave;

FetchFileSave* fetch_file_save_alloc(FuriString* file_path);
void fetch_file_save_free(FetchFileSave* instance);
bool fetch_file_save_write(FetchFileSave* instance, uint8_t* data, size_t size);
void fetch_file_save_remove(FetchFileSave* instance);
