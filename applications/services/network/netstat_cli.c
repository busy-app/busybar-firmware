#include <cli/cli_command.h>

#include <lwip/tcpip.h>
#include <lwip/udp.h>
#include <lwip/priv/tcp_priv.h>

#include <inttypes.h>

#define TAG "NetstatCli"

static const char* netstat_cli_connection_state_string_map[] = {
    [CLOSED] = "CLOSED",
    [LISTEN] = "LISTEN",
    [SYN_SENT] = "SYN-SENT",
    [SYN_RCVD] = "SYN-RECEIVED",
    [ESTABLISHED] = "ESTABLISHED",
    [FIN_WAIT_1] = "FIN-WAIT-1",
    [FIN_WAIT_2] = "FIN-WAIT-2",
    [CLOSE_WAIT] = "CLOSE-WAIT",
    [CLOSING] = "CLOSING",
    [LAST_ACK] = "LAST-ACK",
    [TIME_WAIT] = "TIME-WAIT",
};

static inline const char* netstat_cli_get_connection_state_string(enum tcp_state state) {
    return (state < COUNT_OF(netstat_cli_connection_state_string_map)) ?
               netstat_cli_connection_state_string_map[state] :
               "UNKNOWN";
}

static inline u32_t netstat_cli_get_pcb_unsent_bytes_count(struct tcp_pcb* pcb) {
    u32_t unsent_bytes_count = 0;

    for(struct tcp_seg* seg = pcb->unsent; seg; seg = seg->next) {
        unsent_bytes_count += seg->len;
    }

    return unsent_bytes_count;
}

static inline u32_t netstat_cli_get_pcb_unacked_bytes_count(struct tcp_pcb* pcb) {
    u32_t unacked_bytes_count = 0;

    for(struct tcp_seg* seg = pcb->unacked; seg; seg = seg->next) {
        unacked_bytes_count += seg->len;
    }

    return unacked_bytes_count;
}

static inline u32_t netstat_cli_get_pcb_ooseq_bytes_count(struct tcp_pcb* pcb) {
    u32_t ooseq_bytes_count = 0;

#if TCP_QUEUE_OOSEQ
    for(struct tcp_seg* seg = pcb->ooseq; seg; seg = seg->next) {
        ooseq_bytes_count += seg->len;
    }
#else /* TCP_QUEUE_OOSEQ */
    UNUSED(pcb);
#endif /* TCP_QUEUE_OOSEQ */

    return ooseq_bytes_count;
}

static inline u32_t netstat_cli_get_pcb_refused_bytes_count(struct tcp_pcb* pcb) {
    return (pcb->refused_data) ? pcb->refused_data->tot_len : 0;
}

static void netstat_cli_print_tcp_pcb_entry(struct tcp_pcb* pcb) {
    const char* remote_ip_string;
    u32_t receive_queue_size, send_queue_size;
    switch(pcb->state) {
    case CLOSED: {
        remote_ip_string = "*:*";

        receive_queue_size = 0;
        send_queue_size = 0;
        break;
    }

    case LISTEN: {
        remote_ip_string = "*:*";

#if TCP_LISTEN_BACKLOG
        struct tcp_pcb_listen* lpcb = (struct tcp_pcb_listen*)pcb;
        receive_queue_size = lpcb->accepts_pending;
#else /* TCP_LISTEN_BACKLOG */
        receive_queue_size = 0;
#endif /* TCP_LISTEN_BACKLOG */
        send_queue_size = 0;

        break;
    }

    default: {
        remote_ip_string =
            ipaddr_ntoa_r(&pcb->remote_ip, alloca(IPADDR_STRLEN_MAX), IPADDR_STRLEN_MAX);

        receive_queue_size = netstat_cli_get_pcb_ooseq_bytes_count(pcb) +
                             netstat_cli_get_pcb_refused_bytes_count(pcb);

        send_queue_size = netstat_cli_get_pcb_unsent_bytes_count(pcb) +
                          netstat_cli_get_pcb_unacked_bytes_count(pcb);
        break;
    }
    }

    char local_ip_string[IPADDR_STRLEN_MAX];
    ipaddr_ntoa_r(&pcb->local_ip, local_ip_string, sizeof(local_ip_string));

    printf(
        "TCP    %-6" PRIu32 " %-6" PRIu32 " %-21s %-21s %s\r\n",
        receive_queue_size,
        send_queue_size,
        local_ip_string,
        remote_ip_string,
        netstat_cli_get_connection_state_string(pcb->state));
}

static void netstat_cli_print_udp_pcb_entry(struct udp_pcb* pcb) {
    char local_ip_string[IPADDR_STRLEN_MAX];
    ipaddr_ntoa_r(&pcb->local_ip, local_ip_string, sizeof(local_ip_string));

    const char* remote_ip_string =
        (pcb->remote_port != 0) ?
            ipaddr_ntoa_r(&pcb->remote_ip, alloca(IPADDR_STRLEN_MAX), IPADDR_STRLEN_MAX) :
            "*:*";

    printf(
        "UDP    %-6" PRIu32 " %-6" PRIu32 " %-21s %-21s\r\n",
        (u32_t)0,
        (u32_t)0,
        local_ip_string,
        remote_ip_string);
}

static void netstat_cli_print_command_usage(void) {
    printf("Usage: netstat\r\n");
    printf("Print current TCP and UDP network stack state.\r\n");
}

void netstat_cli_command_entry(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    UNUSED(context);

    if(!furi_string_empty(args)) {
        netstat_cli_print_command_usage();
        return;
    }

    printf("Proto  Recv-Q Send-Q Local Address         Foreign Address       State\r\n");

    LOCK_TCPIP_CORE();

    /* TCP PCBs */
    for(size_t i = 0; i < COUNT_OF(tcp_pcb_lists); i++) {
        for(struct tcp_pcb* pcb = *tcp_pcb_lists[i]; pcb; pcb = pcb->next) {
            netstat_cli_print_tcp_pcb_entry(pcb);
        }
    }

    /* UDP PCBs */
    for(struct udp_pcb* pcb = udp_pcbs; pcb; pcb = pcb->next) {
        netstat_cli_print_udp_pcb_entry(pcb);
    }

    UNLOCK_TCPIP_CORE();
}
