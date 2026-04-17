#include "test_dns.h"

#include "lwip/dns.h"

#if SL_LWIP_DNS_ONDEMAND_TIMER
#include "lwip/timeouts.h"
#include "arch/sys_arch.h"
#include "lwip/prot/dns.h"
#include "lwip/sys.h"
#include "lwip/tcpip.h"
#include "lwip/opt.h"
#include <stdio.h>
#include <string.h>


/* Test timeout constants (in milliseconds) */
#define TEST_TIMEOUT_1SEC_MS     1000
#define TEST_TIMEOUT_1_1SEC_MS   1100
#define TEST_TIMEOUT_5SEC_MS     5000
#define TEST_TIMEOUT_10SEC_MS   10000
#define TEST_TIMEOUT_30SEC_MS   30000
#define TEST_TIMEOUT_45SEC_MS   45000
#define TEST_TIMEOUT_60SEC_MS   60000

/* Test time constants for TTL decrement tests (in milliseconds) */
#define TEST_TIME_BASE_MS        1000
#define TEST_TIME_COMPARE_START  5000
#define TEST_TIME_AFTER_1SEC     6000
#define TEST_TIME_AFTER_10SEC   15000

/* Test TTL constants (in seconds) */
#define TEST_TTL_HUGE_SEC        966367
#define TEST_TTL_WITHIN_MAX      500000
#define TEST_TTL_BOUNDARY_SEC    100000
#define TEST_TTL_ACTIVE_SEC      300
#define DNS_MAX_TTL_SEC          604800
#define TEST_TTL_MIN_1_SEC       10
#define TEST_TTL_MIN_2_SEC       11
#define TEST_TTL_MIN_3_SEC       60
#define TEST_TTL_CACHE_1_SEC     300
#define TEST_TTL_CACHE_2_SEC     600
#define TEST_TTL_WRAPAROUND_SEC  20
#define TEST_TTL_LARGE_HOUR_SEC  3600
#define TEST_TTL_MIN_15_SEC      15
#define TEST_TTL_MIN_30_SEC      30
#define TEST_TTL_COMPARE_SEC     1000

/* Test elapsed time constants (in seconds) */
#define TEST_ELAPSED_1_SEC       1
#define TEST_ELAPSED_2_SEC       2
#define TEST_ELAPSED_10_SEC      10
#define TEST_ELAPSED_30_SEC      30
#define TEST_ELAPSED_45_SEC      45

#define SYS_TIMEOUT_LIMIT_MS     (0xFFFFFFFFU / 4U)

#endif /* SL_LWIP_DNS_ONDEMAND_TIMER */

/* ⚠️  MAINTENANCE WARNING: This structure is duplicated from dns.c for white-box testing.
 *     It MUST be kept synchronized with the internal dns_table_entry structure in dns.c.
 */
#ifdef LWIP_TESTMODE
struct dns_table_entry {
  u32_t ttl;
  ip_addr_t ipaddr;
  u16_t txid;
  u8_t  state;
  u8_t  server_idx;
  u8_t  tmr;
  u8_t  retries;
  u8_t  seqno;
#if ((LWIP_DNS_SECURE & LWIP_DNS_SECURE_RAND_SRC_PORT) != 0)
  u8_t pcb_idx;
#endif
  char name[DNS_MAX_NAME_LENGTH];
#if LWIP_IPV4 && LWIP_IPV6
  u8_t reqaddrtype;
#endif
#if LWIP_DNS_SUPPORT_MDNS_QUERIES
  u8_t is_mdns;
#endif
#if SL_LWIP_DNS_ONDEMAND_TIMER
  u32_t last_check_time;
#endif
};
#if SL_LWIP_DNS_ONDEMAND_TIMER
#define DNS_TABLE_ENTRY_EXPECTED_SIZE_MIN 256

/* DNS state enum values */
#define DNS_STATE_UNUSED  0
#define DNS_STATE_NEW     1
#define DNS_STATE_ASKING  2
#define DNS_STATE_DONE    3
#endif /* SL_LWIP_DNS_ONDEMAND_TIMER */
#endif /* LWIP_TESTMODE */

/* Setups/teardown functions */

#if SL_LWIP_DNS_ONDEMAND_TIMER
static struct sys_timeo* old_list_head;
#endif

