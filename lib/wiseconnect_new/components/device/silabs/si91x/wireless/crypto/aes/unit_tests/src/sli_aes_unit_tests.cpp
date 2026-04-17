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

#include "gtest/gtest.h"
extern "C" {
#include "sli_aes_fake_functions.h"
}

extern "C" void *my_host_get_buffer_data_fake(sl_wifi_buffer_t *, uint16_t, uint16_t *);
class AESMultipartTest : public ::testing::Test {
protected:
  void SetUp() override
  {
    RESET_FAKE(sli_si91x_driver_send_command);
    FFF_RESET_HISTORY();
  }

  void TearDown() override
  {
  }
};

// Test with NULL message pointer
TEST(sl_aes_unit_tests, aes_multipart_null_msg_returns_null_pointer)
{
  sl_si91x_aes_config_t config = {
    .aes_mode        = SL_SI91X_AES_CTR,              // AES Mode
    .encrypt_decrypt = SL_SI91X_AES_ENCRYPT,          // Encrypt
    .msg             = NULL,                          // NULL message
    .msg_length      = 16,                            // Length of the message
    .iv              = (uint8_t *)"TestIV1234567890", // Pointer to the Initialization vector
    .key_config      = { 0 }                          // Key configuration (default initialization)
  };

  uint8_t output[16];
  uint16_t chunk_length = 16;
  uint8_t aes_flags     = FIRST_CHUNK;

  sl_status_t status = sl_si91x_aes_multipart(&config, chunk_length, aes_flags, output);
  EXPECT_EQ(status, SL_STATUS_NULL_POINTER);
}

// Test: Missing IV for CTR mode
TEST(sl_aes_unit_tests, aes_multipart_missing_iv_returns_invalid_parameter)
{
  sl_si91x_aes_config_t config = { .aes_mode        = SL_SI91X_AES_CTR,
                                   .encrypt_decrypt = SL_SI91X_AES_ENCRYPT,
                                   .msg             = (uint8_t *)"TestMessage123456",
                                   .msg_length      = 16,
                                   .iv              = NULL,
                                   .key_config      = { 0 } };
  uint8_t output[16];
  sl_status_t status = sl_si91x_aes_multipart(&config, 16, FIRST_CHUNK, output);
  EXPECT_EQ(status, SL_STATUS_INVALID_PARAMETER);
}

// Test with invalid chunk length
TEST(sl_aes_unit_tests, aes_multipart_invalid_chunk_length_returns_invalid_parameter)
{
  sl_si91x_aes_config_t config = {
    .aes_mode        = SL_SI91X_AES_CTR,               // AES Mode
    .encrypt_decrypt = SL_SI91X_AES_ENCRYPT,           // Encrypt
    .msg             = (uint8_t *)"TestMessage123456", // Pointer to the input message
    .msg_length      = 16,                             // Length of the message
    .iv              = (uint8_t *)"TestIV1234567890",  // Pointer to the Initialization vector
    .key_config      = { 0 }                           // Key configuration (default initialization)
  };

  uint8_t output[16];
  uint16_t chunk_length = 15; // Not a multiple of AES_BLOCK_SIZE
  uint8_t aes_flags     = FIRST_CHUNK;

  sl_status_t status = sl_si91x_aes_multipart(&config, chunk_length, aes_flags, output);
  EXPECT_EQ(status, SL_STATUS_INVALID_PARAMETER);
}

// Test with unsupported AES mode
TEST(sl_aes_unit_tests, aes_multipart_ctr_mode_null_iv_returns_invalid_parameter)
{
  sl_si91x_aes_config_t config = {
    .aes_mode        = SL_SI91X_AES_CTR, // Valid mode
    .encrypt_decrypt = SL_SI91X_AES_ENCRYPT,
    .msg             = (uint8_t *)"TestMessage123456", // Pointer to the input message
    .msg_length      = 16,                             // Length of the message
    .iv              = NULL,                           // Invalid IV for AES_CTR
    .key_config      = { 0 }                           // Key configuration (default initialization)
  };

  uint8_t output[16];
  uint16_t chunk_length = 16;
  uint8_t aes_flags     = FIRST_CHUNK;

  sl_status_t status = sl_si91x_aes_multipart(&config, chunk_length, aes_flags, output);
  EXPECT_EQ(status, SL_STATUS_INVALID_PARAMETER);
}

// Test with valid config and successful AES operation
TEST(sl_aes_unit_tests, aes_multipart_valid_ctr_returns_ok)
{
  sl_si91x_aes_config_t ctr_encrypt_config = { .aes_mode        = SL_SI91X_AES_CTR,
                                               .encrypt_decrypt = SL_SI91X_AES_ENCRYPT,
                                               .msg             = (uint8_t *)"CTR_Encrypt_Input1",
                                               .msg_length      = 16,
                                               .iv              = (uint8_t *)"CTR_IV_1234567890",
                                               .key_config      = { 0 } };
  uint8_t ctr_encrypt_output[16];
  uint16_t ctr_encrypt_chunk_length = 16;
  uint8_t ctr_encrypt_flags         = FIRST_CHUNK;

  sli_si91x_driver_send_command_fake.return_val  = SL_STATUS_OK;
  sli_wifi_host_get_buffer_data_fake.custom_fake = my_host_get_buffer_data_fake;
  sl_status_t ctr_encrypt_status =
    sl_si91x_aes_multipart(&ctr_encrypt_config, ctr_encrypt_chunk_length, ctr_encrypt_flags, ctr_encrypt_output);

  // Validate the output buffer contains the expected pattern
  for (int i = 0; i < 16; ++i) {
    EXPECT_EQ(ctr_encrypt_output[i], 0xAB);
  }
  EXPECT_EQ(ctr_encrypt_status, SL_STATUS_OK);
}

