/***************************************************************************/
/**
 * @file sleeptimer_hal_test_main.c
 * @brief Sleeptimer HAL Unity Test Functions
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/
#include "sl_sleeptimer.h"
#include "rsi_debug.h"
#include "unity.h"

/*******************************************************************************
 ***************************  Defines / Macros  ********************************
 ******************************************************************************/
#define TOOGLE_DELAY_MS1_ONESHOT  160000 //sleeptimer1 oneshot timeout in ticks
#define TOOGLE_DELAY_MS1_PERIODIC 400    //sleeptimer1 periodic timeout in ms

#define TIME_UNIX_EPOCH                     (1970u)
#define TIME_NTP_EPOCH                      (1900u)
#define TIME_ZIGBEE_EPOCH                   (2000u)
#define TIME_64_EPOCH                       TIME_NTP_EPOCH
#define TIME_NTP_UNIX_EPOCH_DIFF            (TIME_UNIX_EPOCH - TIME_NTP_EPOCH)
#define TIME_ZIGBEE_UNIX_EPOCH_DIFF         (TIME_ZIGBEE_EPOCH - TIME_UNIX_EPOCH)
#define TIME_DAY_COUNT_NTP_TO_UNIX_EPOCH    (TIME_NTP_UNIX_EPOCH_DIFF * 365u + 17u)   ///< 70 years and 17 leap days
#define TIME_DAY_COUNT_ZIGBEE_TO_UNIX_EPOCH (TIME_ZIGBEE_UNIX_EPOCH_DIFF * 365u + 7u) ///< 30 years and 7 leap days
#define TIME_SEC_PER_DAY                    (60u * 60u * 24u)
#define TIME_NTP_EPOCH_OFFSET_SEC           (TIME_DAY_COUNT_NTP_TO_UNIX_EPOCH * TIME_SEC_PER_DAY)
#define TIME_ZIGBEE_EPOCH_OFFSET_SEC        (TIME_DAY_COUNT_ZIGBEE_TO_UNIX_EPOCH * TIME_SEC_PER_DAY)
#define TIME_DAY_PER_YEAR                   (365u)
#define TIME_SEC_PER_YEAR                   (TIME_SEC_PER_DAY * TIME_DAY_PER_YEAR)
#define TIME_UNIX_TIMESTAMP_MAX             (0x7FFFFFFF)
#define TIME_64_BIT_UNIX_TIMESTAMP_MAX      (0x497968BD7F) /// Max 64 bit timestamp supported is 11:59:59 PM 12/31/11899
#define TIME_UNIX_YEAR_MAX                  (2038u - TIME_NTP_EPOCH)  ///< Max UNIX year based from a 1900 epoch
#define TIME_64_BIT_YEAR_MAX                (11899u - TIME_NTP_EPOCH) ///< Max 64 bit format year based from a 1900 epoch
#define TIME_64_TO_32_EPOCH_OFFSET_SEC      TIME_NTP_EPOCH_OFFSET_SEC
#define TIME_UNIX_TO_NTP_MAX                (0xFFFFFFFF - TIME_NTP_EPOCH_OFFSET_SEC)
/*******************************************************************************
 ***************************  Global Variables   *******************************
 ******************************************************************************/

/*******************************************************************************
 ************************  Static Function Prototypes   ************************
 ******************************************************************************/
//Sleeptimer timeout callbacks
static void on_timeout_timer(sl_sleeptimer_timer_handle_t *handle,
                             void *data); //sleeptimer1 callback
/*******************************************************************************
 ***************************  Local Variables   *******************************
 ******************************************************************************/
sl_sleeptimer_timer_handle_t test_timer; //sleeptimer handle
static uint32_t max_ms    = 0;
bool test_oneshot_timeout = true;
/*******************************************************************************
 ************************  Test Function Prototypes  ****************************
 ******************************************************************************/
