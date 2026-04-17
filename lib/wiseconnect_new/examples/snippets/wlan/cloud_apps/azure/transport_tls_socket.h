/* Copyright (c) Microsoft Corporation.
 * Licensed under the MIT License. */

/* SSL Context Handle */
typedef void *SSLContextHandle;
typedef void *SocketHandle;
typedef struct NetworkContext NetworkContext_t;

typedef struct TlsTransportParams {
  SocketHandle xTCPSocket;
  SSLContextHandle xSSLContext;
} TlsTransportParams_t;

/**
 * @brief Contains the credentials necessary for TLS connection setup.
 */
typedef struct NetworkCredentials {
  /**
     * @brief To use ALPN, set this to a NULL-terminated list of supported
     * protocols in decreasing order of preference.
     */
  const char **ppcAlpnProtos;

  /**
     * @brief Disable server name indication (SNI) for a TLS session.
     */
  BaseType_t xDisableSni;

  const uint8_t *pucRootCa;     /**< @brief String representing a trusted server root certificate. */
  size_t xRootCaSize;           /**< @brief Size associated with #NetworkCredentials.pRootCa. */
  const uint8_t *pucClientCert; /**< @brief String representing the client certificate. */
  size_t xClientCertSize;       /**< @brief Size associated with #NetworkCredentials.pClientCert. */
  const uint8_t *pucPrivateKey; /**< @brief String representing the client certificate's private key. */
  size_t xPrivateKeySize;       /**< @brief Size associated with #NetworkCredentials.pPrivateKey. */
} NetworkCredentials_t;

/**
 * @brief TLS Connect / Disconnect return status.
 */
typedef enum TlsTransportStatus {
  eTLSTransportSuccess = 0,        /**< Function successfully completed. */
  eTLSTransportInvalidParameter,   /**< At least one parameter was invalid. */
  eTLSTransportInsufficientMemory, /**< Insufficient memory required to establish connection. */
  eTLSTransportInvalidCredentials, /**< Provided credentials were invalid. */
  eTLSTransportHandshakeFailed,    /**< Performing TLS handshake with server failed. */
  eTLSTransportInternalError,      /**< A call to a system API resulted in an internal error. */
  eTLSTransportConnectFailure,     /**< Initial connection to the server failed. */
  eTLSTransportCAVerifyFailed      /**< Verification of TLS CA cert failed. */
} TlsTransportStatus_t;

/**
 * @brief Connect to TLS endpoint
 *
 * @param[in] pxNetworkContext Pointer to the Network context.
 * @param[in] pcHostName Pointer to NULL terminated hostname.
 * @param[in] usPort Port to connect to.
 * @param[in] pxNetworkCredentials Pointer to network credentials.
 * @param[in] ulReceiveTimeoutMs Receive timeout.
 * @param[in] ulSendTimeoutMs Send timeout.
 * @return A #TlsTransportStatus_t with the result of the operation.
 */
TlsTransportStatus_t TLS_Socket_Connect(NetworkContext_t *pxNetworkContext,
                                        const char *pcHostName,
                                        uint16_t usPort,
                                        const NetworkCredentials_t *pxNetworkCredentials,
                                        uint32_t ulReceiveTimeoutMs,
                                        uint32_t ulSendTimeoutMs);

/**
 * @brief Disconnect the TLS connection
 *
 * @param[in] pxNetworkContext Pointer to the Network context.
 */
void TLS_Socket_Disconnect(NetworkContext_t *pxNetworkContext);

/**
 * @brief Receive data from TLS.
 *
 * @param pxNetworkContext Pointer to the Network context.
 * @param pvBuffer Buffer used for receiving data.
 * @param xBytesToRecv Size of the buffer.
 * @return An #int32_t number of bytes copied.
 */
int32_t TLS_Socket_Recv(NetworkContext_t *pxNetworkContext, void *pvBuffer, size_t xBytesToRecv);

/**
 * @brief Send data using TLS.
 *
 * @param pxNetworkContext Pointer to the Network context.
 * @param pvBuffer Buffer that contains data to be sent.
 * @param xBytesToSend Length of the data to be sent.
 * @return An #int32_t number of bytes successfully sent.
 */
int32_t TLS_Socket_Send(NetworkContext_t *pxNetworkContext, const void *pvBuffer, size_t xBytesToSend);
