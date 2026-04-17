#ifndef SLI_CPC_ASSERT_H
#define SLI_CPC_ASSERT_H

#include "cmsis_compiler.h"

#if !defined(DEBUG_EFM) && !defined(DEBUG_EFM_USER)

#define SLI_CPC_ASSERT(condition) (void)(condition)

#else

#define SLI_CPC_ASSERT(condition) sli_cpc_assert(condition, __FILE__, __LINE__)

__NO_RETURN void sli_cpc_assert_fail(const char *file, int line);

__STATIC_FORCEINLINE void sli_cpc_assert(int condition, const char *file, int line)
{
  if (!condition) {
    sli_cpc_assert_fail(file, line);
  }
}

#endif

#endif /* SLI_CPC_ASSERT_H */