void test_sleeptimer_init(void);
void test_sleeptimer_get_tick_count(void);
void test_sleeptimer_get_tick_count64(void);
void test_sleeptimer_start_timer(void);
void test_sleeptimer_restart_timer(void);
void test_sleeptimer_is_timer_running(void);
void test_sleeptimer_start_periodic_timer(void);
void test_sleeptimer_restart_periodic_timer(void);
void test_sleeptimer_stop_timer(void);
void test_sleeptimer_get_timer_time_remaining(void);
void test_sleeptimer_get_remaining_time_of_first_timer(void);
void test_sleeptimer_get_timer_frequency(void);
void test_sleeptimer_get_clock_accuracy(void);
void test_sleeptimer_delay_millisecond(void);
void test_sleeptimer_ms_to_tick(void);
void test_sleeptimer_ms32_to_tick(void);
void test_sleeptimer_get_max_ms32_conversion(void);
void test_sleeptimer_tick_to_ms(void);
void test_sleeptimer_tick64_to_ms(void);
void test_sleeptimer_is_power_manager_early_restore_timer_latest_to_expire(void);
void test_sleeptimer_start_timer_ms(void);
void test_sleeptimer_restart_timer_ms(void);
void test_sleeptimer_start_periodic_timer_ms(void);
void test_sleeptimer_restart_periodic_timer_ms(void);

/******************************************************************************
 * Main function in which all the test cases are tested using unity framework
 ******************************************************************************/
int app_init()
{
  UnityBeginGroup("Sleeptimer");

  RUN_TEST(test_sleeptimer_init, __LINE__);
  RUN_TEST(test_sleeptimer_get_tick_count, __LINE__);
  RUN_TEST(test_sleeptimer_get_tick_count64, __LINE__);
  RUN_TEST(test_sleeptimer_start_timer, __LINE__);
  RUN_TEST(test_sleeptimer_restart_timer, __LINE__);
  RUN_TEST(test_sleeptimer_start_periodic_timer, __LINE__);
  RUN_TEST(test_sleeptimer_restart_periodic_timer, __LINE__);
  RUN_TEST(test_sleeptimer_get_timer_frequency, __LINE__);
  RUN_TEST(test_sleeptimer_get_clock_accuracy, __LINE__);
  RUN_TEST(test_sleeptimer_is_timer_running, __LINE__);
  RUN_TEST(test_sleeptimer_get_timer_time_remaining, __LINE__);
  RUN_TEST(test_sleeptimer_get_remaining_time_of_first_timer, __LINE__);
  RUN_TEST(test_sleeptimer_delay_millisecond, __LINE__);
  RUN_TEST(test_sleeptimer_get_max_ms32_conversion, __LINE__);
  RUN_TEST(test_sleeptimer_ms_to_tick, __LINE__);
  RUN_TEST(test_sleeptimer_ms32_to_tick, __LINE__);
  RUN_TEST(test_sleeptimer_tick_to_ms, __LINE__);
  RUN_TEST(test_sleeptimer_tick64_to_ms, __LINE__);
  RUN_TEST(test_sleeptimer_is_power_manager_early_restore_timer_latest_to_expire, __LINE__);
  RUN_TEST(test_sleeptimer_start_timer_ms, __LINE__);
  RUN_TEST(test_sleeptimer_restart_timer_ms, __LINE__);
  RUN_TEST(test_sleeptimer_start_periodic_timer_ms, __LINE__);
  RUN_TEST(test_sleeptimer_restart_periodic_timer_ms, __LINE__);
  RUN_TEST(test_sleeptimer_stop_timer, __LINE__);

  UnityEnd();
  UnityPrintf("END");
  while (1) {
  }
}

/*******************************************************************************
 * Function to test initialization of Sleeptimer
 ******************************************************************************/
void test_sleeptimer_init(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing Sleeptimer Init Timer \n");
  sl_status_t status;

  status = sl_sleeptimer_init();
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  UnityPrintf("Sleeptimer Init Timer successfully \n");

  UnityPrintf("Sleeptimer Init Timer test completed successfully \n");
}

/*******************************************************************************
 * Function to test getting the current timer count
 ******************************************************************************/
