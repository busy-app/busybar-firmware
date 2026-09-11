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

async function decodeImage (buffer: ArrayBuffer, mime: string) {
  if (typeof ImageDecoder === 'undefined') {
    throw new Error('ImageDecoder is not available in this browser');
  }

  const decoder = new ImageDecoder({ data: buffer, type: mime });

  await decoder.tracks.ready;

  const track = decoder.tracks.selectedTrack;

  if (!track) {
    throw new Error('Image has no decodable track');
  }

  await decoder.completed;

  const frameCount = Math.max(1, track.frameCount);
  const frames: DecodedFrameMessage[] = [];
  let width = 0;
  let height = 0;

  for (let index = 0; index < frameCount; index++) {
    const result = await decoder.decode({ frameIndex: index });
    const image = result.image;

    width = image.displayWidth;
    height = image.displayHeight;

    const canvas = new OffscreenCanvas(width, height);
    const context = canvas.getContext('2d', { willReadFrequently: true });

    if (!context) {
      image.close();
      throw new Error('Could not create canvas context');
    }

    context.drawImage(image, 0, 0);

    const imageData = context.getImageData(0, 0, width, height);
    const durationMs = image.duration ? image.duration / 1000 : DEFAULT_FRAME_DURATION_MS;

    image.close();
    frames.push(toMessageFrame(imageData, durationMs));
  }

  decoder.close();

  return { width, height, frames };
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
