/*******************************************************************************
 * @file
 * @brief 
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/

#include <gtest/gtest.h>
extern "C" {
#include "sli_hal_si91x.h"
#include "sli_hal_si91x_fake_functions.h"
#include "sl_constants.h"
}

// Custom fake function for sl_redirect_log
void sl_redirect_log(const char *format, ...)
{
  va_list args;
  va_start(args, format);
  vprintf(format, args); // Simulate logging
  va_end(args);
}

sl_status_t fake_buffer_manager_allocate_buffer(const sli_buffer_manager_pool_types_t p, 
                                                const sli_buffer_manager_allocation_types_t at,
                                                const uint32_t t,
                                                sli_buffer_t *buffer) {
  *buffer = (sli_buffer_t)malloc(1800);
  return SL_STATUS_OK;
}


sl_status_t fake_buffer_manager_free_buffer(sli_buffer_t buffer) {
  free((void *)buffer);
  return SL_STATUS_OK;
}

void sli_hal_si91x_reset_fake(void)
{
  osThreadNew_reset();
  osEventFlagsNew_reset();
  osEventFlagsSet_reset();
  osThreadTerminate_reset();
  osEventFlagsDelete_reset();
  sli_queue_manager_enqueue_reset();
  sli_queue_manager_dequeue_reset();
}

TEST(sli_hal_si91x, sli_hal_si91x_init_thread_creation_failed)
{
  osThreadId_t thread_id                   = NULL;
  osThreadNew_fake.return_val              = thread_id;
  sli_queue_manager_init_fake.return_val   = SL_STATUS_OK;
  sli_queue_manager_deinit_fake.return_val = SL_STATUS_OK;
  sl_status_t status                       = sli_hal_si91x_init();
  EXPECT_TRUE(osThreadNew_fake.call_count == 1);
  EXPECT_TRUE(sli_queue_manager_init_fake.call_count == 3);
  EXPECT_TRUE(sli_queue_manager_deinit_fake.call_count == 3);
  EXPECT_TRUE(status == SL_STATUS_FAIL);
}

TEST(sli_hal_si91x, sli_hal_si91x_init_event_flag_creation_failed)
{
  osThreadNew_reset();
  osThreadId_t thread_id      = (osThreadId_t)0x1234;
  osThreadNew_fake.return_val = thread_id;
  osEventFlagsNew_reset();
  osEventFlagsId_t event_flags_id = NULL;
  osEventFlagsNew_fake.return_val = event_flags_id;
  sli_queue_manager_init_reset();
  sli_queue_manager_deinit_reset();
  sli_queue_manager_init_fake.return_val   = SL_STATUS_OK;
  sli_queue_manager_deinit_fake.return_val = SL_STATUS_OK;
  osThreadTerminate_reset();
  osThreadTerminate_fake.return_val = osOK;
  sl_status_t status                = sli_hal_si91x_init();
  EXPECT_TRUE(osThreadNew_fake.call_count == 1);
  EXPECT_TRUE(osEventFlagsNew_fake.call_count == 1);
  EXPECT_TRUE(sli_queue_manager_init_fake.call_count == 3);
  EXPECT_TRUE(osThreadTerminate_fake.call_count == 1);
  EXPECT_TRUE(sli_queue_manager_deinit_fake.call_count == 3);
  EXPECT_TRUE(status == SL_STATUS_FAIL);
}

TEST(sli_hal_si91x, sli_hal_si91x_init_success)
{
  osThreadNew_reset();
  osEventFlagsNew_reset();
  osThreadId_t thread_id          = (osThreadId_t)0x1234;
  osThreadNew_fake.return_val     = thread_id;
  osEventFlagsId_t event_flags_id = (osEventFlagsId_t)0x5678;
  osEventFlagsNew_fake.return_val = event_flags_id;
  sli_queue_manager_init_reset();
  sli_queue_manager_init_fake.return_val = SL_STATUS_OK;
  sl_status_t status                     = sli_hal_si91x_init();
  EXPECT_TRUE(osThreadNew_fake.call_count == 1);
  EXPECT_TRUE(osEventFlagsNew_fake.call_count == 1);
  EXPECT_TRUE(sli_queue_manager_init_fake.call_count == 3);
  EXPECT_TRUE(status == SL_STATUS_OK);
  osThreadTerminate_reset();
  osThreadTerminate_fake.return_val = osOK;
  osEventFlagsDelete_reset();
  osEventFlagsDelete_fake.return_val = osOK;
  sli_queue_manager_deinit_reset();
  sli_queue_manager_deinit_fake.return_val = SL_STATUS_OK;
  sli_hal_si91x_deinit();
}

TEST(sli_hal_si91x, sli_hal_si91x_deinit_without_init)
{
  osEventFlagsDelete_reset();
  osThreadTerminate_reset();
  sli_queue_manager_deinit_reset();
  sli_queue_manager_deinit_fake.return_val = SL_STATUS_NOT_INITIALIZED;
  sl_status_t status                       = sli_hal_si91x_deinit();
  EXPECT_TRUE(status == SL_STATUS_OK);
  EXPECT_TRUE(sli_queue_manager_deinit_fake.call_count == 3);
}

TEST(sli_hal_si91x, sli_hal_si91x_deinit_success)
{
  osThreadNew_reset();
  osEventFlagsNew_reset();
  osThreadId_t thread_id          = (osThreadId_t)0x1234;
  osThreadNew_fake.return_val     = thread_id;
  osEventFlagsId_t event_flags_id = (osEventFlagsId_t)0x5678;
  osEventFlagsNew_fake.return_val = event_flags_id;
  sli_queue_manager_init_reset();
  sli_queue_manager_init_fake.return_val = SL_STATUS_OK;
  sl_status_t status                     = sli_hal_si91x_init();
  osThreadTerminate_reset();
  osThreadTerminate_fake.return_val = osOK;
  osEventFlagsDelete_reset();
  osEventFlagsDelete_fake.return_val = osOK;
  sli_queue_manager_deinit_reset();
  sli_queue_manager_deinit_fake.return_val = SL_STATUS_OK;
  status                                   = sli_hal_si91x_deinit();
  EXPECT_TRUE(status == SL_STATUS_OK);
  EXPECT_TRUE(osThreadTerminate_fake.call_count == 1);
  EXPECT_TRUE(osEventFlagsDelete_fake.call_count == 1);
  EXPECT_TRUE(sli_queue_manager_deinit_fake.call_count == 3);
}

TEST(sli_hal_si91x, sli_hal_si91x_command_send_packet_success)
{
  sli_hal_si91x_reset_fake();
  osThreadId_t thread_id          = (osThreadId_t)0x1234;
  osThreadNew_fake.return_val     = thread_id;
  osEventFlagsId_t event_flags_id = (osEventFlagsId_t)0x5678;
  osEventFlagsNew_fake.return_val = event_flags_id;
  sli_buffer_manager_allocate_buffer_fake.custom_fake = fake_buffer_manager_allocate_buffer;
  sli_buffer_manager_free_buffer_fake.custom_fake = fake_buffer_manager_free_buffer;
  sli_queue_manager_init_reset();
  sli_queue_manager_init_fake.return_val    = SL_STATUS_OK;
  sl_status_t status                        = sli_hal_si91x_init();
  uint8_t packet[10]                        = { 0 };
  sli_queue_manager_enqueue_fake.return_val = SL_STATUS_OK;
  status                                    = sli_hal_si91x_command_send_packet(packet, sizeof(packet), NULL, NULL);
  EXPECT_TRUE(status == SL_STATUS_IN_PROGRESS);
  EXPECT_TRUE(osThreadNew_fake.call_count == 1);
  EXPECT_TRUE(osEventFlagsNew_fake.call_count == 1);
  EXPECT_TRUE(sli_queue_manager_init_fake.call_count == 3);
  osThreadTerminate_fake.return_val        = osOK;
  sli_queue_manager_deinit_fake.return_val = SL_STATUS_OK;
  osEventFlagsDelete_fake.return_val       = osOK;
  status                                   = sli_hal_si91x_deinit();
  EXPECT_TRUE(status == SL_STATUS_OK);
}

TEST(sli_hal_si91x, sli_hal_si91x_data_send_packet_success)
{
  sli_hal_si91x_reset_fake();
  osThreadId_t thread_id          = (osThreadId_t)0x1234;
  osThreadNew_fake.return_val     = thread_id;
  osEventFlagsId_t event_flags_id = (osEventFlagsId_t)0x5678;
  osEventFlagsNew_fake.return_val = event_flags_id;
  sli_buffer_manager_allocate_buffer_fake.custom_fake = fake_buffer_manager_allocate_buffer;
  sli_buffer_manager_free_buffer_fake.custom_fake = fake_buffer_manager_free_buffer;
  sli_queue_manager_init_reset();
  sli_queue_manager_init_fake.return_val    = SL_STATUS_OK;
  sl_status_t status                        = sli_hal_si91x_init();
  uint8_t packet[10]                        = { 0 };
  sli_queue_manager_enqueue_fake.return_val = SL_STATUS_OK;
  status                                    = sli_hal_si91x_data_send_packet(packet, sizeof(packet), NULL, NULL);
  EXPECT_TRUE(status == SL_STATUS_IN_PROGRESS);
  EXPECT_TRUE(osThreadNew_fake.call_count == 1);
  EXPECT_TRUE(osEventFlagsNew_fake.call_count == 1);
  EXPECT_TRUE(sli_queue_manager_init_fake.call_count == 3);
  osThreadTerminate_fake.return_val        = osOK;
  sli_queue_manager_deinit_fake.return_val = SL_STATUS_OK;
  osEventFlagsDelete_fake.return_val       = osOK;
  status                                   = sli_hal_si91x_deinit();
  EXPECT_TRUE(status == SL_STATUS_OK);
}

TEST(sli_hal_si91x, sli_hal_si91x_common_send_packet_success)
{
  sli_hal_si91x_reset_fake();
  osThreadId_t thread_id          = (osThreadId_t)0x1234;
  osThreadNew_fake.return_val     = thread_id;
  osEventFlagsId_t event_flags_id = (osEventFlagsId_t)0x5678;
  osEventFlagsNew_fake.return_val = event_flags_id;
  sli_buffer_manager_allocate_buffer_fake.custom_fake = fake_buffer_manager_allocate_buffer;
  sli_buffer_manager_free_buffer_fake.custom_fake = fake_buffer_manager_free_buffer;
  sli_queue_manager_init_reset();
  sli_queue_manager_init_fake.return_val    = SL_STATUS_OK;
  sl_status_t status                        = sli_hal_si91x_init();
  uint8_t packet[10]                        = { 0 };
  sli_queue_manager_enqueue_fake.return_val = SL_STATUS_OK;
  status                                    = sli_hal_si91x_common_send_packet(packet, sizeof(packet), NULL, NULL);
  EXPECT_TRUE(status == SL_STATUS_IN_PROGRESS);
  EXPECT_TRUE(osThreadNew_fake.call_count == 1);
  EXPECT_TRUE(osEventFlagsNew_fake.call_count == 1);
  EXPECT_TRUE(sli_queue_manager_init_fake.call_count == 3);
  osThreadTerminate_fake.return_val        = osOK;
  sli_queue_manager_deinit_fake.return_val = SL_STATUS_OK;
  osEventFlagsDelete_fake.return_val       = osOK;
  status                                   = sli_hal_si91x_deinit();
  EXPECT_TRUE(status == SL_STATUS_OK);
}

TEST(sli_hal_si91x, sli_hal_si91x_ble_send_packet_success)
{
  sli_hal_si91x_reset_fake();
  osThreadId_t thread_id          = (osThreadId_t)0x1234;
  osThreadNew_fake.return_val     = thread_id;
  osEventFlagsId_t event_flags_id = (osEventFlagsId_t)0x5678;
  osEventFlagsNew_fake.return_val = event_flags_id;
  sli_buffer_manager_allocate_buffer_fake.custom_fake = fake_buffer_manager_allocate_buffer;
  sli_buffer_manager_free_buffer_fake.custom_fake = fake_buffer_manager_free_buffer;
  sli_queue_manager_init_reset();
  sli_queue_manager_init_fake.return_val    = SL_STATUS_OK;
  sl_status_t status                        = sli_hal_si91x_init();
  uint8_t packet[10]                        = { 0 };
  sli_queue_manager_enqueue_fake.return_val = SL_STATUS_OK;
  status                                    = sli_hal_si91x_ble_send_packet(packet, sizeof(packet), NULL, NULL);
  EXPECT_TRUE(status == SL_STATUS_IN_PROGRESS);
  EXPECT_TRUE(osThreadNew_fake.call_count == 1);
  EXPECT_TRUE(osEventFlagsNew_fake.call_count == 1);
  EXPECT_TRUE(sli_queue_manager_init_fake.call_count == 3);
  osThreadTerminate_fake.return_val        = osOK;
  sli_queue_manager_deinit_fake.return_val = SL_STATUS_OK;
  osEventFlagsDelete_fake.return_val       = osOK;
  status                                   = sli_hal_si91x_deinit();
  EXPECT_TRUE(status == SL_STATUS_OK);
}