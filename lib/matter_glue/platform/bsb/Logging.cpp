#include <lib/support/logging/CHIPLogging.h>

#define TAG "Matter"

using namespace chip;

namespace chip {
namespace Logging {
namespace Platform {

void LogV(const char* module, uint8_t category, const char* fmt, va_list args) {
    if(IsCategoryEnabled(category)) {
        FuriString* message = furi_string_alloc();
        FuriLogLevel level;

        if(category == kLogCategory_Error) {
            level = FuriLogLevelError;
        } else if(category == kLogCategory_Detail) {
            level = FuriLogLevelDebug;
        } else {
            level = FuriLogLevelInfo;
        }

        furi_string_printf(message, "\b[%s] ", module);
        furi_string_cat_vprintf(message, fmt, args);

        furi_log_print_format(level, TAG, furi_string_get_cstr(message));

        furi_string_free(message);
    }
}

} // namespace Platform
} // namespace Logging
} // namespace chip
