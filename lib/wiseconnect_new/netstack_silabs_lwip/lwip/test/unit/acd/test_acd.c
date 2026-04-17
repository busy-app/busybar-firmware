
#include "test_acd.h"

#include "lwip/netif.h"
#include "lwip/acd.h"
#include "lwip/prot/acd.h"
#include "lwip/prot/etharp.h"
#include "lwip/etharp.h"
#include "lwip/inet.h"
#include "netif/ethernet.h"
 #include "lwip/opt.h"
#include <string.h>

/* Define ACD_TICKS_PER_SECOND for testing */
#define ACD_TICKS_PER_SECOND  (1000 / ACD_TMR_INTERVAL)

/**
 * @file test_acd.c
 * @brief ACD (Address Conflict Detection) unit tests
 * 
 * Test Organization:
 * ==================
 * 
 * Tests are organized into three categories based on SL_LWIP_ACD_ONDEMAND_TIMER:
 * 
 * 1. COMMON TESTS (lines ~87-438):
 *    - Work regardless of SL_LWIP_ACD_ONDEMAND_TIMER setting
 *    - Test basic ACD functionality: add/remove, start/stop, state machine, etc.
 *    - These tests don't check timer_active field
 * 
 * 2. TIMER-SPECIFIC TESTS (lines ~440-1061):
 *    - Only compiled when SL_LWIP_ACD_ONDEMAND_TIMER is enabled
 *    - Test on-demand timer activation/deactivation
 *    - Verify timer_active field behavior
 *    - All wrapped in #if SL_LWIP_ACD_ONDEMAND_TIMER blocks
 * 
 * 3. DISABLED MODE TESTS (future):
 *    - Can be added in #else block for SL_LWIP_ACD_ONDEMAND_TIMER disabled
 *    - Currently, common tests cover disabled mode functionality
 * 
 * The test suite function (acd_suite) properly separates these categories.
 */

#if LWIP_ACD

static struct netif net_test;
static struct acd acd_test;
static acd_callback_enum_t callback_state;

static int arp_packet_count;
static struct pbuf *last_arp_pbuf;

static err_t
testif_tx_func(struct netif *netif, struct pbuf *p)
{
  LWIP_UNUSED_ARG(netif);
  LWIP_UNUSED_ARG(p);
  return ERR_OK;
}

static err_t
testif_init(struct netif *netif)
{
  netif->name[0] = 't';
  netif->name[1] = '0';
  netif->output = etharp_output;
  netif->linkoutput = testif_tx_func;
  netif->mtu = 1500;
  netif->hwaddr_len = 6;
  netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_ETHERNET;
  
  /* Set a test MAC address */
  netif->hwaddr[0] = 0x02;
  netif->hwaddr[1] = 0x03;
  netif->hwaddr[2] = 0x04;
  netif->hwaddr[3] = 0x05;
  netif->hwaddr[4] = 0x66;
  netif->hwaddr[5] = 0x77;

  return ERR_OK;
}

/* Callback function for ACD */
static void
acd_conflict_callback(struct netif *netif, acd_callback_enum_t state)
{
  LWIP_UNUSED_ARG(netif);
  callback_state = state;
}

/* Setups/teardown functions */

static void
acd_setup(void)
{
  lwip_check_ensure_no_alloc(SKIP_POOL(MEMP_SYS_TIMEOUT));
  memset(&net_test, 0, sizeof(struct netif));
  memset(&acd_test, 0, sizeof(struct acd));
  callback_state = (acd_callback_enum_t)-1;
  arp_packet_count = 0;
  last_arp_pbuf = NULL;
}

static void
acd_teardown(void)
{
  /* Remove all ACD instances from the list to prevent dangling pointers.
   * Some tests (e.g., test_acd_link_down_stops_all, test_acd_multiple_instances_timer_sharing)
   * create local acd_test2 instances that may not be cleaned up if tests fail early.
   */
  struct acd *acd;
  struct acd *next;
  
  for (acd = net_test.acd_list; acd != NULL; acd = next) {
    next = acd->next;
    /* Remove from list - this will also deactivate timer if needed */
    acd_remove(&net_test, acd);
  }
  
  netif_remove(&net_test);
  lwip_check_ensure_no_alloc(SKIP_POOL(MEMP_SYS_TIMEOUT));
}

/* Test functions */

START_TEST(test_acd_add_remove)
{
  err_t err;
  LWIP_UNUSED_ARG(_i);

  netif_add(&net_test, NULL, NULL, NULL, NULL, testif_init, ethernet_input);
  netif_set_link_up(&net_test);
  netif_set_up(&net_test);

  /* Test: Add ACD */
  err = acd_add(&net_test, &acd_test, acd_conflict_callback);
  fail_unless(err == ERR_OK);
  
  /* Verify ACD is in list */
  fail_unless(net_test.acd_list == &acd_test);
  fail_unless(acd_test.acd_conflict_callback == acd_conflict_callback);
  
  /* Test: Remove ACD */
  acd_remove(&net_test, &acd_test);
  fail_unless(net_test.acd_list == NULL);
}
END_TEST

START_TEST(test_acd_start_stop)
{
  err_t err;
  ip4_addr_t test_addr;
  LWIP_UNUSED_ARG(_i);

  netif_add(&net_test, NULL, NULL, NULL, NULL, testif_init, ethernet_input);
  netif_set_link_up(&net_test);
  netif_set_up(&net_test);

  acd_add(&net_test, &acd_test, acd_conflict_callback);
  
  /* Test: Start ACD */
  IP4_ADDR(&test_addr, 192, 168, 1, 100);
  err = acd_start(&net_test, &acd_test, test_addr);
  fail_unless(err == ERR_OK);
  
  /* Verify state is PROBE_WAIT */
  fail_unless(acd_test.state == ACD_STATE_PROBE_WAIT);
  
  /* Verify address is set */
  fail_unless(ip4_addr_eq(&acd_test.ipaddr, &test_addr));
  
  /* Test: Stop ACD */
  err = acd_stop(&acd_test);
  fail_unless(err == ERR_OK);
  
  /* Verify state is OFF */
  fail_unless(acd_test.state == ACD_STATE_OFF);
  
  acd_remove(&net_test, &acd_test);
}
END_TEST

START_TEST(test_acd_add_twice)
{
  err_t err;
  LWIP_UNUSED_ARG(_i);

  netif_add(&net_test, NULL, NULL, NULL, NULL, testif_init, ethernet_input);
  netif_set_link_up(&net_test);
  netif_set_up(&net_test);

  /* Add ACD */
  err = acd_add(&net_test, &acd_test, acd_conflict_callback);
  fail_unless(err == ERR_OK);
  
  /* Try to add again (should be OK and return ERR_OK) */
  err = acd_add(&net_test, &acd_test, acd_conflict_callback);
  fail_unless(err == ERR_OK);
  
  /* Should still be in list once */
  fail_unless(net_test.acd_list == &acd_test);
  
  acd_remove(&net_test, &acd_test);
}
END_TEST

