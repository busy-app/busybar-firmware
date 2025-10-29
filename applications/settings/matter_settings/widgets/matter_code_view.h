#pragma once

#include <gui/widget.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct MatterCodeView MatterCodeView;

MatterCodeView* matter_code_view_alloc(Widget* parent);

void matter_code_view_free(MatterCodeView* instance);

Widget* matter_code_view_get_base(MatterCodeView* instance);

void matter_code_view_set_logo_path(MatterCodeView* instance, const char* path);

void matter_code_view_set_codes(MatterCodeView* instance, const char* qr, const char* manual);

#ifdef __cplusplus
}
#endif
