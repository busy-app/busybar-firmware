/*******************************************************************************
 * @ingroup ddfu_patch
 * @file
 * Main header file for the @ref ddfu_patch library.
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
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
#ifndef DDFU_PATCH_H
#define DDFU_PATCH_H

/**
 * @addtogroup ddfu_patch Delta DFU Patch
 *
 * This library provides an interface for applying a patch file to an existing firmware file to
 * create a new firmware file. The patch file may be created by the @ref ddfu_diff library.
 *
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/**
 * Patch status.
 *
 * The description string for each status can be obtained with `ddfu_patch_status_str()`.
 */
enum ddfu_patch_status {
  DDFU_PATCH_STATUS_OK,                      /**< Patch applied successfully. */
  DDFU_PATCH_STATUS_ERR,                     /**< Patch apply failed. */
  DDFU_PATCH_STATUS_READ_PATCH_FAILED,       /**< Failed to read patch data. */
  DDFU_PATCH_STATUS_WRITE_NEW_FW_FAILED,     /**< Failed to write new firmware data. */
  DDFU_PATCH_STATUS_READ_OLD_FW_FAILED,      /**< Failed to read old firmware data. */
  DDFU_PATCH_STATUS_SEEK_NEW_FW_FAILED,      /**< Failed to seek new firmware position. */
  DDFU_PATCH_STATUS_INVALID_PARAMS,          /**< Invalid parameters. */
  DDFU_PATCH_STATUS_INVALID_HEADER,          /**< Invalid patch header. */
  DDFU_PATCH_STATUS_UNSUPPORTED_VERSION,     /**< Unsupported patch version. */
  DDFU_PATCH_STATUS_UNSUPPORTED_INSTRUCTION, /**< Unsupported patch instruction. */
};

/**
 * Get patch status description.
 *
 * @param status Patch status.
 * @return String representation of the patch status.
 */
const char *ddfu_patch_status_str(enum ddfu_patch_status status);

/**
 * Patch I/O buffer.
 *
 * Buffer for patch I/O operations. If provided buffer is small, larger instructions will be
 * completed in multiple steps. The size should be multiple of two, for example 128 bytes.
 */
struct ddfu_patch_io_buffer {
  uint8_t *data; /**< Buffer data. */
  size_t size;   /**< Buffer size. */
};

/**
 * Patch input/output interface.
 *
 * Function pointers to user-provided I/O operations.
 *
 * In case the implementation of these functions needs some context, a `user_ctx` pointer provided
 * to `ddfu_patch_apply()` will be passed on to these functions.
 */
struct ddfu_patch_io {
  /**
   * Read the old firmware.
   *
   * Read data from the old firmware at the specified offset.
   *
   * @note The implementation must check that the read is within the bounds of the old firmware, and
   * return `false` if it is not.
   *
   * @param offset Offset in old firmware.
   * @param nbyte Number of bytes to read.
   * @param out_buf Output buffer.
   * @param user_ctx User defined callback context.
   * @retval true Success
   * @retval false Failure
   */
  bool (*read_old_firmware)(size_t offset, size_t nbyte, uint8_t *out_buf, void *user_ctx);

  /**
   * Write the new firmware.
   *
   * Append data to the new firmware. The new firmware is written sequentially from the beginning to
   * the end, and thus there is no offset parameter.
   *
   * @note The implementation must check that the write does not exceed the maximum size of the new
   * firmware, and return `false` if it does.
   *
   * @param buf Buffer with new firmware data.
   * @param nbyte Number of bytes to write.
   * @param user_ctx User defined callback context.
   * @retval true Success
   * @retval false Failure
   */
  bool (*write_new_firmware)(uint8_t *buf, size_t nbyte, void *user_ctx);

  /**
   * Seek the new firmware position forward by offset.
   *
   * Seek the new firmware position forward by `offset` bytes without writing anything. This is used
   * to skip segment gaps in the new firmware.
   *
   * @note The implementation must check that the new position does not exceed the maximum size of
   * the new firmware, and return `false` if it does.
   *
   * @param offset Number of bytes to seek forward.
   * @param user_ctx User defined callback context.
   * @retval true Success
   * @retval false Failure
   */
  bool (*seek_new_firmware)(size_t offset, void *user_ctx);

  /**
   * Read the patch.
   *
   * Read next `nbyte` bytes of the patch data. The patch data is read sequentially from the
   * beginning to the end, and thus there is no offset parameter.
   *
   * @note The implementation must check that the read is within the bounds of the patch data, and
   * return `false` if it is not.
   *
   * @param nbyte Number of bytes to read.
   * @param out_buf Output buffer.
   * @param user_ctx User defined callback context.
   * @retval true Success
   * @retval false Failure
   */
  bool (*read_patch)(size_t nbyte, void *out_buf, void *user_ctx);

  /**
   * Check if the patch is completely read.
   *
   * @param user_ctx User defined callback context.
   * @retval true Patch is completely read.
   * @retval false There is more patch data to read.
   */
  bool (*is_end_of_patch)(void *user_ctx);
};

/**
 * Apply a patch.
 *
 * Apply a patch to an existing firmware file to create a new firmware file. The operation depends
 * on I/O functions provided by the caller in the `io` parameter. In case the implementation of
 * these functions needs some context, a `user_ctx` pointer may be used for that purpose. The
 * pointer is passed on to the I/O functions. Some of the I/O functions operate like a stream, and
 * the `user_ctx` pointer may be used to keep track of the current position in the stream. The
 * caller must ensure that the state is initialized properly so that the streams start at the
 * beginning when this function is called.
 *
 * @param io Patch input/output interface.
 * @param io_buffer Memory buffer for patch I/O operations.
 * @param user_ctx User defined callback context.
 * @return #ddfu_patch_status
 */
enum ddfu_patch_status ddfu_patch_apply(const struct ddfu_patch_io *io,
                                        const struct ddfu_patch_io_buffer *io_buffer,
                                        void *user_ctx);

#ifdef __cplusplus
}
#endif

#endif // DDFU_PATCH_H
/// @}
