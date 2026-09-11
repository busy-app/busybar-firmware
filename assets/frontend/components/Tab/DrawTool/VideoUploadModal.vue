<template>
  <ModalGeneric
    v-model:open="isOpen"
    data-id="modal-draw-tool-video-upload"
    :title="isEditing ? 'Edit video' : 'Add video'"
    wide
    show-close-button
    :primary-action-props="{
      label: isEditing ? 'Apply changes' : 'Insert video',
      disabled: !animation || isDecoding,
      onClick: insertVideo
    }"
    :secondary-action-props="{
      label: 'Cancel',
      variant: 'ghost',
      onClick: close
    }"
  >
    <template #body>
      <UFileUpload
        v-if="!sourceFile"
        v-model="sourceFile"
        data-id="draw-tool-video-upload"
        :accept="FRAME_SOURCE_ACCEPT"
        class="w-full rounded-xl"
        label="Upload video, GIF or .anim"
        :description="`Drag and drop to upload. Files up to ${bytesToSize(DRAW_TOOL_VIDEO_MAX_FILE_BYTES)}.`"
        :ui="{
          base: 'cursor-pointer',
          icon: 'size-6',
          label: 'text-lg',
          description: 'text-sm'
        }"
      >
        <template #actions>
          <UButton
            label="Select file"
            color="neutral"
            class="mt-2"
          />
        </template>
      </UFileUpload>

      <div
        v-else
        class="flex flex-col gap-4"
      >
        <div class="flex items-center justify-between gap-4">
          <div class="min-w-0">
            <p class="truncate font-medium">{{ sourceFile.name }}</p>
            <p class="text-sm text-muted">
              {{ bytesToSize(sourceFile.size) }}
              <template v-if="handle">
                · {{ handle.width }}×{{ handle.height }} · {{ formatSeconds(handle.duration) }}
                <template v-if="handle.nativeFps"> · {{ handle.nativeFps }} fps</template>
                · {{ adapter?.label }}
              </template>
            </p>
          </div>

          <UButton
            label="Replace"
            color="neutral"
            variant="ghost"
            icon="i-bi-upload"
            :disabled="isDecoding"
            @click="replaceFile"
          />
        </div>

        <div
          v-if="fileError"
          class="rounded-md bg-error/10 px-4 py-3 text-sm text-error"
        >
          {{ fileError }}
        </div>

        <template v-else>
          <div class="flex flex-wrap items-end gap-4">
            <UFormField label="FPS">
              <USelect
                v-model="fps"
                :items="fpsOptions"
                :disabled="isDecoding"
                class="w-28"
              />
            </UFormField>

            <UFormField label="Fit">
              <USelect
                v-model="fit"
                :items="VIDEO_FIT_OPTIONS"
                :disabled="isDecoding"
                class="w-44"
              />
            </UFormField>

            <UFormField
              v-if="fit === 'cover'"
              label="Zoom"
              class="min-w-40 flex-1"
            >
              <USlider
                v-model="cropScalePercent"
                :min="0"
                :max="100"
                :step="1"
                :disabled="isDecoding"
                @change="scheduleRender"
              />
            </UFormField>
          </div>

          <UFormField
            v-if="handle && handle.duration > 0.2"
            :ui="{ description: 'text-xs' }"
          >
            <TabDrawToolTrimRange
              v-model:start="trimStart"
              v-model:end="trimEnd"
              :duration="handle.duration"
              :min-length="minWindowSeconds"
              :max-length="maxWindowSeconds"
              :step="TRIM_STEP_SECONDS"
              :disabled="isDecoding"
              class="mt-1"
              @change="commitTrim"
            />
          </UFormField>

          <div
            v-if="handle"
            class="flex w-full items-center justify-center overflow-hidden rounded-md bg-neutral-950 ring-1 ring-default"
          >
            <div
              ref="cropContainerRef"
              class="relative select-none"
              :style="{
                aspectRatio: `${handle.width} / ${handle.height}`,
                width: `min(100%, ${Math.round(PREVIEW_MAX_HEIGHT_PX * handle.width / handle.height)}px)`
              }"
            >
              <video
                v-if="handle.kind === 'video' && handle.previewUrl"
                ref="previewVideoRef"
                :src="handle.previewUrl"
                class="h-full w-full object-contain"
                muted
                loop
                playsinline
                @loadeddata="startPreviewLoop"
              />
              <canvas
                v-else
                ref="backdropCanvasRef"
                class="h-full w-full object-contain [image-rendering:pixelated]"
              />

              <div
                v-if="fit === 'cover' && cropRectStyle"
                class="absolute rounded-sm ring-2 shadow-[0_0_0_9999px_rgba(0,0,0,0.6)] transition-[opacity]"
                :class="cropRectClass"
                :style="cropRectStyle"
                @pointerdown="handleCropPointerDown"
                @pointermove="handleCropPointerMove"
                @pointerup="handleCropPointerUp"
                @pointercancel="handleCropPointerUp"
              >
                <span
                  class="pointer-events-none absolute inset-x-0 -top-6 flex items-center justify-center gap-1 text-center text-xs"
                  :class="isDecoding ? 'text-white/50' : 'text-white/80'"
                >
                  <UIcon
                    v-if="isDecoding"
                    name="i-ri-restart-line"
                    class="size-3 animate-spin"
                  />
                  {{ isDecoding ? 'Updating preview…' : 'Drag to choose the visible area' }}
                </span>
              </div>

              <span
                v-else
                class="pointer-events-none absolute inset-x-0 bottom-2 flex items-center justify-center gap-1 text-center text-xs text-white/70"
              >
                <UIcon
                  v-if="isDecoding"
                  name="i-ri-restart-line"
                  class="size-3 animate-spin"
                />
                {{ isDecoding ? 'Updating preview…' : fitHint }}
              </span>
            </div>
          </div>

          <div class="relative aspect-[72/16] w-full overflow-hidden rounded-md bg-neutral-950 ring-1 ring-default">
            <AnimationPlayer
              v-if="animation"
              :animation="animation"
            />

            <div
              v-if="!animation || isDecoding || isPreviewStale"
              class="absolute inset-0 flex flex-col items-center justify-center gap-2 bg-neutral-950/80 px-6 text-sm text-muted"
            >
              <template v-if="isDecoding">
                <span>Extracting frames {{ decodeDone }} / {{ decodeTotal }}</span>
                <UProgress
                  :model-value="decodeDone"
                  :max="decodeTotal"
                  size="sm"
                  class="max-w-64"
                />
              </template>
              <template v-else-if="isPreviewStale">
                <span class="text-xs">Preview shows the previous settings</span>
                <UButton
                  data-id="draw-tool-video-rerender"
                  label="Update preview"
                  icon="i-ri-restart-line"
                  color="neutral"
                  size="xs"
                  @click="runDecode"
                />
              </template>
              <span v-else-if="errorMessage">{{ errorMessage }}</span>
              <span v-else>Loading…</span>
            </div>
          </div>

          <p
            v-if="animation"
            class="text-sm text-muted"
          >
            <template v-if="isPreviewStale">
              Pending · {{ fps }} fps · {{ formatSeconds(trimEnd - trimStart) }}
            </template>
            <template v-else>
              {{ animation.frames.length }} frames · {{ animation.fps }} fps · {{ formatSeconds(animation.frames.length / animation.fps) }}
            </template>
          </p>
        </template>
      </div>
    </template>
  </ModalGeneric>
