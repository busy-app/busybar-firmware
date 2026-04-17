/*******************************************************************************
* @file  network_sl_api_wrapper.c
* @brief 
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

#include <string.h>
#include "network_interface.h"
#include "socket.h"
#include "sl_si91x_socket_support.h"
#include "netinet6_in6.h"
#include "sl_net_dns.h"
#include "sl_constants.h"
#include "sl_si91x_constants.h"
#include "sl_si91x_socket_constants.h"
#include "sl_si91x_socket_utility.h"
#include "sl_bsd_utility.h"
#include "sl_si91x_socket_types.h"
#include "sl_si91x_core_utilities.h"
#include "sl_si91x_protocol_types.h"
#include "errno.h"
#include "aws_iot_error.h"
#include "aws_iot_config.h"
#include "cmsis_os2.h"
#include "sl_string.h"
#include "sl_wifi_callback_framework.h"
//Application level variables for QOS1. Extern these variables in the application for QOS1.
volatile uint8_t pub_state;
volatile uint8_t qos1_publish_handle;
volatile uint8_t select_given;
osSemaphoreId_t select_sem;


/******************************************************
*                    Macros
******************************************************/
#define IPV6              (1 << 0)
#define CLIENT_PORT       4002
#define MAX_DNS_REQ_COUNT 5
#define TLS_SOCKET        2
#define SL_CERT_INDEX_0   0
#define MQTT_TLS_PORT 443
#define ALPN_AMZN_MQTT_CA "x-amzn-mqtt-ca"
#define SEMAPHORE_TIMEOUT_MS 640000 // Timeout in milliseconds (640 seconds) for QoS1 PUBACK semaphore acquisition.
                                    // This accounts for maximum TCP retransmission timeout (AWS_IOT_MAX_RETRANSMISSION_TIMEOUT = 128s)
                                    // with default maximum retries (SL_SO_MAXRETRY = 10), allowing sufficient time for
                                    // PUBACK reception before timing out.

/******************************************************
*               Variable Declarations
******************************************************/
static int32_t sli_si91x_get_aws_error(int32_t status);
static int32_t sli_si91x_connect_to_network(Network *n, uint8_t flags, const sl_ip_address_t *addr, int dst_port, int src_port);
int32_t _iot_tls_verify_cert(const void *data, const int *crt, int depth, const uint32_t *flags);


/******************************************************
*               Function Definitions
******************************************************/
static int32_t sli_si91x_get_aws_error(int32_t status)
{
  switch (status) {
    case EBADF:
    case EINVAL:
    case EISCONN:
    case EFAULT:
    case ENOBUFS:
    case EPROTOTYPE:
    case EAFNOSUPPORT:
    case EADDRINUSE:
    case ENOPROTOOPT:
      return TCP_CONNECTION_ERROR;
    // TODO: Handle a mapping to convert Si91x firmware errors to AWS errors.
    case SL_STATUS_SI91X_SOCKET_READ_TIMEOUT:
      return MQTT_NOTHING_TO_READ;
    default:
      return FAILURE;
  }
}

int32_t _iot_tls_verify_cert(const void *data, const int *crt, int depth, const uint32_t *flags)
{
  UNUSED_PARAMETER(data);
  UNUSED_PARAMETER(crt);
  UNUSED_PARAMETER(depth);
  UNUSED_PARAMETER(flags);
  // TODO: Implement this function if external stack is used.
  // NOTE: Verification of certificate takes place in firmware when internal stack feature is used. So API returns SUCCESS.
  return SUCCESS;
}

IoT_Error_t iot_tls_init(Network *pNetwork,
                         char *pRootCALocation,
                         char *pDeviceCertLocation,
                         char *pDevicePrivateKeyLocation,
                         char *pDestinationURL,
                         uint16_t DestinationPort,
                         uint32_t timeout_ms,
                         bool ServerVerificationFlag)
{
  pNetwork->tlsConnectParams.DestinationPort           = DestinationPort;
  pNetwork->tlsConnectParams.pDestinationURL           = pDestinationURL;
  pNetwork->tlsConnectParams.pDeviceCertLocation       = pDeviceCertLocation;
  pNetwork->tlsConnectParams.pDevicePrivateKeyLocation = pDevicePrivateKeyLocation;
  pNetwork->tlsConnectParams.pRootCALocation           = pRootCALocation;
  pNetwork->tlsConnectParams.timeout_ms                = timeout_ms;
  pNetwork->tlsConnectParams.ServerVerificationFlag    = ServerVerificationFlag;

  pNetwork->connect     = iot_tls_connect;
  pNetwork->read        = iot_tls_read;
  pNetwork->write       = iot_tls_write;
  pNetwork->disconnect  = iot_tls_disconnect;
  pNetwork->isConnected = iot_tls_is_connected;
  pNetwork->destroy     = iot_tls_destroy;

  return SUCCESS;
}

