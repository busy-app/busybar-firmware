import { decodeAnimation } from '@/util/anim2seq';
import type { TimedFrame } from '@/util/videoFrames';

export interface DecodedFrameSet {
  width: number;
  height: number;
  fps?: number;
  frames: TimedFrame[];
}

type DecodeRequest
  = | { type: 'anim'; buffer: ArrayBuffer }
    | { type: 'image'; buffer: ArrayBuffer; mime: string };

type WorkerFrame = { width: number; height: number; durationMs: number; buffer: ArrayBuffer };

type WorkerResponse
  = | { id: number; ok: true; width: number; height: number; fps?: number; frames: WorkerFrame[] }
    | { id: number; ok: false; error: string };

let worker: Worker | null = null;
let requestCounter = 0;
const pending = new Map<number, { resolve: (value: DecodedFrameSet) => void; reject: (error: Error) => void }>();

function getWorker (): Worker | null {
  if (typeof Worker === 'undefined') {
    return null;
  }

  if (worker) {
    return worker;
  }

  try {
    worker = new Worker(new URL('../../workers/frameDecoder.ts', import.meta.url), { type: 'module' });
  } catch {
    return null;
  }

  worker.onmessage = (event: MessageEvent<WorkerResponse>) => {
    const response = event.data;
    const entry = pending.get(response.id);

    if (!entry) {
      return;
    }

    pending.delete(response.id);

    if (!response.ok) {
      entry.reject(new Error(response.error));
      return;
    }

    entry.resolve({
      width: response.width,
      height: response.height,
      fps: response.fps,
      frames: response.frames.map(frame => ({
        imageData: new ImageData(new Uint8ClampedArray(frame.buffer), frame.width, frame.height),
        durationMs: frame.durationMs
      }))
    });
  };

  worker.onerror = () => {
    pending.forEach(entry => entry.reject(new Error('Frame decoder worker crashed')));
    pending.clear();
    worker?.terminate();
    worker = null;
  };

  return worker;
}

function decodeAnimOnMainThread (buffer: ArrayBuffer): DecodedFrameSet {
  const animation = decodeAnimation(buffer);
  const frameMs = 1000 / Math.max(1, animation.fps);

  return {
    width: animation.width,
    height: animation.height,
    fps: animation.fps,
    frames: animation.frames.map(frame => ({ imageData: frame.imageData, durationMs: frame.duration * frameMs }))
  };
}

export function decodeFramesInWorker (request: DecodeRequest): Promise<DecodedFrameSet> {
  const activeWorker = getWorker();

  if (!activeWorker) {
    if (request.type === 'anim') {
      return Promise.resolve(decodeAnimOnMainThread(request.buffer));
    }

    return Promise.reject(new Error('Image decoding requires a worker in this browser'));
  }

  const id = ++requestCounter;

  return new Promise((resolve, reject) => {
    pending.set(id, { resolve, reject });
    activeWorker.postMessage({ id, ...request }, [request.buffer]);
  });
}

export function decodeAnimationFrames (buffer: ArrayBuffer) {
  return decodeFramesInWorker({ type: 'anim', buffer });
}

export function decodeImageFrames (buffer: ArrayBuffer, mime: string) {
  return decodeFramesInWorker({ type: 'image', buffer, mime });
}