void test_sleeptimer_get_tick_count(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing Sleeptimer Get Counter \n");
  uint32_t count;

  UnityPrintf("Testing with correct parameters \n");
  count = sl_sleeptimer_get_tick_count();
  UnityPrintf("Counter = %ld \n", count);
  UnityPrintf("Sleeptimer Get Counter successfully \n");

  UnityPrintf("Sleeptimer Get Counter test completed \n");
}

/*******************************************************************************
 * Function to test getting the current timer count for 64 bit.
 ******************************************************************************/
void test_sleeptimer_get_tick_count64(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing Sleeptimer Get Counter for 64 bit \n");
  uint64_t count;

  UnityPrintf("Testing with correct parameters \n");
  count = sl_sleeptimer_get_tick_count64();
  UnityPrintf("Counter = %ld \n", count);
  UnityPrintf("Sleeptimer Get Counter for 64 bit successfully \n");

  UnityPrintf("Sleeptimer Get Counter for 64 bit test completed \n");
}

/*******************************************************************************
 * Function to test the Sleeptimer start.
 ******************************************************************************/
void test_sleeptimer_start_timer(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing Sleeptimer start \n");
  sl_status_t status;

  status = sl_sleeptimer_start_timer(NULL,
                                     TOOGLE_DELAY_MS1_ONESHOT,
                                     on_timeout_timer,
                                     NULL,
                                     0,
                                     SL_SLEEPTIMER_NO_HIGH_PRECISION_HF_CLOCKS_REQUIRED_FLAG);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_NULL_POINTER, status);
  UnityPrintf("Testing with null pointer successfully \n");

  UnityPrintf("Testing with correct parameters \n");
  status = sl_sleeptimer_start_timer(&test_timer,
                                     TOOGLE_DELAY_MS1_ONESHOT,
                                     on_timeout_timer,
                                     NULL,
                                     0,
                                     SL_SLEEPTIMER_NO_HIGH_PRECISION_HF_CLOCKS_REQUIRED_FLAG);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  UnityPrintf("Sleeptimer start successfully \n");

  UnityPrintf("Testing with correct parameters to check timer is running or not \n");
  status = sl_sleeptimer_start_timer(&test_timer,
                                     TOOGLE_DELAY_MS1_ONESHOT,
                                     on_timeout_timer,
                                     NULL,
                                     0,
                                     SL_SLEEPTIMER_NO_HIGH_PRECISION_HF_CLOCKS_REQUIRED_FLAG);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_NOT_READY, status);
  UnityPrintf("Sleeptimer start timer to check timer is running or not successfully \n");

  UnityPrintf("Sleeptimer start test completed \n");
}

/*******************************************************************************
 * Function to test restart the Sleeptimer.
 ******************************************************************************/
void test_sleeptimer_restart_timer(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing Sleeptimer restart \n");
  sl_status_t status;

  status = sl_sleeptimer_restart_timer(NULL,
                                       TOOGLE_DELAY_MS1_ONESHOT,
                                       on_timeout_timer,
                                       NULL,
                                       0,
                                       SL_SLEEPTIMER_NO_HIGH_PRECISION_HF_CLOCKS_REQUIRED_FLAG);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_NULL_POINTER, status);
  UnityPrintf("Testing with null pointer successfully \n");

  UnityPrintf("Testing with correct parameters \n");
  status = sl_sleeptimer_restart_timer(&test_timer,
                                       TOOGLE_DELAY_MS1_ONESHOT,
                                       on_timeout_timer,
                                       NULL,
                                       0,
                                       SL_SLEEPTIMER_NO_HIGH_PRECISION_HF_CLOCKS_REQUIRED_FLAG);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  UnityPrintf("Sleeptimer restart successfully \n");

  UnityPrintf("Sleeptimer restart test completed \n");
}

/*******************************************************************************
 * Function to test timer is running or not.
 ******************************************************************************/