IoT_Error_t iot_tls_is_connected(Network *pNetwork)
{
  UNUSED_PARAMETER(pNetwork);
  /* Use this to add implementation which can check for physical layer disconnect */
  return NETWORK_PHYSICAL_LAYER_CONNECTED;
}

static int32_t sli_si91x_connect_to_network(Network *n, uint8_t flags, const sl_ip_address_t *addr, int dst_port, int src_port)
{
  int32_t status  =0;
#ifdef SLI_SI91X_ENABLE_IPV6
  struct sockaddr_in6 client_addr = { 0 };
  struct sockaddr_in6 server_addr = { 0 };
  uint32_t socket_length = sizeof(struct sockaddr_in6);
  client_addr.sin6_family = AF_INET6;
  client_addr.sin6_port   = src_port;
  server_addr.sin6_family = AF_INET6;
  server_addr.sin6_port   = dst_port;
  memcpy(&server_addr.sin6_addr.__u6_addr.__u6_addr8, &addr->ip.v6.value, SL_IPV6_ADDRESS_LENGTH);
  n->socket_id = socket( AF_INET6 , SOCK_STREAM, IPPROTO_TCP);
#else
  struct sockaddr_in client_addr   = { 0 };
  struct sockaddr_in server_addr   = { 0 };
  uint32_t socket_length = sizeof(struct sockaddr_in);
  client_addr.sin_family = AF_INET;
  client_addr.sin_port   = (uint16_t)src_port;
  server_addr.sin_family = AF_INET;
  server_addr.sin_port   = (uint16_t)dst_port;
  memcpy(&server_addr.sin_addr.s_addr, &addr->ip.v4.value, SL_IPV4_ADDRESS_LENGTH);
  n->socket_id = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
#endif
  if (n->socket_id < 0) {
    return sli_si91x_get_aws_error(errno);
  }

  if (flags & TLS_SOCKET) {
    if (setsockopt(n->socket_id, SOL_TCP, TCP_ULP, TLS, sizeof(TLS))) {
      return sli_si91x_get_aws_error(errno);
    }

    uint8_t certificate_index = SL_CERT_INDEX_0;
    if (setsockopt(n->socket_id, SOL_SOCKET, SL_SO_CERT_INDEX, &certificate_index, sizeof(certificate_index))) {
      return sli_si91x_get_aws_error(errno);
    }

    // Configure max retransmission timeout using configurable macro.
    int max_retry = AWS_IOT_MAX_RETRANSMISSION_TIMEOUT;
    if ( setsockopt(n->socket_id, SOL_TCP, SO_MAX_RETRANSMISSION_TIMEOUT_VALUE, &max_retry, sizeof(max_retry))){
                return sli_si91x_get_aws_error(errno);
    }

    if(dst_port == MQTT_TLS_PORT) {
      int socket_return_value = 0;

      // Calculate the length of the ALPN data
      uint16_t alpn_length = (uint16_t)sl_strlen(ALPN_AMZN_MQTT_CA);

      // Allocate memory for the sl_si91x_socket_type_length_value_t structure
      sl_si91x_socket_type_length_value_t *alpn_value =
        (sl_si91x_socket_type_length_value_t *)malloc(sizeof(sl_si91x_socket_type_length_value_t) + alpn_length);

      if (alpn_value == NULL) {
        SL_DEBUG_LOG("\r\nMemory allocation failed for ALPN value\r\n");
        return FAILURE;
      }

      // Set the type to 2 for ALPN extension
      alpn_value->type = SL_SI91X_TLS_EXTENSION_ALPN_TYPE;

      // Set the length of the ALPN data
      alpn_value->length = alpn_length;

      // Copy the ALPN data into the value field
      memcpy(alpn_value->value, ALPN_AMZN_MQTT_CA, alpn_length);

      socket_return_value = setsockopt(n->socket_id,
                                                      SOL_SOCKET,
                                                      SL_SO_TLS_ALPN,
                                                      alpn_value,
                                                      sizeof(sl_si91x_socket_type_length_value_t) + alpn_length);

      if (socket_return_value < 0) {
        SL_DEBUG_LOG("\r\nSet Socket option failed with bsd error: %d\r\n", errno);
        close(n->socket_id);
        free(alpn_value);
        return sli_si91x_get_aws_error(errno);
      }

      // Free the allocated memory after usage
      free(alpn_value);
    }
  }
  
  // Configure SNI (Server Name Indication)
        if (n->tlsConnectParams.pDestinationURL != NULL) {
          uint16_t sni_length = (uint16_t)sl_strlen(n->tlsConnectParams.pDestinationURL);

          // Allocate memory for the SNI configuration
          sl_si91x_socket_type_length_value_t *sni_value =
            (sl_si91x_socket_type_length_value_t *)malloc(sizeof(sl_si91x_socket_type_length_value_t) + sni_length);

          if (sni_value == NULL) {
            SL_DEBUG_LOG("\r\nMemory allocation failed for SNI value\r\n");
            return FAILURE;
          }

          // Set the type to SNI extension
          sni_value->type = SL_SI91X_TLS_EXTENSION_SNI_TYPE;

          // Set the length of the SNI data
          sni_value->length = sni_length;

          // Copy the SNI data (hostname) into the value field
          memcpy(sni_value->value, n->tlsConnectParams.pDestinationURL, sni_length);

          // Set the SNI option on the socket
          if (setsockopt(n->socket_id,
                         SOL_SOCKET,
                         SL_SO_TLS_SNI,
                         sni_value,
                         sizeof(sl_si91x_socket_type_length_value_t) + sni_length)
  < 0) {
            SL_DEBUG_LOG("\r\nSet Socket option for SNI failed with bsd error: %d\r\n", errno);
            close(n->socket_id);
            free(sni_value);
            return sli_si91x_get_aws_error(errno);
          }

          // Free the allocated memory after usage
          free(sni_value);
        }

  status = bind( n->socket_id, (struct sockaddr *)&client_addr, socket_length);
  if (status < 0) {
    close( n->socket_id);
    return sli_si91x_get_aws_error(errno);
  }

  status = connect( n->socket_id, (struct sockaddr *)&server_addr, socket_length);
  if (status < 0) {
    close( n->socket_id);
    return sli_si91x_get_aws_error(errno);
  }
  return SUCCESS;
}

