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
    bool stats;
} NetstatCliArguments;

static const NetstatCliArguments netstat_cli_default_arguments = {
    .continuous = false,
    .help = false,
    .stats = false,
};

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

static void netstat_cli_format_ip_port(const ip_addr_t* ip, u16_t port, FuriString* buffer) {
    const char* ip_string;
    if(ip == NULL || ip_addr_isany(ip)) {
        ip_string = "*";
    } else {
        ip_string = ipaddr_ntoa_r(ip, alloca(IPADDR_STRLEN_MAX), IPADDR_STRLEN_MAX);
    }

    if(port == 0) {
        furi_string_printf(buffer, "%s:*", ip_string);
    } else {
        furi_string_printf(buffer, "%s:%" U16_F, ip_string, port);
    }
}

static u32_t netstat_cli_get_tcp_seg_bytes_count(struct tcp_seg* seg) {
    u32_t bytes_count = 0;

    for(; seg; seg = seg->next) {
        bytes_count += TCP_TCPLEN(seg);
    }

    return bytes_count;
}

static void netstat_cli_print_tcp_pcb_entry(struct tcp_pcb* pcb, FuriString* buffer) {
    FuriString* remote_ip_string = furi_string_alloc();
    FuriString* local_ip_string = furi_string_alloc();

    u32_t receive_queue_size, send_queue_size;
    switch(pcb->state) {
    case CLOSED: {
        netstat_cli_format_ip_port(NULL, 0, remote_ip_string);

        receive_queue_size = 0;
        send_queue_size = 0;
        break;
    }

    case LISTEN: {
        netstat_cli_format_ip_port(NULL, 0, remote_ip_string);

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
        netstat_cli_format_ip_port(&pcb->remote_ip, pcb->remote_port, remote_ip_string);

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

    netstat_cli_format_ip_port(&pcb->local_ip, pcb->local_port, local_ip_string);

    furi_string_cat_printf(
        buffer,
        "TCP    %-6" U32_F " %-6" U32_F
        " %-22s %-22s %s" ANSI_ERASE_LINE(ANSI_ERASE_FROM_CURSOR_TO_END) "\r\n",
        receive_queue_size,
        send_queue_size,
        furi_string_get_cstr(local_ip_string),
        furi_string_get_cstr(remote_ip_string),
        netstat_cli_get_connection_state_string(pcb->state));

    furi_string_free(local_ip_string);
    furi_string_free(remote_ip_string);
}

static void netstat_cli_print_udp_pcb_entry(struct udp_pcb* pcb, FuriString* buffer) {
    FuriString* remote_ip_string = furi_string_alloc();
    FuriString* local_ip_string = furi_string_alloc();

    netstat_cli_format_ip_port(&pcb->remote_ip, pcb->remote_port, remote_ip_string);
    netstat_cli_format_ip_port(&pcb->local_ip, pcb->local_port, local_ip_string);

    furi_string_cat_printf(
        buffer,
        "UDP    %-6" U32_F " %-6" U32_F
        " %-22s %-22s" ANSI_ERASE_LINE(ANSI_ERASE_FROM_CURSOR_TO_END) "\r\n",
        (u32_t)0,
        (u32_t)0,
        furi_string_get_cstr(local_ip_string),
        furi_string_get_cstr(remote_ip_string));

    furi_string_free(local_ip_string);
    furi_string_free(remote_ip_string);
}

static bool netstat_cli_print_pcb_table(void) {
    printf(
        "%-6s %-6s %-6s %-22s %-22s %s" ANSI_ERASE_LINE(ANSI_ERASE_FROM_CURSOR_TO_END) "\r\n",
        "Proto",
        "Recv-Q",
        "Send-Q",
        "Local Address",
        "Foreign Address",
        "State");

    FuriString* output_buffer = furi_string_alloc();

    LOCK_TCPIP_CORE();

    /* TCP PCBs */
    for(size_t i = 0; i < COUNT_OF(tcp_pcb_lists); i++) {
        for(struct tcp_pcb* pcb = *tcp_pcb_lists[i]; pcb; pcb = pcb->next) {
            netstat_cli_print_tcp_pcb_entry(pcb, output_buffer);
        }
    }

    /* UDP PCBs */
    for(struct udp_pcb* pcb = udp_pcbs; pcb; pcb = pcb->next) {
        netstat_cli_print_udp_pcb_entry(pcb, output_buffer);
    }

    UNLOCK_TCPIP_CORE();

    printf("%s", furi_string_get_cstr(output_buffer));

    furi_string_free(output_buffer);
    return true;
}

static bool netstat_cli_print_memp_stats(void) {
#if LWIP_STATS && MEMP_STATS && LWIP_STATS_DISPLAY
    printf(
        "%-20s %6s %6s %9s %6s %6s" ANSI_ERASE_LINE(ANSI_ERASE_FROM_CURSOR_TO_END) "\r\n",
        "Pool",
        "Used",
        "Total",
        "Watermark",
        "Errors",
        "%Util");

    FuriString* output_buffer = furi_string_alloc();

    LOCK_TCPIP_CORE();

    for(size_t i = 0; i < COUNT_OF(memp_pools); i++) {
        const struct memp_desc* pool_desc = memp_pools[i];
        struct stats_mem* pool_stats = lwip_stats.memp[i];

        u32_t percent_used =
            (pool_stats->avail > 0) ? ((pool_stats->used * 100) / pool_stats->avail) : 0;

        furi_string_cat_printf(
            output_buffer,
            "%-20s %6" MEM_SIZE_F " %6" MEM_SIZE_F " %9" MEM_SIZE_F " %6" STAT_COUNTER_F
            " %6" PRIu32 ANSI_ERASE_LINE(ANSI_ERASE_FROM_CURSOR_TO_END) "\r\n",
            pool_desc->desc,
            pool_stats->used,
            pool_stats->avail,
            pool_stats->max,
            pool_stats->err,
            percent_used);
    }

    UNLOCK_TCPIP_CORE();

    printf("%s", furi_string_get_cstr(output_buffer));

    furi_string_free(output_buffer);
    return true;
#else /* LWIP_STATS && MEMP_STATS && LWIP_STATS_DISPLAY */
    printf("Statistics not enabled in lwIP configuration.\r\n");
    return false;
#endif /* LWIP_STATS && MEMP_STATS && LWIP_STATS_DISPLAY */
}

static void netstat_cli_print_command_usage(void) {
    printf("Usage: netstat [options]\r\n");
    printf("Print network stack state.\r\n");
    printf("Options:\r\n");
    printf("  -c, --continuous    Continuous output mode (refresh every 1s)\r\n");
    printf("  -s, --stats         Show lwIP buffer statistics\r\n");
    printf("  -h, --help          Show this help message\r\n");
}

static void netstat_cli_parse_arguments(FuriString* args_string, NetstatCliArguments* args) {
    *args = netstat_cli_default_arguments;

    FuriString* arg = furi_string_alloc();
    while(args_read_string_and_trim(args_string, arg)) {
        if(furi_string_equal_str(arg, "-c") || furi_string_equal_str(arg, "--continuous")) {
            args->continuous = true;
        } else if(furi_string_equal_str(arg, "-s") || furi_string_equal_str(arg, "--stats")) {
            args->stats = true;
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

    bool (*print_callback)(void);
    if(args.stats) {
        print_callback = netstat_cli_print_memp_stats;
    } else {
        print_callback = netstat_cli_print_pcb_table;
    }

    if(args.continuous) {
        while(!cli_is_pipe_broken_or_is_etx_next_char(pipe)) {
            printf(ANSI_CURSOR_POS("1", "1"));

            bool did_print_fail = !print_callback();

            printf(ANSI_ERASE_DISPLAY(ANSI_ERASE_FROM_CURSOR_TO_END));
            fflush(stdout);

            if(did_print_fail) break;

            furi_delay_ms(CONTINUOUS_REFRESH_DELAY_MS);
        }
    } else {
        print_callback();
    }
}