void test_sleeptimer_is_timer_running(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing Sleeptimer is running \n");
  sl_status_t status;
  bool is_running = false;
  UnityPrintf("Testing with null parameters \n");
  status = sl_sleeptimer_is_timer_running(NULL, &is_running);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_NULL_POINTER, status);
  UnityPrintf("Testing null parameter successfully \n");
  UnityPrintf("Testing with null parameters \n");
  status = sl_sleeptimer_is_timer_running(&test_timer, NULL);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_NULL_POINTER, status);
  UnityPrintf("Testing null parameter successfully \n");
  UnityPrintf("Testing with correct parameters \n");
  status = sl_sleeptimer_is_timer_running(&test_timer, &is_running);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  UnityPrintf("is_running = %d \n", is_running);
  UnityPrintf("Sleeptimer is running successfully \n");
  test_oneshot_timeout = false;

  UnityPrintf("Sleeptimer is running test completed \n");
}

/*******************************************************************************
 * Function to test the Sleeptimer start periodic timer
 ******************************************************************************/
void test_sleeptimer_start_periodic_timer(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing Sleeptimer start periodic timer \n");
  sl_status_t status;

  status = sl_sleeptimer_start_periodic_timer(NULL,
                                              TOOGLE_DELAY_MS1_ONESHOT,
                                              on_timeout_timer,
                                              NULL,
                                              0,
                                              SL_SLEEPTIMER_NO_HIGH_PRECISION_HF_CLOCKS_REQUIRED_FLAG);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_NULL_POINTER, status);
  UnityPrintf("Testing with null pointer successfully \n");

  sl_sleeptimer_stop_timer(&test_timer);
  UnityPrintf("Testing with correct parameters \n");
  status = sl_sleeptimer_start_periodic_timer(&test_timer,
                                              TOOGLE_DELAY_MS1_ONESHOT,
                                              on_timeout_timer,
                                              NULL,
                                              0,
                                              SL_SLEEPTIMER_NO_HIGH_PRECISION_HF_CLOCKS_REQUIRED_FLAG);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  UnityPrintf("Sleeptimer start periodic timer successfully \n");

  UnityPrintf("Testing with correct parameters to check timer is running or not \n");
  status = sl_sleeptimer_start_periodic_timer(&test_timer,
                                              TOOGLE_DELAY_MS1_ONESHOT,
                                              on_timeout_timer,
                                              NULL,
                                              0,
                                              SL_SLEEPTIMER_NO_HIGH_PRECISION_HF_CLOCKS_REQUIRED_FLAG);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_INVALID_STATE, status);
  UnityPrintf("Sleeptimer start periodic timer to check timer is running or not successfully \n");

  UnityPrintf("Sleeptimer start periodic timer test completed \n");
}

/*******************************************************************************
 * Function to test get timer frequency
 ******************************************************************************/
void test_sleeptimer_get_timer_frequency(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing Sleeptimer get timer frequency \n");
  uint32_t timer_freq;

  timer_freq = sl_sleeptimer_get_timer_frequency();
  UnityPrintf("Timer Frequency = %ld \n", timer_freq);
  TEST_ASSERT_EQUAL_UINT32(DEFAULT_32KHZ_XTAL_CLOCK, timer_freq);
  UnityPrintf("Sleeptimer get timer frequency successfully \n");

  UnityPrintf("Sleeptimer get timer frequency completed \n");
}

/*******************************************************************************
 * Function to test restart the periodic timer
 ******************************************************************************/
void test_sleeptimer_restart_periodic_timer(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing Sleeptimer start periodic timer \n");
  sl_status_t status;

  status = sl_sleeptimer_restart_periodic_timer(NULL,
                                                TOOGLE_DELAY_MS1_ONESHOT,
                                                on_timeout_timer,
                                                NULL,
                                                0,
                                                SL_SLEEPTIMER_NO_HIGH_PRECISION_HF_CLOCKS_REQUIRED_FLAG);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_NULL_POINTER, status);
  UnityPrintf("Testing with null pointer successfully \n");

  UnityPrintf("Testing with correct parameters \n");
  status = sl_sleeptimer_restart_periodic_timer(&test_timer,
                                                TOOGLE_DELAY_MS1_ONESHOT,
                                                on_timeout_timer,
                                                NULL,
                                                0,
                                                SL_SLEEPTIMER_NO_HIGH_PRECISION_HF_CLOCKS_REQUIRED_FLAG);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  UnityPrintf("Sleeptimer start periodic timer successfully \n");

  UnityPrintf("Sleeptimer start periodic timer test completed \n");
}

