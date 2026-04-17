#include "core/sl_zigbee_stack.h"
#include "sl_code_classification.h"

sl_status_t sl_legacy_buffer_manager_really_set_linked_buffers_length(sli_buffer_manager_buffer_t *buffer,
                                                                      uint16_t newLength)
{
  if (newLength <= sli_legacy_buffer_manager_get_buffer_length(*buffer)) {
    sli_legacy_buffer_manager_set_buffer_length(*buffer, newLength);
    return SL_STATUS_OK;
  } else {
    return sl_legacy_buffer_manager_extend_linked_buffer(*buffer, newLength - sli_legacy_buffer_manager_get_buffer_length(*buffer));
  }
}

//----------------------------------------------------------------
// Wrappers for the MessageBuffer interface.

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_BUFFER_MANAGER, SL_CODE_CLASS_TIME_CRITICAL)
sl_status_t sli_legacy_packet_buffer_really_copy_to_linked_buffers(const uint8_t *contents,
                                                                   sli_buffer_manager_buffer_t buffer,
                                                                   uint16_t startIndex,
                                                                   uint16_t length,
                                                                   uint8_t direction)
{
  uint8_t *bufferPointer = sli_legacy_buffer_manager_get_buffer_pointer(buffer) + startIndex;

  if (length == 0 || bufferPointer == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  if (startIndex + length > sl_legacy_buffer_manager_message_buffer_length(buffer)) {
    return SL_STATUS_WOULD_OVERFLOW;
  }

  if (direction == 0) {         // from buffer to RAM
    memmove((uint8_t *) contents, bufferPointer, length);
  } else {  // from RAM to buffer
    memmove(bufferPointer, (uint8_t *) contents, length);
  }
  return SL_STATUS_OK;
}

uint8_t *sl_legacy_buffer_manager_get_linked_buffers_pointer(sli_buffer_manager_buffer_t buffer, uint16_t index)
{
  assert(index < sl_legacy_buffer_manager_message_buffer_length(buffer));
  return sli_legacy_buffer_manager_get_buffer_pointer(buffer) + index;
}

uint8_t sl_legacy_buffer_manager_get_linked_buffers_byte(sli_buffer_manager_buffer_t buffer, uint16_t index)
{
  uint8_t *bufferPointer = sli_legacy_buffer_manager_get_buffer_pointer(buffer);
  if (bufferPointer != NULL) {
    assert(index < sl_legacy_buffer_manager_message_buffer_length(buffer));
    bufferPointer += index;
    return *bufferPointer;
  }
  return NULL_BUFFER;
}

void sl_legacy_buffer_manager_set_linked_buffers_byte(sli_buffer_manager_buffer_t buffer, uint16_t index, uint8_t byte)
{
  assert(index < sl_legacy_buffer_manager_message_buffer_length(buffer));
  uint8_t *bufferPointer = sli_legacy_buffer_manager_get_buffer_pointer(buffer);
  if (bufferPointer != NULL) {
    bufferPointer += index;
    *bufferPointer = byte;
  }
}

// Count the number of buffers needed, allocate them, then copy the data over.

sli_buffer_manager_buffer_t sl_legacy_buffer_manager_copy_linked_buffers(sli_buffer_manager_buffer_t buffer)
{
  return sli_legacy_buffer_manager_fill_buffer(sli_legacy_buffer_manager_get_buffer_pointer(buffer),
                                               sli_legacy_buffer_manager_get_buffer_length(buffer));
}

void sl_legacy_buffer_manager_copy_buffer_bytes(sli_buffer_manager_buffer_t to,
                                                uint16_t toIndex,
                                                sli_buffer_manager_buffer_t from,
                                                uint16_t fromIndex,
                                                uint16_t count)
{
  memmove(sli_legacy_buffer_manager_get_buffer_pointer(to) + toIndex,
          sli_legacy_buffer_manager_get_buffer_pointer(from) + fromIndex,
          count);
}

uint16_t sl_legacy_buffer_manager_get_linked_buffers_low_high_int16u(sli_buffer_manager_buffer_t buffer,
                                                                     uint16_t index)
{
  return HIGH_LOW_TO_INT(sl_legacy_buffer_manager_get_linked_buffers_byte(buffer, index + 1),
                         sl_legacy_buffer_manager_get_linked_buffers_byte(buffer, index));
}

sl_status_t sl_legacy_buffer_manager_set_linked_buffers_low_high_int16u(sli_buffer_manager_buffer_t buffer,
                                                                        uint16_t index,
                                                                        uint16_t value)
{
  uint8_t temp[2];
  temp[0] = LOW_BYTE(value);
  temp[1] = HIGH_BYTE(value);
  return sli_legacy_packet_buffer_really_copy_to_linked_buffers(temp, buffer, index, 2, 1);
}

uint32_t sl_legacy_buffer_manager_get_linked_buffers_low_high_int32u(sli_buffer_manager_buffer_t buffer,
                                                                     uint16_t index)
{
  return (HIGH_LOW_TO_INT(sl_legacy_buffer_manager_get_linked_buffers_byte(buffer, index + 1),
                          sl_legacy_buffer_manager_get_linked_buffers_byte(buffer, index))
          + (((uint32_t)(HIGH_LOW_TO_INT(sl_legacy_buffer_manager_get_linked_buffers_byte(buffer, index + 3),
                                         sl_legacy_buffer_manager_get_linked_buffers_byte(buffer, index + 2))))
             << 16));
}

sl_status_t sl_legacy_buffer_manager_set_linked_buffers_low_high_int32u(sli_buffer_manager_buffer_t buffer,
                                                                        uint16_t index,
                                                                        uint32_t value)
{
  uint8_t temp[4];
  temp[0] = BYTE_0(value);
  temp[1] = BYTE_1(value);
  temp[2] = BYTE_2(value);
  temp[3] = BYTE_3(value);
  return sl_legacy_buffer_manager_copy_to_linked_buffers(temp, buffer, index, 4);
}

sli_buffer_manager_buffer_t
sl_legacy_buffer_manager_fill_stack_buffer(unsigned int count, ...)
{
  va_list argPointer;
  sli_buffer_manager_buffer_t buffer = sli_legacy_buffer_manager_allocate_buffer(count);
  uint8_t i;
  uint8_t *contents = sl_legacy_buffer_manager_linked_buffer_contents(buffer);

  if (buffer == SL_ZIGBEE_NULL_MESSAGE_BUFFER) {
    return SL_ZIGBEE_NULL_MESSAGE_BUFFER;
  }

  va_start(argPointer, count);
  for (i = 0; i < count; i++, contents++) {
    *contents = LOW_BYTE(va_arg(argPointer, int));
  }
  va_end(argPointer);

  return buffer;
}

#ifdef SL_ZIGBEE_TEST
#define testAssert assert
#else
#define testAssert(x) do {} while (0)
#endif

static uint16_t writeMessage(uint8_t *contents,
                             uint16_t maxLength,
                             const char *format,
                             va_list elements);

//----------------------------------------------------------------
// A simple printf()-like facility for creating messages.  For now this only
// works for messages that don't cross buffer boundaries.  See sl_zigbee_stack.h
// for a longer description.
//
// This has not been released to customers, mostly because of the restriction
// about crossing buffer boundaries.

// arbitrary maximum packet length
#define SLI_LEGACY_PACKET_BUFFER_WRITE_MESSAGE_BOUNDARY 64
sli_buffer_manager_buffer_t sli_legacy_packet_buffer_make_message_using_va_list(uint16_t startIndex,
                                                                                const char *format,
                                                                                va_list argPointer)
{
  sli_buffer_manager_buffer_t message = sli_legacy_buffer_manager_allocate_buffer(SLI_LEGACY_PACKET_BUFFER_WRITE_MESSAGE_BOUNDARY);
  uint8_t *contents = sl_legacy_buffer_manager_message_buffer_contents(message);
  uint16_t length;

  if (message == SL_ZIGBEE_NULL_MESSAGE_BUFFER) {
    return message;
  }

  length = writeMessage(contents + startIndex,
                        SLI_LEGACY_PACKET_BUFFER_WRITE_MESSAGE_BOUNDARY,
                        format,
                        argPointer);
  if (sl_legacy_buffer_manager_set_message_buffer_length(message, length + startIndex)
      != SL_STATUS_OK) {
    return SL_ZIGBEE_NULL_MESSAGE_BUFFER;
  }

  return message;
}

uint16_t sli_legacy_packet_buffer_write_message(uint8_t* buffer,
                                                uint16_t maxLength,
                                                uint16_t startIndex,
                                                const char *format,
                                                ...)
{
  va_list argPointer;
  uint16_t length;

  va_start(argPointer, format);
  length = sli_legacy_packet_buffer_write_message_using_va_list(buffer, maxLength, startIndex, format, argPointer);
  va_end(argPointer);

  return length;
}

uint16_t sli_legacy_packet_buffer_write_message_using_va_list(uint8_t* buffer,
                                                              uint16_t maxLength,
                                                              uint16_t startIndex,
                                                              const char *format,
                                                              va_list argPointer)
{
  uint16_t length = writeMessage(buffer + startIndex,
                                 maxLength - startIndex,
                                 format,
                                 argPointer);
  testAssert(length <= maxLength);

  return length;
}

sli_buffer_manager_buffer_t sli_legacy_packet_buffer_make_message(uint16_t startIndex, const char *format, ...)
{
  sli_buffer_manager_buffer_t message;
  va_list argPointer;

  va_start(argPointer, format);
  message = sli_legacy_packet_buffer_make_message_using_va_list(startIndex, format, argPointer);
  va_end(argPointer);

  return message;
}

static uint16_t writeMessage(uint8_t *contents,
                             uint16_t maxLength,
                             const char *format,
                             va_list elements)
{
  uint8_t *finger = contents;
  uint16_t i;

  for (i = 0; i < maxLength && *format != '\0'; i++, format++) {
    uint8_t next = *format;
    if (next == '1'
        || next == '2') {
      int element = va_arg(elements, int);
      *finger++ = LOW_BYTE(element);
      if (next == '2') {
        i++;
        if (i > maxLength) {
          goto writeMessageDone;
        }
        *finger++ = HIGH_BYTE(element);
      }
    } else if (next == '4') {
      uint32_t element = va_arg(elements, long int);
      if (i + 4 > maxLength) {
        goto writeMessageDone;
      }
      *finger++ = BYTE_0(element);
      *finger++ = BYTE_1(element);
      *finger++ = BYTE_2(element);
      *finger++ = BYTE_3(element);
      i += 3;
    } else {
      uint8_t *bytes = (uint8_t*)va_arg(elements, int*);
      uint16_t count;
      if ('2' < next && next <= '9') {
        count = next - (uint8_t)('0');
      } else if ('A' <= next && next <= 'G') {
        count = (next - (uint8_t)('A')) + 10U;
      } else {
        testAssert(next == 'p' || next == 's');
        count = va_arg(elements, int);
      }
      if (i + count >= maxLength) {
        goto writeMessageDone;
      }
      memmove(finger, bytes, count);
      finger += count;
      i += count - 1;
    }
  }
  writeMessageDone:
  return finger - contents;
}