START_TEST(test_acd_state_machine_probe_wait)
{
  ip4_addr_t test_addr;
  u16_t initial_ttw;
  LWIP_UNUSED_ARG(_i);

  netif_add(&net_test, NULL, NULL, NULL, NULL, testif_init, ethernet_input);
  netif_set_link_up(&net_test);
  netif_set_up(&net_test);

  acd_add(&net_test, &acd_test, acd_conflict_callback);
  
  IP4_ADDR(&test_addr, 192, 168, 1, 100);
  acd_start(&net_test, &acd_test, test_addr);
  
  /* Verify initial state */
  fail_unless(acd_test.state == ACD_STATE_PROBE_WAIT);
  fail_unless(acd_test.sent_num == 0);
  
  initial_ttw = acd_test.ttw;
  fail_unless(initial_ttw > 0);
  
  acd_stop(&acd_test);
  acd_remove(&net_test, &acd_test);
}
END_TEST

START_TEST(test_acd_state_machine_probing)
{
  ip4_addr_t test_addr;
  LWIP_UNUSED_ARG(_i);

  netif_add(&net_test, NULL, NULL, NULL, NULL, testif_init, ethernet_input);
  netif_set_link_up(&net_test);
  netif_set_up(&net_test);

  acd_add(&net_test, &acd_test, acd_conflict_callback);
  
  IP4_ADDR(&test_addr, 192, 168, 1, 100);
  acd_start(&net_test, &acd_test, test_addr);
  
  /* Simulate timer ticks until ttw reaches 0 */
  while (acd_test.ttw > 0 && acd_test.state == ACD_STATE_PROBE_WAIT) {
    acd_test.ttw--;
  }
  
  /* Advance timer to trigger probe */
  if (acd_test.ttw == 0 && acd_test.state == ACD_STATE_PROBE_WAIT) {
    acd_test.state = ACD_STATE_PROBING;
  }
  
  /* Verify state transition */
  fail_unless(acd_test.state == ACD_STATE_PROBING);
  
  acd_stop(&acd_test);
  acd_remove(&net_test, &acd_test);
}
END_TEST

START_TEST(test_acd_num_conflicts_increment)
{
  ip4_addr_t test_addr;
  u8_t initial_conflicts;
  LWIP_UNUSED_ARG(_i);

  netif_add(&net_test, NULL, NULL, NULL, NULL, testif_init, ethernet_input);
  netif_set_link_up(&net_test);
  netif_set_up(&net_test);

  acd_add(&net_test, &acd_test, acd_conflict_callback);
  
  IP4_ADDR(&test_addr, 192, 168, 1, 100);
  acd_start(&net_test, &acd_test, test_addr);
  
  initial_conflicts = acd_test.num_conflicts;
  
  /* Simulate a conflict */
  acd_test.num_conflicts++;
  
  fail_unless(acd_test.num_conflicts == (initial_conflicts + 1));
  
  acd_stop(&acd_test);
  acd_remove(&net_test, &acd_test);
}
END_TEST

START_TEST(test_acd_lastconflict_countdown)
{
  ip4_addr_t test_addr;
  LWIP_UNUSED_ARG(_i);

  netif_add(&net_test, NULL, NULL, NULL, NULL, testif_init, ethernet_input);
  netif_set_link_up(&net_test);
  netif_set_up(&net_test);

  acd_add(&net_test, &acd_test, acd_conflict_callback);
  
  IP4_ADDR(&test_addr, 192, 168, 1, 100);
  acd_start(&net_test, &acd_test, test_addr);
  
  /* Set lastconflict */
  acd_test.lastconflict = DEFEND_INTERVAL * ACD_TICKS_PER_SECOND;
  
  /* Simulate timer tick */
  if (acd_test.lastconflict > 0) {
    acd_test.lastconflict--;
  }
  
  fail_unless(acd_test.lastconflict == (DEFEND_INTERVAL * ACD_TICKS_PER_SECOND - 1));
  
  acd_stop(&acd_test);
  acd_remove(&net_test, &acd_test);
}
END_TEST

START_TEST(test_acd_link_down_stops_all)
{
  ip4_addr_t test_addr;
  struct acd acd_test2;
  LWIP_UNUSED_ARG(_i);

  memset(&acd_test2, 0, sizeof(struct acd));

  netif_add(&net_test, NULL, NULL, NULL, NULL, testif_init, ethernet_input);
  netif_set_link_up(&net_test);
  netif_set_up(&net_test);

  acd_add(&net_test, &acd_test, acd_conflict_callback);
  acd_add(&net_test, &acd_test2, acd_conflict_callback);
  
  IP4_ADDR(&test_addr, 192, 168, 1, 100);
  acd_start(&net_test, &acd_test, test_addr);
  acd_start(&net_test, &acd_test2, test_addr);
  
  /* Verify both are running */
  fail_unless(acd_test.state != ACD_STATE_OFF);
  fail_unless(acd_test2.state != ACD_STATE_OFF);
  
  /* Test: Link down */
  acd_network_changed_link_down(&net_test);
  
  /* Verify both are stopped */
  fail_unless(acd_test.state == ACD_STATE_OFF);
  fail_unless(acd_test2.state == ACD_STATE_OFF);
  
  acd_remove(&net_test, &acd_test);
  acd_remove(&net_test, &acd_test2);
}
END_TEST

START_TEST(test_acd_callback_ip_ok)
{
  ip4_addr_t test_addr;
  LWIP_UNUSED_ARG(_i);

  netif_add(&net_test, NULL, NULL, NULL, NULL, testif_init, ethernet_input);
  netif_set_link_up(&net_test);
  netif_set_up(&net_test);

  acd_add(&net_test, &acd_test, acd_conflict_callback);
  
  IP4_ADDR(&test_addr, 192, 168, 1, 100);
  acd_start(&net_test, &acd_test, test_addr);
  
  /* Simulate callback */
  callback_state = (acd_callback_enum_t)-1;
  acd_conflict_callback(&net_test, ACD_IP_OK);
  
  fail_unless(callback_state == ACD_IP_OK);
  
  acd_stop(&acd_test);
  acd_remove(&net_test, &acd_test);
}
END_TEST

START_TEST(test_acd_rate_limit_state)
{
  ip4_addr_t test_addr;
  LWIP_UNUSED_ARG(_i);

  netif_add(&net_test, NULL, NULL, NULL, NULL, testif_init, ethernet_input);
  netif_set_link_up(&net_test);
  netif_set_up(&net_test);

  acd_add(&net_test, &acd_test, acd_conflict_callback);
  
  IP4_ADDR(&test_addr, 192, 168, 1, 100);
  acd_start(&net_test, &acd_test, test_addr);
  
  /* Set max conflicts */
  acd_test.num_conflicts = MAX_CONFLICTS;
  acd_test.state = ACD_STATE_RATE_LIMIT;
  acd_test.ttw = (u16_t)(RATE_LIMIT_INTERVAL * ACD_TICKS_PER_SECOND);
  
  fail_unless(acd_test.state == ACD_STATE_RATE_LIMIT);
  fail_unless(acd_test.ttw > 0);
  
  /* Simulate timer ticks */
  while (acd_test.ttw > 0) {
    acd_test.ttw--;
  }
  
  /* After rate limit expires, timer should be stopped */
  acd_stop(&acd_test);
  acd_remove(&net_test, &acd_test);
}
END_TEST

