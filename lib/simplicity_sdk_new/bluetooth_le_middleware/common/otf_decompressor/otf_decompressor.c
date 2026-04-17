#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "otf_decompressor.h"

typedef enum {
  SUSPEND_POINT_NONE,
  SUSPEND_POINT_MAP,
  SUSPEND_POINT_OFFSET
} suspend_point_e;

typedef struct swell_state_s {
  uint16_t pattern_mask;
  uint16_t pattern_offset;
  uint8_t  pattern_map;
  uint8_t  pattern_length;
  suspend_point_e suspend_state;
} swell_state_t;

static swell_state_t swell_state = {
  .pattern_mask   = OTF_CFG_PATTERN_MASK_INIT,
  .pattern_offset = 0,
  .pattern_map    = 0,
  .pattern_length = 0,
  .suspend_state  = SUSPEND_POINT_NONE,
};

void otf_unpack_init(void)
{
  swell_state.pattern_mask   = OTF_CFG_PATTERN_MASK_INIT;
  swell_state.pattern_offset = 0;
  swell_state.pattern_map    = 0;
  swell_state.pattern_length = 0;
  swell_state.suspend_state  = SUSPEND_POINT_NONE;
}

sl_status_t otf_unpack_chunk(const void *code, const size_t chunk_size,
                             void *data, size_t data_offset, size_t data_length,
                             size_t* decompressed_length)
{
  const uint8_t *src = code;
  uint8_t       *dst = &(((uint8_t *)data)[data_offset]);
  uint8_t       *dst_end = (uint8_t *)data + data_length;
  uint8_t       *src_end = (uint8_t *)code + chunk_size;

  while (dst < dst_end && src < src_end) {
    if (swell_state.suspend_state == SUSPEND_POINT_NONE) {
      if ((swell_state.pattern_mask <<= 1) == OTF_CFG_PATTERN_MASK_LIMIT) {
        swell_state.pattern_mask = 1;
        swell_state.pattern_map = *src++;
        if (src >= src_end) {
          swell_state.suspend_state = SUSPEND_POINT_MAP;
          break;
        }
      }
    } else if (swell_state.suspend_state == SUSPEND_POINT_MAP) {
      swell_state.suspend_state = SUSPEND_POINT_NONE;
    }

    if (swell_state.pattern_map & swell_state.pattern_mask) {
      uint8_t *pattern_source;
      if (swell_state.suspend_state == SUSPEND_POINT_NONE) {
        swell_state.pattern_length = (*src >> OTF_CFG_PATTERN_SHIFT_COUNT) + OTF_CFG_PEER_MIN;
        swell_state.pattern_offset = (*src++ << OTF_CFG_BYTE_WIDTH);
        if (src < src_end) {
          swell_state.pattern_offset |= *src++;
        } else {
          swell_state.suspend_state = SUSPEND_POINT_OFFSET;
          break;
        }
      } else {
        swell_state.suspend_state = SUSPEND_POINT_NONE;
        swell_state.pattern_offset |= *src++;
      }
      swell_state.pattern_offset &= OTF_CFG_OFFSET_MASK;

      if ((pattern_source = dst - swell_state.pattern_offset) < (uint8_t *)data) {
        return SL_STATUS_FAIL;
      }
      while (swell_state.pattern_length-- && dst < dst_end)
        *dst++ = *pattern_source++;
    } else {
      *dst++ = *src++;
      swell_state.suspend_state = SUSPEND_POINT_NONE;
    }
  }
  *decompressed_length = dst - &(((uint8_t *)data)[data_offset]);
  return SL_STATUS_OK;
}