</template>

<script setup lang="ts">
import { createAnimationFromFrames } from '@/util/anim2seq';
import type { DecodedAnimation } from '@/util/anim2seq';
import {
  getCoverCropRect,
  renderVideoFrames,
  sliceFrameCache,
  VIDEO_CROP_MIN_SCALE,
  VIDEO_FIT_OPTIONS,
  VIDEO_FPS_OPTIONS,
  VIDEO_FRAME_SUPERSAMPLE
} from '@/util/videoFrames';
import type { FrameCache, VideoCropState, VideoFitMode } from '@/util/videoFrames';
import { FRAME_SOURCE_ACCEPT, resolveFrameSourceAdapter } from '@/util/frameSources';
import type { FrameSourceAdapter, FrameSourceHandle } from '@/util/frameSources';
import type { VideoShapeSource } from '@/util/drawTool';

type RetainedSession = {
  file: File;
  handle: FrameSourceHandle | null;
  cache: FrameCache | null;
  fps: number;
};

const fpsOptions = VIDEO_FPS_OPTIONS
  .filter(value => value <= DRAW_TOOL_VIDEO_MAX_FPS)
  .map(value => ({ label: String(value), value }));
const RENDER_DEBOUNCE_MS = 60;
const TRIM_STEP_SECONDS = 0.1;
const PREVIEW_MAX_HEIGHT_PX = 360;

