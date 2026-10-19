import { decompressFrame, parseGIF } from 'gifuct-js';
import { decodeAnimation } from '../util/anim2seq';
import { getVideoFrameCacheSize } from '../util/videoFrames';
import { VIDEO_SOURCE_MAX_FRAME_PIXELS, VIDEO_SOURCE_MAX_FRAMES } from '../util/videoLimits';

type GifImageFrame = Parameters<typeof decompressFrame>[0];

type DecodeRequest
  = | { id: number; type: 'anim'; buffer: ArrayBuffer; maxFrames?: number }
    | { id: number; type: 'image'; buffer: ArrayBuffer; mime: string };

type DecodedFrameMessage = {
  width: number;
  height: number;
  durationMs: number;
  buffer: ArrayBuffer;
};

type DecodeResponse
  = | { id: number; ok: true; width: number; height: number; fps?: number; frames: DecodedFrameMessage[] }
    | { id: number; ok: false; error: string };

const DEFAULT_FRAME_DURATION_MS = 100;
const GIF_DISPOSAL_RESTORE_BACKGROUND = 2;
const GIF_DISPOSAL_RESTORE_PREVIOUS = 3;

function toMessageFrame (imageData: ImageData, durationMs: number): DecodedFrameMessage {
  return {
    width: imageData.width,
    height: imageData.height,
    durationMs,
    buffer: imageData.data.buffer as ArrayBuffer
  };
}

async function decodeAnim (buffer: ArrayBuffer, maxFrames?: number) {
  const animation = decodeAnimation(buffer, maxFrames);
  const frameMs = 1000 / Math.max(1, animation.fps);

  return {
    width: animation.width,
    height: animation.height,
    fps: animation.fps,
    frames: animation.frames.map(frame => toMessageFrame(frame.imageData, frame.holdFrames * frameMs))
  };
}

function clearRect (canvas: Uint8ClampedArray, canvasWidth: number, canvasHeight: number, dims: { left: number; top: number; width: number; height: number }) {
  const left = Math.max(0, dims.left);
  const right = Math.min(canvasWidth, dims.left + dims.width);

  if (right <= left) {
    return;
  }

  for (let y = Math.max(0, dims.top); y < Math.min(canvasHeight, dims.top + dims.height); y++) {
    canvas.fill(0, ((y * canvasWidth) + left) * 4, ((y * canvasWidth) + right) * 4);
  }
}

function createGifScaler (sourceWidth: number, sourceHeight: number, targetWidth: number, targetHeight: number) {
  if (typeof OffscreenCanvas === 'undefined') {
    return null;
  }

  const source = new OffscreenCanvas(sourceWidth, sourceHeight);
  const sourceContext = source.getContext('2d');
  const target = new OffscreenCanvas(targetWidth, targetHeight);
  const targetContext = target.getContext('2d', { willReadFrequently: true });

  if (!sourceContext || !targetContext) {
    return null;
  }

  targetContext.imageSmoothingEnabled = true;
  targetContext.imageSmoothingQuality = 'high';

  const sourceImage = new ImageData(sourceWidth, sourceHeight);

  return (pixels: Uint8ClampedArray) => {
    sourceImage.data.set(pixels);
    sourceContext.putImageData(sourceImage, 0, 0);
    targetContext.drawImage(source, 0, 0, targetWidth, targetHeight);

    return targetContext.getImageData(0, 0, targetWidth, targetHeight).data.buffer as ArrayBuffer;
  };
}

// Not ImageDecoder: it is unavailable on the device's plain-http origin, so gifuct is the only path.
function decodeGif (buffer: ArrayBuffer) {
  const gif = parseGIF(buffer);
  const width = gif.lsd.width;
  const height = gif.lsd.height;
  const imageFrames = gif.frames.filter((frame): frame is GifImageFrame => 'image' in frame && !!frame.image);

  if (!width || !height || !imageFrames.length) {
    throw new Error('GIF has no decodable frames');
  }

  if (imageFrames.length > VIDEO_SOURCE_MAX_FRAMES) {
    throw new Error(`This GIF has ${imageFrames.length} frames. The limit is ${VIDEO_SOURCE_MAX_FRAMES}.`);
  }

  const isOversized = width * height > VIDEO_SOURCE_MAX_FRAME_PIXELS
    || imageFrames.some(({ image: { descriptor } }) => descriptor.width * descriptor.height > VIDEO_SOURCE_MAX_FRAME_PIXELS);

  if (isOversized) {
    throw new Error('This GIF is too large to decode. Scale it down and try again.');
  }

  const size = getVideoFrameCacheSize(width, height, imageFrames.length);
  const scale = size.width === width && size.height === height
    ? null
    : createGifScaler(width, height, size.width, size.height);
  const outputWidth = scale ? size.width : width;
  const outputHeight = scale ? size.height : height;
  const canvas = new Uint8ClampedArray(width * height * 4);
  const frames: DecodedFrameMessage[] = [];

  // gifuct yields per-frame patches; compositing them and applying disposal is up to us.
  // Decompress one frame at a time so only a single patch is alive.
  imageFrames.forEach(imageFrame => {
    const frame = decompressFrame(imageFrame, gif.gct, true);
    const { left, top, width: patchWidth, height: patchHeight } = frame.dims;
    const previous = frame.disposalType === GIF_DISPOSAL_RESTORE_PREVIOUS ? canvas.slice() : null;

    for (let y = 0; y < patchHeight; y++) {
      const canvasY = top + y;

      if (canvasY < 0 || canvasY >= height) {
        continue;
      }

      for (let x = 0; x < patchWidth; x++) {
        const canvasX = left + x;
        const source = ((y * patchWidth) + x) * 4;

        if (canvasX < 0 || canvasX >= width || frame.patch[source + 3] === 0) {
          continue;
        }

        canvas.set(frame.patch.subarray(source, source + 4), ((canvasY * width) + canvasX) * 4);
      }
    }

    frames.push({
      width: outputWidth,
      height: outputHeight,
      durationMs: frame.delay || DEFAULT_FRAME_DURATION_MS,
      buffer: scale ? scale(canvas) : canvas.slice().buffer
    });

    if (frame.disposalType === GIF_DISPOSAL_RESTORE_BACKGROUND) {
      clearRect(canvas, width, height, frame.dims);
    } else if (previous) {
      canvas.set(previous);
    }
  });

  return { width: outputWidth, height: outputHeight, frames };
}

async function decodeImage (buffer: ArrayBuffer, mime: string) {
  if (mime !== 'image/gif') {
    throw new Error('Unsupported image. Use a GIF or a video instead.');
  }

  return decodeGif(buffer);
}

self.onmessage = async (event: MessageEvent<DecodeRequest>) => {
  const request = event.data;

  try {
    const result = request.type === 'anim'
      ? await decodeAnim(request.buffer, request.maxFrames)
      : await decodeImage(request.buffer, request.mime);

    const response: DecodeResponse = { id: request.id, ok: true, ...result };

    self.postMessage(response, { transfer: result.frames.map(frame => frame.buffer) });
  } catch (error) {
    const response: DecodeResponse = {
      id: request.id,
      ok: false,
      error: error instanceof Error ? error.message : String(error)
    };

    self.postMessage(response);
  }
};
