/********************************************************************************
 * @file  x_config.h
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
#ifndef X_CONFIG_H_
#define X_CONFIG_H_

#include <stdint.h>

#define X_EVENT_HANDLERS_MAX 5
#define X_EVENT_MESSAGE      0x12345678 //message sent when event arrives

#define XAPI_PAYLOAD_SIZE 256

//enable debugging if not disabled
#ifndef NDEBUG
#define X_DEBUG
#endif

//xbuf config
#define XBUF_DATA_SIZE         (69)
#define XBUF_HEADER_RESERVE    (9)
#define XBUF_TAILER_RESERVE    (0)
#define XBUF_GENERAL_DATA_SIZE (XBUF_DATA_SIZE - XBUF_HEADER_RESERVE - XBUF_TAILER_RESERVE)

typedef uint8_t xbuf_size_t;
#define XBUF_IN_RESERVE     6  /*XBUFS min reservation for incoming data packets*/
#define XBUF_OUT_RESERVE    12 /*XBUFS min reservation for outgoing data packets*/
#define XBUF_NOFLOW_RESERVE 8  /*XBUFS min reservation for control packets*/

#define X_POOL_XBUF_SZ (sizeof(xbuf) + XBUF_DATA_SIZE)
//xbuf config ends

//X Message pool size
#define XMESSAGE_POOL_SIZE 20

#if FEATURE_DEBUG && 0
#if FEATURE_STDLIB
#define DEBUGP(...)                                                  \
  {                                                                  \
    fprintf(stderr, "%s:%d:%s\t", __FILE__, __LINE__, __FUNCTION__); \
    fprintf(stderr, __VA_ARGS__);                                    \
    fflush(stderr);                                                  \
  }
#define DEBUGPR(...)              \
  {                               \
    fprintf(stderr, __VA_ARGS__); \
  }

#else
#define DEBUGP(...)                                             \
  {                                                             \
    tfp_printf("%s:%d:%s\t", __FILE__, __LINE__, __FUNCTION__); \
    tfp_printf(__VA_ARGS__);                                    \
  }
#define DEBUGPR(...)         \
  {                          \
    tfp_printf(__VA_ARGS__); \
  }
#endif
#else
#define DEBUGP(...)
#define DEBUGPR(...)
#endif

#endif /* CONFIG_H_ */
