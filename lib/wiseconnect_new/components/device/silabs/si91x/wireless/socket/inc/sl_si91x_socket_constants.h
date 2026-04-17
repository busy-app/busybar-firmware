/********************************************************************************
 * @file  sl_si91x_socket_constants.h
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
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

#pragma once

#include <stdint.h>
#include "cmsis_os2.h"

#ifndef BIT
#define BIT(a) ((uint32_t)1U << a)
#endif

/******************************************************
 *               Socket Configurations
 ******************************************************/
#define SLI_SI91X_NO_ERROR 0 // This is returned by socket functions after successful execution
#define SLI_SI91X_UNDEFINED_ERROR \
  0 // This value is set to errno global variable when a socket API failed due to unknown error.

#define SLI_NUMBER_OF_SOCKETS 20

#define SLI_SI91X_HOST_WAIT_FOR_SELECT_RSP 1000

#define SLI_TCP_RX_WINDOW_SIZE 10

#define SLI_DEFAULT_STREAM_MSS_SIZE_IPV4   1460
#define SLI_DEFAULT_DATAGRAM_MSS_SIZE_IPV4 1472

#define SLI_DEFAULT_STREAM_MSS_SIZE_IPV6   1440
#define SLI_DEFAULT_DATAGRAM_MSS_SIZE_IPV6 1452

#define SLI_MAX_TCP_RETRY_COUNT         10
#define SLI_DEFAULT_TCP_KEEP_ALIVE_TIME 1200

#define SLI_SI91X_CERT_INDEX_0 0
#define SLI_SI91X_CERT_INDEX_1 1
#define SLI_SI91X_CERT_INDEX_2 2

/**
 * @addtogroup SI91X_SOCKET_TLS_EXTENSION_TYPE SiWx91x Socket TLS Extension Type
 * @ingroup SI91X_SOCKET_FUNCTIONS
 * @{
 */
#define SL_SI91X_TLS_EXTENSION_SNI_TYPE  1 ///< TLS extension for SNI
#define SL_SI91X_TLS_EXTENSION_ALPN_TYPE 2 ///< TLS extension for ALPN
/** @} */

#define SLI_SI91X_SOCKET_TCP_CLIENT 0x0000
#define SLI_SI91X_SOCKET_UDP_CLIENT 0x0001
#define SLI_SI91X_SOCKET_TCP_SERVER 0x0002
#define SLI_SI91X_SOCKET_LUDP       0x0004

#define SLI_SI91X_SOCKET_FEAT_SSL                BIT(0) // SAPI maps both SSL and synchronous to BIT(0)
#define SLI_SI91X_SOCKET_FEAT_SYNCHRONOUS        BIT(0)
#define SLI_SI91X_SOCKET_FEAT_LTCP_ACCEPT        BIT(1)
#define SLI_SI91X_WEBSOCKET_FEAT                 BIT(1)
#define SLI_SI91X_SOCKET_FEAT_TCP_ACK_INDICATION BIT(2)
#define SLI_SI91X_SOCKET_FEAT_TCP_RX_WINDOW      BIT(4)
#define SLI_SI91X_SOCKET_FEAT_CERT_INDEX         BIT(5)
#define SLI_SI91X_HIGH_PERFORMANCE_SOCKET        BIT(7)

#define SLI_MAX_RETRANSMISSION_TIME_VALUE 255

/**
 * @addtogroup SI91X_SOCKET_OPTION_NAME SiWx91x Socket Option Name
 * @ingroup SI91X_SOCKET_FUNCTIONS
 * @{ 
 */
