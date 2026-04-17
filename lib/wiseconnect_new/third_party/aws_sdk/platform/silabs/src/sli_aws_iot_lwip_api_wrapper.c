/*******************************************************************************
* @file  sli_aws_iot_lwip_api_wrapper.c
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
#include <string.h>
#include "cmsis_os2.h"
#include "aws_iot_error.h"
#include "network_interface.h"

#include "sl_constants.h"
#include "sl_si91x_constants.h"
#include "sl_si91x_core_utilities.h"
#include "sl_si91x_protocol_types.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"

#include "mbedtls/net_sockets.h"
#include "mbedtls/ssl.h"
#include "mbedtls/x509.h"
#include "mbedtls/x509_crt.h"
#include "mbedtls/error.h"
#include "mbedtls/pk.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/entropy.h"
#include "mbedtls/debug.h"
#include "psa/crypto.h"


//Application level variables for QOS1. Extern these variables in the application for QOS1.
volatile uint8_t pub_state, qos1_publish_handle, select_given;
osSemaphoreId_t select_sem;


/******************************************************
*                    Macros
******************************************************/

#define SLI_CLIENT_PORT   4002
#define SLI_MQTT_TLS_PORT 443

#define ALPN_AMZN_MQTT_CA "x-amzn-mqtt-ca"
#define SLI_TAG "aws_iot"

#define SLI_DNS_TIMEOUT_MS 20000

#if LWIP_IPV6
#define SLI_DNS_SERVER_IPV6 "2001:4860:4860::8888"
#else
#define SLI_DNS_SERVER_IP   "8.8.8.8"
#endif

#define SLI_SSL_WRITE_RETRY_TIMEOUT_MS 10
#define SLI_SSL_READ_RETRY_TIMEOUT_MS  10

#define min(a,b) (a<b)?a:b
#define max(a,b) (a>b)?a:b

#define SLI_TLS_DEBUG 0

/******************************************************
*               Variable Declarations
******************************************************/
volatile int dns_resolved=0;
static ip_addr_t dns_query_response = { 0 };
static uint8_t psa_crypto_initialized = 0;

mbedtls_ctr_drbg_context ctr_drbg;
mbedtls_entropy_context entropy;
mbedtls_ssl_context ssl;
mbedtls_ssl_config conf;
mbedtls_x509_crt cacert;
mbedtls_pk_context pkey;
mbedtls_x509_crt device_cert;

/******************************************************
*               Function Definitions
******************************************************/
#if SLI_TLS_DEBUG
void sli_mbedtls_debug_print(void *ctx, int level, const char *file, int line, const char *str)
{
	((void) level);
	(void)file;
	(void)line;
	fprintf((FILE *) ctx, "%s", str);
}
#endif

// Initialize the hardware entropy source
static int sli_mbedtls_hardware_poll(void *data, unsigned char *output, size_t len, size_t *olen)
{
  (void)data;
  for (size_t i = 0; i < len; i++) {
    output[i] = rand() % 256;
  }
  *olen = len;
  return 0;
}

// TLS send and receive mapping functions
static int sli_ssl_send(void *ctx, const unsigned char *buf, size_t len)
{
  return lwip_write(*(int *)ctx, buf, len);
}

static int sli_ssl_recv(void *ctx, unsigned char *buf, size_t len)
{
  return lwip_read(*(int *)ctx, buf, len);
}

static int sli_ssl_recv_timeout(void *ctx, unsigned char *buf, size_t len, uint32_t timeout)
{
  int ret;
  struct timeval tv;
  fd_set read_fds;
  int fd = *(int *)ctx;

  FD_ZERO(&read_fds);
  FD_SET(fd, &read_fds);

  tv.tv_sec = timeout / 1000;
  tv.tv_usec = (timeout % 1000) * 1000;

  ret = select(fd + 1, &read_fds, NULL, NULL, &tv);
  if (ret == 0) {
    return MBEDTLS_ERR_SSL_TIMEOUT;
  }
  if (ret < 0) {
    return MBEDTLS_ERR_NET_RECV_FAILED;
  }

  return lwip_read(fd, buf, len);
}

