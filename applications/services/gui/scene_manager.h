#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SceneManager SceneManager;

/** Scene Manager events type */
typedef enum {
    SceneManagerEventTypeCustom,
    SceneManagerEventTypeBack,
    SceneManagerEventTypeTick,
} SceneManagerEventType;

/** Scene Manager event
 */
typedef struct {
    SceneManagerEventType type;
    uint32_t event;
} SceneManagerEvent;

typedef void SceneData;

typedef void (*SceneEnterCallback)(void* context);
typedef void (*SceneExitCallback)(void* context);
typedef bool (*SceneEventCallback)(const SceneManagerEvent* event, void* context);

typedef struct {
    SceneEnterCallback enter_callback;
    SceneExitCallback exit_callback;
    SceneEventCallback event_callback;
    size_t data_size;
} Scene;

// TODO: Better types
typedef const Scene* const SceneArray;

SceneManager*
    scene_manager_alloc(const SceneArray* const scenes, uint32_t scene_count, void* context);

void scene_manager_free(SceneManager* instance);

bool scene_manager_handle_custom_event(SceneManager* instance, uint32_t custom_event);

bool scene_manager_handle_back_event(SceneManager* instance);

void scene_manager_handle_tick_event(SceneManager* instance);

uint32_t scene_manager_get_current_scene_id(const SceneManager* instance);

SceneData* scene_manager_get_current_scene_data(const SceneManager* instance);

void scene_manager_next_scene(SceneManager* instance, uint32_t scene_id);

bool scene_manager_previous_scene(SceneManager* instance);

bool scene_manager_has_previous_scene(const SceneManager* instance, uint32_t scene_id);

bool scene_manager_search_and_switch_to_previous_scene(SceneManager* instance, uint32_t scene_id);

bool scene_manager_replace_current_scene(SceneManager* instance, uint32_t scene_id);

#ifdef __cplusplus
}
#endif