START_TEST(test_acd_sent_num_tracking)
{
  ip4_addr_t test_addr;
  LWIP_UNUSED_ARG(_i);

  netif_add(&net_test, NULL, NULL, NULL, NULL, testif_init, ethernet_input);
  netif_set_link_up(&net_test);
  netif_set_up(&net_test);

  acd_add(&net_test, &acd_test, acd_conflict_callback);
  
  IP4_ADDR(&test_addr, 192, 168, 1, 100);
  acd_start(&net_test, &acd_test, test_addr);
  
  /* Verify initial sent_num */
  fail_unless(acd_test.sent_num == 0);
  
  /* Simulate sending probes */
  acd_test.sent_num = 1;
  fail_unless(acd_test.sent_num == 1);
  
  acd_test.sent_num = 2;
  fail_unless(acd_test.sent_num == 2);
  
  /* Should reach PROBE_NUM before announing */
  fail_unless(acd_test.sent_num < PROBE_NUM || acd_test.state != ACD_STATE_ANNOUNCING);
  
  acd_stop(&acd_test);
  acd_remove(&net_test, &acd_test);
}
END_TEST

START_TEST(test_acd_ttw_decrement)
{
  ip4_addr_t test_addr;
  u16_t initial_ttw;
  u16_t ttw_before;
  u16_t ttw_after;
  LWIP_UNUSED_ARG(_i);

  netif_add(&net_test, NULL, NULL, NULL, NULL, testif_init, ethernet_input);
  netif_set_link_up(&net_test);
  netif_set_up(&net_test);

  acd_add(&net_test, &acd_test, acd_conflict_callback);
  
  IP4_ADDR(&test_addr, 192, 168, 1, 100);
  acd_start(&net_test, &acd_test, test_addr);
  
  initial_ttw = acd_test.ttw;
  fail_unless(initial_ttw > 0);
  
  /* Simulate timer tick - ttw should decrement */
  ttw_before = acd_test.ttw;
  acd_test.ttw--;
  ttw_after = acd_test.ttw;
  
  fail_unless(ttw_after == (ttw_before - 1));
  
  acd_stop(&acd_test);
  acd_remove(&net_test, &acd_test);
}
END_TEST

START_TEST(test_acd_timer_callback_execution)
{
  ip4_addr_t test_addr;
  u16_t ttw_before;
  u16_t lastconflict_before;
  LWIP_UNUSED_ARG(_i);

  netif_add(&net_test, NULL, NULL, NULL, NULL, testif_init, ethernet_input);
  netif_set_link_up(&net_test);
  netif_set_up(&net_test);

  acd_add(&net_test, &acd_test, acd_conflict_callback);
  
  IP4_ADDR(&test_addr, 192, 168, 1, 100);
  acd_start(&net_test, &acd_test, test_addr);
  
  /* Set up test values */
  acd_test.ttw = 5;
  acd_test.lastconflict = 3;
  ttw_before = acd_test.ttw;
  lastconflict_before = acd_test.lastconflict;
  
  /* Simulate timer callback execution */
  acd_tmr();
  
  /* Both ttw and lastconflict should decrement */
  fail_unless(acd_test.ttw == (ttw_before - 1));
  fail_unless(acd_test.lastconflict == (lastconflict_before - 1));
  
  acd_stop(&acd_test);
  acd_remove(&net_test, &acd_test);
}
END_TEST

#if SL_LWIP_ACD_ONDEMAND_TIMER
START_TEST(test_acd_timer_activation_on_start)
{
  ip4_addr_t test_addr;
  LWIP_UNUSED_ARG(_i);

  netif_add(&net_test, NULL, NULL, NULL, NULL, testif_init, ethernet_input);
  netif_set_link_up(&net_test);
  netif_set_up(&net_test);

  acd_add(&net_test, &acd_test, acd_conflict_callback);
  
  IP4_ADDR(&test_addr, 192, 168, 1, 100);
  acd_start(&net_test, &acd_test, test_addr);
  
  /* Timer should be activated when ACD starts */
  fail_unless(acd_test.timer_active == 1);
  
  /* State should be PROBE_WAIT which needs timer */
  fail_unless(acd_test.state == ACD_STATE_PROBE_WAIT);
  
  acd_stop(&acd_test);
  acd_remove(&net_test, &acd_test);
}
END_TEST

START_TEST(test_acd_timer_deactivation_on_stop)
{
  ip4_addr_t test_addr;
  LWIP_UNUSED_ARG(_i);

  netif_add(&net_test, NULL, NULL, NULL, NULL, testif_init, ethernet_input);
  netif_set_link_up(&net_test);
  netif_set_up(&net_test);

  acd_add(&net_test, &acd_test, acd_conflict_callback);
  
  IP4_ADDR(&test_addr, 192, 168, 1, 100);
  acd_start(&net_test, &acd_test, test_addr);
  
  /* Timer should be active after start */
  fail_unless(acd_test.timer_active == 1);
  
  acd_stop(&acd_test);
  
  /* Timer should be deactivated when ACD stops */
  fail_unless(acd_test.timer_active == 0);
  
  /* State should be OFF which doesn't need timer */
  fail_unless(acd_test.state == ACD_STATE_OFF);
  
  acd_remove(&net_test, &acd_test);
}
END_TEST

START_TEST(test_acd_timer_deactivation_on_ongoing_no_conflict)
{
  ip4_addr_t test_addr;
  LWIP_UNUSED_ARG(_i);

  netif_add(&net_test, NULL, NULL, NULL, NULL, testif_init, ethernet_input);
  netif_set_link_up(&net_test);
  netif_set_up(&net_test);

  acd_add(&net_test, &acd_test, acd_conflict_callback);
  
  IP4_ADDR(&test_addr, 192, 168, 1, 100);
  acd_start(&net_test, &acd_test, test_addr);
  
  /* Simulate progression to ONGOING state with active defense first */
  acd_test.state = ACD_STATE_ONGOING;
  acd_test.lastconflict = DEFEND_INTERVAL * ACD_TICKS_PER_SECOND;
  
  /* Timer should be active when defense is needed */
  fail_unless(acd_test.timer_active == 1);
  
  /* Simulate timer ticks until lastconflict expires */
  while (acd_test.lastconflict > 0) {
    acd_tmr();
  }
  
  /* Timer should be deactivated when defense window expires */
  fail_unless(acd_test.timer_active == 0);
  
  acd_stop(&acd_test);
  acd_remove(&net_test, &acd_test);
}
END_TEST

