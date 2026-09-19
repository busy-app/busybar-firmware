import type { FrameCache, TimedFrame } from '@/util/videoFrames';

export type FrameSourceKind = 'video' | 'frames';

export interface FrameSourceHandle {
  kind: FrameSourceKind;
  adapterId: string;
  width: number;
  height: number;
  duration: number;
  nativeFps?: number;
  previewUrl?: string;
  video?: HTMLVideoElement;
  frames?: TimedFrame[];
  release: () => void;
}

export interface DecodeFramesOptions {
  fps: number;
  startTime: number;
  endTime: number;
  maxFrames: number;
  minWidth?: number;
  signal?: AbortSignal;
  onProgress?: (done: number, total: number) => void;
}

export interface FrameSourceAdapter {
  id: string;
  label: string;
  accepts: (file: File) => boolean;
  open: (file: File) => Promise<FrameSourceHandle>;
  decode: (handle: FrameSourceHandle, options: DecodeFramesOptions) => Promise<FrameCache>;
}

export function getFileExtension (file: File): string {
  const match = file.name.match(/\.([a-z0-9]+)$/i);

  return match ? match[1].toLowerCase() : '';
}
