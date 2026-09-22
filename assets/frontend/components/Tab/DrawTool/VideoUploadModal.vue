<template>
  <ModalGeneric
    v-model:open="isOpen"
    data-id="modal-draw-tool-video-upload"
    :title="isEditing ? 'Edit video' : 'Add video'"
    wide
    show-close-button
    :primary-action-props="{
      label: isEditing ? 'Apply changes' : 'Insert video',
      disabled: !handle || isDecoding || !!fileError,
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
        :description="`Drag and drop to upload. Files up to ${bytesToSize(VIDEO_MAX_FILE_BYTES)}.`"
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
                · {{ formatSeconds(handle.duration) }}
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
            @click="replaceInputRef?.click()"
          />

          <input
            ref="replaceInputRef"
            type="file"
            class="hidden"
            :accept="FRAME_SOURCE_ACCEPT"
            @change="handleReplaceInput"
          >
        </div>

        <div
          v-if="fileError"
          class="rounded-md bg-error/10 px-4 py-3 text-sm text-error"
        >
          {{ fileError }}
        </div>

        <template v-else>
          <div class="flex flex-wrap items-start gap-4">
            <UFormField
              v-if="fpsOptions.length > 1"
              label="FPS"
              :help="willConvertClips ? `Other clips will be converted to ${fps} fps` : undefined"
              :ui="{ help: 'text-xs text-warning' }"
            >
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

          </div>

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

                <span
                  data-id="draw-tool-video-zoom-grip"
                  :aria-label="`Zoom ${Math.round((1 - (crop.scale - VIDEO_CROP_MIN_SCALE) / (1 - VIDEO_CROP_MIN_SCALE)) * 100)}%`"
                  class="absolute -bottom-1.5 -right-1.5 z-10 flex size-5 cursor-nwse-resize items-center justify-center rounded-full bg-white text-neutral-900 ring-1 ring-black/25"
                  @pointerdown="handleZoomPointerDown"
                  @pointermove="handleZoomPointerMove"
                  @pointerup="handleZoomPointerUp"
                  @pointercancel="handleZoomPointerUp"
                >
                  <UIcon
                    name="i-bi-resize"
                    class="pointer-events-none size-3"
                  />
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

          <TabDrawToolTrimRange
            v-if="handle && handle.duration > 0.2"
            ref="trimRangeRef"
            v-model:start="trimStart"
            v-model:end="trimEnd"
            :duration="handle.duration"
            :min-length="minWindowSeconds"
            :max-length="maxWindowSeconds"
            :step="TRIM_STEP_SECONDS"
            :fps="fps"
            :disabled="isDecoding"
            :current-time="previewTime"
            :playing="previewPlaying"
            @change="commitTrim"
            @seek="seekPreview"
            @toggle-play="togglePreviewPlayback"
            @dragging="isTrimDragging = $event"
          />

          <p class="-mb-2 text-sm font-medium">
            BUSY Bar preview
          </p>

          <div class="relative aspect-[72/16] w-full overflow-hidden rounded-md bg-neutral-950 ring-1 ring-default">
            <AnimationPlayer
              v-if="animation"
              :animation="animation"
              :frame="barPreviewFrame"
            />

            <div
              v-if="!animation || isDecoding || (isPreviewStale && !isTrimDragging)"
              class="absolute inset-0 flex flex-col items-center justify-center gap-2 px-6 text-sm text-muted"
              :class="animation ? 'bg-elevated/80' : 'bg-elevated'"
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
              <template v-else-if="needsFirstRender && !isInstantSource">
                <span class="text-sm font-medium text-default">You can set up your clip first</span>
                <UButton
                  data-id="draw-tool-video-first-render"
                  label="Build preview"
                  icon="i-ri-equalizer-line"
                  color="neutral"
                  size="xs"
                  class="mt-1"
                  @click="() => runDecode()"
                />
              </template>
              <template v-else-if="isPreviewStale && !isTrimDragging && !isInstantSource">
                <span class="text-xs">Preview shows the previous settings</span>
                <UButton
                  data-id="draw-tool-video-rerender"
                  label="Update preview"
                  icon="i-ri-restart-line"
                  color="neutral"
                  size="xs"
                  @click="() => runDecode()"
                />
              </template>
              <span v-else-if="errorMessage">{{ errorMessage }}</span>
              <span v-else>Loading…</span>
            </div>
          </div>
        </template>
      </div>
    </template>
  </ModalGeneric>
</template>

