#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// Pro Tip: change baud rate in
//   `targets/fXX/furi_hal/furi_hal_serial_control.c` -> `furi_hal_serial_control_init`
// to 4 Mbaud C:

typedef struct Profiler Profiler;

Profiler* profiler_alloc(const char* tag);

void profiler_free(Profiler* profiler);

void profiler_prealloc(Profiler* profiler, const char* key);

void profiler_start(Profiler* profiler, const char* key);

void profiler_stop(Profiler* profiler, const char* key);

void profiler_dump(Profiler* profiler);

#ifdef __cplusplus
}
#endif
