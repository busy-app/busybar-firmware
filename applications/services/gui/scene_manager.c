#include "scene_manager.h"

#include <furi.h>

#include <m-array.h>

#define TAG "SceneManager"

#define REMOVE_CONST(x) ((void*)(x))

ARRAY_DEF(SceneIdStack, uint32_t, M_DEFAULT_OPLIST)

struct SceneManager {
    const SceneArray* scenes;
    SceneData** scene_data;
    uint32_t scene_count;
    SceneIdStack_t scene_id_stack;
    void* context;
};

// Implementation

static const Scene* scene_manager_get_current_scene(const SceneManager* instance) {
    const Scene* scene = NULL;

    if(SceneIdStack_size(instance->scene_id_stack)) {
        scene = instance->scenes[scene_manager_get_current_scene_id(instance)];
    }

    return scene;
}

// Public API

SceneManager*
    scene_manager_alloc(const SceneArray* const scenes, uint32_t scene_count, void* context) {
    furi_check(scenes);
    furi_check(scene_count);

    SceneManager* instance = malloc(sizeof(SceneManager));

    instance->scenes = scenes;
    instance->scene_count = scene_count;
    instance->scene_data = malloc(sizeof(SceneData*) * scene_count);
    instance->context = context;

    for(uint32_t i = 0; i < instance->scene_count; ++i) {
        const size_t data_size = instance->scenes[i]->data_size;

        if(data_size) {
            instance->scene_data[i] = malloc(data_size);
        }
    }

    SceneIdStack_init(instance->scene_id_stack);

    return instance;
}

void scene_manager_free(SceneManager* instance) {
    furi_check(instance);

    const Scene* current_scene = scene_manager_get_current_scene(instance);

    if(current_scene) {
        current_scene->exit_callback(instance->context);
    }

    SceneIdStack_clear(instance->scene_id_stack);

    for(uint32_t i = 0; i < instance->scene_count; ++i) {
        SceneData* scene_data = instance->scene_data[i];

        if(scene_data) {
            free(scene_data);
        }
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

    const Scene* current_scene = scene_manager_get_current_scene(instance);

    if(current_scene) {
        consumed = current_scene->event_callback(&event, instance->context);
    }

    return consumed;
}

bool scene_manager_handle_back_event(SceneManager* instance) {
    furi_check(instance);

    bool consumed = false;

    const SceneManagerEvent event = {
        .type = SceneManagerEventTypeBack,
    };

    const Scene* current_scene = scene_manager_get_current_scene(instance);

    if(current_scene) {
        consumed = current_scene->event_callback(&event, instance->context);
    }

    if(!consumed) {
        consumed = scene_manager_previous_scene(instance);
    }

    return consumed;
}

void scene_manager_handle_tick_event(SceneManager* instance) {
    furi_check(instance);

    const SceneManagerEvent event = {
        .type = SceneManagerEventTypeTick,
    };

    const Scene* current_scene = scene_manager_get_current_scene(instance);

    if(current_scene) {
        current_scene->event_callback(&event, instance->context);
    }
}

uint32_t scene_manager_get_current_scene_id(const SceneManager* instance) {
    furi_check(instance);
    return *SceneIdStack_back(REMOVE_CONST(instance->scene_id_stack));
}

SceneData* scene_manager_get_scene_data(const SceneManager* instance, uint32_t scene_id) {
    furi_check(instance);
    // That is important to ensure that the requested scene is the current one
    // If you need data of another scene, make another api for that
    furi_check(scene_id == scene_manager_get_current_scene_id(instance));
    return instance->scene_data[scene_id];
}

void scene_manager_next_scene(SceneManager* instance, uint32_t scene_id) {
    furi_check(instance);
    furi_check(scene_id < instance->scene_count);

    const Scene* current_scene = scene_manager_get_current_scene(instance);

    if(current_scene) {
        current_scene->exit_callback(instance->context);
    }

    SceneIdStack_push_back(instance->scene_id_stack, scene_id);

    const Scene* next_scene = scene_manager_get_current_scene(instance);
    next_scene->enter_callback(instance->context);
}

bool scene_manager_previous_scene(SceneManager* instance) {
    furi_check(instance);

    bool success = false;

    do {
        const Scene* current_scene = scene_manager_get_current_scene(instance);
        if(!current_scene) break;

        current_scene->exit_callback(instance->context);
        SceneIdStack_pop_back(NULL, instance->scene_id_stack);

        const Scene* previous_scene = scene_manager_get_current_scene(instance);
        if(!previous_scene) break;

        previous_scene->enter_callback(instance->context);
        success = true;

    } while(false);

    return success;
}

bool scene_manager_has_previous_scene(const SceneManager* instance, uint32_t scene_id) {
    furi_check(instance);
    furi_check(scene_id < instance->scene_count);

    bool success = false;

    SceneIdStack_it_ct it;
    for(SceneIdStack_it_last(it, instance->scene_id_stack); !SceneIdStack_end_p(it);
        SceneIdStack_previous(it)) {
        if(*SceneIdStack_cref(it) == scene_id) {
            success = true;
            break;
        }
    }

    return success;
}

bool scene_manager_search_and_switch_to_previous_scene(SceneManager* instance, uint32_t scene_id) {
    furi_check(instance);
    furi_check(scene_id < instance->scene_count);

    bool success = false;

    do {
        const Scene* current_scene = scene_manager_get_current_scene(instance);
        if(!current_scene) break;

        current_scene->exit_callback(instance->context);
        SceneIdStack_pop_back(NULL, instance->scene_id_stack);

        SceneIdStack_it_ct it;
        for(SceneIdStack_it_last(it, instance->scene_id_stack); !SceneIdStack_end_p(it);
            SceneIdStack_previous(it)) {
            if(*SceneIdStack_cref(it) == scene_id) {
                break;
            }
        }

        if(SceneIdStack_end_p(it)) break;

        SceneIdStack_pop_until(instance->scene_id_stack, it);
        SceneIdStack_push_back(instance->scene_id_stack, scene_id);

        const Scene* previous_scene = scene_manager_get_current_scene(instance);
        previous_scene->enter_callback(instance->context);

        success = true;
    } while(false);

    return success;
}

bool scene_manager_replace_current_scene(SceneManager* instance, uint32_t scene_id) {
    furi_check(instance);
    furi_check(scene_id < instance->scene_count);

    bool success = false;

    do {
        size_t stack_size = SceneIdStack_size(instance->scene_id_stack);
        if(!stack_size) break;

        const Scene* prev_scene = scene_manager_get_current_scene(instance);
        prev_scene->exit_callback(instance->context);

        SceneIdStack_set_at(instance->scene_id_stack, stack_size - 1, scene_id);

        const Scene* new_scene = scene_manager_get_current_scene(instance);
        new_scene->enter_callback(instance->context);

        success = true;
    } while(false);

    return success;
}