let retained: RetainedSession | null = null;
let isRestoring = false;

const isOpen = defineModel<boolean>('open', { default: false });

const es = useDrawToolEditorStore();

const sourceFile = ref<File | null>(null);
const adapter = shallowRef<FrameSourceAdapter | null>(null);
const handle = shallowRef<FrameSourceHandle | null>(null);
const frameCache = shallowRef<FrameCache | null>(null);
const fps = ref<number>(DRAW_TOOL_VIDEO_DEFAULT_FPS);
const fit = ref<VideoFitMode>('cover');
const crop = ref<VideoCropState>({ offsetX: 0.5, offsetY: 0.5, scale: 1 });
const trimStart = ref(0);
const trimEnd = ref(0);
const cropContainerRef = ref<HTMLDivElement | null>(null);
const previewVideoRef = ref<HTMLVideoElement | null>(null);
const backdropCanvasRef = ref<HTMLCanvasElement | null>(null);
const cropDrag = ref<{ pointerId: number; startX: number; startY: number; startOffsetX: number; startOffsetY: number } | null>(null);
const animation = shallowRef<DecodedAnimation | null>(null);
const isDecoding = ref(false);
const decodeDone = ref(0);
const decodeTotal = ref(0);
const errorMessage = ref<string | null>(null);
const fileError = ref<string | null>(null);
const appliedPreview = ref<{ fps: number; trimStart: number; trimEnd: number } | null>(null);
const editTargetId = ref<string | null>(null);
const decodeAbortController = ref<AbortController | null>(null);
const renderTimer = ref<ReturnType<typeof setTimeout> | null>(null);
const previewFrameHandle = ref<number | null>(null);

const isEditing = computed(() => !!editTargetId.value);

const isDraggingCrop = computed(() => !!cropDrag.value);

const isPreviewStale = computed(() => {
  const applied = appliedPreview.value;

  if (!animation.value || !applied) {
    return false;
  }

  return applied.fps !== fps.value
    || Math.abs(applied.trimStart - trimStart.value) > TRIM_STEP_SECONDS / 2
    || Math.abs(applied.trimEnd - trimEnd.value) > TRIM_STEP_SECONDS / 2;
});

const maxWindowSeconds = computed(() => getVideoMaxDurationSeconds(fps.value));

const minWindowSeconds = computed(() => Math.min(handle.value?.duration ?? 0, 1 / fps.value));

const cropRectClass = computed(() => {
  if (isDecoding.value) {
    return 'cursor-not-allowed opacity-60 ring-white/40';
  }

  return isDraggingCrop.value ? 'cursor-grabbing ring-primary' : 'cursor-move ring-white/90';
});

const cropScalePercent = computed({
  get: () => Math.round(((1 - crop.value.scale) / (1 - VIDEO_CROP_MIN_SCALE)) * 100),
  set: value => {
    const ratio = Math.min(1, Math.max(0, Number(value) / 100));

    crop.value = { ...crop.value, scale: 1 - ratio * (1 - VIDEO_CROP_MIN_SCALE) };
  }
});

const cropRect = computed(() => {
  if (!handle.value) {
    return null;
  }

  return getCoverCropRect(handle.value.width, handle.value.height, WORKSPACE_WIDTH, WORKSPACE_HEIGHT, crop.value);
});

const fitHint = computed(() => fit.value === 'contain'
  ? 'Whole frame fits inside the bar, sides stay black'
  : 'Whole frame is stretched to the bar, proportions change');

const cropRectStyle = computed(() => {
  if (!cropRect.value) {
    return null;
  }

  return {
    left: `${cropRect.value.x * 100}%`,
    top: `${cropRect.value.y * 100}%`,
    width: `${cropRect.value.width * 100}%`,
    height: `${cropRect.value.height * 100}%`,
    touchAction: 'none'
  };
});

function formatSeconds (seconds: number) {
  return `${seconds.toFixed(1)} s`;
}

function close () {
  isOpen.value = false;
}