IoT_Error_t iot_tls_connect(Network *pNetwork, TLSConnectParams *params)
{
  UNUSED_PARAMETER(params);
  int32_t status                     = 0;
  int8_t count                       = MAX_DNS_REQ_COUNT;
  sl_ip_address_t dns_query_response = { 0 };

  do {
#ifdef SLI_SI91X_ENABLE_IPV6
	  status = sl_net_dns_resolve_hostname(pNetwork->tlsConnectParams.pDestinationURL,
	                                        SLI_WIFI_WAIT_FOR_DNS_RESOLUTION,
	  									                    SL_NET_DNS_TYPE_IPV6, 
	                                        &dns_query_response);
#else
    status = sl_net_dns_resolve_hostname(pNetwork->tlsConnectParams.pDestinationURL,
                                       SLI_WIFI_WAIT_FOR_DNS_RESOLUTION,
                                       SL_NET_DNS_TYPE_IPV4, 
                                       &dns_query_response);
#endif
    if (status == SL_STATUS_OK) {
      break;
    }
    count--;
  } while (count > 0);

  if (status) {
    return NETWORK_ERR_NET_UNKNOWN_HOST;
  }

  status = sli_si91x_connect_to_network(pNetwork,
						               	TLS_SOCKET,
                            &dns_query_response,
                            pNetwork->tlsConnectParams.DestinationPort,
                            CLIENT_PORT);
  return status;
}

