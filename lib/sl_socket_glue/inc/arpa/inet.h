#pragma once

#include <sys/socket.h>

#ifdef __cplusplus
extern "C" {
#endif

// https://linux.die.net/man/3/inet_ntop
// https://linux.die.net/man/3/inet_pton

const char *inet_ntop(int af, const void *src, char *dst, socklen_t size);
int inet_pton(int af, const char *src, void *dst);

#ifdef __cplusplus
}
#endif
