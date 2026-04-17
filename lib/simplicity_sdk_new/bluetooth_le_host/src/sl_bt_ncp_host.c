/***************************************************************************//**
 * @brief Adaptation layer between host application and SL_BT_API protocol
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

#include "sl_bt_ncp_host.h"
#include "sli_bt_api.h"
#include "sl_status.h"

// We only have bt and btmesh
#define SL_BGAPI_DEVICE_TYPES 2

bgapi_device_type_queue_t* device_event_queues[SL_BGAPI_DEVICE_TYPES] = { 0 };
static size_t registered_devices_count = 0;

sl_bt_msg_t _sl_bt_cmd_msg;
sl_bt_msg_t _sl_bt_rsp_msg;
sl_bt_msg_t *sl_bt_cmd_msg = &_sl_bt_cmd_msg;
sl_bt_msg_t *sl_bt_rsp_msg = &_sl_bt_rsp_msg;
void (*sl_bt_api_output)(uint32_t len1, uint8_t* data1);
int32_t (*sl_bt_api_input)(uint32_t len1, uint8_t* data1);
int32_t (*sl_bt_api_peek)(void);
uint8_t _sl_bt_queue_buffer[SL_BT_API_QUEUE_LEN * (SL_BGAPI_MSG_HEADER_LEN + SL_BGAPI_MAX_PAYLOAD_SIZE)];

bgapi_device_type_queue_t sl_bt_api_queue = {
  sl_bgapi_dev_type_bt,
  0, // Write and read offsets start from zero.
  0,
  (sl_bt_msg_t *)_sl_bt_queue_buffer,
  SL_BT_API_QUEUE_LEN
};

sl_status_t sli_bgapi_register_device(bgapi_device_type_queue_t *queue)
{
  size_t i;
  for (i = 0; i < SL_BGAPI_DEVICE_TYPES; i++) {
    if (device_event_queues[i] == NULL) {
      device_event_queues[i] = queue;
      registered_devices_count++;
      return SL_STATUS_OK;
    }
  }
  return SL_STATUS_ALREADY_INITIALIZED;
}

sl_status_t sl_bt_api_initialize(tx_func ofunc, rx_func ifunc)
{
  if (!ofunc || !ifunc) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  sl_bt_api_output = ofunc;
  sl_bt_api_input = ifunc;
  sl_bt_api_peek = NULL;
  return sli_bgapi_register_device(&sl_bt_api_queue);
}

sl_status_t sl_bt_api_initialize_nonblock(tx_func ofunc, rx_func ifunc, rx_peek_func pfunc)
{
  if (!ofunc || !ifunc || !pfunc) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  sl_bt_api_output = ofunc;
  sl_bt_api_input = ifunc;
  sl_bt_api_peek = pfunc;
  return sli_bgapi_register_device(&sl_bt_api_queue);
}

/**
 * Check if the device queue has any pending events.
 */
bool sli_bgapi_device_queue_has_events(bgapi_device_type_queue_t *device_queue)
{
  if (device_queue->write_offset != device_queue->read_offset) {
    return true;
  }
  return false;
}

/**
 * Check if any device type other than my_device_type has events waiting.
 */
bool sli_bgapi_other_events_in_queue(enum sl_bgapi_dev_types my_device_type)
{
  size_t i;
  for (i = 0; i < registered_devices_count; i++) {
    // Go through all registered device types that don't match the current device type.
    if (device_event_queues[i] && device_event_queues[i]->device_type != my_device_type) {
      if (sli_bgapi_device_queue_has_events(device_event_queues[i])) {
        // Another device type has some events in the queue
        return true;
      }
    }
  }
  return false;
}

/**
 * This function attempts to read a BGAPI event or response from the input data pipe.
 *
 * If there is no data, or a proper header cannot be recognized, returns NULL and
 * discards any read data.
 *
 * If an event is found, it is put into the corresponding event queue, as indicated
 * by the registered event queue struct device type. Then NULL is returned.
 *
 * If a response is found, it is copied into the response_buffer parameter.
 * Then a pointer to response_buffer is returned.
 */