IoT_Error_t iot_tls_write(Network *pNetwork, unsigned char *pMsg, size_t len, Timer *timer, size_t *written_len)
{
  size_t total_bytes_written = 0; // Use size_t for total_bytes_written to match len's type
  ssize_t bytes_written = 0;
  size_t chunk_size = 0;
  size_t tls_buffer_size = sl_si91x_get_socket_mss(pNetwork->socket_id);

  if (len == 0) {
    return MQTT_TX_BUFFER_TOO_SHORT_ERROR; // Adjusted to only check for 0, as len cannot be negative
  }
  
   do {
    chunk_size = (len - total_bytes_written < tls_buffer_size) ? (len - total_bytes_written) : tls_buffer_size;
    bytes_written = send(pNetwork->socket_id, pMsg + total_bytes_written, chunk_size, 0);
    if (bytes_written <= 0) {
      // If buffer is full, try again
      if (errno == ENOBUFS)
        continue;
      // If send returns 0 or a negative value, treat it as an error
      return NETWORK_SSL_WRITE_ERROR;
    }
    total_bytes_written += bytes_written; // Accumulate bytes written 
  } while (total_bytes_written < len && !has_timer_expired(timer));
  
  *written_len = total_bytes_written; // Update *written_len with the total bytes written
  return (total_bytes_written == len) ? SUCCESS : NETWORK_SSL_WRITE_TIMEOUT_ERROR; // Check total_bytes_written against len
}

IoT_Error_t iot_tls_read(Network *pNetwork, unsigned char *pMsg, size_t len, Timer *timer, size_t *read_len)
{
  int32_t status          = 0;
  int32_t bytes_read      = 0;
  size_t total_bytes_read = 0;
  size_t temp_len         = len;
  size_t chunk_size       = 0;
  size_t tls_buffer_size  = 0;
  sl_si91x_time_value timeout = { 0 };

  if (len <= 0) {
    return MQTT_RX_BUFFER_TOO_SHORT_ERROR;
  }

  timeout.tv_usec = timer->timeout * 1000;

  status = setsockopt(pNetwork->socket_id, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
  if (status) {
    return sli_si91x_get_aws_error(errno);
  }

  if(pub_state == 1){    //This check is for handling PUBACK in QOS1
  // Attempt to acquire the semaphore with a timeout.
  osStatus_t select_status = osSemaphoreAcquire(select_sem, SEMAPHORE_TIMEOUT_MS);

  // Check if semaphore acquisition timed out.
  if (select_status == osErrorTimeout) {
      SL_DEBUG_LOG("Error: Semaphore acquisition timed out. Puback not received.\n");
      return MQTT_REQUEST_TIMEOUT_ERROR; // Return an error code indicating that the MQTT request timed out.
  }
  if (select_status == osErrorParameter) {
    SL_DEBUG_LOG("Error: Invalid parameter in semaphore acquisition.\n");
    return NETWORK_SSL_READ_ERROR; // Return a generic failure code for parameter error.
  }
  qos1_publish_handle = 1;
  pub_state = 0;
  select_given        = 0;
  }
  tls_buffer_size = sl_si91x_get_socket_mss(pNetwork->socket_id);
  do {
    // Calculate the next chunk size, ensuring it does not exceed the remaining length or MSS.
    chunk_size = (temp_len - total_bytes_read < tls_buffer_size) ? (temp_len - total_bytes_read) : tls_buffer_size;
    // Iteratively receive up to chunk_size bytes from the socket into pMsg.
    bytes_read = recv(pNetwork->socket_id, pMsg, chunk_size, 0);
    if (bytes_read == 0) {
      if (total_bytes_read != 0) {
        *read_len = total_bytes_read;
        return SUCCESS;
      }
      return NETWORK_SSL_READ_ERROR;
    } else if (bytes_read < 0) {
      if (total_bytes_read != 0) {
        *read_len = total_bytes_read;
        return SUCCESS;
      }
      return sli_si91x_get_aws_error(sl_wifi_get_saved_firmware_status());
    }

    total_bytes_read += bytes_read;
    pMsg += bytes_read;
    temp_len -= bytes_read;

  } while ((temp_len > 0) && !has_timer_expired(timer));

  *read_len = total_bytes_read;
  return (total_bytes_read >= len) ? SUCCESS : NETWORK_SSL_READ_TIMEOUT_ERROR;
}

IoT_Error_t iot_tls_disconnect(Network *pNetwork)
{
  int32_t status = close(pNetwork->socket_id);
  if (status) {
    return sli_si91x_get_aws_error(status);
  }
  return SUCCESS;
}

IoT_Error_t iot_tls_destroy(Network *pNetwork)
{
  UNUSED_PARAMETER(pNetwork);
  // TODO: Implement this function when external TLS stack is used.
  // NOTE: TLS functionality takes place in firmware when internal stack feature is used. So based on the previous errno value from iot_tls_disconnect() status we return SUCCESS/FAILURE.
  if (errno)
    return FAILURE;
  else
    return SUCCESS;
}
