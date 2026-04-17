/***************************************************************************//**
 * @file
 * @brief IO Stream Internal Functions.
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

#ifndef SLI_IOSTREAM_H
#define SLI_IOSTREAM_H

#include "sl_iostream.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (sli_iostream_on_write_completed_t)(sli_iostream_write_async_op_t *op,
                                                 sl_status_t status,
                                                 void *arg);

struct sli_iostream_write_async_op {
  sl_slist_node_t                    node;
  sli_iostream_on_write_completed_t *on_write_completed;
  void                              *on_write_completed_arg;
  void                              *buffer;
  size_t                             buffer_length;
  void                              *context;
};

/***************************************************************************//**
 * Initialize an asynchronous write operation
 *
 * @param[in] async_op        Async operation structure to be initialized.
 *
 * @param[in] buffer          Buffer that contains the data to output.
 *
 * @param[in] buffer_length   Data length contained in the buffer.
 *
 * @param[in] on_write_completed  Callback function to be called upon completion.
 *
 * @param[in] on_write_completed_arg  Argument to be passed to the callback function.
 *
 * @note The write callback will be called within an interrupt context
 *
 * @return  Status result
 ******************************************************************************/
static inline sl_status_t sli_iostream_init_async_write_op(sli_iostream_write_async_op_t *async_op,
                                                           void *buffer,
                                                           size_t buffer_length,
                                                           sli_iostream_on_write_completed_t on_write_completed,
                                                           void *on_write_completed_arg)
{
  if (buffer_length == 0) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  if (async_op == NULL || buffer == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  async_op->buffer = buffer;
  async_op->buffer_length = buffer_length;
  async_op->on_write_completed = on_write_completed;
  async_op->on_write_completed_arg = on_write_completed_arg;

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Perform the asynchronous write operation
 *
 * @param[in] stream          I/O Stream to be used.
 *                              SL_IOSTREAM_STDOUT;           Default output stream will be used.
 *                              Pointer to specific stream;   Specific stream will be used.
 *
 * @param[in] write_async_op  Initialized async operation structure.
 *
 * @return  Status result
 ******************************************************************************/
sl_status_t sli_iostream_async_write(sl_iostream_t *stream,
                                     sli_iostream_write_async_op_t *write_async_op);

#ifdef __cplusplus
}
#endif

#endif // SLI_IOSTREAM_H
