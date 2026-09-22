import { createFrameScaler, getVideoFrameCacheSize, imageDataToCanvas, resampleTimedFrames } from '@/util/videoFrames';
import type { FrameCache } from '@/util/videoFrames';
import { decodeAnimationFrames, decodeImageFrames } from './decoderClient';
import type { DecodedFrameSet } from './decoderClient';
import { getFileExtension } from './types';
import type { DecodeFramesOptions, FrameSourceAdapter, FrameSourceHandle } from './types';

const IMAGE_MIME_BY_EXTENSION: Record<string, string> = {
  gif: 'image/gif'
};

const SCALE_YIELD_EVERY_FRAMES = 32;

function toHandle (adapterId: string, decoded: DecodedFrameSet): FrameSourceHandle {
  const totalMs = decoded.frames.reduce((sum, frame) => sum + Math.max(1, frame.durationMs), 0);
  const shortestMs = decoded.frames.reduce((shortest, frame) => Math.min(shortest, Math.max(1, frame.durationMs)), Infinity);

  return {
    kind: 'frames',
    adapterId,
    width: decoded.width,
    height: decoded.height,
    duration: totalMs / 1000,
    nativeFps: decoded.fps ?? Math.max(1, Math.round(1000 / shortestMs)),
    frames: decoded.frames,
    release: () => undefined
  };
}

async function decodeFrames (handle: FrameSourceHandle, options: DecodeFramesOptions): Promise<FrameCache> {
  const source = handle.frames;

  if (!source?.length) {
    throw new Error('Frame handle has no frames');
  }

  const resampled = resampleTimedFrames(source, options.fps, options.startTime, options.endTime, options.maxFrames);
  const size = getVideoFrameCacheSize(handle.width, handle.height, resampled.frames.length);
  const needsScale = size.width !== handle.width || size.height !== handle.height;

  let frames = resampled.frames;

  if (needsScale) {
    const scaler = createFrameScaler(size.width, size.height);
    const scaledByFrame = new Map<ImageData, ImageData>();
    const scaled: ImageData[] = [];

    for (let index = 0; index < resampled.frames.length; index++) {
      if (index % SCALE_YIELD_EVERY_FRAMES === 0) {
        await new Promise(resolve => setTimeout(resolve));

        if (options.signal?.aborted) {
          throw new DOMException('Frame decoding aborted', 'AbortError');
        }
      }

      const frame = resampled.frames[index];
      let cached = scaledByFrame.get(frame);

      if (!cached) {
        cached = scaler.scale(imageDataToCanvas(frame));
        scaledByFrame.set(frame, cached);
      }

      scaled.push(cached);
      options.onProgress?.(index + 1, resampled.frames.length);
    }

    frames = scaled;
  }

  options.onProgress?.(frames.length, frames.length);

  return {
    frames,
    width: size.width,
    height: size.height,
    fps: resampled.fps,
    startTime: resampled.startTime,
    endTime: resampled.endTime
  };
}

export const animAdapter: FrameSourceAdapter = {
  id: 'anim',
  label: 'BUSY animation',
  accepts: file => getFileExtension(file) === 'anim',
  open: async file => toHandle('anim', await decodeAnimationFrames(await file.arrayBuffer())),
  decode: decodeFrames
};

export const imageAdapter: FrameSourceAdapter = {
  id: 'image',
  label: 'Animated image',
  accepts: file => {
    const mime = file.type || IMAGE_MIME_BY_EXTENSION[getFileExtension(file)];

    return mime === 'image/gif';
  },
  open: async file => {
    const mime = file.type || IMAGE_MIME_BY_EXTENSION[getFileExtension(file)];

    return toHandle('image', await decodeImageFrames(await file.arrayBuffer(), mime));
  },
  decode: decodeFrames
};
