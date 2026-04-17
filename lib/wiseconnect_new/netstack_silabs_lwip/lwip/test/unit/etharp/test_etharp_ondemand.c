#include "test_etharp.h"

#include "lwip/etharp.h"
#include "lwip/inet.h"
#include "netif/ethernet.h"
#include "lwip/stats.h"
#include "lwip/prot/iana.h"
#include "lwip/opt.h"

/* This test file is only active when SL_LWIP_ETHARP_ONDEMAND_TIMER is enabled */
#if defined(SL_LWIP_ETHARP_ONDEMAND_TIMER) && SL_LWIP_ETHARP_ONDEMAND_TIMER && defined(LWIP_TESTMODE)

#include "lwip/timeouts.h"
#include "arch/sys_arch.h"
#include "lwip/sys.h"
#include <stdio.h>
#include <string.h>

/* Test timeout constants (in milliseconds) */
#define TEST_TIMEOUT_1SEC_MS     1000
#define TEST_TIMEOUT_5SEC_MS     5000
#define TEST_TIMEOUT_10SEC_MS    10000
#define TEST_TIMEOUT_30SEC_MS    30000
#define TEST_TIMEOUT_60SEC_MS    60000
#define TEST_TIMEOUT_270SEC_MS   270000
#define TEST_TIMEOUT_300SEC_MS   300000

/* ARP age constants (matching etharp.c) */
#define ARP_AGE_REREQUEST_USED_UNICAST   (ARP_MAXAGE - 30)   /* 270s */
#define ARP_AGE_REREQUEST_USED_BROADCAST (ARP_MAXAGE - 15)   /* 285s */

/* ARP state enum values (matching etharp.c) */
#define ETHARP_STATE_EMPTY                  0
#define ETHARP_STATE_PENDING                1
#define ETHARP_STATE_STABLE                 2
#define ETHARP_STATE_STABLE_REREQUESTING_1  3
#define ETHARP_STATE_STABLE_REREQUESTING_2  4
#define ETHARP_STATE_STATIC                 5

static struct netif test_netif;
static ip4_addr_t test_ipaddr, test_netmask, test_gw;
static struct eth_addr test_ethaddr = {{0x00, 0x01, 0x02, 0x03, 0x04, 0x05}};
static int linkoutput_ctr;

#ifdef LWIP_TESTMODE
/* ⚠️  MAINTENANCE WARNING: This structure is duplicated from etharp.c for white-box testing.
 *     It MUST be kept synchronized with the internal etharp_entry structure in etharp.c.
 */
struct etharp_entry {
#if ARP_QUEUEING
  struct etharp_q_entry *q;
#else
  struct pbuf *q;
#endif
  ip4_addr_t ipaddr;
  struct netif *netif;
  struct eth_addr ethaddr;
  u16_t ctime;
  u8_t state;
};
#endif /* LWIP_TESTMODE */

/* Helper to check if etharp_timeout_cb is scheduled */
static int
has_etharp_timeout_scheduled(u32_t *when_out)
{
  struct sys_timeo* t = *sys_timeouts_get_next_timeout();
  while (t != NULL) {
    if (t->h == etharp_timeout_cb) {
      if (when_out) { *when_out = t->time; }
      return 1;
    }
    t = t->next;
  }
  return 0;
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
  netif->hwaddr_len = ETH_HWADDR_LEN;
  SMEMCPY(netif->hwaddr, &test_ethaddr, ETH_HWADDR_LEN);
  return ERR_OK;
}

