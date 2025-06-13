/**
 * @file sockets.h
 * @brief Network communication API via asynchronous sockets.
 */
#pragma once

#include "sockets_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief The string key for SocketSrv instance access
 *
 * Get the instance pointer by calling `furi_record_open(RECORD_SOCKETS)`
 */
#define RECORD_SOCKETS "sockets"

/** Opaque Sockets type declaration. */
typedef struct Socket Socket;

/** Enumeration of possible socket event types. */
typedef enum {
    SocketEventTypeReceive, /**< Socket has data available for reading */
    SocketEventTypeAccept, /**< A new client connection has been accepted */
    SocketEventTypeClose, /**< The remote side has closed the connection */
    SocketEventTypeMax, /**< Special value, internal use */
} SocketEventType;

/** Socket event structure. */
typedef struct {
    SocketEventType type; /**< Type of the event that has occurred */
    union {
        struct {
            Socket* client_socket; /**< Pointer to the created socket */
            SocketConnectionInfo
                connection_info; /**< Information relevant to the remote connection */
        } accept; /**< Accept event, emitted on new client connection */
    };
} SocketEvent;

/**
 * @brief Socket event callback function type.
 *
 * @param[in,out] socket Pointer to the socket that has emitted the event
 * @param[in] event Pointer to the event structure with type and data
 * @param[in,out] context Pointer to a user-specific context object
 */
typedef void (*SocketEventCallback)(Socket* socket, const SocketEvent* event, void* context);

/**
 * @brief Create a new socket.
 *
 * @param[in,out] instance Pointer to the SocketSrv service instance
 * @param[in] socket_info Pointer to a structure containing the socket description
 *
 * @returns Pointer to the allocated socket on success, @c NULL otherwise
 */
Socket* socket_alloc(SocketSrv* instance, const SocketInfo* socket_info);

/**
 * @brief Close and delete an existing socket.
 *
 * @warning No other calls on this socket instance are allowed after calling this function.
 *
 * @param[in] socket Pointer to the socket to be free'd
 *
 * @returns SocketStatusOk on success, a SocketStatus error code otherwise
 */
SocketStatus socket_free(Socket* socket);

/**
 * @brief Set the socket event callback.
 *
 * @param[in,out] socket Pointer to the socket to be modified
 * @param[in] callback Pointer to the callback function
 * @param[in,out] context Pointer to the user-specific context object (will be passed to the callback)
 *
 * @returns SocketStatusOk on success, a SocketStatus error code otherwise
 */
SocketStatus
    socket_set_event_callback(Socket* socket, SocketEventCallback callback, void* context);

/**
 * @brief Begin accepting new connections on a socket.
 *
 * This function is asynchronous (does not block). The event callback will be called
 * with the appropriate parameters when a remote client requests a connection.
 *
 * @param[in,out] socket Pointer to the socket to be accepting new connections
 * @param[in] bind_info Pointer to the structure describing the address and port to bind the socket to
 *
 * @returns SocketStatusOk on success, a SocketStatus error code otherwise
 */
SocketStatus socket_accept(Socket* socket, const SocketConnectionInfo* bind_info);

/**
 * @brief Connect to a remote socket.
 *
 * @param[in,out] socket Pointer to the socket to be connected
 * @param[in] connection_info Pointer to the structure describing the address and port to connect the socket to
 *
 * @returns SocketStatusOk on success, a SocketStatus error code otherwise
 */
SocketStatus socket_connect(Socket* socket, const SocketConnectionInfo* connection_info);

/**
 * @brief Send data through the socket.
 *
 * This function is asynchronous (does not block).
 *
 * @note The number of bytes actually sent may be smaller that requested.
 *
 * @param[in,out] socket Pointer to the socket to send through
 * @param[in] data Pointer to the data for sending
 * @param[in] data_size Number of bytes to send
 * @param[out] sent_size Pointer to the variable to hold the number of bytes actually sent (may be @c NULL)
 *
 * @returns SocketStatusOk on success, a SocketStatus error code otherwise
 */
SocketStatus socket_send(Socket* socket, const void* data, size_t data_size, size_t* sent_size);

/**
 * @brief Receive data from the socket.
 *
 * This function is asynchronous (does not block).
 *
 * @note The number of bytes actually received may be smaller that requested.
 *
 * @param[in,out] socket Pointer to the socket to receive from
 * @param[in,out] data Pointer to the data buffer receiving
 * @param[in] data_size Maximum size of the data to receive (must be no more than the buffer size)
 * @param[out] received_size Pointer to the variable to hold the number of bytes actually received (may be @c NULL)
 *
 * @returns SocketStatusOk on success, a SocketStatus error code otherwise
 */
SocketStatus socket_receive(Socket* socket, void* data, size_t data_size, size_t* received_size);

#ifdef __cplusplus
}
#endif
