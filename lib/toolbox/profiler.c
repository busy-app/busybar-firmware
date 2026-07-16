#include "profiler.h"
#include <stdlib.h>
#include <m-dict.h>
#include <furi.h>
#include <furi_hal_cpu.h>

typedef struct {
    uint32_t start;
    uint32_t length;
    uint32_t count;
} ProfilerRecord;

DICT_DEF2(ProfilerRecordDict, const char*, M_CSTR_OPLIST, ProfilerRecord, M_POD_OPLIST)
#define M_OPL_ProfilerRecord_t() DICT_OPLIST(ProfilerRecord, M_CSTR_OPLIST, M_POD_OPLIST)

struct Profiler {
    char* tag;
    ProfilerRecordDict_t records;
};

Profiler* profiler_alloc(const char* tag) {
    furi_check(tag);

    Profiler* profiler = malloc(sizeof(Profiler));
    ProfilerRecordDict_init(profiler->records);
    profiler->tag = strdup(tag);
    return profiler;
}

void profiler_free(Profiler* profiler) {
    furi_check(profiler);

    free(profiler->tag);
    ProfilerRecordDict_clear(profiler->records);
    free(profiler);
}

void profiler_prealloc(Profiler* profiler, const char* key) {
    furi_check(profiler);
    furi_check(key);

    ProfilerRecord record = {
        .start = 0,
        .length = 0,
        .count = 0,
    };

    ProfilerRecordDict_set_at(profiler->records, key, record);
}

void profiler_start(Profiler* profiler, const char* key) {
    furi_check(profiler);
    furi_check(key);

    ProfilerRecord* record = ProfilerRecordDict_get(profiler->records, key);
    if(record == NULL) {
        profiler_prealloc(profiler, key);
        record = ProfilerRecordDict_get(profiler->records, key);
    }

    furi_check(record->start == 0);
    record->start = furi_hal_cpu_get_cycle_count();
}

void profiler_stop(Profiler* profiler, const char* key) {
    furi_check(profiler);
    furi_check(key);

    ProfilerRecord* record = ProfilerRecordDict_get(profiler->records, key);
    furi_check(record != NULL);

    record->length += furi_hal_cpu_get_cycle_count() - record->start;
    record->start = 0;
    record->count++;
}

void profiler_dump(Profiler* profiler) {
    furi_check(profiler);

    FURI_LOG_D(profiler->tag, "Run statistics:");

    ProfilerRecordDict_it_t it;
    for(ProfilerRecordDict_it(it, profiler->records); !ProfilerRecordDict_end_p(it);
        ProfilerRecordDict_next(it)) {
        const ProfilerRecordDict_itref_t* itref = ProfilerRecordDict_cref(it);

        uint32_t count = itref->value.count;
        uint32_t clocks = itref->value.length;
        double clk_fq = furi_hal_cpu_get_frequency();

        double us = (double)clocks / (clk_fq / (double)1'000'000.0);
        double ms = (double)clocks / (clk_fq / (double)1000.0);
        double s = (double)clocks / clk_fq;

        FURI_LOG_D(
            profiler->tag,
            "\t%s[%lu]: %f s, %f ms, %f us, %lu clk",
            itref->key,
            count,
            s,
            ms,
            us,
            clocks);

        if(count > 1) {
            us /= (double)count;
            ms /= (double)count;
            s /= (double)count;
            clocks /= count;

            FURI_LOG_D(
                profiler->tag,
                "\t\t%s[1]: %f s, %f ms, %f us, %lu clk",
                itref->key,
                s,
                ms,
                us,
                clocks);
        }
    }
}