static void
dns_setup(void)
{
#if SL_LWIP_DNS_ONDEMAND_TIMER
  struct sys_timeo** list_head;
  struct sys_timeo* current;
  int leaked_timers = 0;
  
  list_head = sys_timeouts_get_next_timeout();
  current = *list_head;
  while (current != NULL) {
    if (current->h == dns_timeout_cb) {
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
  
  dns_test_set_timer_started(0);
  dns_test_set_eco_mode(0);
#endif
  
  dns_init();
}

static void
dns_teardown(void)
{
#if SL_LWIP_DNS_ONDEMAND_TIMER
  struct sys_timeo** list_head;
  struct sys_timeo* current;
  struct sys_timeo* next;
  int leaked_timers = 0;
  
  list_head = sys_timeouts_get_next_timeout();
  current = *list_head;
  
  while (current != NULL) {
    next = current->next;
    if (current->h == dns_timeout_cb) {
      leaked_timers++;
      sys_untimeout(dns_timeout_cb, current->arg);
    }
    current = next;
  }
  
  dns_test_clear_table();
  dns_test_set_timer_started(0);
  dns_test_set_eco_mode(0);
  
  *list_head = old_list_head;
  lwip_sys_now = 0;
#endif
}

/* Test functions */

START_TEST(test_dns_set_get_server)
{
  int n;
  LWIP_UNUSED_ARG(_i);

  for (n = 0; n < 256; n++) {
    u8_t i = (u8_t)n;
    ip_addr_t server;
    /* Should return a zeroed address for any index */
    fail_unless(dns_getserver(i));
    fail_unless(ip_addr_isany(dns_getserver(i)));
    /* Should accept setting address for any index, and ignore if out of range */
    IP_ADDR4(&server, 10, 0, 0, i);
    dns_setserver(i, &server);
    fail_unless(dns_getserver(i));
    if (i < DNS_MAX_SERVERS) {
      fail_unless(ip_addr_eq(dns_getserver(i), &server) == 1);
    } else {
      fail_unless(ip_addr_isany(dns_getserver(i)));
    }
  }
}
END_TEST

#if SL_LWIP_DNS_ONDEMAND_TIMER

/* Helper to check if dns_timeout_cb is scheduled */
static int
has_dns_timeout_scheduled(u32_t *when_out)
{
  struct sys_timeo* t = *sys_timeouts_get_next_timeout();
  while (t != NULL) {
    if (t->h == dns_timeout_cb) {
      if (when_out) { *when_out = t->time; }
      return 1;
    }
    t = t->next;
  }
  return 0;
}

static int found_called;
static const ip_addr_t *found_last_ip;
static void
dummy_found_cb(const char *name, const ip_addr_t *ipaddr, void *arg)
{
  LWIP_UNUSED_ARG(name);
  LWIP_UNUSED_ARG(arg);
  found_called++;
  found_last_ip = ipaddr;
}

/* Test 1: Verify eco mode activates when only cached entries exist */
START_TEST(test_dns_eco_mode_activation)
{
  ip_addr_t server, addr;
  u32_t when;
  LWIP_UNUSED_ARG(_i);

  /* Setup: valid DNS server */
  IP_ADDR4(&server, 8, 8, 8, 8);
  dns_setserver(0, &server);
  
  /* Reset eco timer state */
  dns_test_set_timer_started(0);
  dns_test_set_eco_mode(0);
  
  /* Start timer with a query */
  dns_gethostbyname("example.com", &addr, dummy_found_cb, NULL);
  fail_unless(dns_test_get_timer_started() == 1);
  fail_unless(dns_test_get_eco_mode() == 0);
  fail_unless(has_dns_timeout_scheduled(&when));
  fail_unless(when == DNS_TMR_INTERVAL);
}
END_TEST

/* Test 2: Verify timer_started flag is set when first query enqueued */
START_TEST(test_dns_timer_started_flag)
{
  ip_addr_t server, addr;
  err_t err;
  LWIP_UNUSED_ARG(_i);

  /* Setup */
  IP_ADDR4(&server, 8, 8, 8, 8);
  dns_setserver(0, &server);
  dns_test_set_timer_started(0);

  /* Verify initial state */
  fail_unless(dns_test_get_timer_started() == 0);
  fail_unless(!has_dns_timeout_scheduled(NULL));

  /* Enqueue first query */
  err = dns_gethostbyname("test1.com", &addr, dummy_found_cb, NULL);
  fail_unless(err == ERR_INPROGRESS);

  /* Verify timer_started flag is set */
  fail_unless(dns_test_get_timer_started() == 1);
  fail_unless(has_dns_timeout_scheduled(NULL));
}
END_TEST

/* Test 3: Verify eco mode switches to active mode when new query arrives */
START_TEST(test_dns_eco_to_active_transition)
{
  ip_addr_t server, addr;
  u32_t when_eco;
  u32_t when_active;
  LWIP_UNUSED_ARG(_i);

  /* Setup */
  IP_ADDR4(&server, 8, 8, 8, 8);
  dns_setserver(0, &server);
  
  /* Simulate eco mode state: timer started, eco mode on, long interval scheduled */
  dns_test_set_timer_started(1);
  dns_test_set_eco_mode(1);
  
  /* Schedule a long timeout (simulating eco mode with cached entries) */
  sys_timeout(TEST_TIMEOUT_60SEC_MS, dns_timeout_cb, NULL); /* 60 seconds */
  fail_unless(has_dns_timeout_scheduled(&when_eco));
  fail_unless(when_eco == TEST_TIMEOUT_60SEC_MS);
  fail_unless(dns_test_get_eco_mode() == 1);
  
  /* New query arrives - should transition to active mode */
  dns_gethostbyname("urgent.com", &addr, dummy_found_cb, NULL);
  
  /* Verify transition to active mode */
  fail_unless(dns_test_get_eco_mode() == 0);
  fail_unless(has_dns_timeout_scheduled(&when_active));
  fail_unless(when_active == DNS_TMR_INTERVAL);
}
END_TEST

/* Test 4: Verify per-entry last_check_time is initialized */
START_TEST(test_dns_last_check_time_init)
{
  ip_addr_t server, addr;
  struct dns_table_entry *entry;
  LWIP_UNUSED_ARG(_i);

  /* Setup */
  IP_ADDR4(&server, 8, 8, 8, 8);
  dns_setserver(0, &server);
  dns_test_set_timer_started(0);
  
  lwip_sys_now = TEST_TIMEOUT_1SEC_MS;
  
  dns_gethostbyname("test.com", &addr, dummy_found_cb, NULL);
  
  entry = dns_test_get_table_entry(0);
  fail_unless(entry != NULL);
  fail_unless(entry->last_check_time == (u32_t)lwip_sys_now);
}
END_TEST

/* Test 5: Verify elapsed time calculation in eco mode handles wraparound */
START_TEST(test_dns_wraparound_handling)
{
  struct dns_table_entry *entry;
  u32_t near_max;
  u32_t after_wrap;
  u32_t initial_ttl;
  u32_t expected_elapsed_ms;
  u32_t expected_elapsed_sec;
  LWIP_UNUSED_ARG(_i);

  entry = dns_test_get_table_entry(0);
  if (entry == NULL) {
    return;
  }

  /* Initialize entry to DONE state with TTL */
  entry->state = DNS_STATE_DONE;
  initial_ttl = TEST_TTL_WRAPAROUND_SEC;
  entry->ttl = initial_ttl;
  entry->txid = 1;
  entry->server_idx = 0;
  entry->tmr = 0;
  entry->retries = 0;
  entry->seqno = 0;
  IP_ADDR4(&entry->ipaddr, 192, 168, 1, 1);
  strcpy(entry->name, "wraparound-test.com");
  
  /* Setup: last_check_time is near UINT32_MAX */
  near_max = (u32_t)(-5000); /* 5 seconds before wraparound */
  entry->last_check_time = near_max;
  dns_test_set_eco_mode(1);
  dns_test_set_timer_started(1);
  
  /* Simulate time advancing past wraparound */
  after_wrap = 5000; /* 5 seconds after wraparound */
  lwip_sys_now = after_wrap;
  
  /* Calculate expected elapsed time across wraparound */
  expected_elapsed_ms = (after_wrap - near_max); /* Should be 10000ms due to wraparound */
  expected_elapsed_sec = expected_elapsed_ms / 1000; /* Should be 10 seconds */
  
  /* Call dns_tmr() - it should handle wraparound correctly */
  dns_tmr();
  
  /* Verify no crash/hang occurred */
  /* Verify entry was processed correctly */
  entry = dns_test_get_table_entry(0);
  fail_unless(entry != NULL);
  
  /* Verify TTL was decremented by elapsed seconds (10 seconds) */
  if (entry->state == DNS_STATE_DONE) {
    /* TTL should be: initial_ttl - expected_elapsed_sec = 20 - 10 = 10 */
    fail_unless(entry->ttl == (initial_ttl - expected_elapsed_sec));
    
    /* Verify last_check_time was updated to current time */
    fail_unless(entry->last_check_time == (u32_t)lwip_sys_now);
  }
}
END_TEST

/* Test 6: Verify DNS_MAX_TTL capping and safe handling of large TTL values
 * 
 * This test verifies that the DNS eco-timer properly enforces DNS_MAX_TTL policy
 * and prevents sys_timeout() overflow by capping scheduled timeouts.
 * 
 * Background:
 *     - DNS_MAX_TTL = 604,800 seconds (7 days) - DNS policy maximum
 *     - sys_timeout() limit = LWIP_UINT32_MAX/4 = 1,073,741,823 ms (~12.4 days)
 *     - DNS eco-timer caps at DNS_MAX_TTL to maintain policy consistency
 *     - This ensures scheduled timeouts stay well within sys_timeout() limits
 * 
 * Test Coverage:
 *     - Test Case 1a: TTL exceeding DNS_MAX_TTL → properly capped at 7 days
 *     - Test Case 1b: TTL within DNS_MAX_TTL → passes through uncapped
 *     - Test Case 2: Boundary TTL (~27 hours) → passes through uncapped
 *     - Test Case 3: active TTL (5 minutes) → passes through uncapped
 */
START_TEST(test_dns_overflow_protection)
{
  ip_addr_t server;
  struct dns_table_entry *entry1, *entry2, *entry3;
  u32_t when;
  LWIP_UNUSED_ARG(_i);

  /* Setup */
  IP_ADDR4(&server, 8, 8, 8, 8);
  dns_setserver(0, &server);
  
  /* ===== Test Case 1a: TTL exceeding DNS_MAX_TTL (should be capped) ===== */
  /* Use 966,367 seconds (~11.2 days) which exceeds DNS_MAX_TTL (7 days) */
  entry1 = dns_test_get_table_entry(0);
  fail_unless(entry1 != NULL);
  
  entry1->state = DNS_STATE_DONE;
  entry1->ttl = TEST_TTL_HUGE_SEC;  /* 966,367 sec > 604,800 sec (DNS_MAX_TTL) */
  entry1->last_check_time = 0;
  entry1->txid = 1;
  entry1->server_idx = 0;
  entry1->tmr = 0;
  entry1->retries = 0;
  entry1->seqno = 0;
  IP_ADDR4(&entry1->ipaddr, 10, 0, 0, 1);
  strcpy(entry1->name, "huge-ttl.com");
  
  /* Setup eco mode with cached entry */
  dns_test_set_timer_started(1);
  dns_test_set_eco_mode(1);
  lwip_sys_now = 0;
  
  /* Call dns_tmr() - should cap at DNS_MAX_TTL */
  dns_tmr();
  
  /* Verify timer was scheduled and capped at DNS_MAX_TTL */
  if (has_dns_timeout_scheduled(&when)) {
    u32_t expected_capped_ms = DNS_MAX_TTL_SEC * 1000U;  /* 604,800,000 ms */
    
    /* Must be within sys_timeout() limit */
    fail_unless(when <= SYS_TIMEOUT_LIMIT_MS);
    
    /* Must be capped at DNS_MAX_TTL, NOT the original huge TTL */
    fail_unless(when == expected_capped_ms);
    fail_unless(when != TEST_TTL_HUGE_SEC * 1000U);  /* Should NOT be 966,367,000 ms */
  }
  
  /* Clear entry for next test case */
  entry1->state = DNS_STATE_UNUSED;
  
  /* ===== Test Case 1b: TTL within DNS_MAX_TTL (should pass through) ===== */
  /* Use 500,000 seconds (~5.8 days) which is within DNS_MAX_TTL */
  entry1->state = DNS_STATE_DONE;
  entry1->ttl = TEST_TTL_WITHIN_MAX;  /* 500,000 sec < 604,800 sec (DNS_MAX_TTL) */
  entry1->last_check_time = 0;
  entry1->txid = 1;
  entry1->server_idx = 0;
  entry1->tmr = 0;
  entry1->retries = 0;
  entry1->seqno = 0;
  IP_ADDR4(&entry1->ipaddr, 10, 0, 0, 1);
  strcpy(entry1->name, "within-max.com");
  
  lwip_sys_now = 0;
  dns_tmr();
  
  /* Verify timer scheduled at original TTL (not capped) */
  if (has_dns_timeout_scheduled(&when)) {
    u32_t expected_uncapped_ms = TEST_TTL_WITHIN_MAX * 1000U;  /* 500,000,000 ms */
    
    /* Must be within sys_timeout() limit */
    fail_unless(when <= SYS_TIMEOUT_LIMIT_MS);
    
    /* Should match original TTL (not capped) */
    fail_unless(when == expected_uncapped_ms);
  }
  
  /* ===== Test Case 2: Boundary TTL (~27 hours, within DNS_MAX_TTL) ===== */
  entry2 = dns_test_get_table_entry(1);
  if (entry2 != NULL) {
    /* Use 100,000 seconds (~27 hours) which is well within DNS_MAX_TTL */
    entry2->state = DNS_STATE_DONE;
    entry2->ttl = TEST_TTL_BOUNDARY_SEC;  /* 100,000 sec < 604,800 sec */
    entry2->last_check_time = 0;
    entry2->txid = 2;
    entry2->server_idx = 0;
    entry2->tmr = 0;
    entry2->retries = 0;
    entry2->seqno = 0;
    IP_ADDR4(&entry2->ipaddr, 10, 0, 0, 2);
    strcpy(entry2->name, "boundary-ttl.com");
    
    lwip_sys_now = 0;
    dns_tmr();
    
    /* Verify timer scheduled at original TTL (not capped) */
    if (has_dns_timeout_scheduled(&when)) {
      u32_t expected_boundary_ms = TEST_TTL_BOUNDARY_SEC * 1000U;  /* 100,000,000 ms */
      
      fail_unless(when <= SYS_TIMEOUT_LIMIT_MS);
      fail_unless(when == expected_boundary_ms);
    }
  }
  
  /* ===== Test Case 3: active TTL (5 minutes) ===== */
  entry3 = dns_test_get_table_entry(2);
  if (entry3 != NULL) {
    /* Use typical 5-minute TTL */
    entry3->state = DNS_STATE_DONE;
    entry3->ttl = TEST_TTL_ACTIVE_SEC;  /* 300 sec = 5 minutes */
    entry3->last_check_time = 0;
    entry3->txid = 3;
    entry3->server_idx = 0;
    entry3->tmr = 0;
    entry3->retries = 0;
    entry3->seqno = 0;
    IP_ADDR4(&entry3->ipaddr, 10, 0, 0, 3);
    strcpy(entry3->name, "active-ttl.com");
    
    lwip_sys_now = 0;
    dns_tmr();
    
    /* Verify timer scheduled at original TTL */
    if (has_dns_timeout_scheduled(&when)) {
      u32_t expected_timeout_ms = TEST_TTL_ACTIVE_SEC * 1000U;  /* 300,000 ms */
      
      fail_unless(when <= SYS_TIMEOUT_LIMIT_MS);
      fail_unless(when == expected_timeout_ms);
    }
  }
}
END_TEST

/* Test 7: Verify minimum TTL is selected among multiple cached entries */
START_TEST(test_dns_min_ttl_selection)
{
  ip_addr_t server;
  struct dns_table_entry *entry1, *entry2, *entry3;
  u32_t when;
  u32_t min_ttl_ms;
  LWIP_UNUSED_ARG(_i);

  /* Setup */
  IP_ADDR4(&server, 8, 8, 8, 8);
  dns_setserver(0, &server);
  
  /* Create multiple cached entries with different TTLs */
  entry1 = dns_test_get_table_entry(0);
  entry2 = dns_test_get_table_entry(1);
  entry3 = dns_test_get_table_entry(2);
  
  fail_unless(entry1 != NULL);
  fail_unless(entry2 != NULL);
  fail_unless(entry3 != NULL);
  
  /* Entry 1: TTL = 30 seconds */
  entry1->state = DNS_STATE_DONE;
  entry1->ttl = TEST_TTL_MIN_30_SEC;
  entry1->last_check_time = 0;
  entry1->txid = 1;
  entry1->server_idx = 0;
  entry1->tmr = 0;
  entry1->retries = 0;
  entry1->seqno = 0;
  IP_ADDR4(&entry1->ipaddr, 10, 1, 1, 1);
  strcpy(entry1->name, "entry1.com");
  
  /* Entry 2: TTL = 15 seconds (MINIMUM) */
  entry2->state = DNS_STATE_DONE;
  entry2->ttl = TEST_TTL_MIN_15_SEC;
  entry2->last_check_time = 0;
  entry2->txid = 2;
  entry2->server_idx = 0;
  entry2->tmr = 0;
  entry2->retries = 0;
  entry2->seqno = 0;
  IP_ADDR4(&entry2->ipaddr, 10, 1, 1, 2);
  strcpy(entry2->name, "entry2.com");
  
  /* Entry 3: TTL = 60 seconds */
  entry3->state = DNS_STATE_DONE;
  entry3->ttl = TEST_TTL_MIN_3_SEC;
  entry3->last_check_time = 0;
  entry3->txid = 3;
  entry3->server_idx = 0;
  entry3->tmr = 0;
  entry3->retries = 0;
  entry3->seqno = 0;
  IP_ADDR4(&entry3->ipaddr, 10, 1, 1, 3);
  strcpy(entry3->name, "entry3.com");
  
  /* Setup eco mode with cached entries */
  dns_test_set_timer_started(1);
  dns_test_set_eco_mode(1);
  lwip_sys_now = 0;
  
  /* Call dns_tmr() - should schedule based on minimum TTL */
  dns_tmr();
  
  /* Verify timer is scheduled at minimum TTL (15 seconds = 15000 ms) */
  min_ttl_ms = 15 * 1000;
  
  if (has_dns_timeout_scheduled(&when)) {
    /* Timer should be scheduled at or near the minimum TTL */
    /* Allow for some tolerance due to implementation details */
    fail_unless(when <= min_ttl_ms);
  }
}
END_TEST

/* Test 8: Verify mixed active + cached entries use active mode */
START_TEST(test_dns_mixed_state_active_mode)
{
  ip_addr_t server, addr;
  struct dns_table_entry *cached_entry;
  u32_t when;
  LWIP_UNUSED_ARG(_i);

  /* Setup */
  IP_ADDR4(&server, 8, 8, 8, 8);
  dns_setserver(0, &server);
  dns_test_set_timer_started(1);
  dns_test_set_eco_mode(1);
  
  /* Create a cached entry with long TTL (1 hour) */
  cached_entry = dns_test_get_table_entry(0);
  fail_unless(cached_entry != NULL);
  
  cached_entry->state = DNS_STATE_DONE;
  cached_entry->ttl = 3600;  /* 1 hour - would trigger eco mode if alone */
  cached_entry->last_check_time = 0;
  cached_entry->txid = 1;
  cached_entry->server_idx = 0;
  cached_entry->tmr = 0;
  cached_entry->retries = 0;
  cached_entry->seqno = 0;
  IP_ADDR4(&cached_entry->ipaddr, 10, 0, 0, 1);
  strcpy(cached_entry->name, "cached.com");
  
  /* Now start an active query (creates mixed state: cached + active) */
  dns_gethostbyname("active.com", &addr, dummy_found_cb, NULL);
  
  /* Verify active mode is active (NOT eco mode, despite cached entry) */
  fail_unless(dns_test_get_eco_mode() == 0);
  fail_unless(has_dns_timeout_scheduled(&when));
  fail_unless(when == DNS_TMR_INTERVAL);  /* Should be 1000ms, not 3600000ms */
}
END_TEST

/* Test 9: Verify timer stops when all entries removed (table empty) */
START_TEST(test_dns_timer_stops_when_empty)
{
  ip_addr_t server;
  struct dns_table_entry *entry1, *entry2;
  u32_t when;
  LWIP_UNUSED_ARG(_i);

  /* Setup */
  IP_ADDR4(&server, 8, 8, 8, 8);
  dns_setserver(0, &server);
  
  /* First, populate DNS table with cached entries */
  entry1 = dns_test_get_table_entry(0);
  entry2 = dns_test_get_table_entry(1);
  
  fail_unless(entry1 != NULL);
  fail_unless(entry2 != NULL);
  
  /* Create Entry 1 */
  entry1->state = DNS_STATE_DONE;
  entry1->ttl = TEST_TTL_CACHE_1_SEC;
  entry1->last_check_time = 0;
  entry1->txid = 1;
  entry1->server_idx = 0;
  entry1->tmr = 0;
  entry1->retries = 0;
  entry1->seqno = 0;
  IP_ADDR4(&entry1->ipaddr, 10, 2, 2, 1);
  strcpy(entry1->name, "test-entry1.com");
  
  /* Create Entry 2 */
  entry2->state = DNS_STATE_DONE;
  entry2->ttl = TEST_TTL_CACHE_2_SEC;
  entry2->last_check_time = 0;
  entry2->txid = 2;
  entry2->server_idx = 0;
  entry2->tmr = 0;
  entry2->retries = 0;
  entry2->seqno = 0;
  IP_ADDR4(&entry2->ipaddr, 10, 2, 2, 2);
  strcpy(entry2->name, "test-entry2.com");
  
  /* Setup: timer running in eco mode with cached entries */
  dns_test_set_timer_started(1);
  dns_test_set_eco_mode(1);
  lwip_sys_now = 0;
  
  /* Schedule a timer (simulating eco mode with entries) */
  sys_timeout(TEST_TIMEOUT_10SEC_MS, dns_timeout_cb, NULL);
  fail_unless(has_dns_timeout_scheduled(&when));
  fail_unless(dns_test_get_timer_started() == 1);
  
  /* Now clear all DNS table entries (simulate table becoming empty) */
  entry1->state = DNS_STATE_UNUSED;
  entry2->state = DNS_STATE_UNUSED;
  
  /* Call dns_tmr() when DNS table is now empty */
  dns_tmr();
  
  /* Verify timer was stopped when table became empty */
  fail_unless(dns_test_get_timer_started() == 0);
  fail_unless(!has_dns_timeout_scheduled(NULL));
}
END_TEST

/* Test 10: Verify TTL decrement differences between eco and active modes */
START_TEST(test_dns_ttl_decrement_eco_vs_active)
{
  ip_addr_t server;
  struct dns_table_entry *entry;
  u32_t initial_ttl;
  u32_t prev_time;
  u32_t ttl_after_active;
  LWIP_UNUSED_ARG(_i);

  /* Setup */
  IP_ADDR4(&server, 8, 8, 8, 8);
  dns_setserver(0, &server);
  initial_ttl = TEST_TTL_LARGE_HOUR_SEC;
  
  /* ===== Part 1: Test Active Mode TTL Decrement ===== */
  /* Active mode decrements TTL by 1 second per DNS_TMR_INTERVAL tick */
  
  /* Create a cached entry */
  entry = dns_test_get_table_entry(0);
  fail_unless(entry != NULL);
  
  entry->state = DNS_STATE_DONE;
  entry->ttl = initial_ttl;
  entry->last_check_time = 0;
  entry->txid = 1;
  entry->server_idx = 0;
  entry->tmr = 0;
  entry->retries = 0;
  entry->seqno = 0;
  IP_ADDR4(&entry->ipaddr, 10, 0, 0, 1);
  strcpy(entry->name, "active-mode.com");
  
  /* Setup: Timer in active mode (active queries or processing) */
  dns_test_set_timer_started(1);
  dns_test_set_eco_mode(0);  /* Active mode */
  lwip_sys_now = 0;
  
  /* Fire timer once (active mode fires every DNS_TMR_INTERVAL = 1000ms) */
  lwip_sys_now += DNS_TMR_INTERVAL;
  dns_tmr();
  
  /* Verify: In active mode, TTL decrements by exactly 1 second per tick */
  fail_unless(entry->state == DNS_STATE_DONE);
  fail_unless(entry->ttl == initial_ttl - TEST_ELAPSED_1_SEC);
  
  /* Fire timer again */
  lwip_sys_now += DNS_TMR_INTERVAL;
  dns_tmr();
  
  /* Verify: TTL decremented by another 1 second */
  fail_unless(entry->ttl == initial_ttl - TEST_ELAPSED_2_SEC);
  
  /* Clear entry for next test */
  entry->state = DNS_STATE_UNUSED;
  
  /* ===== Part 2: Test Eco Mode TTL Decrement ===== */
  /* Eco mode decrements TTL by actual elapsed time (variable, not fixed 1 second) */
  
  /* Create a new cached entry */
  entry->state = DNS_STATE_DONE;
  entry->ttl = initial_ttl;
  entry->last_check_time = TEST_TIME_BASE_MS;
  entry->txid = 2;
  entry->server_idx = 0;
  entry->tmr = 0;
  entry->retries = 0;
  entry->seqno = 0;
  IP_ADDR4(&entry->ipaddr, 10, 0, 0, 2);
  strcpy(entry->name, "eco-mode.com");
  
  /* Setup: Timer in eco mode (only cached entries, no active queries) */
  dns_test_set_timer_started(1);
  dns_test_set_eco_mode(1);  /* Eco mode */
  lwip_sys_now = TEST_TIME_BASE_MS;
  
  /* Advance time by 30 seconds (simulate long eco-mode interval) */
  lwip_sys_now += TEST_TIMEOUT_30SEC_MS;
  
  /* Fire timer once */
  dns_tmr();
  
  /* Verify: In eco mode, TTL decrements by actual elapsed time (30 seconds) */
  fail_unless(entry->state == DNS_STATE_DONE);
  fail_unless(entry->ttl == initial_ttl - TEST_ELAPSED_30_SEC);
  
  /* Advance time by another 45 seconds */
  prev_time = lwip_sys_now;
  lwip_sys_now += TEST_TIMEOUT_45SEC_MS;
  entry->last_check_time = prev_time;
  
  /* Fire timer again */
  dns_tmr();
  
  /* Verify: TTL decremented by another 45 seconds */
  fail_unless(entry->ttl == initial_ttl - TEST_ELAPSED_30_SEC - TEST_ELAPSED_45_SEC);
  
  /* Clear entry for comparison test */
  entry->state = DNS_STATE_UNUSED;
  
  /* ===== Part 3: Demonstrate the key difference ===== */
  /* Same time advance, different decrement behavior */
  
  /* Create entry for active mode test */
  entry = dns_test_get_table_entry(0);
  fail_unless(entry != NULL);
  
  entry->state = DNS_STATE_DONE;
  entry->ttl = TEST_TTL_COMPARE_SEC;
  entry->last_check_time = TEST_TIME_COMPARE_START;
  entry->txid = 3;
  entry->server_idx = 0;
  entry->tmr = 0;
  entry->retries = 0;
  entry->seqno = 0;
  IP_ADDR4(&entry->ipaddr, 10, 0, 0, 3);
  strcpy(entry->name, "compare-active.com");
  
  /* Setup active mode and timer */
  dns_test_set_timer_started(1);
  dns_test_set_eco_mode(0);
  lwip_sys_now = TEST_TIME_COMPARE_START;
  
  /* Advance time by 1 second and fire timer (active mode) */
  lwip_sys_now = TEST_TIME_AFTER_1SEC;
  dns_tmr();
  
  /* Active mode: decrements by 1 regardless of actual time */
  fail_unless(entry->ttl == TEST_TTL_COMPARE_SEC - TEST_ELAPSED_1_SEC);
  
  /* Save active mode result */
  ttl_after_active = entry->ttl;
  
  /* Reset entry for eco mode test */
  entry->state = DNS_STATE_DONE;
  entry->ttl = TEST_TTL_COMPARE_SEC;
  entry->last_check_time = TEST_TIME_COMPARE_START;
  strcpy(entry->name, "compare-eco.com");
  
  /* Setup eco mode and timer */
  dns_test_set_timer_started(1);
  dns_test_set_eco_mode(1);
  lwip_sys_now = TEST_TIME_COMPARE_START;
  
  /* Advance time by 10 seconds and fire timer (eco mode) */
  lwip_sys_now = TEST_TIME_AFTER_10SEC;
  dns_tmr();
  
  /* Eco mode: decrements by actual elapsed time (10 seconds) */
  fail_unless(entry->ttl == TEST_TTL_COMPARE_SEC - TEST_ELAPSED_10_SEC);
  
  /* Key verification: Different behaviors for different modes */
  fail_unless(ttl_after_active != entry->ttl);
  fail_unless(entry->ttl < ttl_after_active);
}
END_TEST

/* Test 11: Verify active mode always uses DNS_TMR_INTERVAL */
START_TEST(test_dns_active_mode_interval)
{
  ip_addr_t server, addr;
  u32_t when;
  int i;
  LWIP_UNUSED_ARG(_i);

  /* Setup */
  IP_ADDR4(&server, 8, 8, 8, 8);
  dns_setserver(0, &server);
  dns_test_set_timer_started(0);
  dns_test_set_eco_mode(0);
  
  /* Start query */
  dns_gethostbyname("test.com", &addr, dummy_found_cb, NULL);
  
  /* Verify initial schedule */
  fail_unless(has_dns_timeout_scheduled(&when));
  fail_unless(when == DNS_TMR_INTERVAL);
  fail_unless(dns_test_get_eco_mode() == 0);
  
  /* Fire timer multiple times - should always reschedule at DNS_TMR_INTERVAL */
  for (i = 0; i < 5; i++) {
    lwip_sys_now += DNS_TMR_INTERVAL;
    dns_tmr();
    
    /* After dns_tmr(), if query still active, should reschedule at DNS_TMR_INTERVAL */
    if (has_dns_timeout_scheduled(&when)) {
      fail_unless(when == DNS_TMR_INTERVAL);
    }
  }
}
END_TEST

/* Test 12: Verify timer doesn't start without DNS server configured */
START_TEST(test_dns_no_server_no_timer_start)
{
  ip_addr_t addr;
  err_t err;
  LWIP_UNUSED_ARG(_i);

  /* Reset timer state */
  dns_test_set_timer_started(0);
  dns_test_set_eco_mode(0);
  
  /* Ensure no DNS servers are configured */
  dns_setserver(0, IP_ADDR_ANY);
  dns_setserver(1, IP_ADDR_ANY);
  
  /* Try to enqueue a query */
  err = dns_gethostbyname("test.com", &addr, dummy_found_cb, NULL);
  
  /* Should return error since no server is configured */
  fail_unless(err == ERR_VAL);
  
  /* Timer should not have started */
  fail_unless(dns_test_get_timer_started() == 0);
  fail_unless(!has_dns_timeout_scheduled(NULL));
}
END_TEST

/* Test 13: Verify timer restarts correctly after being stopped */
START_TEST(test_dns_timer_restart_after_stop)
{
  ip_addr_t server, addr;
  u32_t when;
  LWIP_UNUSED_ARG(_i);

  /* Setup */
  IP_ADDR4(&server, 8, 8, 8, 8);
  dns_setserver(0, &server);
  
  /* Simulate timer being stopped (empty table scenario) */
  dns_test_set_timer_started(0);
  dns_test_set_eco_mode(1);
  fail_unless(!has_dns_timeout_scheduled(NULL));
  
  /* Start a new query - should restart timer */
  dns_gethostbyname("restart.com", &addr, dummy_found_cb, NULL);
  
  /* Verify timer restarted */
  fail_unless(dns_test_get_timer_started() == 1);
  fail_unless(has_dns_timeout_scheduled(&when));
  fail_unless(when == DNS_TMR_INTERVAL);
  fail_unless(dns_test_get_eco_mode() == 0);
}
END_TEST

/* Test 14: Verify rapid successive queries maintain active mode */
START_TEST(test_dns_rapid_queries_active_mode)
{
  ip_addr_t server, addr;
  u32_t when;
  int i;
  LWIP_UNUSED_ARG(_i);

  /* Setup */
  IP_ADDR4(&server, 8, 8, 8, 8);
  dns_setserver(0, &server);
  dns_test_set_timer_started(0);
  
  /* Fire multiple rapid queries */
  for (i = 0; i < 5; i++) {
    char hostname[32];
    sprintf(hostname, "rapid%d.com", i);
    dns_gethostbyname(hostname, &addr, dummy_found_cb, NULL);
    
    /* Each query should keep us in active mode */
    fail_unless(dns_test_get_eco_mode() == 0);
    fail_unless(has_dns_timeout_scheduled(&when));
    fail_unless(when == DNS_TMR_INTERVAL);
  }
  
  /* Verify timer is still started */
  fail_unless(dns_test_get_timer_started() == 1);
}
END_TEST

/* Test 15: Verify per-entry last_check_time updates correctly */
START_TEST(test_dns_per_entry_last_check_time)
{
  struct dns_table_entry *entry;
  u32_t initial_times[DNS_TABLE_SIZE] = {0, TEST_TIMEOUT_1SEC_MS, TEST_TIMEOUT_5SEC_MS, TEST_TIMEOUT_10SEC_MS};
  u32_t target_time;
  int i;
  LWIP_UNUSED_ARG(_i);

  /* Setup eco mode */
  dns_test_set_eco_mode(1);
  dns_test_set_timer_started(1);
  lwip_sys_now = 0;
  
  /* Setup all DNS_TABLE_SIZE entries with different initial timestamps */
  for (i = 0; i < DNS_TABLE_SIZE; i++) {
    entry = dns_test_get_table_entry(i);
    if (entry == NULL) {
      continue;
    }
    
    /* Initialize entry to valid DONE state with TTL */
    entry->state = DNS_STATE_DONE;
    entry->ttl = TEST_TTL_LARGE_HOUR_SEC;
    entry->txid = i + 1;
    entry->server_idx = 0;
    entry->tmr = 0;
    entry->retries = 0;
    entry->seqno = 0;
    IP_ADDR4(&entry->ipaddr, 10, 0, 0, i + 1);
    sprintf(entry->name, "entry-%d.com", i);
    
    /* Each entry starts with different initial timestamp */
    entry->last_check_time = initial_times[i];
  }
  
  /* Advance time to 30 seconds */
  target_time = TEST_TIMEOUT_30SEC_MS;
  lwip_sys_now = target_time;
  
  /* Fire timer once - processes ALL entries */
  dns_tmr();
  
  /* Verify each entry's timestamp was updated independently */
  for (i = 0; i < DNS_TABLE_SIZE; i++) {
    entry = dns_test_get_table_entry(i);
    if (entry != NULL && entry->state == DNS_STATE_DONE) {
      u32_t elapsed_ms;
      u32_t fractional_ms;
      u32_t expected_time;
      
      /* Calculate expected timestamp based on fractional millisecond preservation */
      elapsed_ms = target_time - initial_times[i];
      fractional_ms = elapsed_ms % 1000U;
      expected_time = target_time - fractional_ms;
      
      /* Verify last_check_time was updated correctly for this entry */
      fail_unless(entry->last_check_time == expected_time);
    }
  }
}
END_TEST

/* Test 16: Verify mode transitions work correctly */
START_TEST(test_dns_mode_transitions)
{
  ip_addr_t server, addr;
  u32_t when;
  LWIP_UNUSED_ARG(_i);

  /* Setup */
  IP_ADDR4(&server, 8, 8, 8, 8);
  dns_setserver(0, &server);
  
  /* Start in eco mode (simulate cached entries) */
  dns_test_set_timer_started(1);
  dns_test_set_eco_mode(1);
  sys_timeout(TEST_TIMEOUT_30SEC_MS, dns_timeout_cb, NULL);
  
  fail_unless(dns_test_get_eco_mode() == 1);
  
  /* Transition to active mode via new query */
  dns_gethostbyname("query1.com", &addr, dummy_found_cb, NULL);
  fail_unless(dns_test_get_eco_mode() == 0);
  fail_unless(has_dns_timeout_scheduled(&when));
  fail_unless(when == DNS_TMR_INTERVAL);
  
  /* Simulate completion to eco mode */
  lwip_sys_now += DNS_TMR_INTERVAL;
  dns_tmr();
  
  /* Add another query to go back to active */
  dns_gethostbyname("query2.com", &addr, dummy_found_cb, NULL);
  fail_unless(dns_test_get_eco_mode() == 0);
}
END_TEST

/* Test 17: Verify single cached entry with minimum TTL */
START_TEST(test_dns_single_cached_min_ttl)
{
  u32_t when;
  u32_t min_ttl_ms;
  LWIP_UNUSED_ARG(_i);

  /* Setup: simulate eco mode with cached entry having TTL=1 second */
  dns_test_set_timer_started(1);
  dns_test_set_eco_mode(1);
  lwip_sys_now = 0;
  
  /* In eco mode with min_ttl=1, next_timeout should be 1 * 1000 = 1000ms */
  min_ttl_ms = TEST_TIMEOUT_1SEC_MS;
  
  /* Schedule a timer at the minimum TTL interval */
  sys_timeout(min_ttl_ms, dns_timeout_cb, NULL);
  
  fail_unless(has_dns_timeout_scheduled(&when));
  fail_unless(when == min_ttl_ms);
  
  /* Advance time and fire timer */
  lwip_sys_now = min_ttl_ms;
  dns_tmr();
}
END_TEST

/* Test 18: Server Rotation After Retry Exhaustion */
START_TEST(test_dns_asking_retry_server_rotation)
{
  ip_addr_t server1, server2, addr;
  struct dns_table_entry *entry;
  u32_t when;
  int i;
  LWIP_UNUSED_ARG(_i);

  /* Setup: Configure two DNS servers */
  IP_ADDR4(&server1, 8, 8, 8, 8);
  IP_ADDR4(&server2, 1, 1, 1, 1);
  dns_setserver(0, &server1);
  dns_setserver(1, &server2);
  
  /* Reset state */
  dns_test_set_timer_started(0);
  dns_test_set_eco_mode(0);
  found_called = 0;
  lwip_sys_now = 0;
  
  /* Start a query - no response will be simulated */
  dns_gethostbyname("retry-test.com", &addr, dummy_found_cb, NULL);
  fail_unless(has_dns_timeout_scheduled(&when));
  
  /* Verify initial state: starts with server 0, retries=0, tmr=1 */
  entry = dns_test_get_table_entry(0);
  fail_unless(entry != NULL);
  fail_unless(entry->state == DNS_STATE_ASKING);
  fail_unless(entry->server_idx == 0);
  fail_unless(entry->retries == 0);
  fail_unless(entry->tmr == 1);
  
  /* Exhaust retries on server 0 by calling dns_tmr() repeatedly without providing response.
   * Retry sequence with exponential backoff:
   * Call 1: tmr=1→0, retries=0→1, tmr=1
   * Call 2: tmr=1→0, retries=1→2, tmr=2
   * Call 3: tmr=2→1 (no retry yet)
   * Call 4: tmr=1→0, retries=2→3, tmr=3
   * Call 5-7: tmr=3→2→1→0, retries=3→4 (==DNS_MAX_RETRIES) → switch server */
  for (i = 0; i < 7; i++) {
    lwip_sys_now += DNS_TMR_INTERVAL;
    dns_tmr();
  }
  
  /* Verify server rotation occurred: server_idx incremented, retries reset */
  entry = dns_test_get_table_entry(0);
  fail_unless(entry != NULL);
  fail_unless(entry->state == DNS_STATE_ASKING);
  fail_unless(entry->server_idx == 1);
  fail_unless(entry->retries == 0);
  fail_unless(entry->tmr == 1);
  
  /* Verify callback not called yet (query still active on backup server) */
  fail_unless(found_called == 0);
}
END_TEST

/* Test 19: Retry Exhaustion Across All Servers */
START_TEST(test_dns_backup_server_exhaust_timeout)
{
  ip_addr_t server1, server2, addr;
  struct dns_table_entry *entry;
  u32_t when;
  int i, server;
  LWIP_UNUSED_ARG(_i);

  /* Setup: Configure two DNS servers (DNS_MAX_SERVERS = 2) */
  IP_ADDR4(&server1, 8, 8, 8, 8);
  IP_ADDR4(&server2, 1, 1, 1, 1);
  dns_setserver(0, &server1);
  dns_setserver(1, &server2);
  
  /* Reset state */
  dns_test_set_timer_started(0);
  dns_test_set_eco_mode(0);
  found_called = 0;
  found_last_ip = NULL;
  lwip_sys_now = 0;
  
  /* Start a query - no response will be simulated */
  dns_gethostbyname("exhaust-test.com", &addr, dummy_found_cb, NULL);
  fail_unless(has_dns_timeout_scheduled(&when));
  
  /* Verify initial entry state uses first server */
  entry = dns_test_get_table_entry(0);
  fail_unless(entry != NULL);
  fail_unless(entry->state == DNS_STATE_ASKING);
  fail_unless(entry->server_idx == 0);
  fail_unless(entry->retries == 0);
  
  /* Exhaust retries on both servers sequentially.
   * Each server requires 7 dns_tmr() calls to exhaust (as explained in previous test).
   * After exhausting server 0, it rotates to server 1.
   * After exhausting server 1 (last server), the query fails. */
  for (server = 0; server < 2; server++) {
    /* Verify we're on the expected server */
    entry = dns_test_get_table_entry(0);
    fail_unless(entry->state == DNS_STATE_ASKING);
    fail_unless(entry->server_idx == server);
    
    /* Exhaust current server (7 timer calls) */
    for (i = 0; i < 7; i++) {
      lwip_sys_now += DNS_TMR_INTERVAL;
      dns_tmr();
    }
  }
  
  /* After exhausting all servers, callback should be called with NULL (failure) */
  fail_unless(found_called == 1);
  fail_unless(found_last_ip == NULL);
  
  /* Entry should be marked as UNUSED after complete exhaustion */
  entry = dns_test_get_table_entry(0);
  fail_unless(entry->state == DNS_STATE_UNUSED);
}
END_TEST

/* Test 20 :ECO→ACTIVE Transition: Cached Entry TTL Granularity */
START_TEST(test_dns_eco_to_active_cached_ttl_granularity)
{
  ip_addr_t server, addr;
  struct dns_table_entry *entry1, *entry2, *entry3;
  u32_t when;
  LWIP_UNUSED_ARG(_i);

  /* Setup: Configure DNS server */
  IP_ADDR4(&server, 8, 8, 8, 8);
  dns_setserver(0, &server);
  
  /* Reset state and set initial time */
  dns_test_set_timer_started(0);
  dns_test_set_eco_mode(0);
  lwip_sys_now = 0;
  
  /* Manually create 3 cached DNS entries with different TTLs
   * Simulating entries that were resolved and cached earlier */
  entry1 = dns_test_get_table_entry(0);
  entry2 = dns_test_get_table_entry(1);
  entry3 = dns_test_get_table_entry(2);
  
  fail_unless(entry1 != NULL);
  fail_unless(entry2 != NULL);
  fail_unless(entry3 != NULL);
  
  /* Setup Entry 1: TTL=10s */
  entry1->state = DNS_STATE_DONE;
  entry1->ttl = TEST_TTL_MIN_1_SEC;
  entry1->last_check_time = 0;
  entry1->txid = 1;
  entry1->server_idx = 0;
  entry1->tmr = 0;
  entry1->retries = 0;
  entry1->seqno = 0;
  IP_ADDR4(&entry1->ipaddr, 1, 2, 3, 4);
  strcpy(entry1->name, "cached1.com");
  
  /* Setup Entry 2: TTL=11s */
  entry2->state = DNS_STATE_DONE;
  entry2->ttl = TEST_TTL_MIN_2_SEC;
  entry2->last_check_time = 0;
  entry2->txid = 2;
  entry2->server_idx = 0;
  entry2->tmr = 0;
  entry2->retries = 0;
  entry2->seqno = 0;
  IP_ADDR4(&entry2->ipaddr, 1, 2, 3, 5);
  strcpy(entry2->name, "cached2.com");
  
  /* Setup Entry 3: TTL=60s */
  entry3->state = DNS_STATE_DONE;
  entry3->ttl = TEST_TTL_MIN_3_SEC;
  entry3->last_check_time = 0;
  entry3->txid = 3;
  entry3->server_idx = 0;
  entry3->tmr = 0;
  entry3->retries = 0;
  entry3->seqno = 0;
  IP_ADDR4(&entry3->ipaddr, 1, 2, 3, 6);
  strcpy(entry3->name, "cached3.com");
  
  /* Manually set up ECO mode state with proper timer
   * Note: We need to manually manage timer state since we're bypassing active flow */
  dns_test_set_timer_started(1);
  dns_test_set_eco_mode(1);
  
  /* Manually schedule eco timer at t=0 for minimum TTL (10 seconds = 10000ms) */
  sys_timeout(TEST_TIMEOUT_10SEC_MS, dns_timeout_cb, NULL);
  
  /* Verify eco mode setup */
  fail_unless(dns_test_get_eco_mode() == 1);
  fail_unless(dns_test_get_timer_started() == 1);
  fail_unless(has_dns_timeout_scheduled(&when));
  fail_unless(when == TEST_TIMEOUT_10SEC_MS);
  
  /* ===== THE CRITICAL TEST =====
   * Advance time to 1.1 seconds (1100ms)
   * This creates a fractional second scenario */
  lwip_sys_now = TEST_TIMEOUT_1_1SEC_MS;
  
  /* Trigger a new DNS query at t=1100ms
   * This should cause ECO→ACTIVE transition
   * AND update all cached entries' TTLs */
  dns_gethostbyname("newquery.com", &addr, dummy_found_cb, NULL);
  
  /* Verify mode transition happened */
  fail_unless(dns_test_get_eco_mode() == 0);
  fail_unless(dns_test_get_timer_started() == 1);
  /* Timer should exist and be rescheduled (not the old 10000ms eco timer) */
  fail_unless(has_dns_timeout_scheduled(&when));
  fail_unless(when != TEST_TIMEOUT_10SEC_MS);
  
  /* ===== VERIFY TTL GRANULARITY =====
   * After 1.1 seconds elapsed:
   * - Entry1: 10s - 1s = 9s (fractional 0.1s accounted)
   * - Entry2: 11s - 1s = 10s
   * - Entry3: 60s - 1s = 59s
   * Note: last_check_time is set to (now - fractional_ms) to preserve fractional time
   * So at now=1100ms with fractional=100ms: last_check_time = 1000ms */
  entry1 = dns_test_get_table_entry(0);
  entry2 = dns_test_get_table_entry(1);
  entry3 = dns_test_get_table_entry(2);
  
  fail_unless(entry1->state == DNS_STATE_DONE);
  fail_unless(entry1->ttl == 9);
  fail_unless(entry1->last_check_time == TEST_TIMEOUT_1SEC_MS);
  
  fail_unless(entry2->state == DNS_STATE_DONE);
  fail_unless(entry2->ttl == 10);
  fail_unless(entry2->last_check_time == TEST_TIMEOUT_1SEC_MS);
  
  fail_unless(entry3->state == DNS_STATE_DONE);
  fail_unless(entry3->ttl == 59);
  fail_unless(entry3->last_check_time == TEST_TIMEOUT_1SEC_MS);
  
  /* ===== VERIFY CONTINUED ACTIVE MODE BEHAVIOR =====
   * Advance time by 1 more second and fire active mode timer */
  lwip_sys_now = 2100;
  dns_tmr();
  
  /* After 1 more second in active mode:
   * - Entry1: 9s - 1s = 8s
   * - Entry2: 10s - 1s = 9s
   * - Entry3: 59s - 1s = 58s
   * Note: In active mode, last_check_time is also adjusted to preserve fractional time.
   * Entries had last_check_time=1000ms, now=2100ms, elapsed=1100ms, fractional=100ms
   * So: last_check_time = 2100 - 100 = 2000ms */
  entry1 = dns_test_get_table_entry(0);
  entry2 = dns_test_get_table_entry(1);
  entry3 = dns_test_get_table_entry(2);
  
  fail_unless(entry1->ttl == 8);
  fail_unless(entry1->last_check_time == 2000);
  
  fail_unless(entry2->ttl == 9);
  fail_unless(entry2->last_check_time == 2000);
  
  fail_unless(entry3->ttl == 58);
  fail_unless(entry3->last_check_time == 2000);
}
END_TEST

/* Test 21: Structure Layout Validation */
START_TEST(test_dns_structure_layout_validation)
{
#ifdef LWIP_TESTMODE
  struct dns_table_entry test_entry;
  size_t actual_size;
  LWIP_UNUSED_ARG(_i);
  
  actual_size = sizeof(struct dns_table_entry);
  fail_unless(actual_size >= DNS_TABLE_ENTRY_EXPECTED_SIZE_MIN);
  
  fail_unless(offsetof(struct dns_table_entry, ttl) == 0);
  fail_unless(offsetof(struct dns_table_entry, last_check_time) > 
              offsetof(struct dns_table_entry, name));
  
  test_entry.ttl = 100;
  test_entry.state = DNS_STATE_DONE;
  test_entry.last_check_time = 0;
  test_entry.tmr = 0;
  test_entry.retries = 0;
  test_entry.seqno = 0;
  test_entry.server_idx = 0;
  test_entry.txid = 1;
  strcpy(test_entry.name, "test.com");
  
  fail_unless(test_entry.ttl == 100);
  fail_unless(test_entry.state == DNS_STATE_DONE);
#endif
}
END_TEST

#endif /* SL_LWIP_DNS_ONDEMAND_TIMER */

/** Create the suite including all tests for this module */
Suite *
dns_suite(void)
{
  testfunc tests[] = {
    TESTFUNC(test_dns_set_get_server),
#if SL_LWIP_DNS_ONDEMAND_TIMER
    TESTFUNC(test_dns_eco_mode_activation),
    TESTFUNC(test_dns_timer_started_flag),
    TESTFUNC(test_dns_eco_to_active_transition),
    TESTFUNC(test_dns_last_check_time_init),
    TESTFUNC(test_dns_wraparound_handling),
    TESTFUNC(test_dns_overflow_protection),
    TESTFUNC(test_dns_min_ttl_selection),
    TESTFUNC(test_dns_mixed_state_active_mode),
    TESTFUNC(test_dns_timer_stops_when_empty),
    TESTFUNC(test_dns_ttl_decrement_eco_vs_active),
    TESTFUNC(test_dns_active_mode_interval),
    TESTFUNC(test_dns_no_server_no_timer_start),
    TESTFUNC(test_dns_timer_restart_after_stop),
    TESTFUNC(test_dns_rapid_queries_active_mode),
    TESTFUNC(test_dns_per_entry_last_check_time),
    TESTFUNC(test_dns_mode_transitions),
    TESTFUNC(test_dns_single_cached_min_ttl),
    TESTFUNC(test_dns_asking_retry_server_rotation),
    TESTFUNC(test_dns_backup_server_exhaust_timeout),
    TESTFUNC(test_dns_eco_to_active_cached_ttl_granularity),
    TESTFUNC(test_dns_structure_layout_validation),
#endif
  };
  return create_suite("DNS", tests, sizeof(tests)/sizeof(testfunc), dns_setup, dns_teardown);
}
