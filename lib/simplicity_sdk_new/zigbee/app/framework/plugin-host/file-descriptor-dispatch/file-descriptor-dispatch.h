/***************************************************************************//**
 * @file
 * @brief Definitions for the File Descriptor Dispatch plugin.
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
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

/**
 * @defgroup file-descriptor-dispatch File Descriptor Dispatch
 * @ingroup component host
 * @brief API and Callbacks for the File Descriptor Dispatch Component
 *
 * This plugin provides an API to register a file descriptor and a
 * callback that is called when the select() function notifies the
 * plugin there is data on that file descriptor.
 *
 */

/**
 * @addtogroup file-descriptor-dispatch
 * @{
 */

typedef enum {
  SL_ZIGBEE_AF_FILE_DESCRIPTOR_OPERATION_NONE   = 0x00,
  SL_ZIGBEE_AF_FILE_DESCRIPTOR_OPERATION_READ   = 0x01,
  SL_ZIGBEE_AF_FILE_DESCRIPTOR_OPERATION_WRITE  = 0x02,
  SL_ZIGBEE_AF_FILE_DESCRIPTOR_OPERATION_EXCEPT = 0x03,
  SL_ZIGBEE_AF_FILE_DESCRIPTOR_OPERATION_MAX
} sl_zigbee_af_file_descriptor_operation_t;

typedef void (*sl_zigbee_af_file_descriptor_ready_callback_t)(void* data, sl_zigbee_af_file_descriptor_operation_t operation);

typedef struct {
  sl_zigbee_af_file_descriptor_ready_callback_t callback;
  void* dataPassedToCallback;
  sl_zigbee_af_file_descriptor_operation_t operation;
  int fileDescriptor;
} sl_zigbee_af_file_descriptor_dispatch_struct_t;

/**
 * @name API
 * @{
 */

sl_status_t sl_zigbee_af_file_descriptor_dispatch_add(sl_zigbee_af_file_descriptor_dispatch_struct_t* dispatchStruct);
sl_status_t sl_zigbee_af_file_descriptor_dispatch_wait_for_events(uint32_t timeoutMs);
bool sl_zigbee_af_file_descriptor_dispatch_remove(int fileDescriptor);

/** @} */ // end of name API

/** @brief Bad file descriptor.
 *
 * The indicated file descriptor has been detected to be bad. For example,
 * the file descriptor caused an EBADF error on a select() call. The file
 * descriptor will be omitted from subsequent file descriptor polling. The
 * application responsible for the file descriptor should take steps to
 * recover and clean up.
 *
 * @param fd The bad file descriptor Ver.: always
 */
void sl_zigbee_af_file_descriptor_dispatch_bad_file_descriptor_cb(int fd);

/** @} */ // end of file-descriptor-dispatch
