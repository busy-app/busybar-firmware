#include <gtest/gtest.h>
extern "C" {
#include "MQTTSi91x_lwip_fake_functions.h"
}

// Custom fake function for sl_redirect_log
void sl_redirect_log(const char *format, ...)
{
  va_list args;
  va_start(args, format);
  vprintf(format, args); // Simulate logging
  va_end(args);
}

// --- NetworkInit Tests ---
TEST(MQTTSi91x_lwip, SetsTCPFunctionPointers)
{
    Network n = {};
    NetworkInit(&n);
    EXPECT_NE(n.mqttread, nullptr);
    EXPECT_NE(n.mqttwrite, nullptr);
    EXPECT_NE(n.disconnect, nullptr);
}

// --- NetworkConnect Tests ---
TEST(MQTTSi91x_lwip, NullNetworkReturnsError)
{
    int rc = NetworkConnect(nullptr, 0, (char *)"host", 1883, 0, false);
    EXPECT_EQ(rc, NETWORK_ERROR_NULL_STRUCTURE);
}

TEST(MQTTSi91x_lwip, NullAddressReturnsError)
{
    Network n = {};
    int rc = NetworkConnect(&n, 0, nullptr, 1883, 0, false);
    EXPECT_EQ(rc, NETWORK_ERROR_NULL_ADDRESS);
}

TEST(MQTTSi91x_lwip, InvalidTypeReturnsError)
{
    Network n = {};
    n.transport_type = (mqtt_transport_t)0xFF;
    int rc = NetworkConnect(&n, 0, (char *)"host", 1883, 0, false);
    EXPECT_EQ(rc, NETWORK_ERROR_INVALID_TYPE);
}

TEST(MQTTSi91x_lwip, TCP_CallsTCPHandler_Effect)
{
    Network n;
    memset(&n, 0, sizeof(n));
    n.transport_type = MQTT_TRANSPORT_TCP;
    int rc = NetworkConnect(&n, 1, (char *)"host", 1883, 1234, false);
    EXPECT_EQ(rc, 0);
}

// --- NetworkDisconnect Tests ---
TEST(MQTTSi91x_lwip, NullNetworkPointer_DoesNothing)
{
    NetworkDisconnect(nullptr);
}

TEST(MQTTSi91x_lwip, NullDisconnectPointer_DoesNothing)
{
    Network n = {};
    n.disconnect = nullptr;
    NetworkDisconnect(&n);
}

TEST(MQTTSi91x_lwip, CallsDisconnectFunction)
{
    Network n = {};
    n.disconnect = mqtt_fake_disconnect;
    RESET_FAKE(mqtt_fake_disconnect);
    NetworkDisconnect(&n);
    EXPECT_EQ(mqtt_fake_disconnect_fake.call_count, 1);
    EXPECT_EQ(mqtt_fake_disconnect_fake.arg0_val, &n);
}