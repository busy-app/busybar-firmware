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
#include "MQTTSi91x_fake_functions.h"
}

// --- NetworkInit Tests ---
TEST(MQTTSi91x_unit_tests, SetsTCPFunctionPointers)
{
  Network n;
  memset(&n, 0, sizeof(n));
  n.transport_type = MQTT_TRANSPORT_TCP;
  NetworkInit(&n);
  EXPECT_NE(n.mqttread, nullptr);
  EXPECT_NE(n.mqttwrite, nullptr);
  EXPECT_NE(n.disconnect, nullptr);
}

TEST(MQTTSi91x_unit_tests, SetsWebSocketFunctionPointers)
{
  Network n;
  memset(&n, 0, sizeof(n));
  n.transport_type = MQTT_TRANSPORT_WEBSOCKET;
  NetworkInit(&n);
  EXPECT_NE(n.mqttread, nullptr);
  EXPECT_NE(n.mqttwrite, nullptr);
  EXPECT_NE(n.disconnect, nullptr);
}

// --- NetworkConnect Tests ---
TEST(MQTTSi91x_unit_tests, NullNetworkReturnsError)
{
  int rc = NetworkConnect(nullptr, 0, (char *)"host", 1883, 0, false);
  EXPECT_EQ(rc, NETWORK_ERROR_NULL_STRUCTURE);
}

TEST(MQTTSi91x_unit_tests, NullAddressReturnsError)
{
  Network n;
  memset(&n, 0, sizeof(n));
  int rc = NetworkConnect(&n, 0, nullptr, 1883, 0, false);
  EXPECT_EQ(rc, NETWORK_ERROR_NULL_ADDRESS);
}

TEST(MQTTSi91x_unit_tests, TCP_CallsTCPHandler_Effect)
{
  Network n;
  memset(&n, 0, sizeof(n));
  n.transport_type = MQTT_TRANSPORT_TCP;
  int rc           = NetworkConnect(&n, 1, (char *)"host", 1883, 1234, true);
  EXPECT_EQ(rc, 0);
}

TEST(MQTTSi91x_unit_tests, WebSocket_CallsWebSocketHandler)
{
  Network n;
  memset(&n, 0, sizeof(n));
  n.transport_type = MQTT_TRANSPORT_WEBSOCKET;
  int rc           = NetworkConnect(&n, 1, (char *)"host", 1883, 1234, true);
  EXPECT_EQ(rc, 0);
}

TEST(MQTTSi91x_unit_tests, InvalidTypeReturnsError)
{
  Network n;
  memset(&n, 0, sizeof(n));
  n.transport_type = (mqtt_transport_t)0xFF; // Invalid type
  int rc           = NetworkConnect(&n, 0, (char *)"host", 1883, 0, false);
  EXPECT_EQ(rc, NETWORK_ERROR_INVALID_TYPE);
}

// --- NetworkDisconnect Tests ---
TEST(MQTTSi91x_unit_tests, NullNetworkPointer_DoesNothing)
{
  NetworkDisconnect(nullptr); // No assertion needed, just ensure no crash
}

TEST(MQTTSi91x_unit_tests, NullDisconnectPointer_DoesNothing)
{
  Network n    = {};
  n.disconnect = nullptr;
  NetworkDisconnect(&n); // No assertion needed, just ensure no crash
}

TEST(MQTTSi91x_unit_tests, CallsDisconnectFunction)
{
  Network n    = {};
  n.disconnect = mqtt_fake_disconnect;
  RESET_FAKE(mqtt_fake_disconnect);
  NetworkDisconnect(&n);
  EXPECT_EQ(mqtt_fake_disconnect_fake.call_count, 1);
  EXPECT_EQ(mqtt_fake_disconnect_fake.arg0_val, &n);
}