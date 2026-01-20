/**
 * @file compress.h
 * LZSS based compression HAL API
 */
#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Supported compression types */
typedef enum {
    CompressTypeHeatshrink = 0,
    CompressTypeGzip,
    CompressTypeMax,
} CompressType;

/** Configuration for heatshrink compression */
typedef struct {
    uint16_t window_sz2;
    uint16_t lookahead_sz2;
    uint16_t input_buffer_sz;
} CompressConfigHeatshrink;

/** Default configuration for heatshrink compression. Used for image assets. */
extern const CompressConfigHeatshrink compress_config_heatshrink_default;

/** I/O callback for streamed compression/decompression
 * 
 * @param context user context
 * @param buffer buffer to read/write
 * @param size size of buffer
 * 
 * @return number of bytes read/written, 0 on end of stream, negative on error
 */
typedef int32_t (*CompressIoCallback)(void* context, uint8_t* buffer, size_t size);

/** CompressStreamDecoder control structure */
typedef struct CompressStreamDecoder CompressStreamDecoder;

/** Allocate stream decoder
 *
 * @param      type          Compression type
 * @param[in]  config        Configuration for compression, specific to type
 * @param      read_cb       The read callback for input (compressed) data
 * @param      read_context  The read context
 *
 * @return     CompressStreamDecoder instance
 */
CompressStreamDecoder* compress_stream_decoder_alloc(
    CompressType type,
    const void* config,
    CompressIoCallback read_cb,
    void* read_context);

/** Free stream decoder
 *
 * @param      instance  The CompressStreamDecoder instance
 */
void compress_stream_decoder_free(CompressStreamDecoder* instance);

/** Read uncompressed data chunk from stream decoder
 *
 * @param      instance       The CompressStreamDecoder instance
 * @param      data_out       The data out
 * @param[in]  data_out_size  The data out size
 *
 * @return     true on success, false on EOF or error.
 */
bool compress_stream_decoder_read(
    CompressStreamDecoder* instance,
    uint8_t* data_out,
    size_t data_out_size);

/** Seek to position in uncompressed data stream
 *
 * @param      instance   The CompressStreamDecoder instance
 * @param[in]  position   The position (absolute)
 * 
 * @return     true on success
 * @warning    Backward seeking is not supported
 */
bool compress_stream_decoder_seek(CompressStreamDecoder* instance, size_t position);

/** Get current position in uncompressed data stream
 *
 * @param      instance  The CompressStreamDecoder instance
 *
 * @return     current position
 */
size_t compress_stream_decoder_tell(CompressStreamDecoder* instance);

/** Reset stream decoder to the beginning
 * @warning    Read callback must be repositioned by caller separately
 *
 * @param      instance  The CompressStreamDecoder instance
 *
 * @return     true on success
 */
bool compress_stream_decoder_reset(CompressStreamDecoder* instance);

#ifdef __cplusplus
}
#endif
