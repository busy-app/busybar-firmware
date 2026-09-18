import { decompressFrames, parseGIF } from 'gifuct-js';
import { decodeAnimation } from '../util/anim2seq';

type DecodeRequest
  = | { id: number; type: 'anim'; buffer: ArrayBuffer }
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

async function decodeAnim (buffer: ArrayBuffer) {
  const animation = decodeAnimation(buffer);
  const frameMs = 1000 / Math.max(1, animation.fps);

  return {
    width: animation.width,
    height: animation.height,
    fps: animation.fps,
    frames: animation.frames.map(frame => toMessageFrame(frame.imageData, frame.duration * frameMs))
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

function decodeGif (buffer: ArrayBuffer) {
  const gif = parseGIF(buffer);
  const width = gif.lsd.width;
  const height = gif.lsd.height;
  const parsedFrames = decompressFrames(gif, true);

  if (!width || !height || !parsedFrames.length) {
    throw new Error('GIF has no decodable frames');
  }

  const canvas = new Uint8ClampedArray(width * height * 4);
  const frames: DecodedFrameMessage[] = [];

  parsedFrames.forEach(frame => {
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
      width,
      height,
      durationMs: frame.delay || DEFAULT_FRAME_DURATION_MS,
      buffer: canvas.slice().buffer
    });

    if (frame.disposalType === GIF_DISPOSAL_RESTORE_BACKGROUND) {
      clearRect(canvas, width, height, frame.dims);
    } else if (previous) {
      canvas.set(previous);
    }
  });

  return { width, height, frames };
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
      ? await decodeAnim(request.buffer)
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
