/***************************************************************************//**
 * @file
 * @brief Functions used by the BGAPI protocol to output BGAPI trace over RTT
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc.  Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement.  This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#ifndef SLI_BGAPI_TRACE_H
#define SLI_BGAPI_TRACE_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************//**
 * @addtogroup sli_bgapi_trace BGAPI Trace
 * @{
 *
 * @brief BGAPI trace utility for tracing BGAPI commands and events
 *
 * The functions provided by this component are automatically called by the
 * BGAPI protocol implementation when a command is processed or an event is
 * delivered to the application. The application does not need to and must not
 * call these functions directly.
 *
 ******************************************************************************/

/**
 * @brief Specifies the full type of a BGAPI message.
 */
typedef enum {
  sli_bgapi_trace_message_type_command  = 0x0, /**< (0x0) The message is a BGAPI command */
  sli_bgapi_trace_message_type_response = 0x1, /**< (0x1) The message is a BGAPI response */
  sli_bgapi_trace_message_type_event    = 0x2  /**< (0x2) The message is a BGAPI event */
} sli_bgapi_trace_message_type_t;

/***************************************************************************//**
 * Initialize BGAPI tracing.
 ******************************************************************************/
void sli_bgapi_trace_init(void);

/***************************************************************************//**
 * Output a BGAPI message to the trace channel.
 *
 * @param[in]  type    The BGAPI message type
 * @param[in]  header  The BGAPI message header
 * @param[in]  data    The BGAPI message payload data
 ******************************************************************************/
void sli_bgapi_trace_output_message(sli_bgapi_trace_message_type_t type,
                                    uint32_t header,
                                    const void *data);

/***************************************************************************//**
 * Output a custom log message to the trace channel.
 *
 * This function is intended as a debugging helper for components that implement
 * BGAPI classes/devices or otherwise contribute to the processing of BGAPI
 * commands or events. Such components can use this function to log arbitrary
 * messages so that they can be decoded and displayed in the right sequence with
 * respect to the BGAPI commands, responses, and events.
 *
 * No special formatting of the log message is required by this function, as the
 * bytes of the custom message are written as-is to the RTT stream in the
 * `sl_bgapi_evt_trace_custom_message event` of the BGAPI Device. The BGAPI
 * Trace decoder tool `bgapi_trace` treats the message as a bytearray and prints
 * it out. Typically the message is a human-readable string intended for a
 * developer to read, but callers are free to output some binary data as well.
 * This may be useful if the RTT stream is read with a custom tool, as
 * outputting binary is more efficient than formatting data to a string on the
 * device.
 *
 * The maximum length of the custom message is limited by the 255-byte limit of
 * the bytearray. Messages longer than this are truncated to fit the BGAPI
 * message payload size. The function returns the number of bytes that were
 * successfully output so that the caller can detect truncation.
 *
 * @param[in]  buffer         Buffer that contains the message to output
 * @param[in]  buffer_length  The length of the message to output
 *
 * @return The number of bytes that were successfully output. If BGAPI Trace is
 *   currently stopped (see @ref sli_bgapi_trace_start() and @ref
 *   sli_bgapi_trace_stop()), the function returns 0.
 ******************************************************************************/
size_t sli_bgapi_trace_log_custom_message(const void *buffer,
                                          size_t buffer_length);

/***************************************************************************//**
 * Start the BGAPI Trace.
 *
 * This function is not used by normal applications. In the default configuration
 * the BGAPI Trace will start automatically at application initialization time so
 * that all BGAPI messages get traced. To avoid the RTT writes from blocking the
 * application execution, something on the host computer side needs to read the
 * RTT buffer. This may be inconvenient in situations where the user would like
 * to use one application image for both a normal run where RTT is not read, and
 * for taking debug traces using BGAPI Trace.
 *
 * To facilitate using one image for both normal runs and for tracing, special
 * applications (typically for example a test application used in automated
 * testing) may define `SLI_BGAPI_TRACE_DISABLE_AUTO_START` as a global `#define`
 * macro when building the application. When that macro is defined, the BGAPI
 * Trace is not started automatically and the application can run normally even
 * without anything reading RTT. When the application wants to start tracing in
 * this configuration, it can do so by calling this function
 * sli_bgapi_trace_start().
 *
 * Use @ref sli_bgapi_trace_stop() to stop the BGAPI Trace if needed.
 ******************************************************************************/
void sli_bgapi_trace_start(void);

/***************************************************************************//**
 * Stop the BGAPI Trace.
 *
 * This function is not used by normal applications. See @ref
 * sli_bgapi_trace_start() for a description of special situations where this
 * function might be useful.
 ******************************************************************************/
void sli_bgapi_trace_stop(void);

/***************************************************************************//**
 * Synchronize BGAPI Trace with the host.
 *
 * This function sends a sync event to the host and blocks until the host
 * connects, i.e. until the RTT buffer becomes empty.
 * The BGAPI Trace uses the RTT buffer in blocking mode. As a consequence, if
 * the host doesn't read the RTT buffer, it becomes full at some point depending
 * on the buffer size and the application will be blocked until the host starts
 * reading. This unpredictable behavior can be prevented by calling this
 * function at a planned spot, e.g. at init.
 ******************************************************************************/
void sli_bgapi_trace_sync(void);

/** @} end sli_bgapi_trace */

#ifdef __cplusplus
}
#endif

#endif // SLI_BGAPI_TRACE_H
