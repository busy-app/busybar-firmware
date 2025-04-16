#include "scene_manager.h"

#include <furi.h>

#define TAG "SceneManager"

#define SCENE_INVALID_ID (UINT32_MAX)

struct SceneManager {
    const SceneArray* scenes;
    SceneData** scene_data;
    uint32_t scene_count;
    uint32_t current_scene_id;
    void* context;
};

SceneManager*
    scene_manager_alloc(const SceneArray* const scenes, uint32_t scene_count, void* context) {
    furi_check(scenes);
    furi_check(scene_count);
    furi_check(scene_count < SCENE_INVALID_ID);

    SceneManager* instance = malloc(sizeof(SceneManager));

    instance->scenes = scenes;
    instance->scene_count = scene_count;
    instance->scene_data = malloc(sizeof(SceneData*) * scene_count);
    instance->current_scene_id = SCENE_INVALID_ID;
    instance->context = context;

    for(uint32_t i = 0; i < instance->scene_count; ++i) {
        instance->scene_data[i] = malloc(instance->scenes[i]->data_size);
    }

    return instance;
}

void scene_manager_free(SceneManager* instance) {
    furi_check(instance);

    if(instance->current_scene_id != SCENE_INVALID_ID) {
        const Scene* scene = instance->scenes[instance->current_scene_id];
        scene->exit_callback(instance->context);
    }

    for(uint32_t i = 0; i < instance->scene_count; ++i) {
        free(instance->scene_data[i]);
    }

    free(instance->scene_data);
    free(instance);
}

bool scene_manager_handle_custom_event(SceneManager* instance, uint32_t custom_event) {
    furi_check(instance);

    bool consumed = false;

    const SceneManagerEvent event = {
        .type = SceneManagerEventTypeCustom,
        .event = custom_event,
    };

    if(instance->current_scene_id != SCENE_INVALID_ID) {
        const Scene* scene = instance->scenes[instance->current_scene_id];
        consumed = scene->event_callback(&event, instance->context);
    }

    return consumed;
}

bool scene_manager_handle_back_event(SceneManager* instance) {
    furi_check(instance);

    bool consumed = false;

    const SceneManagerEvent event = {
        .type = SceneManagerEventTypeBack,
    };

    if(instance->current_scene_id != SCENE_INVALID_ID) {
        const Scene* scene = instance->scenes[instance->current_scene_id];
        consumed = scene->event_callback(&event, instance->context);
    }

    if(!consumed) {
        // TODO: Go back one scene
    }

    return consumed;
}

void scene_manager_handle_tick_event(SceneManager* instance) {
    furi_check(instance);

    const SceneManagerEvent event = {
        .type = SceneManagerEventTypeTick,
    };

    if(instance->current_scene_id != SCENE_INVALID_ID) {
        const Scene* scene = instance->scenes[instance->current_scene_id];
        scene->event_callback(&event, instance->context);
    }
}

uint32_t scene_manager_get_current_scene_id(const SceneManager* instance) {
    furi_check(instance);
    return instance->current_scene_id;
}

SceneData* scene_manager_get_current_scene_data(const SceneManager* instance) {
    furi_check(instance);
    return instance->scene_data[instance->current_scene_id];
}

void scene_manager_switch_to_scene(SceneManager* instance, uint32_t scene_id) {
    furi_check(instance);
    furi_check(scene_id < instance->scene_count);

    if(scene_id != instance->current_scene_id) {
        if(instance->current_scene_id != SCENE_INVALID_ID) {
            const Scene* scene = instance->scenes[instance->current_scene_id];
            scene->exit_callback(instance->context);
        }

        instance->current_scene_id = scene_id;

        const Scene* scene = instance->scenes[instance->current_scene_id];
        scene->enter_callback(instance->context);
    }
}