static void
default_netif_add(void)
{
  IP4_ADDR(&test_gw, 192, 168, 0, 1);
  IP4_ADDR(&test_ipaddr, 192, 168, 0, 10);
  IP4_ADDR(&test_netmask, 255, 255, 255, 0);

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

/* Setups/teardown functions */
static struct sys_timeo* old_list_head;

static void
etharp_ondemand_setup(void)
{
  struct sys_timeo** list_head;
  struct sys_timeo* current;
  int leaked_timers = 0;
  
  list_head = sys_timeouts_get_next_timeout();
  current = *list_head;
  while (current != NULL) {
    if (current->h == etharp_timeout_cb) {
      leaked_timers++;
    }
    current = current->next;
  }
  
  if (leaked_timers > 0) {
    fail_unless(leaked_timers == 0);
  }
  
  old_list_head = *list_head;
  *list_head = NULL;
  lwip_sys_now = 0;
  
  etharp_test_set_timer_started(0);
  etharp_test_set_eco_mode(0);
  etharp_test_set_last_timer_time(0);
  etharp_test_clear_table();
  
  default_netif_add();
  linkoutput_ctr = 0;
  
  lwip_check_ensure_no_alloc(SKIP_POOL(MEMP_SYS_TIMEOUT));
}

static void
etharp_ondemand_teardown(void)
{
  struct sys_timeo** list_head;
  struct sys_timeo* current;
  struct sys_timeo* next;
  int leaked_timers = 0;
  
  list_head = sys_timeouts_get_next_timeout();
  current = *list_head;
  
  while (current != NULL) {
    next = current->next;
    if (current->h == etharp_timeout_cb) {
      leaked_timers++;
      sys_untimeout(etharp_timeout_cb, current->arg);
    }
    current = next;
  }
  
  etharp_test_clear_table();
  etharp_test_set_timer_started(0);
  etharp_test_set_eco_mode(0);
  etharp_test_set_last_timer_time(0);
  
  default_netif_remove();
  
  *list_head = old_list_head;
  lwip_sys_now = 0;
  
  lwip_check_ensure_no_alloc(SKIP_POOL(MEMP_SYS_TIMEOUT));
}

/* Test 1: Timer starts when first entry is added */
START_TEST(test_etharp_timer_starts_on_first_entry)
{
  ip4_addr_t ipaddr;
  struct eth_addr ethaddr = {{0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff}};
  u32_t when;
  LWIP_UNUSED_ARG(_i);

  /* Verify timer not running initially */
  fail_unless(etharp_test_get_timer_started() == 0);
  fail_unless(!has_etharp_timeout_scheduled(NULL));
  
  /* Add first STABLE entry */
  IP4_ADDR(&ipaddr, 192, 168, 0, 20);
  etharp_test_init_entry(0, ETHARP_STATE_STABLE, &ipaddr, &ethaddr, &test_netif, 0);
  
  /* Manually trigger what etharp_update_arp_entry does */
  etharp_test_set_timer_started(1);
  sys_timeout(ARP_TMR_INTERVAL, etharp_timeout_cb, NULL);
  
  /* Verify timer started */
  fail_unless(etharp_test_get_timer_started() == 1);
  fail_unless(has_etharp_timeout_scheduled(&when));
  fail_unless(when == ARP_TMR_INTERVAL);
}
END_TEST

/* Test 2: ECO mode activates with only STABLE entries */
START_TEST(test_etharp_eco_mode_activation)
{
  ip4_addr_t ipaddr;
  struct eth_addr ethaddr = {{0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff}};
  u32_t when;
  LWIP_UNUSED_ARG(_i);

  /* Add STABLE entry with age 0 */
  IP4_ADDR(&ipaddr, 192, 168, 0, 20);
  etharp_test_init_entry(0, ETHARP_STATE_STABLE, &ipaddr, &ethaddr, &test_netif, 0);
  
  /* Setup timer - set last_timer_time to prevent aging during this call */
  etharp_test_set_timer_started(1);
  etharp_test_set_eco_mode(0);
  lwip_sys_now = 0;
  etharp_test_set_last_timer_time(0);
  
  /* Call etharp_tmr - should enter ECO mode */
  etharp_tmr();
  
  /* Verify ECO mode activated with long timeout */
  fail_unless(etharp_test_get_eco_mode() == 1);
  fail_unless(has_etharp_timeout_scheduled(&when));
  fail_unless(when > ARP_TMR_INTERVAL);
  /* Entry aged by 1 (0->1), so timeout should be (270-1)*1000 = 269000ms */
  fail_unless(when == 269000);
}
END_TEST

/* Test 3: ACTIVE mode maintained with PENDING entry */
START_TEST(test_etharp_active_mode_with_pending)
{
  ip4_addr_t ipaddr;
  struct eth_addr ethaddr = {{0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff}};
  u32_t when;
  LWIP_UNUSED_ARG(_i);

  /* Add PENDING entry */
  IP4_ADDR(&ipaddr, 192, 168, 0, 20);
  etharp_test_init_entry(0, ETHARP_STATE_PENDING, &ipaddr, &ethaddr, &test_netif, 0);
  
  /* Setup timer in ACTIVE mode */
  etharp_test_set_timer_started(1);
  etharp_test_set_eco_mode(0);
  lwip_sys_now = 0;
  
  /* Call etharp_tmr */
  etharp_tmr();
  
  /* Verify ACTIVE mode maintained */
  fail_unless(etharp_test_get_eco_mode() == 0);
  fail_unless(has_etharp_timeout_scheduled(&when));
  fail_unless(when == ARP_TMR_INTERVAL);
}
END_TEST

/* Test 4: Timer stops when table becomes empty */
START_TEST(test_etharp_timer_stops_when_empty)
{
  LWIP_UNUSED_ARG(_i);

  /* Setup timer running with a scheduled timeout */
  etharp_test_set_timer_started(1);
  etharp_test_set_eco_mode(1);
  etharp_test_set_last_timer_time(0);
  lwip_sys_now = 0;
  sys_timeout(TEST_TIMEOUT_10SEC_MS, etharp_timeout_cb, NULL);
  
  fail_unless(etharp_test_get_timer_started() == 1);
  
  /* Call etharp_tmr with empty table - should set timer_started to 0 */
  etharp_tmr();
  
  /* Verify timer_started flag is cleared */
  fail_unless(etharp_test_get_timer_started() == 0);
  
  /* Note: The timeout is left scheduled because etharp_timer_stop() checks
   * arp_timer_started which was already set to 0 by etharp_tmr().
   * The teardown will clean up the orphaned timeout. */
}
END_TEST

/* Test 5: ECO to ACTIVE transition on new PENDING entry */
START_TEST(test_etharp_eco_to_active_transition)
{
  ip4_addr_t ipaddr1, ipaddr2;
  struct eth_addr ethaddr = {{0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff}};
  u32_t when;
  LWIP_UNUSED_ARG(_i);

  /* Add STABLE entry */
  IP4_ADDR(&ipaddr1, 192, 168, 0, 20);
  etharp_test_init_entry(0, ETHARP_STATE_STABLE, &ipaddr1, &ethaddr, &test_netif, 0);
  
  /* Setup ECO mode */
  etharp_test_set_timer_started(1);
  etharp_test_set_eco_mode(1);
  etharp_test_set_last_timer_time(0);
  lwip_sys_now = 0;
  sys_timeout(TEST_TIMEOUT_270SEC_MS, etharp_timeout_cb, NULL);
  
  fail_unless(etharp_test_get_eco_mode() == 1);
  
  /* Add PENDING entry - should trigger ECO→ACTIVE transition */
  IP4_ADDR(&ipaddr2, 192, 168, 0, 21);
  etharp_test_init_entry(1, ETHARP_STATE_PENDING, &ipaddr2, &ethaddr, &test_netif, 0);
  
  /* Simulate what etharp_query does */
  etharp_test_set_eco_mode(0);
  sys_untimeout(etharp_timeout_cb, NULL);
  sys_timeout(ARP_TMR_INTERVAL, etharp_timeout_cb, NULL);
  etharp_test_set_last_timer_time(lwip_sys_now);
  
  /* Verify transition to ACTIVE */
  fail_unless(etharp_test_get_eco_mode() == 0);
  fail_unless(has_etharp_timeout_scheduled(&when));
  fail_unless(when == ARP_TMR_INTERVAL);
}
END_TEST

/* Test 6: Entry aging in ACTIVE mode (1 second per tick) */
START_TEST(test_etharp_entry_aging_active_mode)
{
  ip4_addr_t ipaddr;
  struct eth_addr ethaddr = {{0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff}};
  struct etharp_entry *entry;
  LWIP_UNUSED_ARG(_i);

  /* Add STABLE entry with age 100 */
  IP4_ADDR(&ipaddr, 192, 168, 0, 20);
  etharp_test_init_entry(0, ETHARP_STATE_STABLE, &ipaddr, &ethaddr, &test_netif, 100);
  
  /* Setup ACTIVE mode */
  etharp_test_set_timer_started(1);
  etharp_test_set_eco_mode(0);
  lwip_sys_now = 0;
  
  /* Call etharp_tmr (ACTIVE mode ages by 1 second) */
  etharp_tmr();
  
  /* Verify age incremented by 1 */
  entry = etharp_test_get_table_entry(0);
  fail_unless(entry != NULL);
  fail_unless(entry->ctime == 101);
}
END_TEST

/* Test 7: Entry aging in ECO mode (elapsed time) */
START_TEST(test_etharp_entry_aging_eco_mode)
{
  ip4_addr_t ipaddr;
  struct eth_addr ethaddr = {{0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff}};
  struct etharp_entry *entry;
  LWIP_UNUSED_ARG(_i);

  /* Add STABLE entry with age 100 */
  IP4_ADDR(&ipaddr, 192, 168, 0, 20);
  etharp_test_init_entry(0, ETHARP_STATE_STABLE, &ipaddr, &ethaddr, &test_netif, 100);
  
  /* Setup ECO mode with initial non-zero time to enable elapsed calculation */
  lwip_sys_now = 1000;  /* Start at 1 second */
  etharp_test_set_timer_started(1);
  etharp_test_set_eco_mode(1);
  etharp_test_set_last_timer_time(lwip_sys_now);  /* Set to current time */
  
  /* Advance time by 30 seconds */
  lwip_sys_now += TEST_TIMEOUT_30SEC_MS;  /* Now at 31 seconds */
  
  /* Call etharp_tmr (ECO mode ages by elapsed time: 30s) */
  etharp_tmr();
  
  /* Verify age incremented by 30 seconds */
  entry = etharp_test_get_table_entry(0);
  fail_unless(entry != NULL);
  fail_unless(entry->ctime == 130);
}
END_TEST

/* Test 8: Entry expiration at ARP_MAXAGE (300s) */
START_TEST(test_etharp_entry_expiration)
{
  ip4_addr_t ipaddr;
  struct eth_addr ethaddr = {{0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff}};
  struct etharp_entry *entry;
  LWIP_UNUSED_ARG(_i);

  /* Add STABLE entry at age 299 (1 second before expiry) */
  IP4_ADDR(&ipaddr, 192, 168, 0, 20);
  etharp_test_init_entry(0, ETHARP_STATE_STABLE, &ipaddr, &ethaddr, &test_netif, 299);
  
  /* Setup ACTIVE mode */
  etharp_test_set_timer_started(1);
  etharp_test_set_eco_mode(0);
  lwip_sys_now = 0;
  
  /* Call etharp_tmr - should age to 300 and expire */
  etharp_tmr();
  
  /* Verify entry expired (state = EMPTY) */
  entry = etharp_test_get_table_entry(0);
  fail_unless(entry != NULL);
  fail_unless(entry->state == ETHARP_STATE_EMPTY);
}
END_TEST

/* Test 9: PENDING entry expiration at ARP_MAXPENDING (5s) */
START_TEST(test_etharp_pending_expiration)
{
  ip4_addr_t ipaddr;
  struct eth_addr ethaddr = {{0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff}};
  struct etharp_entry *entry;
  LWIP_UNUSED_ARG(_i);

  /* Add PENDING entry at age 4 (1 second before expiry) */
  IP4_ADDR(&ipaddr, 192, 168, 0, 20);
  etharp_test_init_entry(0, ETHARP_STATE_PENDING, &ipaddr, &ethaddr, &test_netif, 4);
  
  /* Setup ACTIVE mode */
  etharp_test_set_timer_started(1);
  etharp_test_set_eco_mode(0);
  lwip_sys_now = 0;
  
  /* Call etharp_tmr - should age to 5 and expire */
  etharp_tmr();
  
  /* Verify entry expired */
  entry = etharp_test_get_table_entry(0);
  fail_unless(entry != NULL);
  fail_unless(entry->state == ETHARP_STATE_EMPTY);
}
END_TEST

/* Test 10: Minimum timeout selection with multiple entries */
START_TEST(test_etharp_min_timeout_selection)
{
  ip4_addr_t ipaddr1, ipaddr2, ipaddr3;
  struct eth_addr ethaddr = {{0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff}};
  u32_t when;
  LWIP_UNUSED_ARG(_i);

  /* Entry 1: age 10s, time to unicast = 260s */
  IP4_ADDR(&ipaddr1, 192, 168, 0, 20);
  etharp_test_init_entry(0, ETHARP_STATE_STABLE, &ipaddr1, &ethaddr, &test_netif, 10);
  
  /* Entry 2: age 250s, time to unicast = 20s (MINIMUM) */
  IP4_ADDR(&ipaddr2, 192, 168, 0, 21);
  etharp_test_init_entry(1, ETHARP_STATE_STABLE, &ipaddr2, &ethaddr, &test_netif, 250);
  
  /* Entry 3: age 100s, time to unicast = 170s */
  IP4_ADDR(&ipaddr3, 192, 168, 0, 22);
  etharp_test_init_entry(2, ETHARP_STATE_STABLE, &ipaddr3, &ethaddr, &test_netif, 100);
  
  /* Setup ECO mode */
  etharp_test_set_timer_started(1);
  etharp_test_set_eco_mode(1);
  etharp_test_set_last_timer_time(0);
  lwip_sys_now = 0;
  
  /* Call etharp_tmr - ages by 1s: 10->11, 250->251, 100->101 */
  etharp_tmr();
  
  /* Verify minimum timeout selected: min(259s, 19s, 169s) = 19s = 19000ms */
  fail_unless(has_etharp_timeout_scheduled(&when));
  fail_unless(when == 19000);
}
END_TEST

/* Test 11: Wraparound handling in ECO mode */
START_TEST(test_etharp_wraparound_handling)
{
  ip4_addr_t ipaddr;
  struct eth_addr ethaddr = {{0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff}};
  struct etharp_entry *entry;
  u32_t near_max, after_wrap;
  LWIP_UNUSED_ARG(_i);

  /* Add STABLE entry with age 100 */
  IP4_ADDR(&ipaddr, 192, 168, 0, 20);
  etharp_test_init_entry(0, ETHARP_STATE_STABLE, &ipaddr, &ethaddr, &test_netif, 100);
  
  /* Setup ECO mode near wraparound */
  near_max = (u32_t)(-5000); /* 5 seconds before wraparound */
  etharp_test_set_timer_started(1);
  etharp_test_set_eco_mode(1);
  etharp_test_set_last_timer_time(near_max);
  lwip_sys_now = near_max;
  
  /* Advance time past wraparound */
  after_wrap = 5000; /* 5 seconds after wraparound */
  lwip_sys_now = after_wrap;
  
  /* Call etharp_tmr - should handle wraparound (10s elapsed) */
  etharp_tmr();
  
  /* Verify age incremented by 10 seconds */
  entry = etharp_test_get_table_entry(0);
  fail_unless(entry != NULL);
  fail_unless(entry->ctime == 110);
}
END_TEST

/* Test 12: Static entry handling (not aged) */
START_TEST(test_etharp_static_entry_not_aged)
{
  ip4_addr_t ipaddr;
  struct eth_addr ethaddr = {{0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff}};
  struct etharp_entry *entry;
  LWIP_UNUSED_ARG(_i);

#if ETHARP_SUPPORT_STATIC_ENTRIES
  /* Add STATIC entry */
  IP4_ADDR(&ipaddr, 192, 168, 0, 20);
  etharp_test_init_entry(0, ETHARP_STATE_STATIC, &ipaddr, &ethaddr, &test_netif, 100);
  
  /* Setup ACTIVE mode */
  etharp_test_set_timer_started(1);
  etharp_test_set_eco_mode(0);
  lwip_sys_now = 0;
  
  /* Call etharp_tmr multiple times */
  etharp_tmr();
  etharp_tmr();
  etharp_tmr();
  
  /* Verify age unchanged (static entries not aged) */
  entry = etharp_test_get_table_entry(0);
  fail_unless(entry != NULL);
  fail_unless(entry->state == ETHARP_STATE_STATIC);
  fail_unless(entry->ctime == 100);
#endif
}
END_TEST

/* Test 13: Timer reschedule after entry removal */
START_TEST(test_etharp_timer_reschedule_on_removal)
{
  ip4_addr_t ipaddr1, ipaddr2, ipaddr3;
  struct eth_addr ethaddr = {{0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff}};
  struct etharp_entry *entry;
  LWIP_UNUSED_ARG(_i);

  /* Create 3 entries at different ages */
  IP4_ADDR(&ipaddr1, 192, 168, 0, 20);
  etharp_test_init_entry(0, ETHARP_STATE_STABLE, &ipaddr1, &ethaddr, &test_netif, 50);
  
  IP4_ADDR(&ipaddr2, 192, 168, 0, 21);
  etharp_test_init_entry(1, ETHARP_STATE_STABLE, &ipaddr2, &ethaddr, &test_netif, 100);
  
  IP4_ADDR(&ipaddr3, 192, 168, 0, 22);
  etharp_test_init_entry(2, ETHARP_STATE_STABLE, &ipaddr3, &ethaddr, &test_netif, 150);
  
  /* Setup timer */
  lwip_sys_now = 1000;
  etharp_test_set_timer_started(1);
  etharp_test_set_eco_mode(0);
  
  /* Call etharp_tmr - all entries age, timer schedules */
  etharp_tmr();
  fail_unless(has_etharp_timeout_scheduled(NULL));
  
  /* Remove the oldest entry */
  entry = etharp_test_get_table_entry(2);
  entry->state = ETHARP_STATE_EMPTY;
  
  /* Advance time slightly */
  lwip_sys_now = 2000;
  
  /* Call etharp_tmr again - should reschedule based on remaining entries */
  etharp_tmr();
  
  /* Key test: timer is still scheduled (didn't stop) */
  fail_unless(has_etharp_timeout_scheduled(NULL));
  
  /* Verify remaining entries still exist */
  fail_unless(etharp_test_get_table_entry(0)->state == ETHARP_STATE_STABLE);
  fail_unless(etharp_test_get_table_entry(1)->state == ETHARP_STATE_STABLE);
}
END_TEST

/* Test 14: Mixed STABLE and PENDING entries use ACTIVE mode */
START_TEST(test_etharp_mixed_entries_active_mode)
{
  ip4_addr_t ipaddr1, ipaddr2;
  struct eth_addr ethaddr = {{0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff}};
  u32_t when;
  LWIP_UNUSED_ARG(_i);

  /* Entry 1: STABLE */
  IP4_ADDR(&ipaddr1, 192, 168, 0, 20);
  etharp_test_init_entry(0, ETHARP_STATE_STABLE, &ipaddr1, &ethaddr, &test_netif, 0);
  
  /* Entry 2: PENDING */
  IP4_ADDR(&ipaddr2, 192, 168, 0, 21);
  etharp_test_init_entry(1, ETHARP_STATE_PENDING, &ipaddr2, &ethaddr, &test_netif, 0);
  
  /* Setup timer */
  etharp_test_set_timer_started(1);
  etharp_test_set_eco_mode(0);
  lwip_sys_now = 0;
  
  /* Call etharp_tmr */
  etharp_tmr();
  
  /* Should use ACTIVE mode (1s interval) due to PENDING entry */
  fail_unless(etharp_test_get_eco_mode() == 0);
  fail_unless(has_etharp_timeout_scheduled(&when));
  fail_unless(when == ARP_TMR_INTERVAL);
}
END_TEST

/* Test 15: STABLE_REREQUESTING_1 state transition */
START_TEST(test_etharp_rerequesting_state_transition)
{
  ip4_addr_t ipaddr;
  struct eth_addr ethaddr = {{0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff}};
  struct etharp_entry *entry;
  LWIP_UNUSED_ARG(_i);

  /* Add entry in STABLE_REREQUESTING_1 state */
  IP4_ADDR(&ipaddr, 192, 168, 0, 20);
  etharp_test_init_entry(0, ETHARP_STATE_STABLE_REREQUESTING_1, &ipaddr, &ethaddr, &test_netif, 100);
  
  /* Setup ACTIVE mode */
  etharp_test_set_timer_started(1);
  etharp_test_set_eco_mode(0);
  lwip_sys_now = 0;
  
  /* Call etharp_tmr */
  etharp_tmr();
  
  /* State should transition to STABLE_REREQUESTING_2 */
  entry = etharp_test_get_table_entry(0);
  fail_unless(entry != NULL);
  fail_unless(entry->state == ETHARP_STATE_STABLE_REREQUESTING_2);
}
END_TEST

/* Test 16: STABLE_REREQUESTING_2 state transition */
START_TEST(test_etharp_rerequesting_2_state_transition)
{
  ip4_addr_t ipaddr;
  struct eth_addr ethaddr = {{0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff}};
  struct etharp_entry *entry;
  LWIP_UNUSED_ARG(_i);

  /* Add entry in STABLE_REREQUESTING_2 state */
  IP4_ADDR(&ipaddr, 192, 168, 0, 20);
  etharp_test_init_entry(0, ETHARP_STATE_STABLE_REREQUESTING_2, &ipaddr, &ethaddr, &test_netif, 100);
  
  /* Setup ACTIVE mode */
  etharp_test_set_timer_started(1);
  etharp_test_set_eco_mode(0);
  lwip_sys_now = 0;
  
  /* Call etharp_tmr */
  etharp_tmr();
  
  /* State should transition back to STABLE */
  entry = etharp_test_get_table_entry(0);
  fail_unless(entry != NULL);
  fail_unless(entry->state == ETHARP_STATE_STABLE);
}
END_TEST

/* Test 17: Rerequesting entries force ACTIVE mode in ECO */
START_TEST(test_etharp_rerequesting_forces_active_mode)
{
  ip4_addr_t ipaddr;
  struct eth_addr ethaddr = {{0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff}};
  u32_t when;
  LWIP_UNUSED_ARG(_i);

  /* Add entry in STABLE_REREQUESTING_1 state */
  IP4_ADDR(&ipaddr, 192, 168, 0, 20);
  etharp_test_init_entry(0, ETHARP_STATE_STABLE_REREQUESTING_1, &ipaddr, &ethaddr, &test_netif, 270);
  
  /* Setup ECO mode */
  etharp_test_set_timer_started(1);
  etharp_test_set_eco_mode(1);
  lwip_sys_now = 0;
  
  /* Call etharp_tmr */
  etharp_tmr();
  
  /* Should force ACTIVE mode for rerequesting state handling */
  fail_unless(etharp_test_get_eco_mode() == 0);
  fail_unless(has_etharp_timeout_scheduled(&when));
  fail_unless(when == ARP_TMR_INTERVAL);
}
END_TEST

/* Test 18: ECO mode with entries at different ages */
START_TEST(test_etharp_eco_mode_different_ages)
{
  ip4_addr_t ipaddr1, ipaddr2, ipaddr3;
  struct eth_addr ethaddr = {{0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff}};
  u32_t when;
  LWIP_UNUSED_ARG(_i);

  /* Entry 1: age 10s */
  IP4_ADDR(&ipaddr1, 192, 168, 0, 20);
  etharp_test_init_entry(0, ETHARP_STATE_STABLE, &ipaddr1, &ethaddr, &test_netif, 10);
  
  /* Entry 2: age 50s */
  IP4_ADDR(&ipaddr2, 192, 168, 0, 21);
  etharp_test_init_entry(1, ETHARP_STATE_STABLE, &ipaddr2, &ethaddr, &test_netif, 50);
  
  /* Entry 3: age 200s */
  IP4_ADDR(&ipaddr3, 192, 168, 0, 22);
  etharp_test_init_entry(2, ETHARP_STATE_STABLE, &ipaddr3, &ethaddr, &test_netif, 200);
  
  /* Setup ECO mode */
  etharp_test_set_timer_started(1);
  etharp_test_set_eco_mode(1);
  etharp_test_set_last_timer_time(0);
  lwip_sys_now = 0;
  
  /* Call etharp_tmr - ages by 1: 10->11, 50->51, 200->201 */
  etharp_tmr();
  
  /* Minimum timeout should be for entry 3: 270-201 = 69s */
  fail_unless(has_etharp_timeout_scheduled(&when));
  fail_unless(when == 69000);
}
END_TEST

/* Test 19: Age capping at ARP_MAXAGE */
START_TEST(test_etharp_age_capping)
{
  ip4_addr_t ipaddr;
  struct eth_addr ethaddr = {{0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff}};
  struct etharp_entry *entry;
  LWIP_UNUSED_ARG(_i);

  /* Add entry at age 280 */
  IP4_ADDR(&ipaddr, 192, 168, 0, 20);
  etharp_test_init_entry(0, ETHARP_STATE_STABLE, &ipaddr, &ethaddr, &test_netif, 280);
  
  /* Setup ECO mode with initial non-zero time */
  lwip_sys_now = 1000;
  etharp_test_set_timer_started(1);
  etharp_test_set_eco_mode(1);
  etharp_test_set_last_timer_time(lwip_sys_now);
  
  /* Advance time by 100 seconds (280+100=380, should cap at 300 and expire) */
  lwip_sys_now += 100000;
  
  /* Call etharp_tmr */
  etharp_tmr();
  
  /* Verify entry was capped at 300 and then expired */
  entry = etharp_test_get_table_entry(0);
  fail_unless(entry != NULL);
  fail_unless(entry->state == ETHARP_STATE_EMPTY);
}
END_TEST

/* Test 20: Last timer time updated correctly */
START_TEST(test_etharp_last_timer_time_update)
{
  ip4_addr_t ipaddr;
  struct eth_addr ethaddr = {{0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff}};
  u32_t time_before, time_after;
  LWIP_UNUSED_ARG(_i);

  /* Add STABLE entry */
  IP4_ADDR(&ipaddr, 192, 168, 0, 20);
  etharp_test_init_entry(0, ETHARP_STATE_STABLE, &ipaddr, &ethaddr, &test_netif, 0);
  
  /* Setup ECO mode */
  etharp_test_set_timer_started(1);
  etharp_test_set_eco_mode(1);
  etharp_test_set_last_timer_time(0);
  lwip_sys_now = 5000;
  
  time_before = etharp_test_get_last_timer_time();
  
  /* Call etharp_tmr */
  etharp_tmr();
  
  time_after = etharp_test_get_last_timer_time();
  
  /* Verify last_timer_time updated */
  fail_unless(time_after == lwip_sys_now);
  fail_unless(time_after > time_before);
}
END_TEST

/* Test 21: Multiple entries all at same age */
START_TEST(test_etharp_multiple_same_age)
{
  ip4_addr_t ipaddr1, ipaddr2, ipaddr3;
  struct eth_addr ethaddr = {{0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff}};
  u32_t when;
  LWIP_UNUSED_ARG(_i);

  IP4_ADDR(&ipaddr1, 192, 168, 0, 20);
  etharp_test_init_entry(0, ETHARP_STATE_STABLE, &ipaddr1, &ethaddr, &test_netif, 100);
  
  IP4_ADDR(&ipaddr2, 192, 168, 0, 21);
  etharp_test_init_entry(1, ETHARP_STATE_STABLE, &ipaddr2, &ethaddr, &test_netif, 100);
  
  IP4_ADDR(&ipaddr3, 192, 168, 0, 22);
  etharp_test_init_entry(2, ETHARP_STATE_STABLE, &ipaddr3, &ethaddr, &test_netif, 100);
  
  etharp_test_set_timer_started(1);
  etharp_test_set_eco_mode(1);
  etharp_test_set_last_timer_time(0);
  lwip_sys_now = 0;
  
  etharp_tmr();
  
  fail_unless(has_etharp_timeout_scheduled(&when));
  fail_unless(when == 169000);
}
END_TEST

/* Test 22: Rapid timer calls in ACTIVE mode */
START_TEST(test_etharp_rapid_active_calls)
{
  ip4_addr_t ipaddr;
  struct eth_addr ethaddr = {{0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff}};
  struct etharp_entry *entry;
  int i;
  LWIP_UNUSED_ARG(_i);

  IP4_ADDR(&ipaddr, 192, 168, 0, 20);
  etharp_test_init_entry(0, ETHARP_STATE_STABLE, &ipaddr, &ethaddr, &test_netif, 0);
  
  etharp_test_set_timer_started(1);
  etharp_test_set_eco_mode(0);
  lwip_sys_now = 0;
  
  for (i = 0; i < 10; i++) {
    lwip_sys_now += ARP_TMR_INTERVAL;
    etharp_tmr();
  }
  
  entry = etharp_test_get_table_entry(0);
  fail_unless(entry != NULL);
  fail_unless(entry->ctime == 10);
}
END_TEST

/* Test 23: Expiring multiple entries */
START_TEST(test_etharp_expire_multiple_entries)
{
  ip4_addr_t ipaddr1, ipaddr2, ipaddr3;
  struct eth_addr ethaddr = {{0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff}};
  struct etharp_entry *entry1, *entry2, *entry3;
  LWIP_UNUSED_ARG(_i);

  IP4_ADDR(&ipaddr1, 192, 168, 0, 20);
  etharp_test_init_entry(0, ETHARP_STATE_STABLE, &ipaddr1, &ethaddr, &test_netif, 299);
  
  IP4_ADDR(&ipaddr2, 192, 168, 0, 21);
  etharp_test_init_entry(1, ETHARP_STATE_STABLE, &ipaddr2, &ethaddr, &test_netif, 299);
  
  IP4_ADDR(&ipaddr3, 192, 168, 0, 22);
  etharp_test_init_entry(2, ETHARP_STATE_STABLE, &ipaddr3, &ethaddr, &test_netif, 299);
  
  etharp_test_set_timer_started(1);
  etharp_test_set_eco_mode(0);
  lwip_sys_now = 0;
  
  etharp_tmr();
  
  entry1 = etharp_test_get_table_entry(0);
  entry2 = etharp_test_get_table_entry(1);
  entry3 = etharp_test_get_table_entry(2);
  
  fail_unless(entry1->state == ETHARP_STATE_EMPTY);
  fail_unless(entry2->state == ETHARP_STATE_EMPTY);
  fail_unless(entry3->state == ETHARP_STATE_EMPTY);
}
END_TEST

/* Test 24: ECO mode with single entry aging to expiry */
START_TEST(test_etharp_eco_single_entry_to_expiry)
{
  ip4_addr_t ipaddr;
  struct eth_addr ethaddr = {{0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff}};
  struct etharp_entry *entry;
  u16_t initial_age, age_after_step1;
  LWIP_UNUSED_ARG(_i);

  IP4_ADDR(&ipaddr, 192, 168, 0, 20);
  etharp_test_init_entry(0, ETHARP_STATE_STABLE, &ipaddr, &ethaddr, &test_netif, 200);
  initial_age = 200;
  
  lwip_sys_now = 1000;
  etharp_test_set_timer_started(1);
  etharp_test_set_eco_mode(0);
  
  etharp_tmr();
  entry = etharp_test_get_table_entry(0);
  fail_unless(entry->ctime > initial_age);
  fail_unless(etharp_test_get_eco_mode() == 1);
  fail_unless(has_etharp_timeout_scheduled(NULL));
  age_after_step1 = entry->ctime;
  
  lwip_sys_now = 95000;
  etharp_tmr();
  entry = etharp_test_get_table_entry(0);
  fail_unless(entry->ctime > age_after_step1 + 80);
  fail_unless(entry->state == ETHARP_STATE_STABLE);
  
  lwip_sys_now = 105000;
  etharp_tmr();
  entry = etharp_test_get_table_entry(0);
  fail_unless(entry->state == ETHARP_STATE_EMPTY);
  fail_unless(etharp_test_get_timer_started() == 0);
}
END_TEST

/* Test 25: PENDING entry retransmission */
START_TEST(test_etharp_pending_retransmission)
{
  ip4_addr_t ipaddr;
  struct eth_addr ethaddr = {{0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff}};
  struct etharp_entry *entry;
  int initial_count;
  LWIP_UNUSED_ARG(_i);

  IP4_ADDR(&ipaddr, 192, 168, 0, 20);
  etharp_test_init_entry(0, ETHARP_STATE_PENDING, &ipaddr, &ethaddr, &test_netif, 1);
  
  etharp_test_set_timer_started(1);
  etharp_test_set_eco_mode(0);
  lwip_sys_now = 0;
  
  initial_count = linkoutput_ctr;
  etharp_tmr();
  
  fail_unless(linkoutput_ctr > initial_count);
  
  entry = etharp_test_get_table_entry(0);
  fail_unless(entry->state == ETHARP_STATE_PENDING);
  fail_unless(entry->ctime == 2);
}
END_TEST

/* Test 26: End-to-end test 3.2 - Unicast probe at 270s */
START_TEST(test_etharp_unicast_probe_at_270s)
{
  ip4_addr_t ipaddr;
  struct eth_addr ethaddr = {{0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff}};
  struct etharp_entry *entry;
  u32_t when;
  LWIP_UNUSED_ARG(_i);

  IP4_ADDR(&ipaddr, 192, 168, 0, 20);
  etharp_test_init_entry(0, ETHARP_STATE_STABLE, &ipaddr, &ethaddr, &test_netif, 270);
  
  etharp_test_set_timer_started(1);
  etharp_test_set_eco_mode(1);
  etharp_test_set_last_timer_time(0);
  lwip_sys_now = 0;
  
  etharp_tmr();
  
  fail_unless(has_etharp_timeout_scheduled(&when));
  fail_unless(when <= 15000);
  
  entry = etharp_test_get_table_entry(0);
  fail_unless(entry != NULL);
  fail_unless(entry->state == ETHARP_STATE_STABLE);
  fail_unless(entry->ctime == 271);
  fail_unless(entry->ctime >= ARP_AGE_REREQUEST_USED_UNICAST);
  fail_unless(entry->ctime < ARP_AGE_REREQUEST_USED_BROADCAST);
  
  entry->state = ETHARP_STATE_STABLE_REREQUESTING_1;
  etharp_tmr();
  
  entry = etharp_test_get_table_entry(0);
  fail_unless(entry->state == ETHARP_STATE_STABLE_REREQUESTING_2);
  
  etharp_tmr();
  
  entry = etharp_test_get_table_entry(0);
  fail_unless(entry->state == ETHARP_STATE_STABLE);
  fail_unless(entry->ctime >= 271);
}
END_TEST

/** Create the suite including all tests for this module */
Suite* etharp_ondemand_suite(void)
{
  testfunc tests[] = {
    TESTFUNC(test_etharp_timer_starts_on_first_entry),
    TESTFUNC(test_etharp_eco_mode_activation),
    TESTFUNC(test_etharp_active_mode_with_pending),
    TESTFUNC(test_etharp_timer_stops_when_empty),
    TESTFUNC(test_etharp_eco_to_active_transition),
    TESTFUNC(test_etharp_entry_aging_active_mode),
    TESTFUNC(test_etharp_entry_aging_eco_mode),
    TESTFUNC(test_etharp_entry_expiration),
    TESTFUNC(test_etharp_pending_expiration),
    TESTFUNC(test_etharp_min_timeout_selection),
    TESTFUNC(test_etharp_wraparound_handling),
    TESTFUNC(test_etharp_static_entry_not_aged),
    TESTFUNC(test_etharp_timer_reschedule_on_removal),
    TESTFUNC(test_etharp_mixed_entries_active_mode),
    TESTFUNC(test_etharp_rerequesting_state_transition),
    TESTFUNC(test_etharp_rerequesting_2_state_transition),
    TESTFUNC(test_etharp_rerequesting_forces_active_mode),
    TESTFUNC(test_etharp_eco_mode_different_ages),
    TESTFUNC(test_etharp_age_capping),
    TESTFUNC(test_etharp_last_timer_time_update),
    TESTFUNC(test_etharp_multiple_same_age),
    TESTFUNC(test_etharp_rapid_active_calls),
    TESTFUNC(test_etharp_expire_multiple_entries),
    TESTFUNC(test_etharp_eco_single_entry_to_expiry),
    TESTFUNC(test_etharp_pending_retransmission),
    TESTFUNC(test_etharp_unicast_probe_at_270s),
  };
  return create_suite("ETHARP_ONDEMAND", tests, sizeof(tests)/sizeof(testfunc), 
                      etharp_ondemand_setup, etharp_ondemand_teardown);
}

#else /* SL_LWIP_ETHARP_ONDEMAND_TIMER not enabled */

/* Stub suite function when on-demand timer feature is not enabled */
Suite *
etharp_ondemand_suite(void)
{
  /* Return NULL suite when feature is disabled - tests won't run */
  return NULL;
}

#endif /* SL_LWIP_ETHARP_ONDEMAND_TIMER && LWIP_TESTMODE */
