#pragma once

#if defined(STM32U595xx)
#define TARGET_F20
#elif defined(SI917)
#define TARGET_F64
#else
#error "Unsupported MCU"
#endif

#include <lwip/sockets.h>

typedef struct SocketSrv SocketSrv;