/*******************************************************************************
 * Function to test get clock accuracy.
 ******************************************************************************/
void test_sleeptimer_get_clock_accuracy(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing Sleeptimer get clock accuracy \n");
  uint16_t clock = 0;

  clock = sl_sleeptimer_get_clock_accuracy();
  UnityPrintf("%ld \n", clock);
  UnityPrintf("Sleeptimer get clock accuracy successfully \n");

  UnityPrintf("Sleeptimer get clock accuracy test completed \n");
}

/*******************************************************************************
 * Function to test stop the Sleeptimer
 ******************************************************************************/
void test_sleeptimer_stop_timer(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing Sleeptimer stop \n");
  sl_status_t status;

  status = sl_sleeptimer_stop_timer(NULL);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_NULL_POINTER, status);
  UnityPrintf("Testing with null pointer successfully \n");

  UnityPrintf("Testing with correct parameters \n");
  status = sl_sleeptimer_stop_timer(&test_timer);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  UnityPrintf("Sleeptimer stopped successfully \n");

  UnityPrintf("Sleeptimer stop test completed \n");
}

/*******************************************************************************
 * Function to test get timer time remaining
 ******************************************************************************/
void test_sleeptimer_get_timer_time_remaining(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing Sleeptimer get time remaining \n");
  sl_status_t status;
  uint32_t time;

  status = sl_sleeptimer_get_timer_time_remaining(NULL, &time);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_NULL_POINTER, status);
  UnityPrintf("Testing with null pointer successfully \n");

  status = sl_sleeptimer_get_timer_time_remaining(&test_timer, NULL);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_NULL_POINTER, status);
  UnityPrintf("Testing with null pointer successfully \n");

  UnityPrintf("Testing with correct parameters \n");
  status = sl_sleeptimer_get_timer_time_remaining(&test_timer, &time);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  UnityPrintf("Sleeptimer get time remaining successfully \n");

  UnityPrintf("Sleeptimer get time remaining test completed \n");
}

/*******************************************************************************
 * Function to test get remaining time of first timer
 ******************************************************************************/
void test_sleeptimer_get_remaining_time_of_first_timer(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing Sleeptimer get remaining time of first timer \n");
  sl_status_t status;
  uint32_t time;

  UnityPrintf("Testing with correct parameters \n");
  status =
    sl_sleeptimer_get_remaining_time_of_first_timer(SL_SLEEPTIMER_NO_HIGH_PRECISION_HF_CLOCKS_REQUIRED_FLAG, &time);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  UnityPrintf("Sleeptimer get remaining time of first timer successfully \n");

  UnityPrintf("Sleeptimer get remaining time of first timer test completed \n");
}

/***************************************************************************/ /**
 * Function to test delay millisecond
 ******************************************************************************/
void test_sleeptimer_delay_millisecond(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing Sleeptimer delay millisecond \n");
  uint16_t delay_ms = 5;

  sl_sleeptimer_delay_millisecond(delay_ms);
  UnityPrintf("Sleeptimer delay millisecond successfully \n");

  UnityPrintf("Sleeptimer delay millisecond completed \n");
}

/***************************************************************************/ /**
 * Function to test convert ms to tick.
 ******************************************************************************/
void test_sleeptimer_ms_to_tick(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing Sleeptimer ms to tick \n");
  uint32_t tick_count = 0;
  uint16_t delay_ms   = 5;

  tick_count = sl_sleeptimer_ms_to_tick(delay_ms);
  UnityPrintf("%ld \n", tick_count);
  UnityPrintf("Sleeptimer ms to tick successfully \n");

  UnityPrintf("Sleeptimer ms to tick completed \n");
}

