export type VideoFitMode = 'cover' | 'contain' | 'stretch';

export interface VideoCropRect {
  x: number;
  y: number;
  width: number;
  height: number;
}

export interface VideoCropState {
  offsetX: number;
  offsetY: number;
  scale: number;
}

export interface TimedFrame {
  imageData: ImageData;
  durationMs: number;
}

export interface FrameCache {
  frames: ImageData[];
  width: number;
  height: number;
  fps: number;
  startTime: number;
  endTime: number;
}

export interface RenderVideoFramesOptions {
  width: number;
  height: number;
  fit: VideoFitMode;
  crop?: VideoCropRect;
}

export const VIDEO_FRAME_SUPERSAMPLE = 4;
export const VIDEO_SEEK_TIMEOUT_MS = 8000;
export const VIDEO_METADATA_TIMEOUT_MS = 15000;
export const VIDEO_CROP_MIN_SCALE = 0.2;
export const VIDEO_FRAME_CACHE_MEMORY_BUDGET = 64 * 1024 * 1024;
export const VIDEO_FRAME_CACHE_MAX_WIDTH = 720;
export const VIDEO_DEFAULT_FRAME_DURATION_MS = 100;

export const VIDEO_FIT_OPTIONS: Array<{ label: string; value: VideoFitMode }> = [
  { label: 'Fill (crop)', value: 'cover' },
  { label: 'Fit (letterbox)', value: 'contain' },
  { label: 'Stretch', value: 'stretch' }
];

export const VIDEO_FPS_OPTIONS = [10, 15, 24, 30, 60];

export function getCoverCropRect (
  sourceWidth: number,
  sourceHeight: number,
  targetWidth: number,
  targetHeight: number,
  crop: VideoCropState
): VideoCropRect {
  const targetRatio = targetWidth / targetHeight;
  const sourceRatio = sourceWidth / sourceHeight;
  const scale = Math.min(1, Math.max(VIDEO_CROP_MIN_SCALE, crop.scale));
  const maxWidth = sourceRatio > targetRatio ? sourceHeight * targetRatio : sourceWidth;
  const maxHeight = sourceRatio > targetRatio ? sourceHeight : sourceWidth / targetRatio;
  const width = (maxWidth * scale) / sourceWidth;
  const height = (maxHeight * scale) / sourceHeight;

  return {
    x: clamp01(crop.offsetX) * (1 - width),
    y: clamp01(crop.offsetY) * (1 - height),
    width,
    height
  };
}

export function getVideoFrameCacheSize (
  sourceWidth: number,
  sourceHeight: number,
  frameCount: number,
  minWidth = 1
) {
  const aspect = sourceWidth / Math.max(1, sourceHeight);
  const budgetPerFrame = VIDEO_FRAME_CACHE_MEMORY_BUDGET / Math.max(1, frameCount);
  const budgetWidth = Math.sqrt((budgetPerFrame * aspect) / 4);
  const cappedWidth = Math.min(VIDEO_FRAME_CACHE_MAX_WIDTH, budgetWidth);
  const width = Math.max(1, Math.floor(Math.min(sourceWidth, Math.max(cappedWidth, minWidth))));

  return {
    width,
    height: Math.max(1, Math.round(width / aspect))
  };
}

export function getFrameGrid (fps: number, startTime: number, endTime: number, maxFrames: number) {
  const safeFps = Math.max(1, Math.round(fps));
  const safeStart = Math.max(0, startTime);
  const safeEnd = Math.max(safeStart, endTime);
  const frameCount = Math.max(1, Math.min(maxFrames, Math.floor((safeEnd - safeStart) * safeFps)));

  return { fps: safeFps, startTime: safeStart, endTime: safeStart + frameCount / safeFps, frameCount };
}

