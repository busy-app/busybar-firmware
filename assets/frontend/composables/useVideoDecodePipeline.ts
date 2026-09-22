import type { Ref } from 'vue';
import { createAnimationFromFrames } from '@/util/anim2seq';
import type { DecodedAnimation } from '@/util/anim2seq';
import { WORKSPACE_HEIGHT, WORKSPACE_WIDTH } from '@/util/drawTool';
import { renderVideoFrames, sliceFrameCache, VIDEO_MAX_FRAMES } from '@/util/videoFrames';
import type { FrameCache, VideoCropRect, VideoFitMode } from '@/util/videoFrames';
import type { FrameSourceAdapter, FrameSourceHandle } from '@/util/frameSources';

export const TRIM_STEP_SECONDS = 0.1;

const RENDER_DEBOUNCE_MS = 60;

interface VideoDecodePipelineOptions {
  adapter: Ref<FrameSourceAdapter | null>;
  handle: Ref<FrameSourceHandle | null>;
  fps: Ref<number>;
  fit: Ref<VideoFitMode>;
  cropRect: Ref<VideoCropRect | null>;
  trimStart: Ref<number>;
  trimEnd: Ref<number>;
}

export function useVideoDecodePipeline (options: VideoDecodePipelineOptions) {
  const frameCache = shallowRef<FrameCache | null>(null);
  const sourceCache = shallowRef<FrameCache | null>(null);
  const animation = shallowRef<DecodedAnimation | null>(null);
  const isDecoding = ref(false);
  const decodeDone = ref(0);
  const decodeTotal = ref(0);
  const errorMessage = ref<string | null>(null);
  const appliedPreview = ref<{ fps: number; trimStart: number; trimEnd: number } | null>(null);

  let abortController: AbortController | null = null;
  let renderTimer: ReturnType<typeof setTimeout> | null = null;

  const isInstantSource = computed(() => options.handle.value?.kind === 'frames');

  const isPreviewStale = computed(() => {
    const applied = appliedPreview.value;

    if (!animation.value || !applied) {
      return false;
    }

    return applied.fps !== options.fps.value
      || Math.abs(applied.trimStart - options.trimStart.value) > TRIM_STEP_SECONDS / 2
      || Math.abs(applied.trimEnd - options.trimEnd.value) > TRIM_STEP_SECONDS / 2;
  });

  const needsFirstRender = computed(() => !!options.handle.value
    && !animation.value
    && !appliedPreview.value
    && !isDecoding.value
    && !errorMessage.value);

  function abort () {
    if (renderTimer) {
      clearTimeout(renderTimer);
      renderTimer = null;
    }

    abortController?.abort();
    abortController = null;
    isDecoding.value = false;
  }

  function resetAnimation () {
    animation.value = null;
    errorMessage.value = null;
    appliedPreview.value = null;
  }

  function reset () {
    abort();
    resetAnimation();
    frameCache.value = null;
    sourceCache.value = null;
  }

  function markApplied (cache: FrameCache) {
    const frameSeconds = 1 / cache.fps;

    if (Math.abs(cache.startTime - options.trimStart.value) >= frameSeconds) {
      options.trimStart.value = cache.startTime;
    }

    if (Math.abs(cache.endTime - options.trimEnd.value) >= frameSeconds) {
      options.trimEnd.value = cache.endTime;
    }

    appliedPreview.value = { fps: cache.fps, trimStart: options.trimStart.value, trimEnd: options.trimEnd.value };
  }

  function runRender () {
    const cache = frameCache.value;

    if (!cache) {
      return;
    }

    try {
      const frames = renderVideoFrames(cache, {
        width: WORKSPACE_WIDTH,
        height: WORKSPACE_HEIGHT,
        fit: options.fit.value,
        crop: options.cropRect.value ?? undefined
      });

      animation.value = createAnimationFromFrames(frames, cache.fps);
      errorMessage.value = null;
    } catch (error) {
      resetAnimation();
      errorMessage.value = error instanceof Error ? error.message : String(error);
    }
  }

  function scheduleRender () {
    if (!frameCache.value) {
      return;
    }

    if (renderTimer) {
      clearTimeout(renderTimer);
    }

    renderTimer = setTimeout(() => {
      renderTimer = null;
      runRender();
    }, RENDER_DEBOUNCE_MS);
  }

  async function runDecode (decodeOptions?: { quiet?: boolean }) {
    const activeAdapter = options.adapter.value;
    const activeHandle = options.handle.value;

    if (!activeAdapter || !activeHandle) {
      return;
    }

    const quiet = decodeOptions?.quiet === true;

    abort();

    if (!quiet) {
      resetAnimation();
      frameCache.value = null;
      sourceCache.value = null;
    }

    const controller = new AbortController();

    abortController = controller;
    isDecoding.value = !quiet;
    decodeDone.value = 0;
    decodeTotal.value = 0;

    try {
      const cache = await activeAdapter.decode(activeHandle, {
        fps: options.fps.value,
        startTime: options.trimStart.value,
        endTime: options.trimEnd.value,
        maxFrames: VIDEO_MAX_FRAMES,
        signal: controller.signal,
        onProgress: (done, total) => {
          decodeDone.value = done;
          decodeTotal.value = total;
        }
      });

      if (controller.signal.aborted) {
        return;
      }

      frameCache.value = cache;
      sourceCache.value = cache;
      markApplied(cache);
      runRender();
    } catch (error) {
      if (controller.signal.aborted) {
        return;
      }

      resetAnimation();
      errorMessage.value = error instanceof Error ? error.message : String(error);
    } finally {
      if (abortController === controller) {
        abortController = null;
        isDecoding.value = false;
      }
    }
  }

  function commitTrim () {
    const cache = sourceCache.value ?? frameCache.value;
    const sliced = cache && cache.fps === options.fps.value
      ? sliceFrameCache(cache, options.trimStart.value, options.trimEnd.value)
      : null;

    if (!sliced) {
      if (isInstantSource.value) {
        runDecode({ quiet: true });
      }

      return;
    }

    frameCache.value = sliced;
    markApplied(sliced);
    runRender();
  }

  function adoptCache (cache: FrameCache, sliced: FrameCache) {
    sourceCache.value = cache;
    frameCache.value = sliced;
    markApplied(sliced);
    runRender();
  }

  return {
    frameCache,
    sourceCache,
    animation,
    isDecoding,
    decodeDone,
    decodeTotal,
    errorMessage,
    isInstantSource,
    isPreviewStale,
    needsFirstRender,
    abort,
    reset,
    runDecode,
    runRender,
    scheduleRender,
    commitTrim,
    adoptCache
  };
}
