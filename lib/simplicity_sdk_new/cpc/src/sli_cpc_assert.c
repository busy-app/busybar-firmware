#include "sli_cpc_assert.h"

#include "sl_assert.h"

#if defined(DEBUG_EFM) || defined(DEBUG_EFM_USER)

__WEAK __NO_RETURN void sli_cpc_assert_fail(const char *file, int line)
{
  assertEFM(file, line);
  for (;; ) {
    /* Loop to prevent returning from this function. */
  }
}

#endif
