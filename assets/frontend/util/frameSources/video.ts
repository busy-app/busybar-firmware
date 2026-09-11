import {
  createFrameScaler,
  getFrameGrid,
  getVideoFrameCacheSize,
  seekVideo,
  VIDEO_METADATA_TIMEOUT_MS,
  VIDEO_SEEK_TIMEOUT_MS
} from '@/util/videoFrames';
import type { FrameCache } from '@/util/videoFrames';
import { getFileExtension } from './types';
import type { DecodeFramesOptions, FrameSourceAdapter, FrameSourceHandle } from './types';

const VIDEO_EXTENSIONS = new Set(['mp4', 'm4v', 'mov', 'mkv', 'webm', 'ogv', 'avi']);

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

async function estimateNativeFps (video: HTMLVideoElement): Promise<number | undefined> {
  if (!('requestVideoFrameCallback' in video)) {
    return undefined;
  }

  return new Promise(resolve => {
    const MIN_SAMPLES = 6;
    const MIN_SPAN_SECONDS = 0.4;
    const MAX_SAMPLES = 40;
    let first: number | null = null;
    let last: number | null = null;
    let count = 0;

    const finish = () => {
      window.clearTimeout(timeout);
      video.pause();

      if (first === null || last === null || count < MIN_SAMPLES || last - first < MIN_SPAN_SECONDS) {
        resolve(undefined);
        return;
      }

      const estimate = Math.round(count / (last - first));

      resolve(estimate >= 2 ? estimate : undefined);
    };

    const timeout = window.setTimeout(finish, 1500);

    const tick = (_now: number, metadata: VideoFrameCallbackMetadata) => {
      if (first === null) {
        first = metadata.mediaTime;
      } else {
        count += 1;
        last = metadata.mediaTime;
      }

      if (count >= MAX_SAMPLES || (count >= MIN_SAMPLES && last !== null && last - first >= MIN_SPAN_SECONDS)) {
        finish();
        return;
      }

      video.requestVideoFrameCallback(tick);
    };

    video.requestVideoFrameCallback(tick);
    video.play().catch(finish);
  });
}

function openVideo (file: File): Promise<FrameSourceHandle> {
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

      const nativeFps = await estimateNativeFps(video);
      await seekVideo(video, 0).catch(() => undefined);

      resolve({
        kind: 'video',
        adapterId: videoAdapter.id,
        width: video.videoWidth,
        height: video.videoHeight,
        duration,
        nativeFps,
        previewUrl: url,
        video,
        release: () => {
          video.pause();
          video.removeAttribute('src');
          video.load();
          URL.revokeObjectURL(url);
        }
      });
    };

    const handleError = () => {
      cleanup();
      URL.revokeObjectURL(url);
      reject(new Error('This browser could not decode the video. Try re-saving it as H.264 MP4.'));
    };

    video.addEventListener('loadeddata', handleLoaded);
    video.addEventListener('error', handleError);
    video.src = url;
    video.load();
  });
}

async function decodeVideo (handle: FrameSourceHandle, options: DecodeFramesOptions): Promise<FrameCache> {
  const video = handle.video;

  if (!video) {
    throw new Error('Video handle has no media element');
  }

  const grid = getFrameGrid(options.fps, options.startTime, Math.min(options.endTime, handle.duration), options.maxFrames);
  const size = getVideoFrameCacheSize(handle.width, handle.height, grid.frameCount, options.minWidth);
  const scaler = createFrameScaler(size.width, size.height);
  const frames: ImageData[] = [];

  for (let index = 0; index < grid.frameCount; index++) {
    if (options.signal?.aborted) {
      throw new DOMException('Video frame decoding aborted', 'AbortError');
    }

    const time = Math.min(grid.startTime + index / grid.fps, Math.max(0, handle.duration - 0.001));
    await seekVideo(video, time);

    frames.push(scaler.scale(video));
    options.onProgress?.(index + 1, grid.frameCount);
  }

  return {
    frames,
    width: size.width,
    height: size.height,
    fps: grid.fps,
    startTime: grid.startTime,
    endTime: grid.endTime
  };
}

export const videoAdapter: FrameSourceAdapter = {
  id: 'video',
  label: 'Video',
  accepts: file => file.type.startsWith('video/') || VIDEO_EXTENSIONS.has(getFileExtension(file)),
  open: openVideo,
  decode: decodeVideo
};