<script setup lang="ts">
import {
  getCoverCropRect,
  getVideoMaxDurationSeconds,
  sliceFrameCache,
  VIDEO_CROP_MIN_SCALE,
  VIDEO_DEFAULT_FPS,
  VIDEO_FIT_OPTIONS,
  VIDEO_FPS_OPTIONS,
  VIDEO_MAX_FILE_BYTES,
  VIDEO_MAX_FPS
} from '@/util/videoFrames';
import type { FrameCache, VideoCropState, VideoFitMode } from '@/util/videoFrames';
import { FRAME_SOURCE_ACCEPT, resolveFrameSourceAdapter } from '@/util/frameSources';
import type { FrameSourceAdapter, FrameSourceHandle } from '@/util/frameSources';
import type { VideoShapeSource } from '@/util/drawTool';
import { TRIM_STEP_SECONDS, useVideoDecodePipeline } from '@/composables/useVideoDecodePipeline';
import { useVideoSourcePreview } from '@/composables/useVideoSourcePreview';
import { useCropGesture } from '@/composables/useCropGesture';

type RetainedSession = {
  file: File;
  handle: FrameSourceHandle | null;
  cache: FrameCache | null;
  fps: number;
};

const FPS_CHOICES = VIDEO_FPS_OPTIONS.filter(value => value <= VIDEO_MAX_FPS);
const RETAINED_CACHE_MAX_BYTES = 32 * 1024 * 1024;
const PREVIEW_MAX_HEIGHT_PX = 360;

let retained: RetainedSession | null = null;
let isRestoring = false;

const isOpen = defineModel<boolean>('open', { default: false });

const es = useDrawToolEditorStore();

const sourceFile = ref<File | null>(null);
const adapter = shallowRef<FrameSourceAdapter | null>(null);
const handle = shallowRef<FrameSourceHandle | null>(null);
const fps = ref<number>(VIDEO_DEFAULT_FPS);
const fit = ref<VideoFitMode>('cover');
const crop = ref<VideoCropState>({ offsetX: 0.5, offsetY: 0.5, scale: 1 });
const trimStart = ref(0);
const trimEnd = ref(0);
const fileError = ref<string | null>(null);
const editTargetId = ref<string | null>(null);
const isTrimDragging = ref(false);

const cropContainerRef = ref<HTMLDivElement | null>(null);
const previewVideoRef = ref<HTMLVideoElement | null>(null);
const backdropCanvasRef = ref<HTMLCanvasElement | null>(null);
const replaceInputRef = ref<HTMLInputElement | null>(null);
const trimRangeRef = ref<{ focus: () => void } | null>(null);

const isEditing = computed(() => !!editTargetId.value);

const cropRect = computed(() => {
  if (!handle.value) {
    return null;
  }

  return getCoverCropRect(handle.value.width, handle.value.height, WORKSPACE_WIDTH, WORKSPACE_HEIGHT, crop.value);
});

const {
  animation,
  isDecoding,
  decodeDone,
  decodeTotal,
  errorMessage,
  isInstantSource,
  isPreviewStale,
  needsFirstRender,
  frameCache,
  sourceCache,
  reset: resetPipeline,
  runDecode,
  scheduleRender,
  commitTrim,
  adoptCache
} = useVideoDecodePipeline({ adapter, handle, fps, fit, cropRect, trimStart, trimEnd });

const {
  playing: previewPlaying,
  time: previewTime,
  start: startPreviews,
  startVideoLoop: startPreviewLoop,
  stop: stopPreviewLoop,
  toggle: togglePreviewPlayback,
  seek: seekPreview,
  syncToTrim,
  reset: resetPreview
} = useVideoSourcePreview({
  handle,
  trimStart,
  trimEnd,
  videoRef: previewVideoRef,
  canvasRef: backdropCanvasRef,
  isActive: isOpen
});

const {
  isMoving: isDraggingCrop,
  reset: resetCropGesture,
  onMoveDown: handleCropPointerDown,
  onMoveMove: handleCropPointerMove,
  onMoveUp: handleCropPointerUp,
  onZoomDown: handleZoomPointerDown,
  onZoomMove: handleZoomPointerMove,
  onZoomUp: handleZoomPointerUp
} = useCropGesture({
  crop,
  cropRect,
  containerRef: cropContainerRef,
  disabled: isDecoding,
  onCommit: scheduleRender
});

const canvasFps = computed(() => {
  const other = es.videoShapes.find(shape => shape.id !== editTargetId.value);

  return other ? other.fps : null;
});

const willConvertClips = computed(() => canvasFps.value !== null && fps.value !== canvasFps.value);

const fpsOptions = computed(() => {
  const allowed = getAllowedFps(handle.value?.nativeFps);
  const extra = [fps.value, canvasFps.value].filter((value): value is number => value !== null);
  const values = [...new Set([...allowed, ...extra])]
    .filter(value => canvasFps.value === null || value === canvasFps.value || value === fps.value || es.canSetTimelineFps(value, editTargetId.value))
    .sort((a, b) => a - b);

  return values.map(value => ({ label: String(value), value }));
});

const barPreviewFrame = computed(() => {
  const cache = frameCache.value;
  const frameCount = animation.value?.frames.length ?? 0;

  if (!cache || frameCount === 0) {
    return 0;
  }

  const index = Math.floor(((previewTime.value - cache.startTime) * cache.fps) + 1e-6);

  return Math.min(frameCount - 1, Math.max(0, index));
});