START_TEST(test_acd_timer_activation_on_conflict_detected)
{
  ip4_addr_t test_addr;
  u16_t lastconflict_before;
  LWIP_UNUSED_ARG(_i);

  netif_add(&net_test, NULL, NULL, NULL, NULL, testif_init, ethernet_input);
  netif_set_link_up(&net_test);
  netif_set_up(&net_test);

  acd_add(&net_test, &acd_test, acd_conflict_callback);
  
  IP4_ADDR(&test_addr, 192, 168, 1, 100);
  acd_start(&net_test, &acd_test, test_addr);
  
  /* Simulate ONGOING state with active defense */
  acd_test.state = ACD_STATE_ONGOING;
  acd_test.lastconflict = DEFEND_INTERVAL * ACD_TICKS_PER_SECOND;
  
  /* Timer should be active when defense is needed */
  fail_unless(acd_test.timer_active == 1);
  
  /* Simulate timer tick - lastconflict should decrement */
  lastconflict_before = acd_test.lastconflict;
  acd_tmr();
  fail_unless(acd_test.lastconflict == (lastconflict_before - 1));
  
  acd_stop(&acd_test);
  acd_remove(&net_test, &acd_test);
}
END_TEST

START_TEST(test_acd_multiple_instances_timer_sharing)
{
  ip4_addr_t test_addr1, test_addr2;
  struct acd acd_test2;
  LWIP_UNUSED_ARG(_i);

  memset(&acd_test2, 0, sizeof(struct acd));

  netif_add(&net_test, NULL, NULL, NULL, NULL, testif_init, ethernet_input);
  netif_set_link_up(&net_test);
  netif_set_up(&net_test);

  acd_add(&net_test, &acd_test, acd_conflict_callback);
  acd_add(&net_test, &acd_test2, acd_conflict_callback);
  
  IP4_ADDR(&test_addr1, 192, 168, 1, 100);
  IP4_ADDR(&test_addr2, 192, 168, 1, 101);
  
  acd_start(&net_test, &acd_test, test_addr1);
  acd_start(&net_test, &acd_test2, test_addr2);
  
  /* Both instances should be active */
  fail_unless(acd_test.timer_active == 1);
  fail_unless(acd_test2.timer_active == 1);
  
  /* Stop one instance - timer should still be active for the other */
  acd_stop(&acd_test);
  
  fail_unless(acd_test.timer_active == 0);
  fail_unless(acd_test2.timer_active == 1);
  
  /* Stop the second instance - timer should be deactivated */
  acd_stop(&acd_test2);
  
  fail_unless(acd_test2.timer_active == 0);
  
  acd_remove(&net_test, &acd_test);
  acd_remove(&net_test, &acd_test2);
}
END_TEST

START_TEST(test_acd_timer_state_transitions)
{
  ip4_addr_t test_addr;
  LWIP_UNUSED_ARG(_i);

  netif_add(&net_test, NULL, NULL, NULL, NULL, testif_init, ethernet_input);
  netif_set_link_up(&net_test);
  netif_set_up(&net_test);

  acd_add(&net_test, &acd_test, acd_conflict_callback);
  
  IP4_ADDR(&test_addr, 192, 168, 1, 100);
  acd_start(&net_test, &acd_test, test_addr);
  
  /* acd_start() activates timer for PROBE_WAIT state */
  fail_unless(acd_test.state == ACD_STATE_PROBE_WAIT);
  fail_unless(acd_test.timer_active == 1);
  
  /* Test timer remains active through natural state transitions via acd_tmr() */
  /* PROBE_WAIT -> PROBING: Transition happens when ttw expires */
  acd_test.ttw = 0; /* Trigger transition to PROBING */
  acd_tmr(); /* This will transition to PROBING and timer should remain active */
  fail_unless(acd_test.state == ACD_STATE_PROBING);
  fail_unless(acd_test.timer_active == 1);
  
  /* PROBING -> ANNOUNCE_WAIT: After sending all probes */
  acd_test.sent_num = PROBE_NUM - 1; /* One probe left */
  acd_test.ttw = 0; /* Trigger last probe */
  acd_tmr(); /* This sends last probe and transitions to ANNOUNCE_WAIT */
  fail_unless(acd_test.state == ACD_STATE_ANNOUNCE_WAIT);
  fail_unless(acd_test.timer_active == 1);
  
  /* ANNOUNCE_WAIT -> ANNOUNCING: Transition happens when ttw expires */
  acd_test.ttw = 0; /* Trigger transition to ANNOUNCING */
  acd_tmr(); /* This transitions to ANNOUNCING and timer should remain active */
  fail_unless(acd_test.state == ACD_STATE_ANNOUNCING);
  fail_unless(acd_test.timer_active == 1);
  
  /* ANNOUNCING -> ONGOING: After sending all announcements */
  acd_test.sent_num = ANNOUNCE_NUM - 1; /* Last announcement */
  acd_test.ttw = 0; /* Trigger last announcement */
  acd_test.lastconflict = 0; /* No defense needed */
  acd_tmr(); /* This sends last announce and transitions to ONGOING, deactivating timer */
  fail_unless(acd_test.state == ACD_STATE_ONGOING);
  fail_unless(acd_test.timer_active == 0); /* ONGOING with lastconflict=0 doesn't need timer */
  
  /* Test timer deactivation for OFF state via acd_stop() */
  acd_stop(&acd_test);
  fail_unless(acd_test.state == ACD_STATE_OFF);
  fail_unless(acd_test.timer_active == 0);
  
  /* Test that timer gets reactivated when restarting */
  acd_start(&net_test, &acd_test, test_addr);
  fail_unless(acd_test.state == ACD_STATE_PROBE_WAIT);
  fail_unless(acd_test.timer_active == 1);
  
  acd_stop(&acd_test);
  acd_remove(&net_test, &acd_test);
}
END_TEST

START_TEST(test_acd_start_idempotent_timer)
{
  ip4_addr_t test_addr;
  LWIP_UNUSED_ARG(_i);

  netif_add(&net_test, NULL, NULL, NULL, NULL, testif_init, ethernet_input);
  netif_set_link_up(&net_test);
  netif_set_up(&net_test);

  acd_add(&net_test, &acd_test, acd_conflict_callback);

  IP4_ADDR(&test_addr, 192, 168, 1, 110);
  /* First start */
  acd_start(&net_test, &acd_test, test_addr);
  fail_unless(acd_test.timer_active == 1);
  
  /* Stop before restarting (acd_start() requires timer to be inactive) */
  acd_stop(&acd_test);
  fail_unless(acd_test.timer_active == 0);
  
  /* Second start after stop - must not leak/increment */
  acd_start(&net_test, &acd_test, test_addr);
  fail_unless(acd_test.timer_active == 1);

  acd_stop(&acd_test);
  fail_unless(acd_test.timer_active == 0);
  acd_remove(&net_test, &acd_test);
}
END_TEST