#define SL_SI91X_SO_RCVTIME                          20 ///< Enable receive timeout
#define SL_SI91X_SO_TCP_KEEPALIVE                    26 ///< To configure the TCP keep alive
#define SL_SI91X_SO_HIGH_PERFORMANCE_SOCKET          38 ///< To configure the high performance socket
#define SL_SI91X_SO_CERT_INDEX                       46 ///< To enable set certificate index
#define SL_SI91X_SO_SSL_ENABLE                       37 ///< To enable SSL
#define SL_SI91X_SO_SSL_V_1_0_ENABLE                 42 ///< To enable SSL 1.0
#define SL_SI91X_SO_SSL_V_1_1_ENABLE                 43 ///< To enable SSL 1.1
#define SL_SI91X_SO_SSL_V_1_2_ENABLE                 44 ///< To enable SSL 1.2
#define SL_SI91x_SO_TCP_ACK_INDICATION               45 ///< To enable TCP ACK indication feature
#define SL_SI91X_SO_MAX_RETRANSMISSION_TIMEOUT_VALUE 48 ///< to configure max retransmission timeout value
#define SL_SI91X_IP_TOS                              48 ///< To configure TOS
#define SL_SI91X_SO_SSL_V_1_3_ENABLE                 49 ///< To enable SSL 1.3
#define SL_SI91X_SO_MAXRETRY                         24 ///< To enable max TCP retry count
#define SL_SI91X_SO_MSS                              40 ///< To configure the TCP MSS
#define SL_SI91X_SO_SOCK_VAP_ID                      25 ///< To configure the socket VAP ID
#define SL_SI91X_SO_TLS_SNI                          47 ///< To configure the TLS SNI extension
#define SL_SI91X_SO_TLS_ALPN                         50 ///< To configure the TLS ALPN extension
#define SL_SI91X_SO_DTLS_ENABLE                      51 ///< To enable DTLS
#define SL_SI91X_SO_DTLS_V_1_0_ENABLE                52 ///< To enable DTLS 1.0
#define SL_SI91X_SO_DTLS_V_1_2_ENABLE                53 ///< To enable DTLS 1.2
/** @} */

/**
 * @addtogroup SI91X_SOCKET_SHUTDOWN_OPTION SiWx91x Socket Shutdown Option
 * @ingroup SI91X_SOCKET_FUNCTIONS
 * @{ 
 */
#define SHUTDOWN_BY_ID   0 ///< Shutdown by ID
#define SHUTDOWN_BY_PORT 1 ///< Shutdown by port
/** @} */

/******************************************************
 *                 SSL features
 ******************************************************/

/**
 * @brief Default ciphers.
 *
 * This macro defines the default set of ciphers.
 */

#define SL_SI91X_TLS_DEFAULT_CIPHERS                                                                               \
  (SL_SI91X_TLS_DHE_RSA_WITH_AES_256_CBC_SHA256 | SL_SI91X_TLS_DHE_RSA_WITH_AES_128_CBC_SHA256                     \
   | SL_SI91X_TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA384 | SL_SI91X_TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA256               \
   | SL_SI91X_TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA384 | SL_SI91X_TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256           \
   | SL_SI91X_TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA | SL_SI91X_TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA                     \
   | SL_SI91X_TLS_DHE_RSA_WITH_AES_256_CBC_SHA | SL_SI91X_TLS_DHE_RSA_WITH_AES_128_CBC_SHA                         \
   | SL_SI91X_TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA | SL_SI91X_TLS_DHE_RSA_WITH_3DES_EDE_CBC_SHA                    \
   | SL_SI91X_TLS_ECDHE_RSA_WITH_3DES_EDE_CBC_SHA | SL_SI91X_TLS_ECDHE_ECDSA_WITH_3DES_EDE_CBC_SHA                 \
   | SL_SI91X_TLS_RSA_WITH_AES_256_CBC_SHA256 | SL_SI91X_TLS_RSA_WITH_AES_128_CBC_SHA256                           \
   | SL_SI91X_TLS_RSA_WITH_AES_256_CBC_SHA | SL_SI91X_TLS_RSA_WITH_AES_128_CBC_SHA                                 \
   | SL_SI91X_TLS_RSA_WITH_AES_128_CCM_8 | SL_SI91X_TLS_RSA_WITH_AES_256_CCM_8                                     \
   | SL_SI91X_TLS_ECDHE_ECDSA_WITH_AES_128_CCM_8 | SL_SI91X_TLS_ECDHE_ECDSA_WITH_AES_256_CCM_8                     \
   | SL_SI91X_TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256 | SL_SI91X_TLS_ECDHE_ECDSA_WITH_AES_256_GCM_SHA384           \
   | SL_SI91X_TLS_ECDHE_RSA_WITH_CHACHA20_POLY1305_SHA256 | SL_SI91X_TLS_ECDHE_ECDSA_WITH_CHACHA20_POLY1305_SHA256 \
   | SL_SI91X_TLS_DHE_RSA_WITH_CHACHA20_POLY1305_SHA256)