void sli_dns_callback(const char *name, const ip_addr_t *ipaddr, void *callback_arg)
{
  if (ipaddr != NULL) {
#if LWIP_IPV6 & LWIP_IPV4
    // For Dual Stack
    char addr_str[INET6_ADDRSTRLEN] = { 0 };
    inet_ntop(AF_INET6, &ipaddr->u_addr.ip6.addr, addr_str, sizeof(addr_str));
#elif LWIP_IPV6
    // For IPv6
    char addr_str[INET6_ADDRSTRLEN] = { 0 };
    inet_ntop(AF_INET6, &ipaddr->addr, addr_str, sizeof(addr_str));
#else
    // For IPv4
    char addr_str[INET_ADDRSTRLEN] = { 0 };
    inet_ntop(AF_INET, &ipaddr->addr, addr_str, sizeof(addr_str));
#endif
    printf("\r\nResolved IP address for %s: %s\r\n", name, addr_str);

    dns_resolved = 1;
    dns_query_response = *ipaddr;
  } else {
    printf("\r\nDNS resolution failed for %s, error:%d\r\n", name, (int)callback_arg);
  }
}

static int32_t sli_si91x_connect_to_network(Network *pNetwork, ip_addr_t *server_addr, int dst_port, int src_port)
{
  int32_t status = 0;
  int sockfd;
#if LWIP_IPV6
  struct sockaddr_in6 server6;
  struct sockaddr_in6 src_addr6;
#else
  struct sockaddr_in server;
  struct sockaddr_in src_addr;
#endif

  // Create socket
#if LWIP_IPV6
  sockfd = lwip_socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
#else
  sockfd = lwip_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
#endif
  if (sockfd < 0) {
    return sockfd;
  }

  // Prepare source address
#if LWIP_IPV6
  src_addr6.sin6_family = AF_INET6;
  src_addr6.sin6_port = htons(src_port);
  src_addr6.sin6_addr = in6addr_any;

  // Bind socket to source address
  status = lwip_bind(sockfd, (struct sockaddr *)&src_addr6, sizeof(src_addr6));
#else
  src_addr.sin_family = AF_INET;
  src_addr.sin_port = htons(src_port);
  src_addr.sin_addr.s_addr = INADDR_ANY;

  // Bind socket to source address
  status = lwip_bind(sockfd, (struct sockaddr *)&src_addr, sizeof(src_addr));
#endif
  if (status < 0) {
    lwip_close(sockfd);
    return status;
  }

  // Store socket in pNetwork
  pNetwork->socket_id = sockfd;

  // Prepare server address
#if LWIP_IPV6 & LWIP_IPV4
  server6.sin6_family = AF_INET6;
  server6.sin6_port = htons(dst_port);
  memcpy(server6.sin6_addr.un.u32_addr, server_addr->u_addr.ip6.addr, sizeof(server_addr->u_addr.ip6.addr));

  // Connect socket to server
  status = lwip_connect(sockfd, (struct sockaddr *)&server6, sizeof(server6));
#elif LWIP_IPV6
  server6.sin6_family = AF_INET6;
  server6.sin6_port = htons(dst_port);
  memcpy(server6.sin6_addr.un.u32_addr, server_addr->addr, sizeof(server6.sin6_addr.un.u32_addr));

  // Connect socket to server
  status = lwip_connect(sockfd, (struct sockaddr *)&server6, sizeof(server6));
#else
  server.sin_family = AF_INET;
  server.sin_port = htons(dst_port);
  server.sin_addr.s_addr = server_addr->addr;

  // Connect socket to server
  status = lwip_connect(sockfd, (struct sockaddr *)&server, sizeof(server));
#endif
  if (status < 0) {
    lwip_close(sockfd);
    return status;
  }

  return SUCCESS;
}

/************************* TLS Wrapper Functions *************************/

