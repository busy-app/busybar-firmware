#include <furi.h>

#include <gui/gui.h>
#include <gui/modules/anim_player.h>
#include <input/input.h>

#define TRACKS_PATH APP_ASSETS_PATH("animations/tracks_72x16.anim")
#define TRAIN_PATH  APP_ASSETS_PATH("animations/train_203x15.anim")

int32_t anim_test_app(void* arg) {
    UNUSED(arg);

    Gui* gui = furi_record_open(RECORD_GUI);
    GuiLayer* layer = gui_get_layer(gui, GuiLayerIdMain);
    Widget* root = gui_layer_get_root_widget(layer, GuiDisplayIdFront);
    AnimPlayer* tracks;
    AnimPlayer* train;

    FuriSemaphore* exit = furi_semaphore_alloc(1, 0);
    int train_pos_10ths = -203 * 10;
    int velocity = 1;

    bool input_handler(const InputEvent* event, void* context) {
        UNUSED(context);

        if(event->type == InputTypeShort) {
            if(event->key == InputKeyUp) {
                train_pos_10ths += velocity;
            } else if(event->key == InputKeyDown) {
                train_pos_10ths -= velocity;
            } else if(event->key == InputKeyBack) {
                furi_semaphore_release(exit);
            } else if(event->key == InputKeyOk) {
                if(velocity == 1)
                    velocity = 10;
                else
                    velocity = 1;
            } else {
                return false;
            }
        } else {
            return false;
        }

        // gui is locked here
        anim_player_set_offset(train, (float)train_pos_10ths / 10.0f, 0);

        return true;
    }

    with_gui(gui, {
        tracks = anim_player_alloc(root);
        train = anim_player_alloc(root);

        anim_player_set_source(tracks, TRACKS_PATH);

        widget_set_max_size(anim_player_get_base(train), 72, 15);
        anim_player_set_source_ex(train, TRAIN_PATH, AnimPlayerOptionIntermediateInternalBuffer);
        anim_player_set_offset(train, -203, 0);

        gui_layer_add_input_callback(layer, input_handler, NULL);
    });

    furi_check(furi_semaphore_acquire(exit, FuriWaitForever) == FuriStatusOk);

    with_gui(gui, {
        gui_layer_remove_input_callback(layer, input_handler);

        anim_player_free(train);
        anim_player_free(tracks);
    });

    furi_semaphore_free(exit);
    furi_record_close(RECORD_GUI);
    return 0;
}