function abortDecode () {
  if (renderTimer.value) {
    clearTimeout(renderTimer.value);
    renderTimer.value = null;
  }

  decodeAbortController.value?.abort();
  decodeAbortController.value = null;
  isDecoding.value = false;
}

function resetAnimation () {
  animation.value = null;
  errorMessage.value = null;
  appliedPreview.value = null;
}

function markPreviewApplied (cache: FrameCache) {
  appliedPreview.value = { fps: cache.fps, trimStart: cache.startTime, trimEnd: cache.endTime };
}

function releaseRetained () {
  retained?.handle?.release();
  retained = null;
}

function releaseHandle () {
  stopPreviewLoop();

  if (handle.value && handle.value !== retained?.handle) {
    handle.value.release();
  }

  handle.value = null;
}

function resetState () {
  abortDecode();
  resetAnimation();
  releaseHandle();
  frameCache.value = null;
  sourceFile.value = null;
  adapter.value = null;
  fileError.value = null;
  editTargetId.value = null;
  cropDrag.value = null;
  crop.value = { offsetX: 0.5, offsetY: 0.5, scale: 1 };
  fps.value = DRAW_TOOL_VIDEO_DEFAULT_FPS;
  fit.value = 'cover';
  trimStart.value = 0;
  trimEnd.value = 0;
}

function retainSession () {
  if (!sourceFile.value) {
    return;
  }

  if (retained && retained.file !== sourceFile.value) {
    releaseRetained();
  }

  retained = {
    file: sourceFile.value,
    handle: handle.value?.kind === 'video' ? handle.value : null,
    cache: frameCache.value,
    fps: fps.value
  };

  if (handle.value && handle.value.kind !== 'video') {
    handle.value.release();
  }

  stopPreviewLoop();
  handle.value = null;
}

function replaceFile () {
  releaseRetained();
  editTargetId.value = null;
  resetState();
}

function clampTrim (start: number, end: number, movedStart: boolean) {
  const duration = handle.value?.duration ?? 0;
  const minLength = minWindowSeconds.value;
  const maxLength = maxWindowSeconds.value;
  let nextStart = Math.min(Math.max(0, start), duration);
  let nextEnd = Math.min(Math.max(0, end), duration);

  if (nextEnd - nextStart > maxLength) {
    if (movedStart) {
      nextEnd = nextStart + maxLength;
    } else {
      nextStart = nextEnd - maxLength;
    }
  }

  if (nextEnd - nextStart < minLength) {
    if (movedStart) {
      nextStart = Math.max(0, nextEnd - minLength);
    } else {
      nextEnd = Math.min(duration, nextStart + minLength);
    }
  }

  trimStart.value = nextStart;
  trimEnd.value = nextEnd;
}

function commitTrim () {
  const cache = frameCache.value;

  if (!cache || cache.fps !== fps.value) {
    return;
  }

  const sliced = sliceFrameCache(cache, trimStart.value, trimEnd.value);

  if (!sliced) {
    return;
  }

  frameCache.value = sliced;
  markPreviewApplied(sliced);
  runRender();
}

function scheduleRender () {
  if (!frameCache.value) {
    return;
  }

  if (renderTimer.value) {
    clearTimeout(renderTimer.value);
  }

  renderTimer.value = setTimeout(() => {
    renderTimer.value = null;
    runRender();
  }, RENDER_DEBOUNCE_MS);
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
      fit: fit.value,
      crop: cropRect.value ?? undefined
    });

    animation.value = createAnimationFromFrames(frames, cache.fps);
    errorMessage.value = null;
  } catch (error) {
    resetAnimation();
    errorMessage.value = error instanceof Error ? error.message : String(error);
  }
}

