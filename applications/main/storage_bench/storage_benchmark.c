#include <storage/storage.h>
#include <toolbox/dir_walk.h>
#include <furi_hal.h>

#define TAG "StorageBenchmark"

typedef enum {
    StorageBenchmarkEventMount = 1 << 0,
} StorageBenchmarkEvent;

static void storage_benchmark_print_error(FS_Error error) {
    FURI_LOG_E(TAG, "Storage error: %s\r\n", storage_error_get_desc(error));
}

#define BENCHMARK_PRINT(...) FURI_LOG_I(TAG, __VA_ARGS__)

static void storage_benchmark_tree(Storage* storage) {
    DirWalk* dir_walk = dir_walk_alloc(storage);
    FuriString* name;
    name = furi_string_alloc();

    uint32_t total_files = 0;
    uint32_t total_dirs = 0;

    BENCHMARK_PRINT("Listing /ext directory");

    if(dir_walk_open(dir_walk, "/ext")) {
        FileInfo fileinfo;
        while(dir_walk_read(dir_walk, name, &fileinfo) == DirWalkOK) {
            if(file_info_is_dir(&fileinfo)) {
                // BENCHMARK_PRINT("\t[D] %s", furi_string_get_cstr(name));
                total_dirs++;
            } else {
                // BENCHMARK_PRINT(
                //     "\t[F] %s %lub", furi_string_get_cstr(name), (uint32_t)(fileinfo.size));
                total_files++;
            }
        }
    } else {
        storage_benchmark_print_error(dir_walk_get_error(dir_walk));
    }

    BENCHMARK_PRINT("Total files: %lu", total_files);
    BENCHMARK_PRINT("Total directories: %lu", total_dirs);

    furi_string_free(name);
    dir_walk_free(dir_walk);
}

static void storage_benchmark_file_write(Storage* storage, size_t blocks) {
    size_t buffer_size = blocks * 512;

    File* file = storage_file_alloc(storage);

    if(!storage_file_open(file, "/ext/benchmark.bin", FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
        FURI_LOG_E(TAG, "Failed to open file");
        return;
    }

    uint8_t* buffer = malloc(buffer_size);
    for(size_t i = 0; i < buffer_size; i++) {
        buffer[i] = i % 256;
    }

    uint32_t start = DWT->CYCCNT;
    uint32_t end;
    bool error = false;
    const size_t iterations = 10;

    for(size_t i = 0; i < iterations; i++) {
        size_t bytes_written = storage_file_write(file, buffer, buffer_size);

        if(bytes_written != buffer_size) {
            FURI_LOG_E(TAG, "Tried to write %zu bytes, wrote %zu", buffer_size, bytes_written);
            error = true;
            break;
        }
    }

    end = DWT->CYCCNT;

    if(error) {
        FURI_LOG_E(TAG, "Failed to write %zu blocks", blocks);
    } else {
        float seconds = (float)(end - start) / furi_hal_cortex_instructions_per_microsecond() /
                        1000000 / iterations;
        float speed_kb = (float)(buffer_size) / seconds / 1024;
        FURI_LOG_I(
            TAG,
            "Write %zu bytes took %0.3f ms, speed %0.2f kb/s (%0.2f Mbit/s)",
            buffer_size,
            (double)seconds * 1000,
            (double)speed_kb,
            (double)speed_kb * 8 / 1024);
    }

    storage_file_free(file);
    free(buffer);
}

static void storage_benchmark_file_read(Storage* storage, size_t blocks) {
    size_t buffer_size = blocks * 512;

    File* file = storage_file_alloc(storage);
    if(!storage_file_open(file, "/ext/benchmark.bin", FSAM_READ, FSOM_OPEN_EXISTING)) {
        FURI_LOG_E(TAG, "Failed to open file");
        return;
    }

    uint8_t* buffer = malloc(buffer_size);

    uint32_t start = DWT->CYCCNT;
    uint32_t end;
    bool error = false;
    const size_t iterations = 10;

    for(size_t i = 0; i < iterations; i++) {
        size_t bytes_read = storage_file_read(file, buffer, buffer_size);

        if(bytes_read != buffer_size) {
            FURI_LOG_E(TAG, "Tried to read %zu bytes, read %zu", buffer_size, bytes_read);
            error = true;
            break;
        }
    }

    end = DWT->CYCCNT;

    if(error) {
        FURI_LOG_E(TAG, "Failed to read %zu blocks", blocks);
    } else {
        float seconds = (float)(end - start) / furi_hal_cortex_instructions_per_microsecond() /
                        1000000 / iterations;
        float speed_kb = (float)(buffer_size) / seconds / 1024;
        FURI_LOG_I(
            TAG,
            "Read %zu bytes took %0.3f ms, speed %0.2f kb/s (%0.2f Mbit/s)",
            buffer_size,
            (double)seconds * 1000,
            (double)speed_kb,
            (double)speed_kb * 8 / 1024);

        for(size_t i = 0; i < buffer_size; i++) {
            if(buffer[i] != i % 256) {
                FURI_LOG_E(TAG, "Data mismatch at address %zu: %u != %u", i, buffer[i], i % 256);
                break;
            }
        }
    }

    storage_file_free(file);
    free(buffer);
}

static void storage_benchmark_file(Storage* storage, size_t blocks) {
    storage_benchmark_file_write(storage, blocks);
    storage_benchmark_file_read(storage, blocks);
}

static void do_storage_benchmark(Storage* storage) {
    FS_Error err = storage_sd_status(storage);

    if(err == FSE_OK) {
        FURI_LOG_I(TAG, "SD card is alive");
        storage_benchmark_tree(storage);
        storage_benchmark_file(storage, 1);
        storage_benchmark_file(storage, 2);
        storage_benchmark_file(storage, 5);
        storage_benchmark_file(storage, 10);
        storage_benchmark_file(storage, 50);
        storage_benchmark_file(storage, 64);
        storage_benchmark_file(storage, 128);
        storage_benchmark_file(storage, 256);
    } else {
        FURI_LOG_E(TAG, "SD card is dead");
        return;
    }
}

int32_t storage_benchmark(void* p) {
    UNUSED(p);

    Storage* storage = furi_record_open(RECORD_STORAGE);
    do_storage_benchmark(storage);
    furi_record_close(RECORD_STORAGE);

    return 0;
}
