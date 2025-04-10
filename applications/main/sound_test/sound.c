#include <furi.h>

#include <m-array.h>
#include <toolbox/m_cstr_dup.h>
#include <toolbox/path.h>
#include <toolbox/dir_walk.h>

#include <audio/audio.h>
#include <storage/storage.h>
#include <gui/gui.h>
#include <gui/modules/var_item_list.h>
#include <gui/modules/label.h>

#define TAG "Sound"

#define BTN_NAME_START "START"
#define BTN_NAME_BACK  "BACK"
#define BTN_NAME_OK    "OK"

#define SOUND_FOLDER_ROOT     "sound"
#define SOUND_FOLDER_KEY(key) SOUND_FOLDER_ROOT "/" key

#define SOUND_FOLDER_BUTTON_START SOUND_FOLDER_KEY(BTN_NAME_START)
#define SOUND_FOLDER_BUTTON_BACK  SOUND_FOLDER_KEY(BTN_NAME_BACK)
#define SOUND_FOLDER_BUTTON_OK    SOUND_FOLDER_KEY(BTN_NAME_OK)

#define SOUND_NO_DATA "No data"

typedef enum {
    SoundCustomEventExit = 1UL << 0,
    SoundCustomEventEdit = 1UL << 1,
    SoundCustomEventTest = 1UL << 2,

    SoundCustomEventSoundStart = 1UL << 3,
    SoundCustomEventSoundBack = 1UL << 4,
    SoundCustomEventSoundOk = 1UL << 5,
} SoundCustomEvent;

typedef enum {
    SoundAppModeEdit,
    SoundAppModeTest
} SoundAppMode;

typedef enum {
    SoundButtonStart,
    SoundButtonBack,
    SoundButtonOk,

    SoundButtonNum,
} SoundButtonNames;

ARRAY_DEF(ButtonSoundArray, const char*, M_CSTR_DUP_OPLIST); //-V575

typedef struct {
    size_t size;
    const char** items;
} SoundButtonCollection;

typedef struct {
    FuriEventLoop* event_loop;
    Audio* audio;
    Gui* gui;

    Label* label;
    VarItem* var_list_sound_items[SoundButtonNum];
    VarItemList* var_list;

    Widget* widget_list;
    Widget* widget_label;

    SoundAppMode mode;
    FuriString* label_text;

    ButtonSoundArray_t button_sounds[SoundButtonNum];
} Sound;

static const char* path_folder_prefixes[SoundButtonNum] = {
    [SoundButtonStart] = SOUND_FOLDER_BUTTON_START,
    [SoundButtonOk] = SOUND_FOLDER_BUTTON_OK,
    [SoundButtonBack] = SOUND_FOLDER_BUTTON_BACK,
};

static bool sound_input_callback(const InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);
    Sound* instance = context;

    bool consumed = false;

    if(event->type == InputTypeLong) {
        if(event->key == InputKeyStart || event->key == InputKeyOk) {
            furi_event_loop_set_custom_event(
                instance->event_loop,
                instance->mode == SoundAppModeEdit ? SoundCustomEventTest : SoundCustomEventEdit);
            consumed = true;
        }
    }

    return consumed;
}

static bool sound_top_layer_callback(const InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);
    Sound* instance = context;

    bool consumed = false;
    if(event->type == InputTypeShort) {
        if(instance->mode == SoundAppModeTest) {
            uint32_t custom_event;
            if(event->key == InputKeyStart) {
                custom_event = SoundCustomEventSoundStart;
            } else if(event->key == InputKeyBack) {
                custom_event = SoundCustomEventSoundBack;
            } else if(event->key == InputKeyOk) {
                custom_event = SoundCustomEventSoundOk;
            } else
                return consumed;

            furi_event_loop_set_custom_event(instance->event_loop, custom_event);
            consumed = false;
        }
    } else if(event->type == InputTypeLong && event->key == InputKeyBack) {
        furi_event_loop_set_custom_event(instance->event_loop, SoundCustomEventExit);
        consumed = true;
    }

    return consumed;
}

static void sound_app_switch_mode(Sound* instance, SoundAppMode new_mode) {
    with_gui(instance->gui, {
        Widget* widget_list = instance->widget_list;
        Widget* widget_label = instance->widget_label;

        widget_move_to_foreground(new_mode == SoundAppModeEdit ? widget_list : widget_label);
        widget_move_to_background(new_mode == SoundAppModeEdit ? widget_label : widget_list);

        instance->mode = new_mode;
    });
}

