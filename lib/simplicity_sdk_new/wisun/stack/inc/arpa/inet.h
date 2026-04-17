#ifndef INET_H
#define INET_H

#ifdef __cplusplus
extern "C" {
 //C++ doesn't provide restrict
#ifdef __GNUC__
#define restrict __restrict
#else
#define restrict
#endif
#endif

#include <stdint.h>

/// Transport layer port
typedef uint16_t in_port_t;

/** \brief Converts a 32-bit integer from host to network byte order.
 *  \param hostlong The 32-bit integer in host byte order.
 *  \return The 32-bit integer in network byte order.
 */
uint32_t htonl(uint32_t hostlong);

/** \brief Converts a 16-bit integer from host to network byte order.
 *  \param hostshort The 16-bit integer in host byte order.
 *  \return The 16-bit integer in network byte order.
 */
uint16_t htons(uint16_t hostshort);

/** \brief Converts a 32-bit integer from network to host byte order.
 *  \param netlong The 32-bit integer in network byte order.
 *  \return The 32-bit integer in host byte order.
 */
uint32_t ntohl(uint32_t netlong);

/** \brief Converts a 16-bit integer from network to host byte order.
 *  \param netshort The 16-bit integer in network byte order.
 *  \return The 16-bit integer in host byte order.
 */
uint16_t ntohs(uint16_t netshort);

/** @brief Convert IPv6 addresses from text to binary form
 *  @param af Address family. Can be AF_INET6.
 *  @param src Pointer to the string IP address in human-readable format.
 *  @param dst Pointer to the buffer where the function stores the numeric IP address.
 *
 *  @return int Returns 1 if the conversion succeeds,
 *  returns 0 if the input is not a valid IP address,
 *  and returns -1 with errno set to EAFNOSUPPORT if the af argument is invalid.
 */
int inet_pton(int af, const char *src, void *dst);

/** @brief Convert an IPV6 network address structure to a string.
 *  @param af Address family. Can be AF_INET6.
 *  @param src Pointer to a network address structure.
 *  @param dst Pointer to a buffer where the function stores the string form of the IP address.
 *  @param size The size of the buffer pointed to by dst.
 *
 *  @return const char* Returns a pointer to the buffer containing the text string if the conversion succeeds,
 *  and NULL otherwise, with errno set to EAFNOSUPPORT if the af argument was not a valid address family.
*   or set to ENOSPC if the converted address string would exceed the size given by size.
 */
const char *inet_ntop(int af, const void *restrict src, char *restrict dst, int size);


#ifdef __cplusplus
}
#endif
#endif
