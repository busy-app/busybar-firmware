#include "sl_iostream.h"

#if !defined(UNITY_IOSTREAM_STDOUT)
  #ifdef SL_IOSTREAM_SWO_PRESENT
    #include "sl_iostream_swo.h"
    #define UNITY_IOSTREAM_STDOUT   sl_iostream_swo_handle
  #else
    #define UNITY_IOSTREAM_STDOUT SL_IOSTREAM_STDOUT
  #endif
#else
extern sl_iostream_t  * UNITY_IOSTREAM_STDOUT;
#endif

void unity_iostream_putchar(char c)
{
  sl_iostream_putchar(UNITY_IOSTREAM_STDOUT, c);
}
