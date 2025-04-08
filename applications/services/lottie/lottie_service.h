/**
 * @file lottie_service.h
 * @brief
 */
#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define RECORD_LOTTIE "lottie"

typedef struct LottieService LottieService;

typedef struct LottieServiceTask LottieServiceTask;

typedef void (*LottieServiceTaskCallback)(const void* canvas_buf, void* context);

typedef struct {
    int32_t canvas_width;
    int32_t canvas_height;
    size_t canvas_buf_size;
} LottieServiceTaskInfo;

LottieServiceTask* lottie_service_task_alloc(
    LottieService* instance,
    LottieServiceTaskCallback callback,
    void* context);

void lottie_service_task_free(LottieServiceTask* task);

bool lottie_service_task_set_source(LottieServiceTask* task, const char* file_path);

bool lottie_service_task_override_slot(LottieServiceTask* task, const char* slot_str);

bool lottie_service_task_get_info(const LottieServiceTask* task, LottieServiceTaskInfo* info);