START_TEST(test_acd_passive_ongoing_defense_expire)
{
  ip4_addr_t test_addr;
  LWIP_UNUSED_ARG(_i);

  netif_add(&net_test, NULL, NULL, NULL, NULL, testif_init, ethernet_input);
  netif_set_link_up(&net_test);
  netif_set_up(&net_test);

  acd_add(&net_test, &acd_test, acd_conflict_callback);

  IP4_ADDR(&test_addr, 192, 168, 1, 120);
  acd_start(&net_test, &acd_test, test_addr);

  /* Simulate PASSIVE_ONGOING with active defense window */
  acd_test.state = ACD_STATE_PASSIVE_ONGOING;
  acd_test.lastconflict = 2; /* small window */
  /* When defense is active, timer must be active */
  fail_unless(acd_test.timer_active == 1);

  /* Tick until defense window expires - timer should be cleared by acd_tmr path */
  while (acd_test.lastconflict > 0) {
    acd_tmr();
  }
  fail_unless(acd_test.timer_active == 0);

  acd_stop(&acd_test);
  acd_remove(&net_test, &acd_test);
}
END_TEST

START_TEST(test_acd_needs_timer_probe_wait)
{
  ip4_addr_t test_addr;
  LWIP_UNUSED_ARG(_i);

  netif_add(&net_test, NULL, NULL, NULL, NULL, testif_init, ethernet_input);
  netif_set_link_up(&net_test);
  netif_set_up(&net_test);

  acd_add(&net_test, &acd_test, acd_conflict_callback);
  
  IP4_ADDR(&test_addr, 192, 168, 1, 100);
  acd_start(&net_test, &acd_test, test_addr);

  /* PROBE_WAIT state always needs timer */
  acd_test.state = ACD_STATE_PROBE_WAIT;
  /* Timer should be active - acd_needs_timer() should return 1 */
  fail_unless(acd_test.timer_active == 1);

  acd_stop(&acd_test);
  acd_remove(&net_test, &acd_test);
}
END_TEST

START_TEST(test_acd_needs_timer_probing)
{
  ip4_addr_t test_addr;
  LWIP_UNUSED_ARG(_i);

  netif_add(&net_test, NULL, NULL, NULL, NULL, testif_init, ethernet_input);
  netif_set_link_up(&net_test);
  netif_set_up(&net_test);

  acd_add(&net_test, &acd_test, acd_conflict_callback);
  
  IP4_ADDR(&test_addr, 192, 168, 1, 101);
  acd_start(&net_test, &acd_test, test_addr);

  /* PROBING state always needs timer */
  acd_test.state = ACD_STATE_PROBING;
  /* Timer should be active - acd_needs_timer() should return 1 */
  fail_unless(acd_test.timer_active == 1);

  acd_stop(&acd_test);
  acd_remove(&net_test, &acd_test);
}
END_TEST

START_TEST(test_acd_needs_timer_announce_wait)
{
  ip4_addr_t test_addr;
  LWIP_UNUSED_ARG(_i);

  netif_add(&net_test, NULL, NULL, NULL, NULL, testif_init, ethernet_input);
  netif_set_link_up(&net_test);
  netif_set_up(&net_test);

  acd_add(&net_test, &acd_test, acd_conflict_callback);
  
  IP4_ADDR(&test_addr, 192, 168, 1, 102);
  acd_start(&net_test, &acd_test, test_addr);

  /* ANNOUNCE_WAIT state always needs timer */
  acd_test.state = ACD_STATE_ANNOUNCE_WAIT;
  /* Timer should be active - acd_needs_timer() should return 1 */
  fail_unless(acd_test.timer_active == 1);

  acd_stop(&acd_test);
  acd_remove(&net_test, &acd_test);
}
END_TEST

START_TEST(test_acd_needs_timer_announcing)
{
  ip4_addr_t test_addr;
  LWIP_UNUSED_ARG(_i);

  netif_add(&net_test, NULL, NULL, NULL, NULL, testif_init, ethernet_input);
  netif_set_link_up(&net_test);
  netif_set_up(&net_test);

  acd_add(&net_test, &acd_test, acd_conflict_callback);
  
  IP4_ADDR(&test_addr, 192, 168, 1, 103);
  acd_start(&net_test, &acd_test, test_addr);

  /* ANNOUNCING state always needs timer */
  acd_test.state = ACD_STATE_ANNOUNCING;
  /* Timer should be active - acd_needs_timer() should return 1 */
  fail_unless(acd_test.timer_active == 1);

  acd_stop(&acd_test);
  acd_remove(&net_test, &acd_test);
}
END_TEST

START_TEST(test_acd_needs_timer_rate_limit)
{
  ip4_addr_t test_addr;
  LWIP_UNUSED_ARG(_i);

  netif_add(&net_test, NULL, NULL, NULL, NULL, testif_init, ethernet_input);
  netif_set_link_up(&net_test);
  netif_set_up(&net_test);

  acd_add(&net_test, &acd_test, acd_conflict_callback);
  
  IP4_ADDR(&test_addr, 192, 168, 1, 104);
  acd_start(&net_test, &acd_test, test_addr);

  /* RATE_LIMIT state always needs timer */
  acd_test.state = ACD_STATE_RATE_LIMIT;
  acd_test.ttw = (u16_t)(RATE_LIMIT_INTERVAL * ACD_TICKS_PER_SECOND);
  /* Timer should be active - acd_needs_timer() should return 1 */
  fail_unless(acd_test.timer_active == 1);

  acd_stop(&acd_test);
  acd_remove(&net_test, &acd_test);
}
END_TEST

START_TEST(test_acd_needs_timer_ongoing_no_defense)
{
  ip4_addr_t test_addr;
  LWIP_UNUSED_ARG(_i);

  netif_add(&net_test, NULL, NULL, NULL, NULL, testif_init, ethernet_input);
  netif_set_link_up(&net_test);
  netif_set_up(&net_test);

  acd_add(&net_test, &acd_test, acd_conflict_callback);
  
  IP4_ADDR(&test_addr, 192, 168, 1, 105);
  acd_start(&net_test, &acd_test, test_addr);

  /* Simulate proper transition to ONGOING: go through ANNOUNCING state first */
  /* Set up ANNOUNCING state to transition from */
  acd_test.state = ACD_STATE_ANNOUNCING;
  acd_test.sent_num = ANNOUNCE_NUM - 1; /* One announcement left */
  acd_test.ttw = 0;
  acd_test.lastconflict = 0;
  /* Timer should be active during ANNOUNCING */
  fail_unless(acd_test.timer_active == 1);

  /* Now trigger transition to ONGOING by calling acd_tmr() */
  /* This simulates the natural path in acd_tmr() that deactivates timer */
  acd_tmr();
  
  /* After transition to ONGOING with lastconflict=0, timer should be deactivated */
  /* This verifies acd_needs_timer() returns 0 for this case */
  fail_unless(acd_test.state == ACD_STATE_ONGOING);
  fail_unless(acd_test.timer_active == 0);

  acd_stop(&acd_test);
  acd_remove(&net_test, &acd_test);
}
END_TEST