static int _iot_tls_verify_cert(void *data, mbedtls_x509_crt *crt, int depth, uint32_t *flags)
{
  (void) data;
  (void) crt;
  (void) depth;
  (void) flags;
#if SLI_TLS_DEBUG
  char buf[1024];

  printf("Verify requested for (Depth %d):\r\n", depth);
  mbedtls_x509_crt_info(buf, sizeof(buf) - 1, "", crt);
  printf("\r\n%s\r\n", buf);

  if((*flags) == 0) {
    printf("\r\nThis certificate has no flags\r\n");
  } else {
    printf("\r\nVerify result:%lx\r\n", *flags);
  }
#endif
  return SUCCESS;
}

IoT_Error_t iot_tls_init(Network *pNet,
                         char *pRootCALocation,
                         char *pDeviceCertLocation,
                         char *pDevicePrivateKeyLocation,
                         char *pDestinationURL,
                         uint16_t DestinationPort,
                         uint32_t timeout_ms,
                         bool ServerVerificationFlag)
{
  /* Set up the TLS connect parameters */
  pNet->tlsConnectParams.DestinationPort           = DestinationPort;
  pNet->tlsConnectParams.pDestinationURL           = pDestinationURL;
  pNet->tlsConnectParams.pDeviceCertLocation       = pDeviceCertLocation;
  pNet->tlsConnectParams.pDevicePrivateKeyLocation = pDevicePrivateKeyLocation;
  pNet->tlsConnectParams.pRootCALocation           = pRootCALocation;
  pNet->tlsConnectParams.timeout_ms                = timeout_ms;
  pNet->tlsConnectParams.ServerVerificationFlag    = ServerVerificationFlag;

  /* Set up the network interface */
  pNet->connect     = iot_tls_connect;
  pNet->read        = iot_tls_read;
  pNet->write       = iot_tls_write;
  pNet->disconnect  = iot_tls_disconnect;
  pNet->isConnected = iot_tls_is_connected;
  pNet->destroy     = iot_tls_destroy;

  return SUCCESS;
}

IoT_Error_t iot_tls_is_connected(Network *pNetwork)
{
  UNUSED_PARAMETER(pNetwork);
  /* Use this to add implementation which can check for physical layer disconnect */
  return NETWORK_PHYSICAL_LAYER_CONNECTED;
}

