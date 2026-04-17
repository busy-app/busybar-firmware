#include "sli_tz_secure_psa_key_protection.h"
#include "sli_tz_psa_key_protection_autogen.h"

typedef struct {
  psa_key_id_t start;
  psa_key_id_t end;
} sl_psa_key_range_t;

static const sl_psa_key_range_t protected_ranges[] = SLI_TZ_PSA_KEY_PROTECTION_RANGES_INIT;

static bool sli_psa_key_id_is_protected(psa_key_id_t key_id)
{
  for (size_t i = 0; i < SLI_TZ_PSA_KEY_PROTECTION_RANGE_COUNT; i++) {
    if (key_id >= protected_ranges[i].start && 
        key_id <= protected_ranges[i].end) {
      return true;
    }
  }
  return false;
}

psa_status_t sl_psa_key_check_access(psa_key_id_t key_id)
{
  if (sli_psa_key_id_is_protected(key_id)) {
    return PSA_ERROR_NOT_PERMITTED;
  }
  return PSA_SUCCESS;
}
