#include "test_etharp.h"

#include "lwip/udp.h"
#include "lwip/etharp.h"
#include "lwip/inet.h"
#include "netif/ethernet.h"
#include "lwip/stats.h"
#include "lwip/prot/iana.h"
#if defined(SL_LWIP_ETHARP_ONDEMAND_TIMER) && SL_LWIP_ETHARP_ONDEMAND_TIMER && defined(LWIP_TESTMODE)
#include "lwip/timeouts.h"
#endif

#if !LWIP_STATS || !UDP_STATS || !MEMP_STATS || !ETHARP_STATS
#error "This tests needs UDP-, MEMP- and ETHARP-statistics enabled"
#endif
#if !ETHARP_SUPPORT_STATIC_ENTRIES
#error "This test needs ETHARP_SUPPORT_STATIC_ENTRIES enabled"
#endif

static struct netif test_netif;
static ip4_addr_t test_ipaddr, test_netmask, test_gw;
struct eth_addr test_ethaddr =  {{1,1,1,1,1,1}};
struct eth_addr test_ethaddr2 = {{1,1,1,1,1,2}};
struct eth_addr test_ethaddr3 = {{1,1,1,1,1,3}};
struct eth_addr test_ethaddr4 = {{1,1,1,1,1,4}};
static int linkoutput_ctr;

/* Helper functions */

#if defined(SL_LWIP_ETHARP_ONDEMAND_TIMER) && SL_LWIP_ETHARP_ONDEMAND_TIMER && defined(LWIP_TESTMODE)
/**
 * Safe wrapper for etharp_tmr() when on-demand timer is enabled.
 * Calls timer to process entries but immediately cancels the scheduled timeout
 * to prevent MEMP_SYS_TIMEOUT allocations.
 */
static void
test_safe_etharp_tmr(void)
{
  /* Call etharp_tmr() to process entries.
   * timer_started is already 1 from setup, so it will run and try to reschedule. */
  etharp_tmr();
  
  /* Immediately cancel any scheduled timeout to prevent MEMP_SYS_TIMEOUT allocations. */
  if (etharp_test_get_timer_started()) {
    sys_untimeout(etharp_timeout_cb, NULL);
  }
  
  /* Restore timer_started = 1 so subsequent ARP operations don't schedule new timers. */
  etharp_test_set_timer_started(1);
}
#else
#define test_safe_etharp_tmr() etharp_tmr()
#endif

static void
etharp_remove_all(void)
{
  int i;
#if defined(SL_LWIP_ETHARP_ONDEMAND_TIMER) && SL_LWIP_ETHARP_ONDEMAND_TIMER && defined(LWIP_TESTMODE)
  /* Call etharp_tmr repeatedly using our safe wrapper.
   * This properly removes dynamic entries while preserving static ones. */
  for(i = 0; i < 0xff; i++) {
    test_safe_etharp_tmr();
  }
  
  /* Clean up any stray state and restore timer_started = 1 for ongoing test.
   * This prevents subsequent ARP operations from scheduling new timers. */
  if (etharp_test_get_timer_started()) {
    sys_untimeout(etharp_timeout_cb, NULL);
  }
  etharp_test_set_timer_started(1);  /* Restore to prevent timer scheduling */
  etharp_test_set_eco_mode(0);
#else
  /* call etharp_tmr often enough to have all entries cleaned */
  for(i = 0; i < 0xff; i++) {
    etharp_tmr();
  }
#endif
}

static err_t
default_netif_linkoutput(struct netif *netif, struct pbuf *p)
{
  fail_unless(netif == &test_netif);
  fail_unless(p != NULL);
  linkoutput_ctr++;
  return ERR_OK;
}

static err_t
default_netif_init(struct netif *netif)
{
  fail_unless(netif != NULL);
  netif->linkoutput = default_netif_linkoutput;
  netif->output = etharp_output;
  netif->mtu = 1500;
  netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP;
  netif->hwaddr_len = ETHARP_HWADDR_LEN;
  return ERR_OK;
}

static void
default_netif_add(void)
{
  IP4_ADDR(&test_gw, 192,168,0,1);
  IP4_ADDR(&test_ipaddr, 192,168,0,1);
  IP4_ADDR(&test_netmask, 255,255,0,0);

  fail_unless(netif_default == NULL);
  netif_set_default(netif_add(&test_netif, &test_ipaddr, &test_netmask,
                              &test_gw, NULL, default_netif_init, NULL));
  netif_set_up(&test_netif);
}

static void
default_netif_remove(void)
{
  fail_unless(netif_default == &test_netif);
  netif_remove(&test_netif);
}