export function createFrameScaler (targetWidth: number, targetHeight: number) {
  const canvas = document.createElement('canvas');
  canvas.width = targetWidth;
  canvas.height = targetHeight;

  const context = canvas.getContext('2d', { willReadFrequently: true });

  if (!context) {
    throw new Error('Could not create canvas context');
  }

  context.imageSmoothingEnabled = true;
  context.imageSmoothingQuality = 'high';

  return {
    scale (source: CanvasImageSource): ImageData {
      context.drawImage(source, 0, 0, targetWidth, targetHeight);

      return context.getImageData(0, 0, targetWidth, targetHeight);
    }
  };
}

export function resampleTimedFrames (
  frames: TimedFrame[],
  fps: number,
  startTime: number,
  endTime: number,
  maxFrames: number
): { frames: ImageData[]; fps: number; startTime: number; endTime: number } {
  if (!frames.length) {
    throw new Error('No frames to resample');
  }

  const totalMs = frames.reduce((sum, frame) => sum + Math.max(1, frame.durationMs), 0);
  const grid = getFrameGrid(fps, startTime, Math.min(endTime, totalMs / 1000), maxFrames);
  const output: ImageData[] = [];

  let sourceIndex = 0;
  let sourceEndMs = Math.max(1, frames[0].durationMs);

  for (let index = 0; index < grid.frameCount; index++) {
    const timeMs = (grid.startTime + index / grid.fps) * 1000;

    while (timeMs >= sourceEndMs && sourceIndex < frames.length - 1) {
      sourceIndex += 1;
      sourceEndMs += Math.max(1, frames[sourceIndex].durationMs);
    }

    output.push(frames[sourceIndex].imageData);
  }

  return { frames: output, fps: grid.fps, startTime: grid.startTime, endTime: grid.endTime };
}

export function sliceFrameCache (cache: FrameCache, startTime: number, endTime: number): FrameCache | null {
  const epsilon = 0.5 / cache.fps;

  if (startTime < cache.startTime - epsilon || endTime > cache.endTime + epsilon) {
    return null;
  }

  const startIndex = Math.max(0, Math.round((startTime - cache.startTime) * cache.fps));
  const count = Math.max(1, Math.min(cache.frames.length - startIndex, Math.floor((endTime - startTime) * cache.fps)));

  return {
    frames: cache.frames.slice(startIndex, startIndex + count),
    width: cache.width,
    height: cache.height,
    fps: cache.fps,
    startTime: cache.startTime + startIndex / cache.fps,
    endTime: cache.startTime + (startIndex + count) / cache.fps
  };
}

export function imageDataToCanvas (imageData: ImageData): HTMLCanvasElement {
  const canvas = document.createElement('canvas');
  canvas.width = imageData.width;
  canvas.height = imageData.height;
  canvas.getContext('2d')?.putImageData(imageData, 0, 0);

  return canvas;
}

