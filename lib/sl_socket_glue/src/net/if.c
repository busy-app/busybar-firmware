#include <net/if.h>

static const struct if_nameindex siwx917_interfaces[] = {
    {NET_IF_SIWX917_INTERFACE_INDEX, NET_IF_SIWX917_INTERFACE_NAME},
    {0, NULL},
};

struct if_nameindex* if_nameindex(void) {
    struct if_nameindex* duplicate = malloc(sizeof(siwx917_interfaces));
    memcpy(duplicate, siwx917_interfaces, sizeof(siwx917_interfaces));
    return duplicate;
}

void if_freenameindex(struct if_nameindex *ptr) {
    free(ptr);
}

unsigned int if_nametoindex(const char *ifname) {
    if(strcmp(ifname, NET_IF_SIWX917_INTERFACE_NAME) == 0) {
        return NET_IF_SIWX917_INTERFACE_INDEX;
    }
    return 0;
}

char *if_indextoname(unsigned int ifindex, char *ifname) {
    if(ifindex == NET_IF_SIWX917_INTERFACE_INDEX) {
        return strcpy(ifname, NET_IF_SIWX917_INTERFACE_NAME);
    } else {
        return NULL;
    }
}