static void
create_arp_response(ip4_addr_t *adr)
{
  int k;
  struct eth_hdr *ethhdr;
  struct etharp_hdr *etharphdr;
  struct pbuf *p = pbuf_alloc(PBUF_RAW, sizeof(struct eth_hdr) + sizeof(struct etharp_hdr), PBUF_RAM);
  if(p == NULL) {
    FAIL_RET();
  }
  ethhdr = (struct eth_hdr*)p->payload;
  etharphdr = (struct etharp_hdr*)(ethhdr + 1);

  ethhdr->dest = test_ethaddr;
  ethhdr->src = test_ethaddr2;
  ethhdr->type = htons(ETHTYPE_ARP);

  etharphdr->hwtype = htons(LWIP_IANA_HWTYPE_ETHERNET);
  etharphdr->proto = htons(ETHTYPE_IP);
  etharphdr->hwlen = ETHARP_HWADDR_LEN;
  etharphdr->protolen = sizeof(ip4_addr_t);
  etharphdr->opcode = htons(ARP_REPLY);

  SMEMCPY(&etharphdr->sipaddr, adr, sizeof(ip4_addr_t));
  SMEMCPY(&etharphdr->dipaddr, &test_ipaddr, sizeof(ip4_addr_t));

  k = 6;
  while(k > 0) {
    k--;
    /* Write the ARP MAC-Addresses */
    etharphdr->shwaddr.addr[k] = test_ethaddr2.addr[k];
    etharphdr->dhwaddr.addr[k] = test_ethaddr.addr[k];
    /* Write the Ethernet MAC-Addresses */
    ethhdr->dest.addr[k] = test_ethaddr.addr[k];
    ethhdr->src.addr[k]  = test_ethaddr2.addr[k];
  }

  ethernet_input(p, &test_netif);
}

/* Setups/teardown functions */

static void
etharp_setup(void)
{
#if defined(SL_LWIP_ETHARP_ONDEMAND_TIMER) && SL_LWIP_ETHARP_ONDEMAND_TIMER && defined(LWIP_TESTMODE)
  /* Clear table directly for setup - don't use timer operations */
  etharp_test_clear_table();
  etharp_test_set_timer_started(0);
  etharp_test_set_eco_mode(0);
  etharp_test_set_last_timer_time(0);
#else
  etharp_remove_all();
#endif
  default_netif_add();
#if defined(SL_LWIP_ETHARP_ONDEMAND_TIMER) && SL_LWIP_ETHARP_ONDEMAND_TIMER && defined(LWIP_TESTMODE)
  /* Keep timer_started = 1 so ARP operations don't try to schedule timers.
   * This prevents MEMP_SYS_TIMEOUT allocations during the test. */
  etharp_test_set_timer_started(1);
#endif
  lwip_check_ensure_no_alloc(SKIP_POOL(MEMP_SYS_TIMEOUT));
}

static void
etharp_teardown(void)
{
#if defined(SL_LWIP_ETHARP_ONDEMAND_TIMER) && SL_LWIP_ETHARP_ONDEMAND_TIMER && defined(LWIP_TESTMODE)
  /* Reset timer state, then clear table directly */
  etharp_test_set_timer_started(0);
  etharp_test_clear_table();
  etharp_test_set_eco_mode(0);
  etharp_test_set_last_timer_time(0);
#else
  etharp_remove_all();
#endif
  default_netif_remove();
  lwip_check_ensure_no_alloc(SKIP_POOL(MEMP_SYS_TIMEOUT));
}


/* Test functions */

