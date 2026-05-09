#include "netstat.h"

#include <lwip/tcpip.h>
#include <lwip/priv/tcp_priv.h>

#define TAG                 "Netstat"
#define USAGE_PERCENT_LIMIT 75UL

static const char* monitored_pools = "NETCONN,TCP_PCB,SYS_TIMEOUT,PBUF_POOL";

bool netstat_is_overloaded(NetstatLog log) {
    furi_check(log < NetstatLogMAX);

    bool is_overloaded = false;

#if LWIP_STATS && MEMP_STATS && LWIP_STATS_DISPLAY
    LOCK_TCPIP_CORE();

    for(size_t i = 0; i < COUNT_OF(memp_pools); i++) {
        const struct memp_desc* pool_desc = memp_pools[i];
        struct stats_mem* pool_stats = lwip_stats.memp[i];

        u32_t percent_used =
            (pool_stats->avail > 0) ? ((pool_stats->used * 100) / pool_stats->avail) : 0;

        bool is_monitored = !!strstr(monitored_pools, pool_desc->desc);
        if(!is_monitored) continue;

        if(percent_used >= USAGE_PERCENT_LIMIT) {
            is_overloaded = true;

            if(log == NetstatLogOnOverload) {
                FURI_LOG_W(
                    TAG,
                    "Overload: pool %s: %" MEM_SIZE_F "/%" MEM_SIZE_F "(%lu%% >= %lu%%)",
                    pool_desc->desc,
                    pool_stats->used,
                    pool_stats->avail,
                    percent_used,
                    USAGE_PERCENT_LIMIT);
            }
        }
    }

    UNLOCK_TCPIP_CORE();
#endif

    return is_overloaded;
}