/**
 * @addtogroup SI91X_SOCKET_CIPHERS SiWx91x Socket Ciphers
 * @ingroup SOCKET_CONFIGURATION_FUNCTION
 * @{
 */

#define SL_SI91X_TLS_RSA_WITH_AES_256_CBC_SHA256         BIT(0)  ///< Cipher for RSA with AES 256 CBC SHA256.
#define SL_SI91X_TLS_RSA_WITH_AES_128_CBC_SHA256         BIT(1)  ///< Cipher for RSA with AES 128 CBC SHA256.
#define SL_SI91X_TLS_RSA_WITH_AES_256_CBC_SHA            BIT(2)  ///< Cipher for RSA with AES 256 CBC SHA.
#define SL_SI91X_TLS_RSA_WITH_AES_128_CBC_SHA            BIT(3)  ///< Cipher for RSA with AES 128 CBC SHA.
#define SL_SI91X_TLS_RSA_WITH_AES_128_CCM_8              BIT(4)  ///< Cipher for RSA with AES 128 CCM 8.
#define SL_SI91X_TLS_RSA_WITH_AES_256_CCM_8              BIT(5)  ///< Cipher for RSA with AES 256 CCM 8.
#define SL_SI91X_TLS_ECDHE_ECDSA_WITH_AES_128_CCM_8      BIT(6)  ///< Cipher for ECDHE ECDSA with AES 128 CCM 8.
#define SL_SI91X_TLS_ECDHE_ECDSA_WITH_AES_256_CCM_8      BIT(7)  ///< Cipher for ECDHE ECDSA with AES 256 CCM 8.
#define SL_SI91X_TLS_DHE_RSA_WITH_AES_128_GCM_SHA256     BIT(8)  ///< Cipher for DHE RSA with AES 128 GCM SHA256.
#define SL_SI91X_TLS_DHE_RSA_WITH_AES_256_GCM_SHA384     BIT(9)  ///< Cipher for DHE RSA with AES 256 GCM SHA384.
#define SL_SI91X_TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256   BIT(10) ///< Cipher for ECDHE RSA with AES 128 GCM SHA256.
#define SL_SI91X_TLS_ECDHE_RSA_WITH_AES_256_GCM_SHA384   BIT(11) ///< Cipher for ECDHE RSA with AES 256 GCM SHA384.
#define SL_SI91X_TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256 BIT(12) ///< Cipher for ECDHE ECDSA with AES 128 GCM SHA256.
#define SL_SI91X_TLS_ECDHE_ECDSA_WITH_AES_256_GCM_SHA384 BIT(13) ///< Cipher for ECDHE ECDSA with AES 256 GCM SHA384.
#define SL_SI91X_TLS_DHE_RSA_WITH_AES_256_CBC_SHA256     BIT(14) ///< Cipher for DHE RSA with AES 256 CBC SHA256.
#define SL_SI91X_TLS_DHE_RSA_WITH_AES_128_CBC_SHA256     BIT(15) ///< Cipher for DHE RSA with AES 128 CBC SHA256.
#define SL_SI91X_TLS_DHE_RSA_WITH_AES_256_CBC_SHA        BIT(16) ///< Cipher for DHE RSA with AES 256 CBC SHA.
#define SL_SI91X_TLS_DHE_RSA_WITH_AES_128_CBC_SHA        BIT(17) ///< Cipher for DHE RSA with AES 128 CBC SHA.
#define SL_SI91X_TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA384   BIT(18) ///< Cipher for ECDHE RSA with AES 256 CBC SHA384.
#define SL_SI91X_TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA256   BIT(19) ///< Cipher for ECDHE RSA with AES 128 CBC SHA256.
#define SL_SI91X_TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA      BIT(20) ///< Cipher for ECDHE RSA with AES 256 CBC SHA.
#define SL_SI91X_TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA      BIT(21) ///< Cipher for ECDHE RSA with AES 128 CBC SHA.
#define SL_SI91X_TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA384 BIT(22) ///< Cipher for ECDHE ECDSA with AES 256 CBC SHA384.
#define SL_SI91X_TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256 BIT(23) ///< Cipher for ECDHE ECDSA with AES 128 CBC SHA256.
#define SL_SI91X_TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA    BIT(24) ///< Cipher for ECDHE ECDSA with AES 256 CBC SHA.
#define SL_SI91X_TLS_DHE_RSA_WITH_3DES_EDE_CBC_SHA       BIT(25) ///< Cipher for DHE RSA with 3DES EDE CBC SHA.
#define SL_SI91X_TLS_ECDHE_RSA_WITH_3DES_EDE_CBC_SHA     BIT(26) ///< Cipher for ECDHE RSA with 3DES EDE CBC SHA.
#define SL_SI91X_TLS_ECDHE_ECDSA_WITH_3DES_EDE_CBC_SHA   BIT(27) ///< Cipher for ECDHE ECDSA with 3DES EDE CBC SHA.
#define SL_SI91X_TLS_ECDHE_RSA_WITH_CHACHA20_POLY1305_SHA256 \
  BIT(28) ///< Cipher for ECDHE RSA with CHACHA20 POLY1305 SHA256.