void test_sleeptimer_ms32_to_tick(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing Sleeptimer ms to tick \n");
  sl_status_t status;
  uint32_t delay_ms = 5;
  uint32_t time     = 0;

  UnityPrintf("Testing with correct parameters \n");
  status = sl_sleeptimer_ms32_to_tick(delay_ms, &time);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  UnityPrintf("Sleeptimer ms to tick successfully \n");

  UnityPrintf("Testing with maximum ms \n");
  status = sl_sleeptimer_ms32_to_tick(max_ms + 1, &time);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_INVALID_PARAMETER, status);
  UnityPrintf("Sleeptimer invalid parameter successfully \n");

  UnityPrintf("Sleeptimer ms to tick test completed \n");
}

/***************************************************************************/ /**
 * Function to test get the maximum value that can be passed to the functions
 * that have a 32-bits time or timeout argument expressed in milliseconds.
 ******************************************************************************/
void test_sleeptimer_get_max_ms32_conversion(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing Sleeptimer max ms \n");

  max_ms = sl_sleeptimer_get_max_ms32_conversion();
  UnityPrintf("Sleeptimer max ms successfully \n");

  UnityPrintf("Sleeptimer max ms test completed \n");
}

/*******************************************************************************
 * Function to test converts ticks in milliseconds.
 ******************************************************************************/
void test_sleeptimer_tick_to_ms(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing Sleeptimer convert tick to ms \n");
  uint32_t tick = 32000;
  uint32_t msec = 0;

  msec = sl_sleeptimer_tick_to_ms(tick);
  UnityPrintf("%ld \n", msec);
  UnityPrintf("Sleeptimer convert tick to ms successfully \n");

  UnityPrintf("Sleeptimer convert tick to ms test completed \n");
}

/*******************************************************************************
 * Function to test converts 64-bits ticks in milliseconds.
 ******************************************************************************/
void test_sleeptimer_tick64_to_ms(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing Sleeptimer convert 64-bits tick to ms \n");
  sl_status_t status;
  uint64_t tick = UINT64_MAX / 1000;
  uint64_t msec = 0;

  status = sl_sleeptimer_tick64_to_ms(tick + 1, &msec);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_INVALID_PARAMETER, status);
  UnityPrintf("Sleeptimer testing with invalid parameter successfully \n");

  tick   = 32000;
  status = sl_sleeptimer_tick64_to_ms(tick, &msec);
  UnityPrintf("%ld \n", msec);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  UnityPrintf("Sleeptimer convert 64-bits tick to ms successfully \n");

  UnityPrintf("Sleeptimer convert 64-bits tick to ms test completed \n");
}

/*******************************************************************************
 * Function to test if the power manager's early wakeup expired during the
 * last ISR.
 ******************************************************************************/
void test_sleeptimer_is_power_manager_early_restore_timer_latest_to_expire(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing Sleeptimer is power manager early restore timer latest to expire \n");
  bool is_early_wakup_expired = false;

  is_early_wakup_expired = sl_sleeptimer_is_power_manager_early_restore_timer_latest_to_expire();
  UnityPrintf("%d \n", is_early_wakup_expired);
  UnityPrintf("Sleeptimer convert 64-bits tick to ms successfully \n");

  UnityPrintf("Sleeptimer convert 64-bits tick to ms test completed \n");
}

/*******************************************************************************
 * Function to test starts a 32 bits timer.
 ******************************************************************************/
void test_sleeptimer_start_timer_ms(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing Sleeptimer starts a 32 bits timer \n");
  sl_status_t status;

  sl_sleeptimer_stop_timer(&test_timer);
  UnityPrintf("Testing with correct parameters \n");
  status = sl_sleeptimer_start_timer_ms(&test_timer,
                                        TOOGLE_DELAY_MS1_ONESHOT,
                                        on_timeout_timer,
                                        NULL,
                                        0,
                                        SL_SLEEPTIMER_NO_HIGH_PRECISION_HF_CLOCKS_REQUIRED_FLAG);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  UnityPrintf("Sleeptimer starts a 32 bits timer successfully \n");

  UnityPrintf("Sleeptimer starts a 32 bits timer test completed \n");
}

