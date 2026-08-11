# File Formats {#file_formats}

The BUSY Bar uses a number of custom file formats. This page describes them in
detail, as well as provides rationale for inventing these formats as opposed to
using already established ones.

## Animations (`.anim`)

### Features
  - Color modes:
    - True color (24-bit RGB)
    - True color with alpha (32-bit RGBA)
    - 16-level grayscale (4-bit)
  - Efficient encoding for low-resource devices
  - Run-length encoding (very basic intraframe and interframe lossless
    compression)
  - Named sections (ranges of frame numbers)

### Why invent this?
None of the preexisting public bitmap animation formats support what we call
"sections", i.e. named ranges of frame numbers. They are used all over the
place in the BUSY Bar, mostly in animated menus. Ever seen the "START / SETUP"
one in BUSY, CUSTOM and APPS modes? Those are actually animations with 4
sections:
  - two for when each of the options is resting while selected (`item-0`, `item-1`);
  - another two for the transitions between them (`transition-0-to-1`, `transition-1-to-0`).

If we went with any one of the preexisting formats, we would have to store the
metadata for frame ranges that correspond to these sections somewhere else. By
inventing our own format, we can consolidate this data into one file.

This also comes with other niceties: we can precisely control the tradeoff
between CPU usage during decoding (a limited resource), storage space usage
(also a limited resource), and decoder code size (also a limited resource).
Welcome to microcontrollers.

Nevertheless, this format was assigned the codename `bicycle` (as in "to invent
a bicycle"), which is present in the signature.

| Format    | Deficiencies                                                 |
|-----------|--------------------------------------------------------------|
| GIF       | No section support. No true color support. Size inefficient. |
| APNG      | No section support. Large decoder code size.                 |
| WebP      | No section support. Large decoder code size.                 |
| AVIF      | No section support. Prohibitively large code size.           |
| Lottie    | Vector format - large CPU usage and decoder code size.       |
| bicycle   | Custom solution. Larger-than-ideal decoder code size.        |

### Format description
For precise up-to-date description of file data structures, read
`lib/anim_file/anim_file_format.h`. What's given here is a brief overview to
ease your dive into the aforementioned file.

#### Buffers
Buffers are variants of one frame, encoded to various degrees. The encoder
always starts with a `BGRA8888` image that it needs to turn into a blob that has
the smallest possible size within the limits of this specification. The decoder
starts with this blob (which could be in any of the 6 format combinations), then
possibly apply complicated subpixel math to it for smooth movement, then always
spit out a `BGRA8888` image that LVGL can accept directly.

Firstly, we have two screens with different color capabilities. The front
screen can display true color (24bpp), whereas the back one can only display
4-bit grayscale. Right off the bat, we can pack two pixels of an image for the
back display into one byte. This variant of the image is called the "Packed"
buffer. This "packing" operation is controlled by the file-wide color format
setting.

Then, long runs of equal pixels can be compressed using "run-length" encoding.
In the byte stream of an "Encoded" buffer, it places two main markers: "the
following is a run of N different pixels that I can't summarize", or "the
following one pixel should be replicated N times". N can never be more than 127
in either case.

The encoder thus follows a simple procedure to encode any single frame:
  - "Sheet" buffer: Start with a single `BGRA8888` frame to encode.
  - "Packed" buffer: If the requested color format is different (e.g. `BGR888`
    or `Gray4`), pack pixels: discard the alpha channel, or place two pixels in
    one byte respectively.
  - "Encoded" buffer: Always try to encode the "Packed" buffer using RLE
    encoding. If the resulting size is larger than without it (due to markers
    that it inserts every 127 pixels), use raw encoding (don't write the RLE
    result).

If you were wondering why the source buffer is called a "Sheet" buffer in the
description above, it has to do with subpixel translation in the decoder. Its
pipeline is as follows:
  - "Encoded" buffer: Start with a byte stream from the encoder.
  - "Packed" buffer: If the the encoder chose RLE for this frame, apply
    run-length decoding.
  - "Sheet" buffer: If the file-wide color format field differs from `BGRA8888`,
    unpack each pixel: either insert a fully opaque alpha channel (for `BGR888`)
    or extend the grayscale channel to R, G and B channels (for `Gray4`).
  - "Cutout" buffer: The application would sometimes request to move the
    animation around the screen. We use LVGL, which is perfectly capable of
    handling translation of widgets. Except, it can't do sub-pixel translation:
    all coordinates are integers. We had to invent something to support this
    functionality. As it turned out, thinking about this operation in terms of
    moving a "Cutout" of the original "Sheet" is easier from an implementation
    standpoint. Anyway, if this convoluted operation was requested, its result
    gets put in this buffer.

#### Sequencing
In addition to basic intraframe compression (run-length encoding tries to reduce
the size of equal adjacent pixels), this format also features basic RLE for
interframe compression. Meaning, if the image stays still for a few consecutive
frames, the frames are collapsed into one.

New terminology!
  - "Display frame": a frame that the encoder receives, or a decoder gives out.
    In intraframe terminology, these are referred to as "Sheet buffers".
  - "File frame": a frame that's actually present in the file. This "File frame"
    has a `duration` field which indicates how many "Display frames" this image
    should actually be played for.

The encoder, upon detecting that the image stays still for a few consecutive
frames, generates a "File frame" with `duration > 1`.

The decoder, upon encountering a long "File frame" (with `duration > 1`), simply
passes it through the Buffer pipeline once and ignores the next `duration - 1`
requests to process the next frame, assuming that the output buffer stays
unchanged in between the requests.

#### Sections
Sections are simply named ranges of "Display frame" indices.

Consider this sequence:
```
Display frame idx: 0   1   2   3   4   5   6   7   8   9  10  11  12  13  14
Frame content:     A   
                       B
                           C   C   C
                                       D   D   D   D
                                                       E
                                                           F   F
                                                                   G
                                                                       H
                                                                           I
my_section:                    |               |
```

"Display frames" 2..4 are condensed into one "File frame" C. So are 5..8 (D) and
10..11 (F).

The section "my_section" is in the range 3...7, which starts in the middle of
"File frame" C and ends in the middle of "File frame" D. To relieve the burden
on the decoder, the encoder precomputes the exact "File frame" offset of the
start of a section, and a `duration_override` value.

The encoder states: "my_section" starts on "File frame" C, which you should play
for 2 frames instead of the usual 3, then move on.

The decoder simply follows the commands at the request of the encoder.

#### Detailed definitions
See `lib/anim_file/anim_file_format.h`.

## Images (`.image`)

We use bitmap images in the LVGL format. For its description, refer to
[official documentation](https://lvgl.io/docs/open/9.0/overview/image).
