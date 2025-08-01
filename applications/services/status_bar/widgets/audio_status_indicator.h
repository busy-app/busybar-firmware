#pragma once

#include <gui/widget.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct AudioStatusIndicator AudioStatusIndicator;

AudioStatusIndicator* audio_status_indicator_alloc(Widget* parent);

void audio_status_indicator_free(AudioStatusIndicator* instance);

Widget* audio_status_indicator_get_base(AudioStatusIndicator* instance);

void audio_status_indicator_set_volume(AudioStatusIndicator* instance, float volume);

#ifdef __cplusplus
}
#endif
