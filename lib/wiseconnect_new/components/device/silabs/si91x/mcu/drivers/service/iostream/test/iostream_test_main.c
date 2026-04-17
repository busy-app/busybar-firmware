/***************************************************************************/
/**
 * @file iostream_usart_test_main.c
 * @brief IOStream USART Unity Test Functions
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
#include "sl_iostream.h"
#include "sl_iostream_init_instances.h"
#include "sl_iostream_handles.h"
#include "rsi_debug.h"
#include "unity.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

/*******************************************************************************
 ***************************  Defines / Macros  ********************************
 ******************************************************************************/

/*******************************************************************************
 ***************************  Global Variables   *******************************
 ******************************************************************************/

/*******************************************************************************
 ************************  Static Function Prototypes   ************************
 ******************************************************************************/

/*******************************************************************************
 ***************************  Local Variables   *******************************
 ******************************************************************************/
char c = 0;

/*******************************************************************************
 ************************  Test Function Prototypes  ****************************
 ******************************************************************************/
void test_iostream_set_default(void);
void test_iostream_get_default(void);
void test_iostream_write(void);
void test_iostream_read(void);
void test_iostream_putchar(void);
void test_iostream_getchar(void);
void test_iostream_vprintf(void);
void test_iostream_printf(void);

/******************************************************************************
 * Main function in which all the test cases are tested using unity framework
 ******************************************************************************/
int app_init()
{
  UnityBeginGroup("IOStream USART");

  RUN_TEST(test_iostream_set_default, __LINE__);
  RUN_TEST(test_iostream_get_default, __LINE__);
  RUN_TEST(test_iostream_write, __LINE__);
  RUN_TEST(test_iostream_read, __LINE__);
  RUN_TEST(test_iostream_getchar, __LINE__);
  RUN_TEST(test_iostream_putchar, __LINE__);
  RUN_TEST(test_iostream_vprintf, __LINE__);
  RUN_TEST(test_iostream_printf, __LINE__);

  UnityEnd();
  UnityPrintf("END");
  while (1) {
  }
}

/*******************************************************************************
 * Function to test Set the stream as default I/O Stream.
 ******************************************************************************/
void test_iostream_set_default(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing IOStream USART Set the stream as default I/O Stream. \n");
  sl_status_t status;

  UnityPrintf("Testing with correct parameters \n");
  status = sl_iostream_set_default(sl_iostream_vcom_handle);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  UnityPrintf("Status of API is correct, IOStream USART Set the stream as default I/O Stream. successfully \n");

  UnityPrintf("IOStream USART Set the stream as default I/O Stream. test completed \n");
}

/*******************************************************************************
 * Function to test Get default IO stream.
 ******************************************************************************/
void test_iostream_get_default(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing IOStream USART get default IO stream \n");
  sl_iostream_t *get_default_value;

  UnityPrintf("Testing with correct parameters \n");
  get_default_value = sl_iostream_get_default();
  UnityPrintf("Get_default_value = %ld \n", get_default_value);
  UnityPrintf("IOStream USART get default IO stream successfully \n");

  UnityPrintf("IOStream USART get default IO stream test completed \n");
}

/*******************************************************************************
 * Function to test stream write
 ******************************************************************************/
void test_iostream_write(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing IOStream UART stream write \n");
  sl_status_t status;
  const char test_str[] = "IOstream USART example\r\n";

  UnityPrintf("Testing with correct parameters \n");
  status = sl_iostream_write(sl_iostream_vcom_handle, test_str, strlen(test_str));
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  UnityPrintf("Status of API is correct, IOStream UART stream write successfully \n");

  sl_iostream_set_default(sl_iostream_vcom_handle);
  const char test_str2[] = "This is output on the default stream\r\n";
  sl_iostream_write(SL_IOSTREAM_STDOUT, test_str2, strlen(test_str2));
  printf("Printf uses the default stream, as long as iostream_retarget_stdio is included.\r\n> ");

  UnityPrintf("IOStream UART stream write test completed \n");
}

/*******************************************************************************
 * Function to test stream read
 ******************************************************************************/
void test_iostream_read(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing IOStream UART stream read \n");
  sl_status_t status;
  uint32_t *buffer;
  size_t buffer_len = 1;
  size_t bytes_read;

  UnityPrintf("Testing with correct parameters \n");
  status = sl_iostream_read(sl_iostream_vcom_handle, (uint32_t *)&buffer, buffer_len, &bytes_read);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  UnityPrintf("IOStream UART stream read successfully \n");

  UnityPrintf("IOStream UART stream read test completed \n");
}

/*******************************************************************************
 * Function to test print a character on stream.
 ******************************************************************************/
void test_iostream_putchar(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing IOStream UART print a character on stream \n");
  sl_status_t status;

  UnityPrintf("Testing with correct parameters \n");
  status = sl_iostream_putchar(SL_IOSTREAM_STDOUT, (char)c);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  UnityPrintf("IOStream UART print a character on stream successfully \n");

  UnityPrintf("IOStream UART print a character on stream test completed \n");
}

/*******************************************************************************
 * Function to test stream getchar.
 ******************************************************************************/
void test_iostream_getchar(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing IOStream UART stream getchar \n");
  sl_status_t status;

  UnityPrintf("Testing with correct parameters \n");
  status = sl_iostream_getchar(SL_IOSTREAM_STDOUT, &c);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  UnityPrintf("IOStream UART stream getchar successfully \n");

  UnityPrintf("IOStream UART stream getchar test completed \n");
}

/*******************************************************************************
 * Function to test print a formated string on stream.
 ******************************************************************************/
void test_iostream_vprintf(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing IOStream UART print a formated string on stream \n");
  sl_status_t status;
  const char format[] = "IOstream USART vprintf\r\n";
  va_list test_argp;

  UnityPrintf("Testing with correct parameters \n");
  status = sl_iostream_vprintf(SL_IOSTREAM_STDOUT, format, test_argp);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  UnityPrintf("IOStream UART print a formated string on stream successfully \n");

  UnityPrintf("IOStream UART print a formated string on stream test completed \n");
}

/*******************************************************************************
 * Function to test print a formated string on stream.
 ******************************************************************************/
void test_iostream_printf(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing IOStream UART print a formated string on stream \n");
  sl_status_t status;
  const char format1[] = "IOstream USART vprintf\r\n";

  UnityPrintf("Testing with correct parameters \n");
  status = sl_iostream_printf(SL_IOSTREAM_STDOUT, format1);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  UnityPrintf("IOStream UART print a formated string on stream successfully \n");

  UnityPrintf("IOStream UART print a formated string on stream test completed \n");
}
