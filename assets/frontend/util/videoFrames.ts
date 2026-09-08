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

export interface ExtractVideoFramesOptions {
  fps: number;
  width: number;
  height: number;
  fit: VideoFitMode;
  crop?: VideoCropRect;
  maxDurationSeconds: number;
  signal?: AbortSignal;
  onProgress?: (done: number, total: number) => void;
}

export const VIDEO_CROP_MIN_SCALE = 0.2;

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
  const width = maxWidth * scale;
  const height = maxHeight * scale;

  return {
    x: clamp01(crop.offsetX) * (sourceWidth - width),
    y: clamp01(crop.offsetY) * (sourceHeight - height),
    width,
    height
  };
}

function clamp01 (value: number) {
  return Math.min(1, Math.max(0, value));
}

export interface ExtractedVideoFrames {
  frames: ImageData[];
  truncated: boolean;
}

export const VIDEO_FRAME_SUPERSAMPLE = 4;
export const VIDEO_SEEK_TIMEOUT_MS = 8000;
export const VIDEO_METADATA_TIMEOUT_MS = 15000;

export const VIDEO_FIT_OPTIONS: Array<{ label: string; value: VideoFitMode }> = [
  { label: 'Fill (crop)', value: 'cover' },
  { label: 'Fit (letterbox)', value: 'contain' },
  { label: 'Stretch', value: 'stretch' }
];

export const VIDEO_FPS_OPTIONS = [5, 10, 12, 15, 20, 24, 30];

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

export async function extractVideoFrames (
  source: VideoSource,
  options: ExtractVideoFramesOptions
): Promise<ExtractedVideoFrames> {
  const { video } = source;
  const fps = Math.max(1, Math.round(options.fps));
  const usableDuration = Math.min(source.duration, options.maxDurationSeconds);
  const truncated = source.duration > options.maxDurationSeconds;
  const frameCount = Math.max(1, Math.floor(usableDuration * fps));

  const sampleCanvas = document.createElement('canvas');
  sampleCanvas.width = options.width * VIDEO_FRAME_SUPERSAMPLE;
  sampleCanvas.height = options.height * VIDEO_FRAME_SUPERSAMPLE;
  const sampleContext = sampleCanvas.getContext('2d');

  const outputCanvas = document.createElement('canvas');
  outputCanvas.width = options.width;
  outputCanvas.height = options.height;
  const outputContext = outputCanvas.getContext('2d', { willReadFrequently: true });

  if (!sampleContext || !outputContext) {
    throw new Error('Could not create canvas context');
  }

  sampleContext.imageSmoothingEnabled = true;
  sampleContext.imageSmoothingQuality = 'high';
  outputContext.imageSmoothingEnabled = true;
  outputContext.imageSmoothingQuality = 'high';

  const drawRect = options.fit === 'cover' && options.crop
    ? { sx: options.crop.x, sy: options.crop.y, sw: options.crop.width, sh: options.crop.height, dx: 0, dy: 0, dw: sampleCanvas.width, dh: sampleCanvas.height }
    : getFitDrawRect(source.width, source.height, sampleCanvas.width, sampleCanvas.height, options.fit);
  const frames: ImageData[] = [];

  for (let index = 0; index < frameCount; index++) {
    if (options.signal?.aborted) {
      throw new DOMException('Video frame extraction aborted', 'AbortError');
    }

    const time = Math.min(index / fps, Math.max(0, source.duration - 0.001));
    await seekVideo(video, time);

    sampleContext.fillStyle = '#000000';
    sampleContext.fillRect(0, 0, sampleCanvas.width, sampleCanvas.height);
    sampleContext.drawImage(
      video,
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
    frames.push(outputContext.getImageData(0, 0, options.width, options.height));

    options.onProgress?.(index + 1, frameCount);
  }

  return { frames, truncated };
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