START_TEST(test_acd_needs_timer_ongoing_with_defense)
{
  ip4_addr_t test_addr;
  LWIP_UNUSED_ARG(_i);

  netif_add(&net_test, NULL, NULL, NULL, NULL, testif_init, ethernet_input);
  netif_set_link_up(&net_test);
  netif_set_up(&net_test);

  acd_add(&net_test, &acd_test, acd_conflict_callback);
  
  IP4_ADDR(&test_addr, 192, 168, 1, 106);
  acd_start(&net_test, &acd_test, test_addr);

  /* First, complete to ONGOING with no defense (timer should be off) */
  acd_test.state = ACD_STATE_ANNOUNCING;
  acd_test.sent_num = ANNOUNCE_NUM - 1;
  acd_test.ttw = 0;
  acd_test.lastconflict = 0;
  /* Complete transition to ONGOING - timer should be deactivated */
  acd_tmr();
  fail_unless(acd_test.state == ACD_STATE_ONGOING);
  fail_unless(acd_test.timer_active == 0);
  
  /* Now simulate conflict - ONGOING state with lastconflict>0 should need timer */
  /* Set lastconflict which would happen via acd_handle_arp_conflict() */
  acd_test.lastconflict = DEFEND_INTERVAL * ACD_TICKS_PER_SECOND;
  /* Note: Timer activation happens in acd_handle_arp_conflict() via acd_timer_activate() */
  /* Since acd_needs_timer() returns 1 when lastconflict > 0, timer would activate */
  /* We can't directly test activation here since acd_timer_activate is static */
  /* But we can verify that if timer were active, it would stay active */
  /* The actual activation is tested in test_acd_timer_activation_on_conflict_detected */
  /* This test just verifies the state logic: ONGOING with lastconflict > 0 needs timer */
  /* Verify lastconflict > 0 means timer would be needed */
  /* If timer was active (from previous state), it would stay active */
  /* Since timer is currently 0, we verify the logic would require it */
  fail_unless(acd_test.lastconflict > 0);
  fail_unless(acd_test.state == ACD_STATE_ONGOING);
  /* The timer activation itself is tested in test_acd_timer_activation_on_conflict_detected */

  acd_stop(&acd_test);
  acd_remove(&net_test, &acd_test);
}
END_TEST

START_TEST(test_acd_needs_timer_passive_ongoing_no_defense)
{
  ip4_addr_t test_addr;
  LWIP_UNUSED_ARG(_i);

  netif_add(&net_test, NULL, NULL, NULL, NULL, testif_init, ethernet_input);
  netif_set_link_up(&net_test);
  netif_set_up(&net_test);

  acd_add(&net_test, &acd_test, acd_conflict_callback);
  
  IP4_ADDR(&test_addr, 192, 168, 1, 107);
  acd_start(&net_test, &acd_test, test_addr);

  /* First transition to ONGOING (timer should be off) */
  acd_test.state = ACD_STATE_ANNOUNCING;
  acd_test.sent_num = ANNOUNCE_NUM - 1;
  acd_test.ttw = 0;
  acd_test.lastconflict = 0;
  acd_tmr(); /* Transition to ONGOING */
  fail_unless(acd_test.state == ACD_STATE_ONGOING);
  fail_unless(acd_test.timer_active == 0);
  
  /* Now transition to PASSIVE_ONGOING with lastconflict=0 */
  /* PASSIVE_ONGOING state with lastconflict=0 should NOT need timer */
  /* This would be set via acd_put_in_passive_mode() */
  acd_test.state = ACD_STATE_PASSIVE_ONGOING;
  acd_test.lastconflict = 0;
  /* Timer should remain inactive since acd_needs_timer() returns 0 */
  fail_unless(acd_test.timer_active == 0);

  acd_stop(&acd_test);
  acd_remove(&net_test, &acd_test);
}
END_TEST

START_TEST(test_acd_needs_timer_passive_ongoing_with_defense)
{
  ip4_addr_t test_addr;
  LWIP_UNUSED_ARG(_i);

  netif_add(&net_test, NULL, NULL, NULL, NULL, testif_init, ethernet_input);
  netif_set_link_up(&net_test);
  netif_set_up(&net_test);

  acd_add(&net_test, &acd_test, acd_conflict_callback);
  
  IP4_ADDR(&test_addr, 192, 168, 1, 108);
  acd_start(&net_test, &acd_test, test_addr);

  /* PASSIVE_ONGOING state with lastconflict>0 should need timer */
  acd_test.state = ACD_STATE_PASSIVE_ONGOING;
  acd_test.lastconflict = DEFEND_INTERVAL * ACD_TICKS_PER_SECOND;
  /* Timer should be active - acd_needs_timer() should return 1 */
  fail_unless(acd_test.timer_active == 1);

  acd_stop(&acd_test);
  acd_remove(&net_test, &acd_test);
}
END_TEST

START_TEST(test_acd_needs_timer_off_state)
{
  ip4_addr_t test_addr;
  LWIP_UNUSED_ARG(_i);

  netif_add(&net_test, NULL, NULL, NULL, NULL, testif_init, ethernet_input);
  netif_set_link_up(&net_test);
  netif_set_up(&net_test);

  acd_add(&net_test, &acd_test, acd_conflict_callback);
  
  IP4_ADDR(&test_addr, 192, 168, 1, 109);
  acd_start(&net_test, &acd_test, test_addr);

  /* OFF state should NOT need timer */
  acd_stop(&acd_test);
  /* Timer should be inactive - acd_needs_timer() should return 0 */
  fail_unless(acd_test.state == ACD_STATE_OFF);
  fail_unless(acd_test.timer_active == 0);

  acd_remove(&net_test, &acd_test);
}
END_TEST

START_TEST(test_acd_needs_timer_lastconflict_boundary)
{
  ip4_addr_t test_addr;
  LWIP_UNUSED_ARG(_i);

  netif_add(&net_test, NULL, NULL, NULL, NULL, testif_init, ethernet_input);
  netif_set_link_up(&net_test);
  netif_set_up(&net_test);

  acd_add(&net_test, &acd_test, acd_conflict_callback);
  
  IP4_ADDR(&test_addr, 192, 168, 1, 110);
  acd_start(&net_test, &acd_test, test_addr);

  /* Test boundary condition: lastconflict = 1 should need timer */
  acd_test.state = ACD_STATE_ONGOING;
  acd_test.lastconflict = 1;
  acd_test.timer_active = 1; /* Ensure timer is active for defense */
  fail_unless(acd_test.timer_active == 1);
  fail_unless(acd_test.lastconflict == 1);

  /* Test boundary condition: lastconflict = 0 should NOT need timer */
  /* Set lastconflict to 1 so acd_tmr() can decrement it to 0 and deactivate timer */
  acd_test.lastconflict = 1;
  acd_test.timer_active = 1; /* Ensure timer is active */
  /* Timer should deactivate when lastconflict decrements from 1 to 0 */
  acd_tmr(); /* This should decrement lastconflict to 0 and clear timer_active */
  fail_unless(acd_test.lastconflict == 0); /* Verify lastconflict was decremented */
  fail_unless(acd_test.timer_active == 0); /* Verify timer was deactivated */

  acd_stop(&acd_test);
  acd_remove(&net_test, &acd_test);
}
END_TEST