const maxWindowSeconds = computed(() => getVideoMaxDurationSeconds(fps.value));

const minWindowSeconds = computed(() => Math.min(handle.value?.duration ?? 0, 1 / fps.value));

const cropRectClass = computed(() => {
  if (isDecoding.value) {
    return 'cursor-not-allowed opacity-60 ring-white/40';
  }

  return isDraggingCrop.value ? 'cursor-grabbing ring-primary' : 'cursor-move ring-white/90';
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

function getAllowedFps (nativeFps?: number) {
  if (!nativeFps) {
    return FPS_CHOICES;
  }

  const native = Math.max(1, Math.min(VIDEO_MAX_FPS, Math.round(nativeFps)));
  const allowed = FPS_CHOICES.filter(value => value <= native);

  return allowed.includes(native) ? allowed : [...allowed, native];
}

function getInitialFps (nativeFps?: number) {
  const allowed = getAllowedFps(nativeFps);

  return allowed.includes(VIDEO_DEFAULT_FPS) ? VIDEO_DEFAULT_FPS : Math.max(...allowed);
}

function formatSeconds (seconds: number) {
  return `${seconds.toFixed(1)} s`;
}

function close () {
  isOpen.value = false;
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
  resetPipeline();
  releaseHandle();
  resetCropGesture();
  sourceFile.value = null;
  adapter.value = null;
  fileError.value = null;
  editTargetId.value = null;
  isTrimDragging.value = false;
  crop.value = { offsetX: 0.5, offsetY: 0.5, scale: 1 };
  fps.value = VIDEO_DEFAULT_FPS;
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

  const cache = sourceCache.value ?? frameCache.value;
  const cacheBytes = cache ? cache.frames.length * cache.width * cache.height * 4 : 0;

  retained = {
    file: sourceFile.value,
    handle: handle.value?.kind === 'video' ? handle.value : null,
    cache: cacheBytes <= RETAINED_CACHE_MAX_BYTES ? cache : null,
    fps: fps.value
  };

  if (handle.value && handle.value.kind !== 'video') {
    handle.value.release();
  }

  stopPreviewLoop();
  handle.value = null;
}

function handleReplaceInput (event: Event) {
  const input = event.target as HTMLInputElement;
  const file = input.files?.[0];

  input.value = '';

  if (!file) {
    return;
  }

  releaseRetained();
  editTargetId.value = null;
  crop.value = { offsetX: 0.5, offsetY: 0.5, scale: 1 };
  sourceFile.value = file;
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

async function openFile (file: File, restoreFrom?: VideoShapeSource) {
  resetPipeline();
  releaseHandle();
  resetPreview();
  fileError.value = null;

  if (file.size > VIDEO_MAX_FILE_BYTES) {
    fileError.value = `This file is ${bytesToSize(file.size)}. The limit is ${bytesToSize(VIDEO_MAX_FILE_BYTES)}.`;
    return;
  }

  const resolvedAdapter = resolveFrameSourceAdapter(file);

  if (!resolvedAdapter) {
    fileError.value = 'Unsupported file. Use a video, a GIF or a .anim file.';
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
      fps.value = canvasFps.value ?? getInitialFps(openedHandle.nativeFps);
      clampTrim(0, Math.min(openedHandle.duration, maxWindowSeconds.value), false);
    }

    await nextTick();
    isRestoring = false;
    startPreviews();
    trimRangeRef.value?.focus();

    if (reusable?.cache && reusable.fps === fps.value) {
      const sliced = sliceFrameCache(reusable.cache, trimStart.value, trimEnd.value);

      if (sliced) {
        adoptCache(reusable.cache, sliced);
        return;
      }
    }

    if (isInstantSource.value) {
      await runDecode({ quiet: true });
    }
  } catch (error) {
    errorMessage.value = error instanceof Error ? error.message : String(error);
  }
}

async function insertVideo () {
  if (!animation.value || isPreviewStale.value) {
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
    ? (es.updateVideoShape(editTargetId.value, frames, animation.value.fps, source) ? editTargetId.value : null)
    : es.addVideoShape(frames, animation.value.fps, sourceFile.value.name, source);

  if (!shapeId) {
    errorMessage.value = `Could not apply ${animation.value.fps} fps to the clips already on the canvas.`;
    return;
  }

  retainSession();
  close();

  setTimeout(() => {
    es.selectedShapeId = shapeId;
  });
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

  if (isInstantSource.value) {
    runDecode({ quiet: true });
  }
});

watch(fit, () => {
  if (!isRestoring) {
    scheduleRender();
  }
});

watch([trimStart, trimEnd], syncToTrim);

watch(isOpen, open => {
  if (open) {
    const target = es.videoEditTargetId;
    const pendingFile = es.pendingVideoUploadFile;
    es.videoEditTargetId = null;
    es.pendingVideoUploadFile = null;

    if (target) {
      openForEdit(target);
    } else if (pendingFile) {
      sourceFile.value = pendingFile;
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