IoT_Error_t iot_tls_connect(Network *pNetwork, TLSConnectParams *params)
{
  UNUSED_PARAMETER(params);
  int32_t status = 0;
  ip_addr_t dns_server;
  char info_buf[256];

  // Set DNS server
#if LWIP_IPV6
  if (inet_pton(AF_INET6, SLI_DNS_SERVER_IPV6, &dns_server) == 1) {
    dns_setserver(0, &dns_server);
    printf("\r\nIPv6 DNS server set to %s\r\n", SLI_DNS_SERVER_IPV6);
  } else {
    printf("\r\nInvalid IPv6 DNS address\r\n");
    return NETWORK_ERR_NET_UNKNOWN_HOST;
  }
#else
  if (inet_pton(AF_INET, SLI_DNS_SERVER_IP, &dns_server) == 1) {
    dns_setserver(0, &dns_server);
    printf("\r\nIPv4 DNS server set to %s\r\n", SLI_DNS_SERVER_IP);
  } else {
    printf("\r\nInvalid IPv4 DNS address\r\n");
    return NETWORK_ERR_NET_UNKNOWN_HOST;
  }
#endif

  status = dns_gethostbyname(pNetwork->tlsConnectParams.pDestinationURL, &dns_query_response, &sli_dns_callback, NULL);
  if (status == ERR_OK) {
	  sli_dns_callback(pNetwork->tlsConnectParams.pDestinationURL, &dns_query_response, NULL);
  }

  Timer dns_timer;
  init_timer(&dns_timer);
  dns_resolved = 0;
  countdown_ms(&dns_timer, SLI_DNS_TIMEOUT_MS);

  while(!dns_resolved) {
    if (has_timer_expired(&dns_timer)) {
      return NETWORK_ERR_NET_UNKNOWN_HOST;
    }
    osDelay(100); // Small delay to yield CPU
  }

  status = sli_si91x_connect_to_network(pNetwork, &dns_query_response, pNetwork->tlsConnectParams.DestinationPort, SLI_CLIENT_PORT);
  if (status != SUCCESS) {
    return status;
  }

  // Initialize PSA crypto (required for sisdk mbedTLS components)
  if (!psa_crypto_initialized) {
    status = psa_crypto_init();
    if (status != PSA_SUCCESS) {
      printf("\r\n[TLS] PSA crypto init failed: %ld\r\n", status);
      // Continue anyway as some platforms may not need it
    } else {
      printf("\r\n[TLS] PSA crypto init SUCCESS\r\n");
      psa_crypto_initialized = 1;
    }
  }

  // Initialize mbedTLS entropy and random number generator
  mbedtls_entropy_init(&entropy);
  mbedtls_ctr_drbg_init(&ctr_drbg);
  // Add an entropy source
  status = mbedtls_entropy_add_source(&entropy, sli_mbedtls_hardware_poll, NULL, MBEDTLS_ENTROPY_MIN_HARDWARE, MBEDTLS_ENTROPY_SOURCE_STRONG);
  if (status != 0) {
    return NETWORK_MBEDTLS_ERR_CTR_DRBG_ENTROPY_SOURCE_FAILED;
  }
  // Seed entropy
  status = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, (const unsigned char*)SLI_TAG, strlen(SLI_TAG));
  if (status != 0) {
    return NETWORK_MBEDTLS_ERR_CTR_DRBG_ENTROPY_SOURCE_FAILED;
  }

  // Initialize mbedTLS SSL context
  mbedtls_x509_crt_init(&cacert);
  mbedtls_x509_crt_init(&device_cert);
  mbedtls_pk_init(&pkey);

  // Load certificates and private key
  status = mbedtls_x509_crt_parse(&cacert, (const unsigned char *)pNetwork->tlsConnectParams.pRootCALocation, strlen(pNetwork->tlsConnectParams.pRootCALocation)+1);
  if ( status != 0) {
    return NETWORK_X509_ROOT_CRT_PARSE_ERROR;
  }

  status = mbedtls_x509_crt_parse(&device_cert, (const unsigned char *)pNetwork->tlsConnectParams.pDeviceCertLocation, strlen(pNetwork->tlsConnectParams.pDeviceCertLocation)+1);
  if ( status != 0) {
    return NETWORK_X509_DEVICE_CRT_PARSE_ERROR;
  }

  status = mbedtls_pk_parse_key(&pkey, (const unsigned char *)pNetwork->tlsConnectParams.pDevicePrivateKeyLocation, strlen(pNetwork->tlsConnectParams.pDevicePrivateKeyLocation)+1, (const unsigned char *)"", 0, mbedtls_ctr_drbg_random, NULL);
  if ( status != 0) {
    return NETWORK_PK_PRIVATE_KEY_PARSE_ERROR;
  }

  // Set up SSL configuration
  mbedtls_ssl_init(&ssl);
  mbedtls_ssl_config_init(&conf);
  mbedtls_ssl_config_defaults(&conf, MBEDTLS_SSL_IS_CLIENT, MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT);

  // Debug
#if SLI_TLS_DEBUG
  mbedtls_ssl_conf_dbg(&conf, sli_mbedtls_debug_print, stdout);
  mbedtls_debug_set_threshold(SLI_TLS_DEBUG);