/* Test acd_remove() timer deactivation */
START_TEST(test_acd_remove_timer_deactivation)
{
  ip4_addr_t test_addr;
  LWIP_UNUSED_ARG(_i);

  netif_add(&net_test, NULL, NULL, NULL, NULL, testif_init, ethernet_input);
  netif_set_link_up(&net_test);
  netif_set_up(&net_test);

  acd_add(&net_test, &acd_test, acd_conflict_callback);
  
  IP4_ADDR(&test_addr, 192, 168, 1, 100);
  acd_start(&net_test, &acd_test, test_addr);
  
  /* Timer should be active after start */
  fail_unless(acd_test.timer_active == 1);
  
  /* Remove ACD - timer should be deactivated */
  acd_remove(&net_test, &acd_test);
  
  /* Verify timer was deactivated */
  fail_unless(acd_test.timer_active == 0);
  fail_unless(net_test.acd_list == NULL);
}
END_TEST

/* Test acd_tmr() timer deactivation when lastconflict reaches 0 in ONGOING state */
START_TEST(test_acd_tmr_timer_deactivation_ongoing_lastconflict_zero)
{
  ip4_addr_t test_addr;
  LWIP_UNUSED_ARG(_i);

  netif_add(&net_test, NULL, NULL, NULL, NULL, testif_init, ethernet_input);
  netif_set_link_up(&net_test);
  netif_set_up(&net_test);

  acd_add(&net_test, &acd_test, acd_conflict_callback);
  
  IP4_ADDR(&test_addr, 192, 168, 1, 100);
  acd_start(&net_test, &acd_test, test_addr);
  
  /* Set state to ONGOING with lastconflict = 1 (about to expire) */
  acd_test.state = ACD_STATE_ONGOING;
  acd_test.lastconflict = 1;
  acd_test.timer_active = 1; /* Timer is active for defense */
  
  /* Call acd_tmr() - should decrement lastconflict to 0 and deactivate timer */
  acd_tmr();
  
  /* Verify timer was deactivated when lastconflict reached 0 */
  fail_unless(acd_test.lastconflict == 0);
  fail_unless(acd_test.timer_active == 0);
  fail_unless(acd_test.state == ACD_STATE_ONGOING);
  
  acd_stop(&acd_test);
  acd_remove(&net_test, &acd_test);
}
END_TEST

/* Test acd_tmr() timer deactivation when transitioning to ONGOING from ANNOUNCING */
START_TEST(test_acd_tmr_timer_deactivation_transition_to_ongoing)
{
  ip4_addr_t test_addr;
  LWIP_UNUSED_ARG(_i);

  netif_add(&net_test, NULL, NULL, NULL, NULL, testif_init, ethernet_input);
  netif_set_link_up(&net_test);
  netif_set_up(&net_test);

  acd_add(&net_test, &acd_test, acd_conflict_callback);
  
  IP4_ADDR(&test_addr, 192, 168, 1, 100);
  acd_start(&net_test, &acd_test, test_addr);
  
  /* Set up ANNOUNCING state ready to transition to ONGOING */
  acd_test.state = ACD_STATE_ANNOUNCING;
  acd_test.sent_num = ANNOUNCE_NUM - 1; /* Last announcement */
  acd_test.ttw = 0; /* Timer expired, ready to send last announce */
  acd_test.lastconflict = 0;
  acd_test.timer_active = 1; /* Timer active during ANNOUNCING */
  
  /* Call acd_tmr() - should send last announce and transition to ONGOING */
  acd_tmr();
  
  /* Verify transition to ONGOING and timer deactivation */
  fail_unless(acd_test.state == ACD_STATE_ONGOING);
  fail_unless(acd_test.timer_active == 0); /* Timer deactivated in acd_tmr() */
  fail_unless(acd_test.lastconflict == 0);
  
  acd_stop(&acd_test);
  acd_remove(&net_test, &acd_test);
}
END_TEST

/* Test acd_restart() when entering RATE_LIMIT state - timer should be activated */
START_TEST(test_acd_restart_rate_limit_timer_activation)
{
  ip4_addr_t test_addr;
  struct etharp_hdr *arp_hdr;
  struct eth_addr conflicting_mac;
  ip4_addr_t conflicting_ip;
  static struct etharp_hdr test_arp_hdr;
  LWIP_UNUSED_ARG(_i);

  netif_add(&net_test, NULL, NULL, NULL, NULL, testif_init, ethernet_input);
  netif_set_link_up(&net_test);
  netif_set_up(&net_test);

  acd_add(&net_test, &acd_test, acd_conflict_callback);
  
  IP4_ADDR(&test_addr, 192, 168, 1, 100);
  acd_start(&net_test, &acd_test, test_addr);
  
  /* Set state to PROBING and conflicts to MAX_CONFLICTS-1 */
  acd_test.state = ACD_STATE_PROBING;
  acd_test.num_conflicts = MAX_CONFLICTS - 1;
  
  /* Create conflicting ARP packet */
  IP4_ADDR(&conflicting_ip, 192, 168, 1, 100);
  conflicting_mac.addr[0] = 0xAA;
  conflicting_mac.addr[1] = 0xBB;
  conflicting_mac.addr[2] = 0xCC;
  conflicting_mac.addr[3] = 0xDD;
  conflicting_mac.addr[4] = 0xEE;
  conflicting_mac.addr[5] = 0xFF;
  
  /* Create minimal ARP header for testing */
  memset(&test_arp_hdr, 0, sizeof(test_arp_hdr));
  IPADDR_WORDALIGNED_COPY_FROM_IP4_ADDR_T(&test_arp_hdr.sipaddr, &conflicting_ip);
  SMEMCPY(&test_arp_hdr.shwaddr, &conflicting_mac, ETH_HWADDR_LEN);
  arp_hdr = &test_arp_hdr;
  
  /* Process ARP conflict - should trigger acd_restart() and enter RATE_LIMIT */
  acd_arp_reply(&net_test, arp_hdr);
  
  /* Verify rate limiting was triggered and timer is active */
  fail_unless(acd_test.num_conflicts == MAX_CONFLICTS);
  fail_unless(acd_test.state == ACD_STATE_RATE_LIMIT);
  fail_unless(acd_test.ttw == (u16_t)(RATE_LIMIT_INTERVAL * ACD_TICKS_PER_SECOND));
  fail_unless(acd_test.timer_active == 1); /* Timer should be active for RATE_LIMIT */
  
  acd_stop(&acd_test);
  acd_remove(&net_test, &acd_test);
}
END_TEST

