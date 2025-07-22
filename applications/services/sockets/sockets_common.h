#pragma once

#if defined(STM32U595xx)
#define TARGET_F20
#elif defined(SI917)
#define TARGET_F64
#else
#error "Unsupported MCU"
#endif

#if defined(TARGET_F20)
// TODO: Replace with own headers?
#include <lwip/sockets.h>
#elif defined(TARGET_F64)
#include <socket.h>
#include <select.h>
#else
#error "Unsupported target"
#endif

typedef struct SocketSrv SocketSrv;
