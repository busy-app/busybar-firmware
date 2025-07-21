#pragma once

#include <furi.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NET_IF_SIWX917_INTERFACE_NAME "slsgwlan"
#define NET_IF_SIWX917_INTERFACE_INDEX 1
#define IF_NAMESIZE 10

// https://linux.die.net/man/3/if_nameindex
struct if_nameindex {
    unsigned int if_index;
    char        *if_name;
};

struct if_nameindex *if_nameindex(void);
void if_freenameindex(struct if_nameindex *ptr);

unsigned int if_nametoindex(const char *ifname);
char *if_indextoname(unsigned int ifindex, char *ifname);

#ifdef __cplusplus
}
#endif