#define SL_SI91X_TLS_ECDHE_ECDSA_WITH_CHACHA20_POLY1305_SHA256 \
  BIT(29) ///< Cipher for ECDHE ECDSA with CHACHA20 POLY1305 SHA256.
#define SL_SI91X_TLS_DHE_RSA_WITH_CHACHA20_POLY1305_SHA256 \
  BIT(30)                                ///< Cipher for DHE RSA with CHACHA20 POLY1305 SHA256.
#define SL_SI91X_TLS_NEW_CIPHERS BIT(31) ///< New ciphers.

/** @} */

#if defined(SLI_SI917)
#define SL_SI91X_TLS_EXT_CIPHERS SL_SI91X_TLSV1_3_ALL_CIPHERS

/**
 * @brief All configurable ciphers for TLSv1.3.
 *
 * This macro defines the default set of ciphers for TLSv1.3.
 */

#define SL_SI91X_TLSV1_3_ALL_CIPHERS                                                                                  \
  (SL_SI91X_TLS1_3_AES_128_GCM_SHA256 | SL_SI91X_TLS1_3_AES_256_GCM_SHA384 | SL_SI91X_TLS1_3_CHACHA20_POLY1305_SHA256 \
   | SL_SI91X_TLS1_3_AES_128_CCM_SHA256 | SL_SI91X_TLS1_3_AES_128_CCM_8_SHA256)
#endif

/**
 * @addtogroup SI91X_EXTENDED_CIPHERS SiWx91x Socket Extended Ciphers
 * @ingroup SOCKET_CONFIGURATION_FUNCTION
 * @{
 */

// TLSv1.3 supported ciphers
#if defined(SLI_SI917) || defined(DOXYGEN)
#define SL_SI91X_TLS1_3_AES_128_GCM_SHA256       BIT(0) ///< Cipher for TLS 1.3 with AES 128 GCM SHA256.
#define SL_SI91X_TLS1_3_AES_256_GCM_SHA384       BIT(1) ///< Cipher for TLS 1.3 with AES 256 GCM SHA384.
#define SL_SI91X_TLS1_3_CHACHA20_POLY1305_SHA256 BIT(2) ///< Cipher for TLS 1.3 with CHACHA20 POLY1305 SHA256.
#define SL_SI91X_TLS1_3_AES_128_CCM_SHA256       BIT(3) ///< Cipher for TLS 1.3 with AES 128 CCM SHA256.
#define SL_SI91X_TLS1_3_AES_128_CCM_8_SHA256     BIT(4) ///< Cipher for TLS 1.3 with AES 128 CCM 8 SHA256.
#endif

/** @} */