export function renderVideoFrames (
  cache: FrameCache,
  options: RenderVideoFramesOptions
): ImageData[] {
  const sourceCanvas = document.createElement('canvas');
  sourceCanvas.width = cache.width;
  sourceCanvas.height = cache.height;

  const sampleCanvas = document.createElement('canvas');
  sampleCanvas.width = options.width * VIDEO_FRAME_SUPERSAMPLE;
  sampleCanvas.height = options.height * VIDEO_FRAME_SUPERSAMPLE;

  const outputCanvas = document.createElement('canvas');
  outputCanvas.width = options.width;
  outputCanvas.height = options.height;

  const sourceContext = sourceCanvas.getContext('2d');
  const sampleContext = sampleCanvas.getContext('2d');
  const outputContext = outputCanvas.getContext('2d', { willReadFrequently: true });

  if (!sourceContext || !sampleContext || !outputContext) {
    throw new Error('Could not create canvas context');
  }

  sampleContext.imageSmoothingEnabled = true;
  sampleContext.imageSmoothingQuality = 'high';
  outputContext.imageSmoothingEnabled = true;
  outputContext.imageSmoothingQuality = 'high';

  const drawRect = options.fit === 'cover' && options.crop
    ? {
      sx: options.crop.x * cache.width,
      sy: options.crop.y * cache.height,
      sw: options.crop.width * cache.width,
      sh: options.crop.height * cache.height,
      dx: 0,
      dy: 0,
      dw: sampleCanvas.width,
      dh: sampleCanvas.height
    }
    : getFitDrawRect(cache.width, cache.height, sampleCanvas.width, sampleCanvas.height, options.fit);

  return cache.frames.map(frame => {
    sourceContext.putImageData(frame, 0, 0);

    sampleContext.fillStyle = '#000000';
    sampleContext.fillRect(0, 0, sampleCanvas.width, sampleCanvas.height);
    sampleContext.drawImage(
      sourceCanvas,
      drawRect.sx,
      drawRect.sy,
      drawRect.sw,
      drawRect.sh,
      drawRect.dx,
      drawRect.dy,
      drawRect.dw,
      drawRect.dh
    );

    outputContext.clearRect(0, 0, options.width, options.height);
    outputContext.drawImage(sampleCanvas, 0, 0, options.width, options.height);

    return outputContext.getImageData(0, 0, options.width, options.height);
  });
}

export function seekVideo (video: HTMLVideoElement, time: number): Promise<void> {
  return new Promise((resolve, reject) => {
    if (Math.abs(video.currentTime - time) < 0.0005 && video.readyState >= HTMLMediaElement.HAVE_CURRENT_DATA) {
      resolve();
      return;
    }

    const timeout = window.setTimeout(() => {
      cleanup();
      reject(new Error(`Timed out while seeking video to ${time.toFixed(2)}s`));
    }, VIDEO_SEEK_TIMEOUT_MS);

    const cleanup = () => {
      window.clearTimeout(timeout);
      video.removeEventListener('seeked', handleSeeked);
      video.removeEventListener('error', handleError);
    };

    const handleSeeked = () => {
      cleanup();
      resolve();
    };

    const handleError = () => {
      cleanup();
      reject(new Error('Video decoding failed while seeking'));
    };

    video.addEventListener('seeked', handleSeeked);
    video.addEventListener('error', handleError);
    video.currentTime = time;
  });
}

function getFitDrawRect (
  sourceWidth: number,
  sourceHeight: number,
  targetWidth: number,
  targetHeight: number,
  fit: VideoFitMode
) {
  if (fit === 'stretch') {
    return { sx: 0, sy: 0, sw: sourceWidth, sh: sourceHeight, dx: 0, dy: 0, dw: targetWidth, dh: targetHeight };
  }

  const sourceRatio = sourceWidth / sourceHeight;
  const targetRatio = targetWidth / targetHeight;

  if (fit === 'cover') {
    if (sourceRatio > targetRatio) {
      const sw = sourceHeight * targetRatio;
      return { sx: (sourceWidth - sw) / 2, sy: 0, sw, sh: sourceHeight, dx: 0, dy: 0, dw: targetWidth, dh: targetHeight };
    }

    const sh = sourceWidth / targetRatio;
    return { sx: 0, sy: (sourceHeight - sh) / 2, sw: sourceWidth, sh, dx: 0, dy: 0, dw: targetWidth, dh: targetHeight };
  }

  if (sourceRatio > targetRatio) {
    const dh = targetWidth / sourceRatio;
    return { sx: 0, sy: 0, sw: sourceWidth, sh: sourceHeight, dx: 0, dy: (targetHeight - dh) / 2, dw: targetWidth, dh };
  }

  const dw = targetHeight * sourceRatio;
  return { sx: 0, sy: 0, sw: sourceWidth, sh: sourceHeight, dx: (targetWidth - dw) / 2, dy: 0, dw, dh: targetHeight };
}

function clamp01 (value: number) {
  return Math.min(1, Math.max(0, value));
}
