#include <setting_provider.h>

typedef enum {
    PowerSettingV1IdxChargeLimit,

    PowerSettingV1IdxMAX,
} PowerSettingV1Idx;

typedef struct {
    int charge_limit; //<! Battery charge limit (%)
} PowerSettingsV1;
