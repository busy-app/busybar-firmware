import { animAdapter, imageAdapter } from './frames';
import { videoAdapter } from './video';
import type { FrameSourceAdapter } from './types';

export type { DecodeFramesOptions, FrameSourceAdapter, FrameSourceHandle, FrameSourceKind } from './types';

export const frameSourceAdapters: FrameSourceAdapter[] = [animAdapter, imageAdapter, videoAdapter];

export const FRAME_SOURCE_ACCEPT = 'video/*,image/gif,image/webp,.anim,.mp4,.m4v,.mov,.mkv,.webm';

export function resolveFrameSourceAdapter (file: File): FrameSourceAdapter | null {
  return frameSourceAdapters.find(adapter => adapter.accepts(file)) ?? null;
}