#endif
  
  mbedtls_ssl_conf_verify(&conf, &_iot_tls_verify_cert, NULL);

  if(pNetwork->tlsConnectParams.ServerVerificationFlag == true) {
	  mbedtls_ssl_conf_authmode(&conf, MBEDTLS_SSL_VERIFY_REQUIRED);
  } else {
	  mbedtls_ssl_conf_authmode(&conf, MBEDTLS_SSL_VERIFY_OPTIONAL);
  }

  mbedtls_ssl_conf_rng(&conf, mbedtls_ctr_drbg_random, &ctr_drbg);

  mbedtls_ssl_conf_ca_chain(&conf, &cacert, NULL);
  status = mbedtls_ssl_conf_own_cert(&conf, &device_cert, &pkey);
  if (status != 0) {
    return NETWORK_SSL_CERT_ERROR;
  }

  mbedtls_ssl_conf_read_timeout(&conf, pNetwork->tlsConnectParams.timeout_ms);

  if (pNetwork->tlsConnectParams.DestinationPort == SLI_MQTT_TLS_PORT) {
	  const char *alpnProtocols[] = { ALPN_AMZN_MQTT_CA, NULL };
	  status = mbedtls_ssl_conf_alpn_protocols(&conf, alpnProtocols);
	  if (status != 0) {
		  return NETWORK_SSL_INIT_ERROR;
	  }
  }

  // Set SSL context
  status = mbedtls_ssl_setup(&ssl, &conf);
  if (status != 0) {
    return NETWORK_SSL_INIT_ERROR;
  }
  
  status = mbedtls_ssl_set_hostname(&ssl, pNetwork->tlsConnectParams.pDestinationURL);
  if(status != 0){
	  return NETWORK_SSL_INIT_ERROR;
  }
  
  mbedtls_ssl_set_bio(&ssl, &pNetwork->socket_id, sli_ssl_send, sli_ssl_recv, sli_ssl_recv_timeout);

  printf("\r\nPerforming the SSL/TLS handshake...\r\n");
  while((status = mbedtls_ssl_handshake(&ssl)) != 0) {
    if(status != MBEDTLS_ERR_SSL_WANT_READ && status != MBEDTLS_ERR_SSL_WANT_WRITE) {
      printf("\r\nFailed! mbedtls_ssl_handshake returned -0x%lx\r\n", -status);
      if(status == MBEDTLS_ERR_X509_CERT_VERIFY_FAILED) {
        printf("\r\nUnable to verify the server's certificate.\r\n");
      }
      return SSL_CONNECTION_ERROR;
    }
  }
  printf("\r\nSSL/TLS handshake SUCCESS\r\n");

#if SLI_TLS_DEBUG
  printf("\r\nProtocol is %s Ciphersuite is %s\r\n", mbedtls_ssl_get_version(&ssl), mbedtls_ssl_get_ciphersuite(&ssl));
  status = mbedtls_ssl_get_record_expansion(&ssl);
  if(status >= 0) {
    printf("\r\nRecord expansion is %ld\r\n", status);
  } else {
    printf("\r\nRecord expansion is unknown (compression)\r\n");
  }
#endif

  printf("\r\nVerifying peer server certificate...\r\n");
  if(pNetwork->tlsConnectParams.ServerVerificationFlag == true) {
    status = mbedtls_ssl_get_verify_result(&ssl);
    if(status != 0) {
      mbedtls_x509_crt_verify_info(info_buf, sizeof(info_buf), "  ! ", status);
      printf("\r\n%s\r\n", info_buf);
      return SSL_CONNECTION_ERROR;
    } else {
      status = SUCCESS;
    }
  } else {
    printf("\r\nServer Verification skipped\r\n");
    status = SUCCESS;
  }
  printf("\r\nPeer server certificate verification SUCCESS\r\n");

#if SLI_TLS_DEBUG
  if (mbedtls_ssl_get_peer_cert(&ssl) != NULL) {
    printf("\r\nPeer certificate information:\r\n");
    mbedtls_x509_crt_info(info_buf, sizeof(info_buf) - 1, "      ", mbedtls_ssl_get_peer_cert(&ssl));
    printf("\r\n%s\r\n", info_buf);
  }
#endif

  return SUCCESS;
}

