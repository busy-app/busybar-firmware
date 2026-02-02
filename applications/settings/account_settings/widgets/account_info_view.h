#pragma once

#include <gui/widget.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct AccountInfoView AccountInfoView;

AccountInfoView* account_info_view_front_alloc(Widget* parent);

AccountInfoView* account_info_view_back_alloc(Widget* parent);

void account_info_view_free(AccountInfoView* instance);

void account_info_view_set_state(AccountInfoView* instance, const char* email);

#ifdef __cplusplus
}
#endif
