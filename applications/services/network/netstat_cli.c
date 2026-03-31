#include <cli/cli_command.h>
#include <cli/cli_ansi.h>
#include <cli/args.h>

#include <lwip/tcpip.h>
#include <lwip/udp.h>
#include <lwip/priv/tcp_priv.h>

#include <inttypes.h>

#define CONTINUOUS_REFRESH_DELAY_MS 1000

typedef struct {
    bool continuous;
    bool help;
} NetstatCliArguments;

static const NetstatCliArguments netstat_cli_default_arguments = {
    .continuous = false,
    .help = false,
};

static inline u32_t netstat_cli_get_tcp_seg_bytes_count(struct tcp_seg* seg) {
    u32_t bytes_count = 0;

    for(; seg; seg = seg->next) {
        bytes_count += TCP_TCPLEN(seg);
    }

    return bytes_count;
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

        u32_t ooseq_bytes_count;

#if TCP_QUEUE_OOSEQ
        ooseq_bytes_count = netstat_cli_get_tcp_seg_bytes_count(pcb->ooseq);
#else /* TCP_QUEUE_OOSEQ */
        ooseq_bytes_count = 0;
#endif /* TCP_QUEUE_OOSEQ */

        receive_queue_size =
            ooseq_bytes_count + ((pcb->refused_data) ? pcb->refused_data->tot_len : 0);

        send_queue_size = netstat_cli_get_tcp_seg_bytes_count(pcb->unsent) +
                          netstat_cli_get_tcp_seg_bytes_count(pcb->unacked);
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
        tcp_debug_state_str(pcb->state));
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

static void netstat_cli_print_pcb_table(void) {
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

static void netstat_cli_print_command_usage(void) {
    printf("Usage: netstat [options]\r\n");
    printf("Print network stack state.\r\n");
    printf("Options:\r\n");
    printf("  -c, --continuous    Continuous output mode (refresh every 1s)\r\n");
    printf("  -h, --help          Show this help message\r\n");
}

static void netstat_cli_parse_arguments(FuriString* args_string, NetstatCliArguments* args) {
    *args = netstat_cli_default_arguments;

    FuriString* arg = furi_string_alloc();
    while(args_read_string_and_trim(args_string, arg)) {
        if(furi_string_equal_str(arg, "-c") || furi_string_equal_str(arg, "--continuous")) {
            args->continuous = true;
        } else if(furi_string_equal_str(arg, "-h") || furi_string_equal_str(arg, "--help")) {
            args->help = true;
        } else {
            printf("Unknown argument: %s\r\n", furi_string_get_cstr(arg));
            args->help = true;
            break;
        }
    }

    furi_string_free(arg);
}

void netstat_cli_command_entry(PipeSide* pipe, FuriString* args_string, void* context) {
    UNUSED(context);

    NetstatCliArguments args;
    netstat_cli_parse_arguments(args_string, &args);

    if(args.help) {
        netstat_cli_print_command_usage();
        return;
    }

    if(args.continuous) {
        while(!cli_is_pipe_broken_or_is_etx_next_char(pipe)) {
            printf(ANSI_CURSOR_POS("1", "1"));
            printf(ANSI_ERASE_DISPLAY(ANSI_ERASE_FROM_CURSOR_TO_END));

            netstat_cli_print_pcb_table();

            fflush(stdout);

            furi_delay_ms(CONTINUOUS_REFRESH_DELAY_MS);
        }

        return;
    }

    netstat_cli_print_pcb_table();
}