static const char* sound_var_list_item_get_filename(Sound* instance, SoundButtonNames button) {
    int32_t index = var_item_get_value(instance->var_list_sound_items[button]);
    const char* name = *ButtonSoundArray_cget(instance->button_sounds[button], index);
    return name;
}

static bool
    sound_get_path_to_button_sound(Sound* instance, SoundButtonNames button, FuriString* output) {
    const char* name = sound_var_list_item_get_filename(instance, button);

    if(strncmp(name, SOUND_NO_DATA, strlen(name)) == 0) return false;

    furi_string_set(output, STORAGE_EXT_PATH_PREFIX);
    path_append(output, path_folder_prefixes[button]);
    path_append(output, name);
    furi_string_cat_str(output, ".snd");
    return true;
}

static void sound_app_format_label(Sound* instance) {
    furi_string_reset(instance->label_text);

    int32_t index = var_item_get_value(instance->var_list_sound_items[SoundButtonStart]);
    const char* name = *ButtonSoundArray_cget(instance->button_sounds[SoundButtonStart], index);
    furi_string_cat_printf(
        instance->label_text, "  %s                  %s\n", BTN_NAME_START, name);

    index = var_item_get_value(instance->var_list_sound_items[SoundButtonBack]);
    name = *ButtonSoundArray_cget(instance->button_sounds[SoundButtonBack], index);
    furi_string_cat_printf(
        instance->label_text, "  %s                    %s\n", BTN_NAME_BACK, name);

    index = var_item_get_value(instance->var_list_sound_items[SoundButtonOk]);
    name = *ButtonSoundArray_cget(instance->button_sounds[SoundButtonOk], index);
    furi_string_cat_printf(
        instance->label_text, "  %s                        %s\n", BTN_NAME_OK, name);

    // furi_string_cat_printf(instance->label_text, "  START                  %s\n", text[0]);
    // furi_string_cat_printf(instance->label_text, "  BACK                    %s\n", text[0]);
    // furi_string_cat_printf(instance->label_text, "  OK                        %s\n", text[0]);
}

static void sound_button_play_if_exist(Sound* instance, SoundButtonNames button) {
    FuriString* path = furi_string_alloc();
    if(sound_get_path_to_button_sound(instance, button, path))
        audio_play_file(instance->audio, furi_string_get_cstr(path));
    furi_string_free(path);
}

static void sound_custom_event_callback(uint32_t events, void* context) {
    furi_assert(context);
    Sound* instance = context;

    if(events & SoundCustomEventExit) {
        furi_event_loop_stop(instance->event_loop);
    }

    if(events & SoundCustomEventSoundStart) {
        sound_button_play_if_exist(instance, SoundButtonStart);
    }

    if(events & SoundCustomEventSoundBack) {
        sound_button_play_if_exist(instance, SoundButtonBack);
    }

    if(events & SoundCustomEventSoundOk) {
        sound_button_play_if_exist(instance, SoundButtonOk);
    }

    if(events & SoundCustomEventTest) {
        sound_app_format_label(instance);
        label_set_text_fmt(instance->label, furi_string_get_cstr(instance->label_text));
        sound_app_switch_mode(instance, SoundAppModeTest);
    }

    if(events & SoundCustomEventEdit) {
        sound_app_switch_mode(instance, SoundAppModeEdit);
    }
}

static bool dir_walk_sound_filter(const char* name, FileInfo* fileinfo, void* ctx) {
    UNUSED(ctx);
    FURI_LOG_D(TAG, "Filter %s", name);
    return !(file_info_is_dir(fileinfo) || (strnstr(name, ".snd", strlen(name)) == NULL));
}

static void sound_init_storage(ButtonSoundArray_t array, const char* sound_folder_path) {
    ButtonSoundArray_init(array);
    Storage* storage = furi_record_open(RECORD_STORAGE);
    DirWalk* walk = dir_walk_alloc(storage);
    dir_walk_set_recursive(walk, false);
    dir_walk_set_filter_cb(walk, dir_walk_sound_filter, NULL);

    FuriString* str = furi_string_alloc();
    do {
        if(!dir_walk_open(walk, sound_folder_path)) {
            ButtonSoundArray_push_back(array, SOUND_NO_DATA);
            break;
        }

        FileInfo info;
        while(dir_walk_read(walk, str, &info) == DirWalkOK) {
            path_extract_filename_no_ext(furi_string_get_cstr(str), str);
            const char* result = furi_string_get_cstr(str);
            FURI_LOG_D(TAG, "Add sound: %s", result);
            ButtonSoundArray_push_back(array, result);
        }

        if(ButtonSoundArray_size(array) == 0) {
            ButtonSoundArray_push_back(array, SOUND_NO_DATA);
        }

        dir_walk_close(walk);
    } while(false);
    furi_string_free(str);
    dir_walk_free(walk);
    furi_record_close(RECORD_STORAGE);
}