sl_bt_msg_t* sli_wait_for_bgapi_message(sl_bt_msg_t *response_buffer)
{
  uint32_t msg_length;
  uint32_t msg_type;
  uint32_t header;
  uint32_t device_type;
  uint8_t  *payload;
  sl_bt_msg_t *packet_ptr = NULL, *retVal = NULL;
  int      ret;
  size_t i;
  bgapi_device_type_queue_t *queue = NULL;

  // Read the message header
  ret = sl_bt_api_input(sizeof(header), (uint8_t*)&header);
  if (ret < 0) {
    return 0; // Failed to read the header
  }

  // To read the payload of the message, we need to find where the message needs
  // to be stored, and whether we have sufficient storage space for it.
  msg_type = SL_BGAPI_MSG_TYPE(header);
  msg_length = SL_BGAPI_MSG_LEN(header);
  if (msg_type == (uint32_t)sl_bgapi_msg_type_evt) {
    // The message is an event. Try to find the queue for events of this device
    // type.
    device_type = SL_BGAPI_MSG_DEVICE_TYPE(header);
    for (i = 0; i < SL_BGAPI_DEVICE_TYPES; i++) {
      if ((device_event_queues[i] != NULL)
          && (device_type == device_event_queues[i]->device_type)) {
        queue = device_event_queues[i];
        break;
      }
    }

    // If we have a queue for this device, see if it can fit the received event
    packet_ptr = NULL;
    if (queue != NULL) {
      uint32_t next_write_offset = (queue->write_offset + 1) % queue->len;
      if (((msg_length <= SL_BGAPI_MAX_PAYLOAD_SIZE))
          && (next_write_offset != queue->read_offset)) {
        // The event fits. Remember the pointer and move the write offset to the
        // next slot or wrap around to beginning.
        packet_ptr = &queue->buffer[queue->write_offset];
        queue->write_offset = next_write_offset;
      }
    }
  } else {
    // The message was a response. Note that in the case of a response we don't
    // need to split it into two buffer types, because we can't have multiple
    // pending commands and responses in parallel. Whoever sent the last command
    // will wait for the response. We always commit to filling the response
    // buffer here.
    retVal = response_buffer;

    // Make sure the response can fit into the response buffer
    if (msg_length <= SL_BGAPI_MAX_PAYLOAD_SIZE) {
      // The response fits. Read it into the response buffer.
      packet_ptr = response_buffer;
    } else {
      // The received response is too large to fit into the response buffer. We
      // need to discard the received response and set a generic error response
      // for the caller.
      packet_ptr = NULL;
      struct sl_bt_packet *error_response = (struct sl_bt_packet *)response_buffer;
      uint32_t error_msg_id = SL_BGAPI_MSG_ID(header);
      uint32_t error_msg_len = sizeof(error_response->data.rsp_error);
      error_response->header = SL_BGAPI_MSG_HEADER_FROM_ID_AND_LEN(error_msg_id, error_msg_len);
      error_response->data.rsp_error.result = SL_STATUS_HAS_OVERFLOWED;
    }
  }

  // Make sure we have a place to store the message
  if (packet_ptr != NULL) {
    // We have a buffer that the message will fit to. Set the header and read
    // the payload, if any.
    packet_ptr->header = header;
    if (msg_length > 0) {
      payload = packet_ptr->data.payload;
      ret = sl_bt_api_input(msg_length, payload);
      if (ret < 0) {
        return 0;
      }
    }
  } else {
    // We don't have a buffer to receive to. Discard the payload, if any.
    while (msg_length > 0) {
      uint8_t discard_buf[SL_BGAPI_MAX_PAYLOAD_SIZE];
      size_t bytes_to_read = (msg_length > sizeof(discard_buf)) ? sizeof(discard_buf) : msg_length;
      sl_bt_api_input(bytes_to_read, discard_buf);
      msg_length -= bytes_to_read;
    }
  }

  // Using retVal avoid double handling of event msg types in outer function.
  // If retVal is non-null we got a response packet. If null, an event was placed
  // in one of the event queues.
  return retVal;
}

