import { applyPalette, GIFEncoder, quantize } from 'gifenc';
import type { DecodedAnimation } from '@/util/anim2seq';

export const GIF_MAX_COLORS = 256;
// Viewers blur a 72x16 GIF when enlarging it, so we upscale with nearest neighbour ourselves.
export const GIF_PIXEL_SIZE = 8;

function upscaleIndices (indexed: Uint8Array, width: number, height: number, scale: number): Uint8Array {
  if (scale <= 1) {
    return indexed;
  }

  const outputWidth = width * scale;
  const output = new Uint8Array(outputWidth * height * scale);

  for (let y = 0; y < height; y += 1) {
    const rowStart = y * scale * outputWidth;

    for (let x = 0; x < width; x += 1) {
      const value = indexed[(y * width) + x];
      const columnStart = rowStart + (x * scale);

      for (let offset = 0; offset < scale; offset += 1) {
        output[columnStart + offset] = value;
      }
    }

    const row = output.subarray(rowStart, rowStart + outputWidth);

    for (let copy = 1; copy < scale; copy += 1) {
      output.set(row, rowStart + (copy * outputWidth));
    }
  }

  return output;
}

function flattenOnBlack (data: Uint8ClampedArray): Uint8ClampedArray {
  let flattened: Uint8ClampedArray | null = null;

  for (let offset = 0; offset < data.length; offset += 4) {
    const alpha = data[offset + 3];

    if (alpha === 255) {
      continue;
    }

    flattened ??= data.slice();
    flattened[offset] = (data[offset] * alpha) / 255;
    flattened[offset + 1] = (data[offset + 1] * alpha) / 255;
    flattened[offset + 2] = (data[offset + 2] * alpha) / 255;
    flattened[offset + 3] = 255;
  }

  return flattened ?? data;
}

export function encodeAnimationToGif (animation: DecodedAnimation, scale = GIF_PIXEL_SIZE): Blob {
  if (!animation.frames.length) {
    throw new Error('Animation has no frames');
  }

  const encoder = GIFEncoder();
  const frameMs = 1000 / Math.max(1, animation.fps);

  animation.frames.forEach((frame, index) => {
    const { width, height } = frame.imageData;
    const data = flattenOnBlack(frame.imageData.data);
    // Quantize at source size, then upscale indices: nearest neighbour adds no colours, and it's 64x cheaper.
    const palette = quantize(data, GIF_MAX_COLORS);
    const indexed = applyPalette(data, palette);

    encoder.writeFrame(upscaleIndices(indexed, width, height, scale), width * scale, height * scale, {
      palette,
      delay: Math.max(20, Math.round(frame.holdFrames * frameMs)),
      repeat: 0,
      first: index === 0
    });
  });

  encoder.finish();

  return new Blob([new Uint8Array(encoder.bytes())], { type: 'image/gif' });
}
