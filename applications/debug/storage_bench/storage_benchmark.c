#include <cli/cli_command.h>
#include <cli/cli_ansi.h>
#include <storage/storage.h>
#include <toolbox/dir_walk.h>
#include <furi_hal.h>

#define TAG "StorageBenchmark"

typedef enum {
    StorageBenchmarkEventMount = 1 << 0,
} StorageBenchmarkEvent;

static void storage_printf_error(FS_Error error) {
    printf(ANSI_FG_RED "Storage error: %s\r\n" ANSI_RESET, storage_error_get_desc(error));
}

static void storage_benchmark_tree(Storage* storage) {
    DirWalk* dir_walk = dir_walk_alloc(storage);
    FuriString* name;
    name = furi_string_alloc();

    uint32_t total_files = 0;
    uint32_t total_dirs = 0;

    printf("Listing /ext directory\r\n");

    if(dir_walk_open(dir_walk, STORAGE_EXT_PATH_PREFIX)) {
        FileInfo fileinfo;
        while(dir_walk_read(dir_walk, name, &fileinfo) == DirWalkOK) {
            if(file_info_is_dir(&fileinfo)) {
                // printf("\t[D] %s\r\n", furi_string_get_cstr(name));
                total_dirs++;
            } else {
                // printf(
                //     "\t[F] %s %lub\r\n", furi_string_get_cstr(name), (uint32_t)(fileinfo.size));
                total_files++;
            }
        }
    } else {
        storage_printf_error(dir_walk_get_error(dir_walk));
    }

    printf("Total files: %lu\r\n", total_files);
    printf("Total directories: %lu\r\n", total_dirs);

    furi_string_free(name);
    dir_walk_free(dir_walk);
}

static void storage_benchmark_file_write(Storage* storage, size_t blocks) {
    size_t buffer_size = blocks * 512;

    File* file = storage_file_alloc(storage);

    if(!storage_file_open(file, "/ext/benchmark.bin", FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
        printf(ANSI_FG_RED "Failed to open file\r\n" ANSI_RESET);
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
            printf(
                ANSI_FG_RED "Tried to write %zu bytes, wrote %zu\r\n" ANSI_RESET,
                buffer_size,
                bytes_written);
            error = true;
            break;
        }
    }

    end = DWT->CYCCNT;

    if(error) {
        printf(ANSI_FG_RED "Failed to write %zu blocks\r\n" ANSI_RESET, blocks);
    } else {
        float seconds =
            (float)(end - start) / furi_hal_cpu_get_cycles_per_us() / 1000000 / iterations;
        float speed_kb = (float)(buffer_size) / seconds / 1024;
        printf(
            "Write %zu bytes took %0.3f ms, speed %0.2f KiB/s (%0.2f MiB/s)\r\n",
            buffer_size,
            (double)seconds * 1000,
            (double)speed_kb,
            (double)speed_kb / 1024);
    }

    storage_file_free(file);
    free(buffer);
}

static void storage_benchmark_file_read(Storage* storage, size_t blocks) {
    size_t buffer_size = blocks * 512;

    File* file = storage_file_alloc(storage);
    if(!storage_file_open(file, "/ext/benchmark.bin", FSAM_READ, FSOM_OPEN_EXISTING)) {
        printf(ANSI_FG_RED "Failed to open file\r\n" ANSI_RESET);
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
            printf(
                ANSI_FG_RED "Tried to read %zu bytes, read %zu\r\n" ANSI_RESET,
                buffer_size,
                bytes_read);
            error = true;
            break;
        }
    }

    end = DWT->CYCCNT;

    if(error) {
        printf(ANSI_FG_RED "Failed to read %zu blocks\r\n" ANSI_RESET, blocks);
    } else {
        float seconds =
            (float)(end - start) / furi_hal_cpu_get_cycles_per_us() / 1000000 / iterations;
        float speed_kb = (float)(buffer_size) / seconds / 1024;
        printf(
            "Read %zu bytes took %0.3f ms, speed %0.2f KiB/s (%0.2f MiB/s)\r\n",
            buffer_size,
            (double)seconds * 1000,
            (double)speed_kb,
            (double)speed_kb / 1024);

        for(size_t i = 0; i < buffer_size; i++) {
            if(buffer[i] != i % 256) {
                printf(
                    ANSI_FG_RED "Data mismatch at address %zu: %u != %u\r\n" ANSI_RESET,
                    i,
                    buffer[i],
                    i % 256);
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
    FS_Error err = storage_sd_status(storage, STORAGE_EXT_PATH_PREFIX);

    if(err == FSE_OK) {
        printf("SD card is alive\r\n");
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
        printf(ANSI_FG_RED "SD card is dead\r\n" ANSI_RESET);
        return;
    }
}

void storage_benchmark(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    UNUSED(args);
    UNUSED(context);

    Storage* storage = furi_record_open(RECORD_STORAGE);
    do_storage_benchmark(storage);
    furi_record_close(RECORD_STORAGE);
}