bool sl_bt_event_pending(void)
{
  if (sli_bgapi_device_queue_has_events(&sl_bt_api_queue)) {//event is waiting in queue
    return true;
  }

  //something in uart waiting to be read
  if (sl_bt_api_peek && sl_bt_api_peek()) {
    return true;
  }

  return false;
}

/**
 * Attempts to get an event that is already in the event queue, or an
 * event or response from the input pipe.
 *
 * If BGAPI was initialized in blocking mode, this function will block
 * until a valid event or response is found.
 *
 * Note that this function will return even when an event of a different
 * type that was expected is found. It will return SL_STATUS_BUSY in that case.
 * Otherwise we would be stuck until the event type we checked for arrives.
 **/
sl_status_t sli_bgapi_get_event(int block, sl_bt_msg_t *event, bgapi_device_type_queue_t *device_queue)
{
  sl_bt_msg_t *rsp;
  while (1) {
    // First check if we already have events waiting for us.
    if (sli_bgapi_device_queue_has_events(device_queue)) {
      // Copy event from queue to event parameter, then nudge the read offset forward
      // by one message, or wrap around to beginning.
      memcpy(event, &device_queue->buffer[device_queue->read_offset], sizeof(sl_bt_msg_t));
      device_queue->read_offset = (device_queue->read_offset + 1) % device_queue->len;
      return SL_STATUS_OK;
    } else if (sli_bgapi_other_events_in_queue(device_queue->device_type)) {
      // If some other device type has messages, we need to yield here in order to let the
      // events be processed.
      return SL_STATUS_BUSY;
    }

    //if not blocking and nothing in uart -> out
    if (!block && sl_bt_api_peek && sl_bt_api_peek() == 0) {
      return SL_STATUS_WOULD_BLOCK;
    }

    //read more messages from device
    if ( (rsp = sli_wait_for_bgapi_message(sl_bt_rsp_msg)) ) {
      // Note that we copy the event to the event pointer here only if it is a response.
      // Regular events are handled in the above blocks.
      memcpy(event, rsp, sizeof(sl_bt_msg_t));
      return SL_STATUS_OK;
    }
  }
}

sl_status_t sl_bt_wait_event(sl_bt_msg_t* event)
{
  return sli_bgapi_get_event(1, event, &sl_bt_api_queue);
}

sl_status_t sl_bt_pop_event(sl_bt_msg_t* event)
{
  return sli_bgapi_get_event(0, event, &sl_bt_api_queue);
}

/**
 * This function will block until a response is found from the data pipe.
 * Any events that arrive before the response will be put into their corresponding
 * event queues.
 */
sl_bt_msg_t* sl_bt_wait_response(void)
{
  sl_bt_msg_t* rsp;
  while (1) {
    rsp = sli_wait_for_bgapi_message(sl_bt_rsp_msg); // Will return a valid pointer only if we got a response.
    if (rsp) {
      return rsp;
    }
  }
}

void sl_bt_host_handle_command()
{
  //packet in sl_bt_cmd_msg is waiting for output
  sl_bt_api_output(SL_BGAPI_MSG_HEADER_LEN + SL_BGAPI_MSG_LEN(sl_bt_cmd_msg->header), (uint8_t*)sl_bt_cmd_msg);
  sl_bt_wait_response();
}

void sl_bt_host_handle_command_noresponse()
{
  //packet in sl_bt_cmd_msg is waiting for output
  sl_bt_api_output(SL_BGAPI_MSG_HEADER_LEN + SL_BGAPI_MSG_LEN(sl_bt_cmd_msg->header), (uint8_t*)sl_bt_cmd_msg);
}