// Test with valid config and successful AES CBC encryption operation
TEST(sl_aes_unit_tests, aes_multipart_valid_cbc_returns_ok)
{
  sl_si91x_aes_config_t cbc_encrypt_config = { .aes_mode        = SL_SI91X_AES_CBC,
                                               .encrypt_decrypt = SL_SI91X_AES_ENCRYPT,
                                               .msg             = (uint8_t *)"CBC_Encrypt_Input2",
                                               .msg_length      = 16,
                                               .iv              = (uint8_t *)"CBC_IV_0987654321",
                                               .key_config      = { 0 } };
  uint8_t cbc_encrypt_output[16];
  uint16_t cbc_encrypt_chunk_length = 16;
  uint8_t cbc_encrypt_flags         = FIRST_CHUNK;

  sli_si91x_driver_send_command_fake.return_val  = SL_STATUS_OK;
  sli_wifi_host_get_buffer_data_fake.custom_fake = my_host_get_buffer_data_fake;

  sl_status_t cbc_encrypt_status =
    sl_si91x_aes_multipart(&cbc_encrypt_config, cbc_encrypt_chunk_length, cbc_encrypt_flags, cbc_encrypt_output);

  for (int i = 0; i < 16; ++i) {
    EXPECT_EQ(cbc_encrypt_output[i], 0xAB);
  }
  EXPECT_EQ(cbc_encrypt_status, SL_STATUS_OK);
}

// Test with valid config and successful AES CTR decryption operation
TEST(sl_aes_unit_tests, aes_multipart_valid_ctr_decrypt_returns_ok)
{
  sl_si91x_aes_config_t ctr_decrypt_config = { .aes_mode        = SL_SI91X_AES_CTR,
                                               .encrypt_decrypt = SL_SI91X_AES_DECRYPT,
                                               .msg             = (uint8_t *)"CTR_Decrypt_Input3",
                                               .msg_length      = 16,
                                               .iv              = (uint8_t *)"CTR_IV_1122334455",
                                               .key_config      = { 0 } };
  uint8_t ctr_decrypt_output[16];
  uint16_t ctr_decrypt_chunk_length = 16;
  uint8_t ctr_decrypt_flags         = FIRST_CHUNK;

  sli_si91x_driver_send_command_fake.return_val  = SL_STATUS_OK;
  sli_wifi_host_get_buffer_data_fake.custom_fake = my_host_get_buffer_data_fake;

  sl_status_t ctr_decrypt_status =
    sl_si91x_aes_multipart(&ctr_decrypt_config, ctr_decrypt_chunk_length, ctr_decrypt_flags, ctr_decrypt_output);

  for (int i = 0; i < 16; ++i) {
    EXPECT_EQ(ctr_decrypt_output[i], 0xAB);
  }
  EXPECT_EQ(ctr_decrypt_status, SL_STATUS_OK);
}

// Test multipart operation: process two consecutive AES CTR chunks and verify output
TEST(sl_aes_unit_tests, aes_multipart_valid_ctr_multiple_chunks_returns_ok)
{
  sl_si91x_aes_config_t config = { .aes_mode        = SL_SI91X_AES_CTR,
                                   .encrypt_decrypt = SL_SI91X_AES_ENCRYPT,
                                   .msg             = (uint8_t *)"TestMessage123456TestMessage123456", // 32 bytes
                                   .msg_length      = 32,
                                   .iv              = (uint8_t *)"TestIV1234567890",
                                   .key_config      = { 0 } };
  uint8_t output[32];
  uint16_t chunk_length = 16;
  uint8_t aes_flags     = FIRST_CHUNK;

  sli_si91x_driver_send_command_fake.return_val  = SL_STATUS_OK;
  sli_wifi_host_get_buffer_data_fake.custom_fake = my_host_get_buffer_data_fake;

  // First chunk
  sl_status_t status = sl_si91x_aes_multipart(&config, chunk_length, aes_flags, output);
  EXPECT_EQ(status, SL_STATUS_OK);

  // Next chunk
  config.msg += 16;
  aes_flags = MIDDLE_CHUNK; // or whatever flag is appropriate for your API
  status    = sl_si91x_aes_multipart(&config, chunk_length, aes_flags, output + 16);
  EXPECT_EQ(status, SL_STATUS_OK);

  for (int i = 0; i < 32; ++i) {
    EXPECT_EQ(output[i], 0xAB);
  }
}