IoT_Error_t iot_tls_write(Network *pNetwork, unsigned char *pMsg, size_t len, Timer *timer, size_t *written_len)
{
  UNUSED_PARAMETER(pNetwork);
  UNUSED_PARAMETER(timer);
	size_t txLen = 0;
	int ret = 0;
	Timer writeTimer;

  // Set timeout for write operation
	init_timer(&writeTimer);
	countdown_ms(&writeTimer, SLI_SSL_WRITE_RETRY_TIMEOUT_MS);

	while(len > 0U) {
		ret = mbedtls_ssl_write(&ssl, pMsg, len);

		if(ret > 0) {
			if((size_t) ret > len) {
				printf("More bytes written than requested\n");
				return NETWORK_SSL_WRITE_ERROR;
			}

			// Reset timer after successful write
			init_timer(&writeTimer);
			countdown_ms(&writeTimer, SLI_SSL_WRITE_RETRY_TIMEOUT_MS);

			txLen += ret;
			pMsg += ret;
			len -= ret;
		} else if(ret == MBEDTLS_ERR_SSL_WANT_READ ||
              ret == MBEDTLS_ERR_SSL_WANT_WRITE) {
			if(has_timer_expired(&writeTimer)) {
				*written_len = txLen;
				return NETWORK_SSL_WRITE_TIMEOUT_ERROR;
			}
		} else {
			printf("\r\nmbedtls_ssl_write returned -0x%x\r\n", (unsigned int) -ret);
			return NETWORK_SSL_WRITE_ERROR;
		}
	}

  *written_len = txLen;
	return SUCCESS;
}

IoT_Error_t iot_tls_read(Network *pNetwork, unsigned char *pMsg, size_t len, Timer *timer, size_t *read_len)
{
	size_t rxLen = 0;
	int ret = 0;
  uint32_t read_timeout = pNetwork->tlsConnectParams.timeout_ms;

	Timer readTimer;

  // Set timeout for read operation
	init_timer(&readTimer);
	countdown_ms(&readTimer, SLI_SSL_READ_RETRY_TIMEOUT_MS);

	while(len > 0U) {
    mbedtls_ssl_conf_read_timeout(&conf, max(1, min(read_timeout, left_ms(timer))));
		ret = mbedtls_ssl_read(&ssl, pMsg, len);
    mbedtls_ssl_conf_read_timeout(&conf, read_timeout);
		if(ret > 0) {
			if((size_t) ret > len) {
				printf("\r\nMore bytes read than requested\r\n");
				return NETWORK_SSL_WRITE_ERROR;
			}

			// Reset timer after successful read
			init_timer(&readTimer);
			countdown_ms(&readTimer, SLI_SSL_READ_RETRY_TIMEOUT_MS);

			rxLen += ret;
			pMsg += ret;
			len -= ret;
		} else if(ret == MBEDTLS_ERR_SSL_WANT_READ || 
              ret == MBEDTLS_ERR_SSL_WANT_WRITE ||
              ret == MBEDTLS_ERR_SSL_TIMEOUT) {
			if(has_timer_expired(&readTimer)) {
				*read_len = rxLen;
				if(rxLen == 0U) {
					return NETWORK_SSL_NOTHING_TO_READ;
				} else {
					return NETWORK_SSL_READ_TIMEOUT_ERROR;
				}
			}
		} else {
			printf("\r\nmbedtls_ssl_read returned -0x%x\r\n", (unsigned int) -ret);
			return NETWORK_SSL_READ_ERROR;
		}
	}

	*read_len = rxLen;
	return SUCCESS;
}

IoT_Error_t iot_tls_disconnect(Network *pNetwork)
{
  int32_t status = 0;
  do {
    status = mbedtls_ssl_close_notify(&ssl);
  } while(status == MBEDTLS_ERR_SSL_WANT_WRITE);

  lwip_close((int)pNetwork->socket_id);
  pNetwork->socket_id = -1;
  return SUCCESS;
}

IoT_Error_t iot_tls_destroy(Network *pNetwork)
{
  UNUSED_PARAMETER(pNetwork);
  // TODO: Implement this function when external TLS stack is used.
  // NOTE: TLS functionality takes place in firmware when internal stack feature is used. So based on the previous errno value from iot_tls_disconnect() status we return SUCCESS/FAILURE.
  
  mbedtls_ssl_free(&ssl);
  mbedtls_ssl_config_free(&conf);
  mbedtls_x509_crt_free(&cacert);
  mbedtls_x509_crt_free(&device_cert);
  mbedtls_pk_free(&pkey);
  mbedtls_ctr_drbg_free(&ctr_drbg);
  mbedtls_entropy_free(&entropy);

  if (errno)
    return FAILURE;
  else
    return SUCCESS;
}