START_TEST(test_etharp_table)
{
#if ETHARP_SUPPORT_STATIC_ENTRIES
  err_t err;
#endif /* ETHARP_SUPPORT_STATIC_ENTRIES */
  ssize_t idx;
  const ip4_addr_t *unused_ipaddr;
  struct eth_addr *unused_ethaddr;
  struct udp_pcb* pcb;
  LWIP_UNUSED_ARG(_i);

  if (netif_default != &test_netif) {
    fail("This test needs a default netif");
  }

  linkoutput_ctr = 0;

  pcb = udp_new();
  fail_unless(pcb != NULL);
  if (pcb != NULL) {
    ip4_addr_t adrs[ARP_TABLE_SIZE + 2];
    int i;
    for(i = 0; i < ARP_TABLE_SIZE + 2; i++) {
      IP4_ADDR(&adrs[i], 192,168,0,i+2);
    }
    /* fill ARP-table with dynamic entries */
    for(i = 0; i < ARP_TABLE_SIZE; i++) {
      struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, 10, PBUF_RAM);
      fail_unless(p != NULL);
      if (p != NULL) {
        err_t err2;
        ip_addr_t dst;
        ip_addr_copy_from_ip4(dst, adrs[i]);
        err2 = udp_sendto(pcb, p, &dst, 123);
        fail_unless(err2 == ERR_OK);
        /* etharp request sent? */
        fail_unless(linkoutput_ctr == (2*i) + 1);
        pbuf_free(p);

        /* create an ARP response */
        create_arp_response(&adrs[i]);
        /* queued UDP packet sent? */
        fail_unless(linkoutput_ctr == (2*i) + 2);

        idx = etharp_find_addr(NULL, &adrs[i], &unused_ethaddr, &unused_ipaddr);
        fail_unless(idx == i);
        #if defined(SL_LWIP_ETHARP_ONDEMAND_TIMER) && SL_LWIP_ETHARP_ONDEMAND_TIMER && defined(LWIP_TESTMODE)
        test_safe_etharp_tmr();
        #else
        etharp_tmr();
        #endif
      }
    }
    linkoutput_ctr = 0;
#if ETHARP_SUPPORT_STATIC_ENTRIES
    /* create one static entry */
    err = etharp_add_static_entry(&adrs[ARP_TABLE_SIZE], &test_ethaddr3);
    fail_unless(err == ERR_OK);
    idx = etharp_find_addr(NULL, &adrs[ARP_TABLE_SIZE], &unused_ethaddr, &unused_ipaddr);
    fail_unless(idx == 0);
    fail_unless(linkoutput_ctr == 0);
#endif /* ETHARP_SUPPORT_STATIC_ENTRIES */

    linkoutput_ctr = 0;
    /* fill ARP-table with dynamic entries */
    for(i = 0; i < ARP_TABLE_SIZE; i++) {
      struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, 10, PBUF_RAM);
      fail_unless(p != NULL);
      if (p != NULL) {
        err_t err2;
        ip_addr_t dst;
        ip_addr_copy_from_ip4(dst, adrs[i]);
        err2 = udp_sendto(pcb, p, &dst, 123);
        fail_unless(err2 == ERR_OK);
        /* etharp request sent? */
        fail_unless(linkoutput_ctr == (2*i) + 1);
        pbuf_free(p);

        /* create an ARP response */
        create_arp_response(&adrs[i]);
        /* queued UDP packet sent? */
        fail_unless(linkoutput_ctr == (2*i) + 2);

        idx = etharp_find_addr(NULL, &adrs[i], &unused_ethaddr, &unused_ipaddr);
        if (i < ARP_TABLE_SIZE - 1) {
          fail_unless(idx == i+1);
        } else {
          /* the last entry must not overwrite the static entry! */
          fail_unless(idx == 1);
        }
        #if defined(SL_LWIP_ETHARP_ONDEMAND_TIMER) && SL_LWIP_ETHARP_ONDEMAND_TIMER && defined(LWIP_TESTMODE)
        test_safe_etharp_tmr();
        #else
        etharp_tmr();
        #endif

        
      }
    }
#if ETHARP_SUPPORT_STATIC_ENTRIES
    /* create a second static entry */
    err = etharp_add_static_entry(&adrs[ARP_TABLE_SIZE+1], &test_ethaddr4);
    fail_unless(err == ERR_OK);
    idx = etharp_find_addr(NULL, &adrs[ARP_TABLE_SIZE], &unused_ethaddr, &unused_ipaddr);
    fail_unless(idx == 0);
    idx = etharp_find_addr(NULL, &adrs[ARP_TABLE_SIZE+1], &unused_ethaddr, &unused_ipaddr);
    fail_unless(idx == 2);
    /* and remove it again */
    err = etharp_remove_static_entry(&adrs[ARP_TABLE_SIZE+1]);
    fail_unless(err == ERR_OK);
    idx = etharp_find_addr(NULL, &adrs[ARP_TABLE_SIZE], &unused_ethaddr, &unused_ipaddr);
    fail_unless(idx == 0);
    idx = etharp_find_addr(NULL, &adrs[ARP_TABLE_SIZE+1], &unused_ethaddr, &unused_ipaddr);
    fail_unless(idx == -1);
#endif /* ETHARP_SUPPORT_STATIC_ENTRIES */

    /* check that static entries don't time out */
    etharp_remove_all();
    idx = etharp_find_addr(NULL, &adrs[ARP_TABLE_SIZE], &unused_ethaddr, &unused_ipaddr);
    fail_unless(idx == 0);

#if ETHARP_SUPPORT_STATIC_ENTRIES
    /* remove the first static entry */
    err = etharp_remove_static_entry(&adrs[ARP_TABLE_SIZE]);
    fail_unless(err == ERR_OK);
    idx = etharp_find_addr(NULL, &adrs[ARP_TABLE_SIZE], &unused_ethaddr, &unused_ipaddr);
    fail_unless(idx == -1);
    idx = etharp_find_addr(NULL, &adrs[ARP_TABLE_SIZE+1], &unused_ethaddr, &unused_ipaddr);
    fail_unless(idx == -1);
#endif /* ETHARP_SUPPORT_STATIC_ENTRIES */

    udp_remove(pcb);
  }
}
END_TEST


/** Create the suite including all tests for this module */
Suite *
etharp_suite(void)
{
  testfunc tests[] = {
    TESTFUNC(test_etharp_table)
  };
  return create_suite("ETHARP", tests, sizeof(tests)/sizeof(testfunc), etharp_setup, etharp_teardown);
}
