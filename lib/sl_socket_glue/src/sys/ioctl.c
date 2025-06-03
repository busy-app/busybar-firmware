#include <sys/ioctl.h>
#include <net/if.h>

int ioctl(int fd, unsigned long op, void* arg) {
    UNUSED(fd);
    SocketIoctlRequest req_type = op;

    if(req_type == SIOCGIFFLAGS) {
        furi_check(arg);
        struct ifreq* request = arg;
        if(strcmp(request->ifr_name, NET_IF_SIWX917_INTERFACE_NAME) == 0) {
            short flags = 0;
            // if(ifaddrs_has_at_least_one_addr()) flags |= IFF_UP;
            flags |= IFF_UP;
            flags |= IFF_MULTICAST;
            request->ifr_flags = flags;
            return 0;
        }
    }

    return -1;
}