async function runDecode () {
  const activeAdapter = adapter.value;
  const activeHandle = handle.value;

  if (!activeAdapter || !activeHandle) {
    return;
  }

  abortDecode();
  resetAnimation();
  frameCache.value = null;

  const controller = new AbortController();
  decodeAbortController.value = controller;
  isDecoding.value = true;
  decodeDone.value = 0;
  decodeTotal.value = 0;

  try {
    const cache = await activeAdapter.decode(activeHandle, {
      fps: fps.value,
      startTime: trimStart.value,
      endTime: trimEnd.value,
      maxFrames: DRAW_TOOL_VIDEO_MAX_FRAMES,
      minWidth: WORKSPACE_WIDTH * VIDEO_FRAME_SUPERSAMPLE,
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
    trimStart.value = cache.startTime;
    trimEnd.value = cache.endTime;
    markPreviewApplied(cache);
    runRender();
  } catch (error) {
    if (controller.signal.aborted) {
      return;
    }

    resetAnimation();
    errorMessage.value = error instanceof Error ? error.message : String(error);
  } finally {
    if (decodeAbortController.value === controller) {
      decodeAbortController.value = null;
      isDecoding.value = false;
    }
  }
}

function drawBackdrop () {
  const canvas = backdropCanvasRef.value;
  const firstFrame = handle.value?.frames?.[0]?.imageData;

  if (!canvas || !firstFrame) {
    return;
  }

  canvas.width = firstFrame.width;
  canvas.height = firstFrame.height;
  canvas.getContext('2d')?.putImageData(firstFrame, 0, 0);
}

function cancelPreviewWatchers () {
  const video = previewVideoRef.value;

  if (video && previewFrameHandle.value !== null && typeof video.cancelVideoFrameCallback === 'function') {
    video.cancelVideoFrameCallback(previewFrameHandle.value);
  }

  previewFrameHandle.value = null;
  video?.removeEventListener('timeupdate', handlePreviewTimeUpdate);
}

function stopPreviewLoop () {
  cancelPreviewWatchers();
  previewVideoRef.value?.pause();
}

function keepPreviewInTrim (video: HTMLVideoElement, tolerance: number) {
  if (video.currentTime >= trimEnd.value - tolerance || video.currentTime < trimStart.value - tolerance) {
    video.currentTime = trimStart.value;
  }
}

function handlePreviewTimeUpdate () {
  const video = previewVideoRef.value;

  if (video) {
    keepPreviewInTrim(video, 0.05);
  }
}

function startPreviewLoop () {
  const video = previewVideoRef.value;

  if (!video || handle.value?.kind !== 'video') {
    return;
  }

  cancelPreviewWatchers();
  keepPreviewInTrim(video, 0.02);

  if (video.paused) {
    video.play().catch(() => undefined);
  }

  video.addEventListener('timeupdate', handlePreviewTimeUpdate);

  if (typeof video.requestVideoFrameCallback === 'function') {
    const step = () => {
      if (previewFrameHandle.value === null) {
        return;
      }

      keepPreviewInTrim(video, 0.02);
      previewFrameHandle.value = video.requestVideoFrameCallback(step);
    };

    previewFrameHandle.value = video.requestVideoFrameCallback(step);
  }
}

async function openFile (file: File, restoreFrom?: VideoShapeSource) {
  abortDecode();
  resetAnimation();
  releaseHandle();
  frameCache.value = null;
  fileError.value = null;

  if (file.size > DRAW_TOOL_VIDEO_MAX_FILE_BYTES) {
    fileError.value = `This file is ${bytesToSize(file.size)}. The limit is ${bytesToSize(DRAW_TOOL_VIDEO_MAX_FILE_BYTES)}.`;
    return;
  }

  const resolvedAdapter = resolveFrameSourceAdapter(file);

  if (!resolvedAdapter) {
    fileError.value = 'Unsupported file. Use a video, an animated GIF or WebP, or a .anim file.';
    return;
  }

  adapter.value = resolvedAdapter;

  try {
    const reusable = retained?.file === file ? retained : null;
    const openedHandle = reusable?.handle ?? await resolvedAdapter.open(file);

    if (sourceFile.value !== file) {
      if (openedHandle !== reusable?.handle) {
        openedHandle.release();
      }

      return;
    }

    isRestoring = true;
    handle.value = openedHandle;

    if (restoreFrom) {
      fps.value = restoreFrom.fps;
      fit.value = restoreFrom.fit;
      crop.value = { ...restoreFrom.crop };
      clampTrim(restoreFrom.trimStart, restoreFrom.trimEnd, false);
    } else {
      clampTrim(0, Math.min(openedHandle.duration, maxWindowSeconds.value), false);
    }

    await nextTick();
    isRestoring = false;
    drawBackdrop();
    startPreviewLoop();

    if (reusable?.cache && reusable.fps === fps.value) {
      const sliced = sliceFrameCache(reusable.cache, trimStart.value, trimEnd.value);

      if (sliced) {
        frameCache.value = sliced;
        markPreviewApplied(sliced);
        runRender();
        return;
      }
    }

    await runDecode();
  } catch (error) {
    errorMessage.value = error instanceof Error ? error.message : String(error);
  }
}

function handleCropPointerDown (event: PointerEvent) {
  if (!cropContainerRef.value || isDecoding.value) {
    return;
  }

  event.preventDefault();
  (event.currentTarget as HTMLElement).setPointerCapture(event.pointerId);
  cropDrag.value = {
    pointerId: event.pointerId,
    startX: event.clientX,
    startY: event.clientY,
    startOffsetX: crop.value.offsetX,
    startOffsetY: crop.value.offsetY
  };
}

function handleCropPointerMove (event: PointerEvent) {
  const drag = cropDrag.value;
  const container = cropContainerRef.value;
  const rect = cropRect.value;

  if (!drag || drag.pointerId !== event.pointerId || !container || !rect) {
    return;
  }

  const bounds = container.getBoundingClientRect();
  const freeWidth = (1 - rect.width) * bounds.width;
  const freeHeight = (1 - rect.height) * bounds.height;
  const nextOffsetX = freeWidth > 0 ? drag.startOffsetX + (event.clientX - drag.startX) / freeWidth : drag.startOffsetX;
  const nextOffsetY = freeHeight > 0 ? drag.startOffsetY + (event.clientY - drag.startY) / freeHeight : drag.startOffsetY;

  crop.value = {
    ...crop.value,
    offsetX: Math.min(1, Math.max(0, nextOffsetX)),
    offsetY: Math.min(1, Math.max(0, nextOffsetY))
  };
}

function handleCropPointerUp (event: PointerEvent) {
  const drag = cropDrag.value;

  if (!drag || drag.pointerId !== event.pointerId) {
    return;
  }

  (event.currentTarget as HTMLElement).releasePointerCapture(event.pointerId);
  cropDrag.value = null;

  if (drag.startOffsetX !== crop.value.offsetX || drag.startOffsetY !== crop.value.offsetY) {
    scheduleRender();
  }
}

async function insertVideo () {
  if (isPreviewStale.value) {
    await runDecode();
  }

  if (!animation.value || !sourceFile.value) {
    return;
  }

  const frames = animation.value.frames.map(frame => frame.imageData);
  const source: VideoShapeSource = {
    file: sourceFile.value,
    fps: fps.value,
    fit: fit.value,
    crop: { ...crop.value },
    trimStart: trimStart.value,
    trimEnd: trimEnd.value
  };

  const shapeId = editTargetId.value
    ? (es.updateVideoShape(editTargetId.value, frames, animation.value.fps, source), editTargetId.value)
    : es.addVideoShape(frames, animation.value.fps, sourceFile.value.name, source);

  retainSession();
  close();

  if (shapeId) {
    setTimeout(() => {
      es.selectedShapeId = shapeId;
    });
  }
}

function openForEdit (shapeId: string) {
  const shape = es.getVideoShape(shapeId);

  if (!shape?.source) {
    editTargetId.value = null;
    return;
  }

  editTargetId.value = shapeId;
  sourceFile.value = shape.source.file;
  openFile(shape.source.file, shape.source);
}

watch(sourceFile, file => {
  if (file && !editTargetId.value) {
    openFile(file);
  }
});

watch(fps, () => {
  if (!handle.value || isRestoring) {
    return;
  }

  clampTrim(trimStart.value, trimEnd.value, false);
});

watch(fit, () => {
  if (!isRestoring) {
    scheduleRender();
  }
});

watch([trimStart, trimEnd], () => {
  const video = previewVideoRef.value;

  if (video && handle.value?.kind === 'video') {
    keepPreviewInTrim(video, 0.02);
  }
});

watch(isOpen, open => {
  if (open) {
    const target = es.videoEditTargetId;
    es.videoEditTargetId = null;

    if (target) {
      openForEdit(target);
    }

    return;
  }

  const editedShapeId = editTargetId.value;

  resetState();

  if (editedShapeId && es.hasVideoShapes) {
    es.isTimelinePlaying = true;
    setTimeout(() => {
      if (es.getVideoShape(editedShapeId)) {
        es.selectedShapeId = editedShapeId;
      }
    });
  }
});

onBeforeUnmount(() => {
  resetState();
  releaseRetained();
});
</script>