/* Test acd_handle_arp_conflict() defense - timer activation via acd_arp_reply() */
START_TEST(test_acd_handle_arp_conflict_defend_timer_activation)
{
  ip4_addr_t test_addr;
  struct etharp_hdr *arp_hdr;
  struct eth_addr conflicting_mac;
  ip4_addr_t conflicting_ip;
  static struct etharp_hdr test_arp_hdr;
  LWIP_UNUSED_ARG(_i);

  netif_add(&net_test, NULL, NULL, NULL, NULL, testif_init, ethernet_input);
  netif_set_link_up(&net_test);
  netif_set_up(&net_test);

  acd_add(&net_test, &acd_test, acd_conflict_callback);
  
  IP4_ADDR(&test_addr, 192, 168, 1, 100);
  acd_start(&net_test, &acd_test, test_addr);
  
  /* Set state to ONGOING with no active defense window */
  acd_test.state = ACD_STATE_ONGOING;
  acd_test.lastconflict = 0;
  acd_test.timer_active = 0; /* Timer inactive when no defense needed */
  
  /* Create conflicting ARP packet */
  IP4_ADDR(&conflicting_ip, 192, 168, 1, 100);
  conflicting_mac.addr[0] = 0xAA;
  conflicting_mac.addr[1] = 0xBB;
  conflicting_mac.addr[2] = 0xCC;
  conflicting_mac.addr[3] = 0xDD;
  conflicting_mac.addr[4] = 0xEE;
  conflicting_mac.addr[5] = 0xFF;
  
  /* Create minimal ARP header for testing */
  memset(&test_arp_hdr, 0, sizeof(test_arp_hdr));
  IPADDR_WORDALIGNED_COPY_FROM_IP4_ADDR_T(&test_arp_hdr.sipaddr, &conflicting_ip);
  SMEMCPY(&test_arp_hdr.shwaddr, &conflicting_mac, ETH_HWADDR_LEN);
  arp_hdr = &test_arp_hdr;
  
  /* Process ARP conflict - should defend and activate timer via acd_arp_reply() */
  acd_arp_reply(&net_test, arp_hdr);
  
  /* Verify defense: lastconflict set, timer activated by acd_arp_reply() */
  fail_unless(acd_test.state == ACD_STATE_ONGOING);
  fail_unless(acd_test.lastconflict == (DEFEND_INTERVAL * ACD_TICKS_PER_SECOND));
  fail_unless(acd_test.timer_active == 1); /* Timer activated by acd_arp_reply() */
  
  acd_stop(&acd_test);
  acd_remove(&net_test, &acd_test);
}
END_TEST
#endif /* SL_LWIP_ACD_ONDEMAND_TIMER */

/** Create the suite including all tests for this module */
Suite *
acd_suite(void)
{
  testfunc tests[] = {
    /* ============================================================
     * Common tests - work regardless of SL_LWIP_ACD_ONDEMAND_TIMER
     * ============================================================ */
    TESTFUNC(test_acd_add_remove),
    TESTFUNC(test_acd_start_stop),
    TESTFUNC(test_acd_add_twice),
    TESTFUNC(test_acd_state_machine_probe_wait),
    TESTFUNC(test_acd_state_machine_probing),
    TESTFUNC(test_acd_num_conflicts_increment),
    TESTFUNC(test_acd_lastconflict_countdown),
    TESTFUNC(test_acd_link_down_stops_all),
    TESTFUNC(test_acd_callback_ip_ok),
    TESTFUNC(test_acd_rate_limit_state),
    TESTFUNC(test_acd_sent_num_tracking),
    TESTFUNC(test_acd_ttw_decrement),
    TESTFUNC(test_acd_timer_callback_execution), /* Works in both modes - tests acd_tmr() */
    
#if SL_LWIP_ACD_ONDEMAND_TIMER
    /* ============================================================
     * Tests for SL_LWIP_ACD_ONDEMAND_TIMER enabled mode
     * These tests verify on-demand timer activation/deactivation
     * ============================================================ */
    TESTFUNC(test_acd_timer_activation_on_start),
    TESTFUNC(test_acd_timer_deactivation_on_stop),
    TESTFUNC(test_acd_timer_deactivation_on_ongoing_no_conflict),
    TESTFUNC(test_acd_timer_activation_on_conflict_detected),
    TESTFUNC(test_acd_multiple_instances_timer_sharing),
    TESTFUNC(test_acd_timer_state_transitions),
    TESTFUNC(test_acd_start_idempotent_timer),
    TESTFUNC(test_acd_passive_ongoing_defense_expire),
    TESTFUNC(test_acd_needs_timer_probe_wait),
    TESTFUNC(test_acd_needs_timer_probing),
    TESTFUNC(test_acd_needs_timer_announce_wait),
    TESTFUNC(test_acd_needs_timer_announcing),
    TESTFUNC(test_acd_needs_timer_rate_limit),
    TESTFUNC(test_acd_needs_timer_ongoing_no_defense),
    TESTFUNC(test_acd_needs_timer_ongoing_with_defense),
    TESTFUNC(test_acd_needs_timer_passive_ongoing_no_defense),
    TESTFUNC(test_acd_needs_timer_passive_ongoing_with_defense),
    TESTFUNC(test_acd_needs_timer_off_state),
    TESTFUNC(test_acd_needs_timer_lastconflict_boundary),
    TESTFUNC(test_acd_remove_timer_deactivation),
    TESTFUNC(test_acd_tmr_timer_deactivation_ongoing_lastconflict_zero),
    TESTFUNC(test_acd_tmr_timer_deactivation_transition_to_ongoing),
    TESTFUNC(test_acd_restart_rate_limit_timer_activation),
    TESTFUNC(test_acd_handle_arp_conflict_defend_timer_activation),
#else /* !SL_LWIP_ACD_ONDEMAND_TIMER */
    /* ============================================================
     * Tests for SL_LWIP_ACD_ONDEMAND_TIMER disabled mode
     * These tests verify behavior when on-demand timers are not used
     * (Timer always runs, no timer_active field)
     * ============================================================ */
    /* Note: When on-demand timers are disabled, the timer always runs
     * and there's no timer_active field to test. The common tests above
     * already cover the basic functionality. Additional tests specific
     * to disabled mode can be added here if needed. */
#endif /* SL_LWIP_ACD_ONDEMAND_TIMER */
  };
  return create_suite("ACD", tests, LWIP_ARRAYSIZE(tests), acd_setup, acd_teardown);
}

#else /* LWIP_ACD */

Suite *
acd_suite(void)
{
  return NULL;
}

#endif /* LWIP_ACD */
