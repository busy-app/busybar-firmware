/* Copyright (c) Microsoft Corporation.
 * Licensed under the MIT License. */

/**
 * @file azure_iot_transport_interface.h
 * @brief Transport interface definition to send and receive data over the
 * network.
 */

#ifndef AZURE_IOT_TRANSPORT_INTERFACE_H
#define AZURE_IOT_TRANSPORT_INTERFACE_H

#include <stdint.h>
#include <stddef.h>

/**
 * @brief The transport interface definition.
 *
 * The transport interface is a set of APIs that must be implemented using an
 * external transport layer protocol. The transport interface is defined in
 * @ref transport_interface.h. This interface allows protocols like MQTT
 * to send and receive data over the transport layer. This
 * interface does not handle connection and disconnection to the server of
 * interest. The connection, disconnection, and other transport settings, like
 * timeout and TLS setup, must be handled in the user application.
 * <br>
 *
 * The functions that must be implemented are:<br>
 * - [Transport Receive](@ref AzureIoTTransportRecv_t)
 * - [Transport Send](@ref AzureIoTTransportSend_t)
 *
 * Each of the functions above take in an opaque context @ref struct NetworkContext.
 * The functions above and the context are also grouped together in the
 * @ref AzureIoTTransportInterface_t structure:<br><br>
 * @snippet this define_transportinterface
 * <br>
 *
 * The following steps give guidance on implementing the transport interface:
 *
 * -# Implementing @ref struct NetworkContext<br><br>
 * @snippet this define_networkcontext
 * <br>
 * The implemented struct NetworkContext must contain all of the information
 * that is needed to receive and send data with the @ref AzureIoTTransportRecv_t
 * and the @ref AzureIoTTransportSend_t implementations.<br>
 * In the case of TLS over TCP, struct NetworkContext is typically implemented
 * with the TCP socket context and a TLS context.<br><br>
 * <b>Example code:</b>
 * @code{c}
 * struct NetworkContext
 * {
 *     struct MyTCPSocketContext tcpSocketContext;
 *     struct MyTLSContext tlsContext;
 * };
 * @endcode
 * <br>
 * -# Implementing @ref AzureIoTTransportRecv_t<br><br>
 * @snippet this define_transportrecv
 * <br>
 * This function is expected to populate a buffer, with bytes received from the
 * transport, and return the number of bytes placed in the buffer.
 * In the case of TLS over TCP, @ref AzureIoTTransportRecv_t is typically implemented by
 * calling the TLS layer function to receive data. In case of plaintext TCP
 * without TLS, it is typically implemented by calling the TCP layer receive
 * function. @ref AzureIoTTransportRecv_t may be invoked multiple times by the protocol
 * library, if fewer bytes than were requested to receive are returned.
 * <br><br>
 * <b>Example code:</b>
 * @code{c}
 * int32_t myNetworkRecvImplementation( struct NetworkContext * pNetworkContext,
 *                                      void * pBuffer,
 *                                      size_t bytesToRecv )
 * {
 *     int32_t bytesReceived = 0;
 *     bytesReceived = TLSRecv( pNetworkContext->tlsContext,
 *                              pBuffer,
 *                              bytesToRecv,
 *                              MY_SOCKET_TIMEOUT );
 *     if( bytesReceived < 0 )
 *     {
 *         // Handle socket error.
 *     }
 *     // Handle other cases.
 *
 *     return bytesReceived;
 * }
 * @endcode
 * <br>
 * -# Implementing @ref AzureIoTTransportSend_t<br><br>
 * @snippet this define_transportsend
 * <br>
 * This function is expected to send the bytes, in the given buffer over the
 * transport, and return the number of bytes sent.
 * In the case of TLS over TCP, @ref AzureIoTTransportSend_t is typically implemented by
 * calling the TLS layer function to send data. In case of plaintext TCP
 * without TLS, it is typically implemented by calling the TCP layer send
 * function. @ref AzureIoTTransportSend_t may be invoked multiple times by the protocol
 * library, if fewer bytes than were requested to send are returned.
 * <br><br>
 * <b>Example code:</b>
 * @code{c}
 * int32_t myNetworkSendImplementation( struct NetworkContext * pNetworkContext,
 *                                      const void * pBuffer,
 *                                      size_t bytesToSend )
 * {
 *     int32_t bytesSent = 0;
 *     bytesSent = TLSSend( pNetworkContext->tlsContext,
 *                          pBuffer,
 *                          bytesToSend,
 *                          MY_SOCKET_TIMEOUT );
 *     if( bytesSent < 0 )
 *     {
 *         // Handle socket error.
 *     }
 *     // Handle other cases.
 *
 *     return bytesSent;
 * }
 * @endcode
 */

/**
 * @brief The NetworkContext is an incomplete type. An implementation of this
 * interface must define struct NetworkContext for the system requirements.
 * This context is passed into the network interface functions.
 */
struct NetworkContext;

/**
 * @brief User defined function for receiving data on the network.
 *
 * @param[in] pxNetworkContext Implementation-defined network context.
 * @param[in] pvBuffer Buffer to receive the data into.
 * @param[in] xBytesToRecv Number of bytes requested from the network.
 *
 * @return The number of bytes received or a negative error code.
 */
typedef int32_t ( * AzureIoTTransportRecv_t )( struct NetworkContext * pxNetworkContext,
                                               void * pvBuffer,
                                               size_t xBytesToRecv );

/**
 * @brief User defined function for sending data on the network.
 *
 * @param[in] pxNetworkContext Implementation-defined network context.
 * @param[in] pvBuffer Buffer containing the bytes to send over the network stack.
 * @param[in] xBytesToSend Number of bytes to send over the network.
 *
 * @return The number of bytes sent or a negative error code.
 */
typedef int32_t ( * AzureIoTTransportSend_t )( struct NetworkContext * pxNetworkContext,
                                               const void * pvBuffer,
                                               size_t xBytesToSend );

/**
 * @brief The transport layer interface.
 */
typedef struct AzureIoTTransportInterface
{
    AzureIoTTransportRecv_t xRecv;            /**< Transport receive interface. */
    AzureIoTTransportSend_t xSend;            /**< Transport send interface. */
    struct NetworkContext * pxNetworkContext; /**< Implementation-defined network context. */
} AzureIoTTransportInterface_t;

#endif /* AZURE_IOT_TRANSPORT_INTERFACE_H */
