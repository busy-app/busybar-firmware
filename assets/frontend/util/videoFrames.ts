export type VideoFitMode = 'cover' | 'contain' | 'stretch';

export interface VideoSource {
  video: HTMLVideoElement;
  url: string;
  duration: number;
  width: number;
  height: number;
  release: () => void;
}

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

export interface VideoFrameCache {
  frames: ImageData[];
  width: number;
  height: number;
  fps: number;
  truncated: boolean;
}

export interface DecodeVideoFramesOptions {
  fps: number;
  maxDurationSeconds: number;
  minWidth?: number;
  signal?: AbortSignal;
  onProgress?: (done: number, total: number) => void;
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

export const VIDEO_FIT_OPTIONS: Array<{ label: string; value: VideoFitMode }> = [
  { label: 'Fill (crop)', value: 'cover' },
  { label: 'Fit (letterbox)', value: 'contain' },
  { label: 'Stretch', value: 'stretch' }
];

export const VIDEO_FPS_OPTIONS = [5, 10, 12, 15, 20, 24, 30];

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

export function loadVideoSource (file: File): Promise<VideoSource> {
  const url = URL.createObjectURL(file);
  const video = document.createElement('video');

  video.muted = true;
  video.playsInline = true;
  video.preload = 'auto';
  video.crossOrigin = 'anonymous';

  return new Promise((resolve, reject) => {
    const timeout = window.setTimeout(() => {
      cleanup();
      URL.revokeObjectURL(url);
      reject(new Error('Timed out while reading video metadata'));
    }, VIDEO_METADATA_TIMEOUT_MS);

    const cleanup = () => {
      window.clearTimeout(timeout);
      video.removeEventListener('loadeddata', handleLoaded);
      video.removeEventListener('error', handleError);
    };

    const handleLoaded = async () => {
      cleanup();

      const duration = await resolveVideoDuration(video);

      if (!Number.isFinite(duration) || duration <= 0 || !video.videoWidth || !video.videoHeight) {
        URL.revokeObjectURL(url);
        reject(new Error('Could not determine video duration or size'));
        return;
      }

      resolve({
        video,
        url,
        duration,
        width: video.videoWidth,
        height: video.videoHeight,
        release: () => {
          video.removeAttribute('src');
          video.load();
          URL.revokeObjectURL(url);
        }
      });
    };

    const handleError = () => {
      cleanup();
      URL.revokeObjectURL(url);
      reject(new Error('Could not decode video file'));
    };

    video.addEventListener('loadeddata', handleLoaded);
    video.addEventListener('error', handleError);
    video.src = url;
    video.load();
  });
}

export async function decodeVideoFrames (
  source: VideoSource,
  options: DecodeVideoFramesOptions
): Promise<VideoFrameCache> {
  const fps = Math.max(1, Math.round(options.fps));
  const usableDuration = Math.min(source.duration, options.maxDurationSeconds);
  const truncated = source.duration > options.maxDurationSeconds;
  const frameCount = Math.max(1, Math.floor(usableDuration * fps));
  const size = getVideoFrameCacheSize(source.width, source.height, frameCount, options.minWidth);

  const canvas = document.createElement('canvas');
  canvas.width = size.width;
  canvas.height = size.height;

  const context = canvas.getContext('2d', { willReadFrequently: true });

  if (!context) {
    throw new Error('Could not create canvas context');
  }

  context.imageSmoothingEnabled = true;
  context.imageSmoothingQuality = 'high';

  const frames: ImageData[] = [];

  for (let index = 0; index < frameCount; index++) {
    if (options.signal?.aborted) {
      throw new DOMException('Video frame decoding aborted', 'AbortError');
    }

    const time = Math.min(index / fps, Math.max(0, source.duration - 0.001));
    await seekVideo(source.video, time);

    context.drawImage(source.video, 0, 0, size.width, size.height);
    frames.push(context.getImageData(0, 0, size.width, size.height));

    options.onProgress?.(index + 1, frameCount);
  }

  return { frames, width: size.width, height: size.height, fps, truncated };
}

export function renderVideoFrames (
  cache: VideoFrameCache,
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

async function resolveVideoDuration (video: HTMLVideoElement): Promise<number> {
  if (Number.isFinite(video.duration)) {
    return video.duration;
  }

  await new Promise<void>(resolve => {
    const timeout = window.setTimeout(finish, VIDEO_SEEK_TIMEOUT_MS);

    function finish () {
      window.clearTimeout(timeout);
      video.removeEventListener('durationchange', finish);
      video.removeEventListener('seeked', finish);
      resolve();
    }

    video.addEventListener('durationchange', finish);
    video.addEventListener('seeked', finish);
    video.currentTime = Number.MAX_SAFE_INTEGER;
  });

  const duration = video.duration;

  await seekVideo(video, 0).catch(() => undefined);

  return duration;
}

function seekVideo (video: HTMLVideoElement, time: number): Promise<void> {
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