/*******************************************************************************
 * Function to test restarts a 32 bits timer.
 ******************************************************************************/
void test_sleeptimer_restart_timer_ms(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing Sleeptimer restarts a 32 bits timer \n");
  sl_status_t status;

  UnityPrintf("Testing with correct parameters \n");
  status = sl_sleeptimer_restart_timer_ms(&test_timer,
                                          TOOGLE_DELAY_MS1_ONESHOT,
                                          on_timeout_timer,
                                          NULL,
                                          0,
                                          SL_SLEEPTIMER_NO_HIGH_PRECISION_HF_CLOCKS_REQUIRED_FLAG);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  UnityPrintf("Sleeptimer restarts a 32 bits timer successfully \n");

  UnityPrintf("Sleeptimer restarts a 32 bits timer test completed \n");
}

/*******************************************************************************
 * Function to test starts periodic timer ms.
 ******************************************************************************/
void test_sleeptimer_start_periodic_timer_ms(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing Sleeptimer starts periodic timer ms \n");
  sl_status_t status;

  status = sl_sleeptimer_start_periodic_timer_ms(NULL,
                                                 TOOGLE_DELAY_MS1_ONESHOT,
                                                 on_timeout_timer,
                                                 NULL,
                                                 0,
                                                 SL_SLEEPTIMER_NO_HIGH_PRECISION_HF_CLOCKS_REQUIRED_FLAG);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_NULL_POINTER, status);
  UnityPrintf("Testing with null pointer successfully \n");

  sl_sleeptimer_stop_timer(&test_timer);
  UnityPrintf("Testing with correct parameters \n");
  status = sl_sleeptimer_start_periodic_timer_ms(&test_timer,
                                                 TOOGLE_DELAY_MS1_ONESHOT,
                                                 on_timeout_timer,
                                                 NULL,
                                                 0,
                                                 SL_SLEEPTIMER_NO_HIGH_PRECISION_HF_CLOCKS_REQUIRED_FLAG);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  UnityPrintf("Sleeptimer starts periodic timer ms successfully \n");

  UnityPrintf("Sleeptimer starts periodic timer ms test completed \n");
}

/*******************************************************************************
 * Function to test restarts periodic timer ms.
 ******************************************************************************/
void test_sleeptimer_restart_periodic_timer_ms(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing Sleeptimer restarts periodic timer ms \n");
  sl_status_t status;

  status = sl_sleeptimer_restart_periodic_timer_ms(NULL,
                                                   TOOGLE_DELAY_MS1_ONESHOT,
                                                   on_timeout_timer,
                                                   NULL,
                                                   0,
                                                   SL_SLEEPTIMER_NO_HIGH_PRECISION_HF_CLOCKS_REQUIRED_FLAG);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_NULL_POINTER, status);
  UnityPrintf("Testing with null pointer successfully \n");

  UnityPrintf("Testing with correct parameters \n");
  status = sl_sleeptimer_restart_periodic_timer_ms(&test_timer,
                                                   TOOGLE_DELAY_MS1_ONESHOT,
                                                   on_timeout_timer,
                                                   NULL,
                                                   0,
                                                   SL_SLEEPTIMER_NO_HIGH_PRECISION_HF_CLOCKS_REQUIRED_FLAG);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  UnityPrintf("Sleeptimer restarts periodic timer ms successfully \n");

  UnityPrintf("Sleeptimer restarts periodic timer ms test completed \n");
}

/***************************************************************************/ /**
 * Sleeptimer timeout callback.
 ******************************************************************************/
static void on_timeout_timer(sl_sleeptimer_timer_handle_t *handle, void *data)
{
  (void)&handle;
  (void)&data;
  if (test_oneshot_timeout) {
    UnityPrintf("\r\n *******Timer1 oneshot timeout******* \r\n");
  } else {
    UnityPrintf("\r\n Timer1 periodic timeout \r\n");
  }
}
