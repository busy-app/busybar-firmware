import { applyPalette, GIFEncoder, quantize } from 'gifenc';
import type { DecodedAnimation } from '@/util/anim2seq';

export const GIF_MAX_COLORS = 256;

export function encodeAnimationToGif (animation: DecodedAnimation): Blob {
  if (!animation.frames.length) {
    throw new Error('Animation has no frames');
  }

  const encoder = GIFEncoder();
  const frameMs = 1000 / Math.max(1, animation.fps);

  animation.frames.forEach((frame, index) => {
    const rgba = frame.imageData.data;
    const palette = quantize(rgba, GIF_MAX_COLORS);
    const indexed = applyPalette(rgba, palette);

    encoder.writeFrame(indexed, frame.imageData.width, frame.imageData.height, {
      palette,
      delay: Math.max(20, Math.round(frame.duration * frameMs)),
      repeat: 0,
      first: index === 0
    });
  });

  encoder.finish();

  return new Blob([new Uint8Array(encoder.bytes())], { type: 'image/gif' });
}
