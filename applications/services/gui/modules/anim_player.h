/**
 * @file anim_player.h
 * @brief A widget that plays an AnimFile.
 */
#pragma once

#include <gui/widget.h>
#include <anim_file/anim_file.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct AnimPlayer AnimPlayer;

/**
 * @brief Allocates an `AnimPlayer` GUI element that plays an animation file
 * 
 * @param[in] parent Parent widget
 * 
 * @returns `AnimPlayer` GUI element
 */
AnimPlayer* anim_player_alloc(Widget* parent);

/**
 * @brief Frees an `AnimPlayer` GUI element
 * 
 * @param[inout] instance Widget instance
 */
void anim_player_free(AnimPlayer* instance);

/**
 * @brief Base Widget class
 * 
 * @param[inout] instance Widget instance
 * 
 * @returns Base Widget class
 */
Widget* anim_player_get_base(AnimPlayer* instance);

/**
 * @brief Loads the animation file at the specified path
 * 
 * @param[inout] instance Widget instance
 * @param[in] path File path
 * 
 * @returns `true` = operation successful
 */
bool anim_player_set_source(AnimPlayer* instance, const char* path);

/**
 * @brief Sets the current section to be played back, using a section name
 * 
 * @param[in] anim `AnimPlayer` instance
 * @param[in] flags See `AnimFilePlayFlag`
 * @param[in] name Name of the section (also see: `ANIM_FILE_DEFAULT_SECTION`)
 * 
 * @returns Whether the operation was successful.
 */
bool anim_player_set_section(AnimPlayer* instance, AnimFilePlayFlag flags, const char* name);

/**
 * @brief Helpher function to loop the entire animation.
 * 
 * @param[inout] instance Widget instance
 */
bool anim_player_loop_default(AnimPlayer* instance);

/**
 * @brief Starts or resumes playback of the animation
 * 
 * @note Playback is automatically started in `anim_player_set_source`
 * 
 * @param[inout] instance Widget instance
 * 
 */
void anim_player_start(AnimPlayer* instance);

/**
 * @brief Pauses playback of the animation
 * 
 * @param[inout] instance Widget instance
 */
void anim_player_pause(AnimPlayer* instance);

/**
 * @brief Playback frame callback
 * 
 * @param[inout] instance Widget instance
 * @param[in] info Information about the just-displayed frame
 * @param[inout] context Custom context
 */
typedef void (
    *AnimPlayerFrameCallback)(AnimPlayer* instance, const AnimFileFrameInfo* info, void* context);

/**
 * @brief Calls the specified callback every time a frame is played.
 * 
 * The callback is called from the GUI thread with locked GUI.
 * 
 * @param[inout] instance Widget instance
 * @param[in] callback May be `NULL` to disable the callback
 * @param[inout] context Custom context. Must be `NULL` if `callback` is `NULL`.
 */
void anim_player_set_frame_callback(
    AnimPlayer* instance,
    AnimPlayerFrameCallback callback,
    void* context);

#ifdef __cplusplus
}
#endif
