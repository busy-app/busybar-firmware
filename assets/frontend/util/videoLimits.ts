import type { VideoFitMode } from './videoFrames';

export const VIDEO_DEFAULT_FPS = 15;
export const VIDEO_MAX_FPS = 60;

// Frontend guesses, not verified against firmware limits.
export const VIDEO_MAX_FRAMES = 450;
export const VIDEO_MAX_DURATION_SECONDS = 15;

export const VIDEO_MAX_FILE_BYTES = 4 * 1024 * 1024 * 1024;

// Decoding materializes every source frame before resampling, so cap it up front.
export const VIDEO_SOURCE_MAX_FRAMES = 2000;

export const VIDEO_FRAME_CACHE_MEMORY_BUDGET = 64 * 1024 * 1024;
export const VIDEO_FRAME_CACHE_MAX_WIDTH = 720;

export const VIDEO_SEEK_TIMEOUT_MS = 8000;
export const VIDEO_METADATA_TIMEOUT_MS = 15000;

export const VIDEO_CROP_MIN_SCALE = 0.2;

export const VIDEO_FPS_OPTIONS = [10, 15, 24, 30, 60];

export const VIDEO_FIT_OPTIONS: Array<{ label: string; value: VideoFitMode }> = [
  { label: 'Fill (crop)', value: 'cover' },
  { label: 'Fit (letterbox)', value: 'contain' },
  { label: 'Stretch', value: 'stretch' }
];

export function getVideoMaxDurationSeconds (fps: number): number {
  return Math.min(VIDEO_MAX_DURATION_SECONDS, VIDEO_MAX_FRAMES / Math.max(1, fps));
}
