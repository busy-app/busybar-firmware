/**
 * @brief Structures describing the file format
 * 
 * Terminology:
 *   - Display Frame: an image that's displayed on the screen for one frame time
 *   - File Frame: one encoded frame. One file frame might be shown for several
 *     consecutive display frames.
 *   - Section: User-selectable named range of display frame indices.
 *   - Chunk: part of the file. There's a Sections chunk and a Frames chunk.
 *   - Color buffer: Plain Bgr888 buffer that LVGL accepts directly.
 *   - Packed buffer: Either Bgr888 or Gray4 buffer. Gray4 has to be unpacked
 *     into a Color buffer, Bgr888 can be used directly.
 *   - Encoded buffer: Either RLE-encoded or plain packed buffer. RLE has to be
 *     decoded into a Packed buffer.
 * 
 * All integers are little-endian.
 * 
 * Visualized file layout:
 * 
 * +----------------+
 * | AnimFileHeader |
 * +----------------+-----------------+
 * |    Sections    | AnimFileSection |
 * |     chunk      +-----------------+
 * |                | AnimFileSection |
 * |                +-----------------+
 * |                |       . . .
 * |                +-----------------+
 * |                | AnimFileSection |
 * +----------------+-----------------+---------------------+
 * |     Frames     |    File Frame   | AnimFileFrameHeader |
 * |     chunk      |                 +---------------------+
 * |                |                 | Encoded frame data  |
 * |                +-----------------+---------------------+
 * |                |    File Frame   | AnimFileFrameHeader |
 * |                |                 +---------------------+
 * |                |                 | Encoded frame data  |
 * |                +-----------------+---------------------+
 * |                |       . . .
 * |                +-----------------+---------------------+
 * |                |    File Frame   | AnimFileFrameHeader |
 * |                |                 +---------------------+
 * |                |                 | Encoded frame data  |
 * +----------------+-----------------+---------------------+
 * 
 * There must always be at least one section with the an index of 0 named
 * "whole", covering the entire range of Display Frames.
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <furi.h>

/**
 * @brief Color format. Applies to the entire animation.
 */
typedef enum FURI_PACKED {
    AnimFileColorFormatBgr888, //<! Each pixel is (in order) blue, then green, then red
    AnimFileColorFormatGray4, //<! 2 px in byte: hi nibble = earlier pixel, lo nibble = later pixel
    AnimFileColorFormatMAX,
} AnimFileColorFormat;
static_assert(sizeof(AnimFileColorFormat) == sizeof(uint8_t));

/**
 * @brief Flags. Apply to the entire animation.
 */
typedef enum FURI_PACKED {
    AnimFileFlagMAX = (1 << 0),
} AnimFileFlag;
static_assert(sizeof(AnimFileFlag) == sizeof(uint8_t));

/**
 * @brief Header in the beginning of the file
 */
typedef struct FURI_PACKED {
    char signature[8];

    AnimFileFlag flags;
    uint8_t width;
    uint8_t height;
    AnimFileColorFormat color_format;

    uint8_t fps;
    uint8_t _unused[3];

    uint32_t sections_chunk_length;
    uint32_t frames_chunk_length;

    uint16_t max_encoded_length;
} AnimFileHeader;

#define ANIM_FILE_HEADER_SIGNATURE "BSBanim0" // BUSY Status Bar animation version 0

/**
 * @brief Descriptor of one Section
 */
typedef struct FURI_PACKED {
    uint32_t start; //<! Index of the first Display Frame (inclusive)
    uint32_t end; //<! Index of the last Display Frame (inclusive)
    uint32_t frame_offs; //<! File offset of the first File Frame in this Section
    uint8_t duration_override; //<! In case the Section starts in the middle of a File Frame that spans multiple Display Frames, this field will indicate the adjusted duration of the affected File Frame
    char name[]; //<! NUL-terminated
} AnimFileSection;

/**
 * @brief Encoding mode of one File Frame
 */
typedef enum FURI_PACKED {
    AnimFileFrameEncodingRaw, //<! Plain pixels encoded according to `AnimFileColorFormat`
    AnimFileFrameEncodingRle, //<! Run-length encoding of `Raw` data implemented by `toolbox/rle_encode`. `blk_size` parameter is 3 for `AnimFileColorFormatBgr888`, and 1 for `AnimFileColorFormatGray4`.
    AnimFileFrameEncodingMAX,
} AnimFileFrameEncoding;
static_assert(sizeof(AnimFileFrameEncoding) == sizeof(uint8_t));

/**
 * @brief Header of one File Frame
 */
typedef struct FURI_PACKED {
    AnimFileFrameEncoding encoding;
    uint8_t duration; //<! How many Display Frames to show this File Frame for
    uint16_t encoded_length; //<! Length of encoded frame data following this header
} AnimFileFrameHeader;

#ifdef __cplusplus
}
#endif
