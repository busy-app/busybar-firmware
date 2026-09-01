/**
 * @brief Structures describing the file format
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
 * |                |                 | Encoded mask data   |
 * |                |                 +---------------------+
 * |                |                 | Encoded pixel data  |
 * |                +-----------------+---------------------+
 * |                |    File Frame   | AnimFileFrameHeader |
 * |                |                 +---------------------+
 * |                |                 | Encoded mask data   |
 * |                |                 +---------------------+
 * |                |                 | Encoded frame data  |
 * |                +-----------------+---------------------+
 * |                |       . . .
 * |                +-----------------+---------------------+
 * |                |    File Frame   | AnimFileFrameHeader |
 * |                |                 +---------------------+
 * |                |                 | Encoded mask data   |
 * |                |                 +---------------------+
 * |                |                 | Encoded frame data  |
 * +----------------+-----------------+---------------------+
 * 
 * All integers are little-endian.
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
    AnimFileColorFormatBgra8888, //<! Each pixel is (in order) blue, then green, then red, then alpha
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

    uint16_t max_mask_length;
    uint16_t max_pixel_length;
    uint8_t fps;
    uint8_t _unused[1];

    uint32_t sections_chunk_length;
    uint32_t frames_chunk_length;

    uint32_t section_count;
    uint32_t frame_count;
} AnimFileHeader;

// Busybar Image Container speciallY Crafted for file Length Eradication, major ver. 1
#define ANIM_FILE_HEADER_SIGNATURE "bicycle1"

/**
 * @brief Descriptor of one Section
 */
typedef struct FURI_PACKED {
    uint32_t start; //<! Index of the first Frame (inclusive)
    uint32_t end; //<! Index of the last Frame (inclusive)
    uint32_t frame_offs; //<! File offset of the first Frame in this Section
    char name[]; //<! NUL-terminated
} AnimFileSection;

/**
 * @brief Encoding mode of Frame Mask
 */
typedef enum FURI_PACKED {
    AnimFileMaskEncodingFullyBlack, //<! No mask data. Fully black mask: none of the pixels are present, frame is no different from last one
    AnimFileMaskEncodingFullyWhite, //<! No mask data. Fully white mask: all `width`x`height` pixels are present
    AnimFileMaskEncodingRleFirstBlack, //<! Binary run-length encoding, first pixel is black
    AnimFileMaskEncodingRleFirstWhite, //<! Binary run-length encoding, first pixel is white
    AnimFileMaskEncodingBitmap, //<! Binary bitmap
    AnimFileMaskEncodingMAX,
} AnimFileMaskEncoding;
static_assert(sizeof(AnimFileMaskEncoding) == sizeof(uint8_t));

/**
 * @brief Encoding mode of Frame Pixels
 */
typedef enum FURI_PACKED {
    AnimFilePixelEncodingRaw, //<! Plain pixels encoded according to `AnimFileColorFormat`
    AnimFilePixelEncodingRle, //<! Run-length encoding of `Raw` data implemented by `toolbox/rle_encode`. `blk_size` parameter is 3 for `Bgr888`, 1 for `Gray4`, and 4 for `Bgra8888`
    AnimFilePixelEncodingQoiLike, //<! QOI-like (without header and footer); hash and difference based. https://qoiformat.org/qoi-specification.pdf
    AnimFilePixelEncodingMAX,
} AnimFilePixelEncoding;
static_assert(sizeof(AnimFilePixelEncoding) == sizeof(uint8_t));

#define ANIM_FILE_QOI_OP_RGB         0xfe
#define ANIM_FILE_QOI_OP_RGBA        0xff
#define ANIM_FILE_QOI_SHORT_TAG_MASK 0xC0
#define ANIM_FILE_QOI_SHORT_OP_INDEX 0x00
#define ANIM_FILE_QOI_SHORT_OP_DIFF  0x40
#define ANIM_FILE_QOI_SHORT_OP_LUMA  0x80
#define ANIM_FILE_QOI_SHORT_OP_RUN   0xC0

/**
 * @brief Header of one File Frame
 */
typedef struct FURI_PACKED {
    uint8_t joint_encoding; //<! `PixelEncoding` in bottom 4 bits, `MaskEncoding` in top 4 bits
    uint16_t mask_length; //<! Length of encoded mask data following this header (in bits)
    uint16_t pixel_length; //<! Length of encoded pixel data following the mask data (in bytes)
} AnimFileFrameHeader;

static inline AnimFilePixelEncoding anim_file_px_encoding(uint8_t joint_encoding) {
    return joint_encoding & 0xF;
}

static inline AnimFileMaskEncoding anim_file_mask_encoding(uint8_t joint_encoding) {
    return (joint_encoding & 0xF0) >> 4;
}

#ifdef __cplusplus
}
#endif