static SoundButtonCollection* sound_collection_alloc_from_array(ButtonSoundArray_t array) {
    SoundButtonCollection* instance = malloc(sizeof(SoundButtonCollection));

    instance->size = ButtonSoundArray_size(array);
    instance->items = malloc(sizeof(const char*) * instance->size);
    size_t i = 0;
    ButtonSoundArray_it_t it;
    for(ButtonSoundArray_it(it, array); !ButtonSoundArray_end_p(it); ButtonSoundArray_next(it)) {
        instance->items[i++] = *ButtonSoundArray_cref(it);
    }
    return instance;
}

static void sound_collection_free(SoundButtonCollection* instance) {
    free(instance->items);
    free(instance);
}

static Sound* sound_alloc(void) {
    Sound* instance = malloc(sizeof(Sound));
    instance->event_loop = furi_event_loop_alloc();
    instance->audio = furi_record_open(RECORD_AUDIO);
    instance->gui = furi_record_open(RECORD_GUI);

    instance->label_text = furi_string_alloc();

    furi_event_loop_set_custom_event_callback(
        instance->event_loop, sound_custom_event_callback, instance);

    sound_init_storage(
        instance->button_sounds[SoundButtonStart], EXT_PATH(SOUND_FOLDER_BUTTON_START));
    sound_init_storage(
        instance->button_sounds[SoundButtonBack], EXT_PATH(SOUND_FOLDER_BUTTON_BACK));
    sound_init_storage(instance->button_sounds[SoundButtonOk], EXT_PATH(SOUND_FOLDER_BUTTON_OK));

    with_gui(instance->gui, {
        GuiLayer* top_layer = gui_get_layer(instance->gui, GuiLayerIdTop);
        gui_layer_add_input_callback(top_layer, sound_top_layer_callback, instance);

        GuiLayer* main_layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_add_input_callback(main_layer, sound_input_callback, instance);

        instance->widget_list = gui_layer_get_root_widget(main_layer, GuiDisplayIdBack);
        instance->var_list = var_item_list_alloc(instance->widget_list);

        SoundButtonCollection* buf =
            sound_collection_alloc_from_array(instance->button_sounds[SoundButtonStart]);
        instance->var_list_sound_items[SoundButtonStart] = var_item_list_add_selector(
            instance->var_list, BTN_NAME_START, NULL, buf->items, buf->size, NULL, NULL);
        sound_collection_free(buf);

        buf = sound_collection_alloc_from_array(instance->button_sounds[SoundButtonBack]);
        instance->var_list_sound_items[SoundButtonBack] = var_item_list_add_selector(
            instance->var_list, BTN_NAME_BACK, NULL, buf->items, buf->size, NULL, NULL);
        sound_collection_free(buf);

        buf = sound_collection_alloc_from_array(instance->button_sounds[SoundButtonOk]);
        instance->var_list_sound_items[SoundButtonOk] = var_item_list_add_selector(
            instance->var_list, BTN_NAME_OK, NULL, buf->items, buf->size, NULL, NULL);
        sound_collection_free(buf);

        instance->widget_label = widget_alloc(instance->widget_list);
        instance->label = label_alloc(instance->widget_label);

        sound_app_format_label(instance);
        label_set_text_fmt(instance->label, furi_string_get_cstr(instance->label_text));
    });
    sound_app_switch_mode(instance, SoundAppModeEdit);

    return instance;
}

static void sound_free(Sound* instance) {
    with_gui(instance->gui, {
        GuiLayer* top_layer = gui_get_layer(instance->gui, GuiLayerIdTop);
        gui_layer_remove_input_callback(top_layer, sound_top_layer_callback);

        GuiLayer* main_layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_remove_input_callback(main_layer, sound_input_callback);

        var_item_list_free(instance->var_list);
        label_free(instance->label);
        widget_free(instance->widget_label);
    });

    for(size_t i = 0; i < SoundButtonNum; i++) {
        ButtonSoundArray_clear(instance->button_sounds[i]);
    }

    furi_string_free(instance->label_text);
    furi_record_close(RECORD_GUI);
    furi_record_close(RECORD_AUDIO);

    furi_event_loop_free(instance->event_loop);
    free(instance);
}

int32_t sound_app(void* arg) {
    UNUSED(arg);

    Sound* instance = sound_alloc();
    furi_event_loop_run(instance->event_loop);
    sound_free(instance);

    return 0;
}
