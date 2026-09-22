import type { Ref } from 'vue';
import type { FrameSourceHandle } from '@/util/frameSources';

interface VideoSourcePreviewOptions {
  handle: Ref<FrameSourceHandle | null>;
  trimStart: Ref<number>;
  trimEnd: Ref<number>;
  videoRef: Ref<HTMLVideoElement | null>;
  canvasRef: Ref<HTMLCanvasElement | null>;
  isActive: Ref<boolean>;
}

export function useVideoSourcePreview (options: VideoSourcePreviewOptions) {
  const playing = ref(true);
  const time = ref(0);

  let backdropFrameHandle: number | null = null;
  let backdropSeekMs: number | null = null;
  let videoFrameHandle: number | null = null;

  function stopBackdropLoop () {
    if (backdropFrameHandle !== null) {
      cancelAnimationFrame(backdropFrameHandle);
      backdropFrameHandle = null;
    }
  }

  function startBackdropLoop () {
    stopBackdropLoop();

    const canvas = options.canvasRef.value;
    const frames = options.handle.value?.frames;
    const context = canvas?.getContext('2d');

    if (!canvas || !context || !frames?.length) {
      return;
    }

    canvas.width = frames[0].imageData.width;
    canvas.height = frames[0].imageData.height;

    const frameEnds: number[] = [];
    let totalMs = 0;

    for (const frame of frames) {
      totalMs += Math.max(1, frame.durationMs);
      frameEnds.push(totalMs);
    }

    let lastTimestamp: number | null = null;
    let elapsedMs = 0;
    let drawnIndex = -1;

    const step = (timestamp: number) => {
      const startMs = Math.min(options.trimStart.value * 1000, totalMs);
      const endMs = options.trimEnd.value > 0 ? Math.min(options.trimEnd.value * 1000, totalMs) : totalMs;
      const windowMs = Math.max(1, endMs - startMs);

      if (playing.value && lastTimestamp !== null) {
        elapsedMs += timestamp - lastTimestamp;
      }

      lastTimestamp = timestamp;

      if (backdropSeekMs !== null) {
        elapsedMs = backdropSeekMs - startMs;
        backdropSeekMs = null;
      }

      const timeMs = startMs + (((elapsedMs % windowMs) + windowMs) % windowMs);

      time.value = timeMs / 1000;

      const foundIndex = frameEnds.findIndex(end => timeMs < end);
      const index = foundIndex < 0 ? frames.length - 1 : foundIndex;

      if (index !== drawnIndex) {
        context.putImageData(frames[index].imageData, 0, 0);
        drawnIndex = index;
      }

      backdropFrameHandle = requestAnimationFrame(step);
    };

    backdropFrameHandle = requestAnimationFrame(step);
  }

  function handleTimeUpdate () {
    const video = options.videoRef.value;

    if (video) {
      keepInTrim(video, 0.05);
    }
  }

  function cancelVideoWatchers () {
    const video = options.videoRef.value;

    if (video && videoFrameHandle !== null && typeof video.cancelVideoFrameCallback === 'function') {
      video.cancelVideoFrameCallback(videoFrameHandle);
    }

    videoFrameHandle = null;
    video?.removeEventListener('timeupdate', handleTimeUpdate);
  }

  function keepInTrim (video: HTMLVideoElement, tolerance: number) {
    const endsAtVideoEnd = Number.isFinite(video.duration) && options.trimEnd.value >= video.duration - 1 / 30;
    const pastEnd = !endsAtVideoEnd && video.currentTime >= options.trimEnd.value - tolerance;

    if (pastEnd || video.currentTime < options.trimStart.value - tolerance) {
      video.currentTime = options.trimStart.value;
    }

    time.value = video.currentTime;

    if (playing.value && video.paused && !video.seeking && options.isActive.value) {
      video.play().catch(() => undefined);
    }
  }

  function startVideoLoop () {
    const video = options.videoRef.value;

    if (!video || options.handle.value?.kind !== 'video') {
      return;
    }

    cancelVideoWatchers();
    keepInTrim(video, 0.02);

    if (playing.value && video.paused) {
      video.play().catch(() => undefined);
    }

    video.addEventListener('timeupdate', handleTimeUpdate);

    if (typeof video.requestVideoFrameCallback === 'function') {
      const step = () => {
        if (videoFrameHandle === null) {
          return;
        }

        keepInTrim(video, 0.02);
        videoFrameHandle = video.requestVideoFrameCallback(step);
      };

      videoFrameHandle = video.requestVideoFrameCallback(step);
    }
  }

  function start () {
    startBackdropLoop();
    startVideoLoop();
  }

  function stop () {
    stopBackdropLoop();
    cancelVideoWatchers();
    options.videoRef.value?.pause();
  }

  function toggle () {
    playing.value = !playing.value;

    const video = options.videoRef.value;

    if (!video || options.handle.value?.kind !== 'video') {
      return;
    }

    if (playing.value) {
      keepInTrim(video, 0.02);
    } else {
      video.pause();
    }
  }

  function seek (target: number) {
    const latestTime = Math.max(options.trimStart.value, options.trimEnd.value - 0.05);
    const nextTime = Math.min(Math.max(target, options.trimStart.value), latestTime);
    const video = options.videoRef.value;

    time.value = nextTime;

    if (options.handle.value?.kind === 'video' && video) {
      video.currentTime = nextTime;
    } else {
      backdropSeekMs = nextTime * 1000;
    }
  }

  function syncToTrim () {
    const video = options.videoRef.value;

    if (video && options.handle.value?.kind === 'video') {
      keepInTrim(video, 0.02);
    }
  }

  function reset () {
    playing.value = true;
    time.value = 0;
    backdropSeekMs = null;
  }

  return { playing, time, start, startVideoLoop, stop, toggle, seek, syncToTrim, reset };
}
