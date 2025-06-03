#include <sys/socket.h>
#include <sl_si91x_protocol_types.h>
#include <furi.h>

// https://linux.die.net/man/2/recvmsg

ssize_t recvmsg(int sockfd, struct msghdr *msg, int flags) {
    ssize_t total = 0;

    sl_si91x_time_value timeout = {
        .tv_sec = 0,
        .tv_usec = 10,
    };
    furi_check(flags & MSG_DONTWAIT);
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    // matter only ever requests msg_iovlen==1 \(-_-)/
    for(size_t i = 0; i < msg->msg_iovlen; i++) {
        struct iovec* iovec = &msg->msg_iov[i];
        ssize_t ret = recvfrom(sockfd, iovec->iov_base, iovec->iov_len, 0, NULL, NULL);

        if(ret < 0) return ret;
        total += ret;
    }

    return total;
}

ssize_t sendmsg(int sockfd, const struct msghdr *msg, int flags) {
    ssize_t total = 0;

    for(size_t i = 0; i < msg->msg_iovlen; i++) {
        struct iovec* iovec = &msg->msg_iov[i];
        ssize_t ret = sendto(sockfd, iovec->iov_base, iovec->iov_len, 0, msg->msg_name, msg->msg_namelen);

        if(ret < 0) return ret;
        total += ret;
    }

    return total;
}
