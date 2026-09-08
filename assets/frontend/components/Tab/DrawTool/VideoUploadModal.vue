<template>
  <ModalGeneric
    v-model:open="isOpen"
    data-id="modal-draw-tool-video-upload"
    title="Add video"
    wide
    show-close-button
    :primary-action-props="{
      label: 'Insert video',
      disabled: !animation || isExtracting,
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
        v-if="!videoFile"
        v-model="videoFile"
        data-id="draw-tool-video-upload"
        accept="video/*"
        class="w-full rounded-xl"
        label="Upload video"
        :description="`Drag and drop to upload. First ${DRAW_TOOL_VIDEO_MAX_DURATION_SECONDS} seconds will be used.`"
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
            <p class="truncate font-medium">{{ videoFile.name }}</p>
            <p class="text-sm text-muted">
              {{ bytesToSize(videoFile.size) }}
              <template v-if="videoSource">
                · {{ videoSource.width }}×{{ videoSource.height }} · {{ formatSeconds(videoSource.duration) }}
              </template>
            </p>
          </div>

          <UButton
            label="Replace"
            color="neutral"
            variant="ghost"
            icon="i-bi-upload"
            :disabled="isExtracting"
            @click="resetVideo"
          />
        </div>

        <div class="flex flex-wrap items-end gap-4">
          <UFormField label="FPS">
            <USelect
              v-model="fps"
              :items="fpsOptions"
              :disabled="isExtracting"
              class="w-28"
            />
          </UFormField>

          <UFormField label="Fit">
            <USelect
              v-model="fit"
              :items="VIDEO_FIT_OPTIONS"
              :disabled="isExtracting"
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
              :min="VIDEO_CROP_MIN_SCALE * 100"
              :max="100"
              :step="1"
              :disabled="isExtracting"
              @change="scheduleExtraction"
            />
          </UFormField>
        </div>

        <div
          v-if="videoSource"
          ref="cropContainerRef"
          class="relative mx-auto select-none overflow-hidden rounded-md bg-neutral-950 ring-1 ring-default"
          :style="{
            aspectRatio: `${videoSource.width} / ${videoSource.height}`,
            width: `min(100%, ${Math.round(320 * videoSource.width / videoSource.height)}px)`
          }"
        >
          <video
            :src="videoSource.url"
            class="h-full w-full object-contain"
            autoplay
            muted
            loop
            playsinline
          />

          <div
            v-if="fit === 'cover' && cropRectStyle"
            class="absolute cursor-move rounded-sm ring-2 ring-white/90 shadow-[0_0_0_9999px_rgba(0,0,0,0.6)]"
            :class="isDraggingCrop ? 'ring-primary' : ''"
            :style="cropRectStyle"
            @pointerdown="handleCropPointerDown"
            @pointermove="handleCropPointerMove"
            @pointerup="handleCropPointerUp"
            @pointercancel="handleCropPointerUp"
          >
            <span class="pointer-events-none absolute inset-x-0 -top-6 text-center text-xs text-white/80">
              Drag to choose the visible area
            </span>
          </div>
        </div>

        <div class="relative aspect-[72/16] w-full overflow-hidden rounded-md bg-neutral-950 ring-1 ring-default">
          <AnimationPlayer
            v-if="animation"
            :animation="animation"
          />

          <div
            v-if="!animation || isExtracting"
            class="absolute inset-0 flex flex-col items-center justify-center gap-2 bg-neutral-950/70 px-6 text-sm text-muted"
          >
            <template v-if="isExtracting">
              <span>Extracting frames {{ extractionDone }} / {{ extractionTotal }}</span>
              <UProgress
                :model-value="extractionDone"
                :max="extractionTotal"
                size="sm"
                class="max-w-64"
              />
            </template>
            <span v-else-if="errorMessage">{{ errorMessage }}</span>
            <span v-else>Loading video…</span>
          </div>
        </div>

        <p
          v-if="animation"
          class="text-sm text-muted"
        >
          {{ animation.frames.length }} frames · {{ animation.fps }} fps · {{ formatSeconds(animation.frames.length / animation.fps) }}
          <template v-if="isTruncated">
            · trimmed to first {{ DRAW_TOOL_VIDEO_MAX_DURATION_SECONDS }} s
          </template>
        </p>
      </div>
    </template>
  </ModalGeneric>
</template>

<script setup lang="ts">
import { createAnimationFromFrames } from '@/util/anim2seq';
import type { DecodedAnimation } from '@/util/anim2seq';
import {
  extractVideoFrames,
  getCoverCropRect,
  loadVideoSource,
  VIDEO_CROP_MIN_SCALE,
  VIDEO_FIT_OPTIONS,
  VIDEO_FPS_OPTIONS
} from '@/util/videoFrames';
import type { VideoCropState, VideoFitMode, VideoSource } from '@/util/videoFrames';

const isOpen = defineModel<boolean>('open', { default: false });

const es = useDrawToolEditorStore();

const fpsOptions = VIDEO_FPS_OPTIONS.map(value => ({ label: String(value), value }));
const EXTRACTION_DEBOUNCE_MS = 150;

const videoFile = ref<File | null>(null);
const videoSource = ref<VideoSource | null>(null);
const fps = ref<number>(DRAW_TOOL_VIDEO_DEFAULT_FPS);
const fit = ref<VideoFitMode>('cover');
const crop = ref<VideoCropState>({ offsetX: 0.5, offsetY: 0.5, scale: 1 });
const cropContainerRef = ref<HTMLDivElement | null>(null);
const cropDrag = ref<{ pointerId: number; startX: number; startY: number; startOffsetX: number; startOffsetY: number } | null>(null);
const animation = ref<DecodedAnimation | null>(null);
const isTruncated = ref(false);
const isExtracting = ref(false);
const extractionDone = ref(0);
const extractionTotal = ref(0);
const errorMessage = ref<string | null>(null);
const extractionAbortController = ref<AbortController | null>(null);
const extractionTimer = ref<ReturnType<typeof setTimeout> | null>(null);

const isDraggingCrop = computed(() => !!cropDrag.value);

const cropScalePercent = computed({
  get: () => Math.round(crop.value.scale * 100),
  set: value => {
    crop.value = { ...crop.value, scale: Number(value) / 100 };
  }
});

const cropRect = computed(() => {
  if (!videoSource.value) {
    return null;
  }

  return getCoverCropRect(videoSource.value.width, videoSource.value.height, WORKSPACE_WIDTH, WORKSPACE_HEIGHT, crop.value);
});

const cropRectStyle = computed(() => {
  if (!videoSource.value || !cropRect.value) {
    return null;
  }

  return {
    left: `${(cropRect.value.x / videoSource.value.width) * 100}%`,
    top: `${(cropRect.value.y / videoSource.value.height) * 100}%`,
    width: `${(cropRect.value.width / videoSource.value.width) * 100}%`,
    height: `${(cropRect.value.height / videoSource.value.height) * 100}%`,
    touchAction: 'none'
  };
});

function formatSeconds (seconds: number) {
  return `${seconds.toFixed(1)} s`;
}

function abortExtraction () {
  if (extractionTimer.value) {
    clearTimeout(extractionTimer.value);
    extractionTimer.value = null;
  }

  extractionAbortController.value?.abort();
  extractionAbortController.value = null;
  isExtracting.value = false;
}

function resetAnimation () {
  animation.value = null;
  isTruncated.value = false;
  errorMessage.value = null;
}

function resetVideo () {
  abortExtraction();
  resetAnimation();
  videoSource.value?.release();
  videoSource.value = null;
  videoFile.value = null;
  cropDrag.value = null;
  crop.value = { offsetX: 0.5, offsetY: 0.5, scale: 1 };
}

function close () {
  isOpen.value = false;
}

function scheduleExtraction () {
  if (!videoSource.value) {
    return;
  }

  if (extractionTimer.value) {
    clearTimeout(extractionTimer.value);
  }

  extractionTimer.value = setTimeout(() => {
    extractionTimer.value = null;
    runExtraction();
  }, EXTRACTION_DEBOUNCE_MS);
}

async function runExtraction () {
  const source = videoSource.value;

  if (!source) {
    return;
  }

  abortExtraction();
  errorMessage.value = null;

  const controller = new AbortController();
  extractionAbortController.value = controller;
  isExtracting.value = true;
  extractionDone.value = 0;
  extractionTotal.value = 0;

  try {
    const result = await extractVideoFrames(source, {
      fps: fps.value,
      width: WORKSPACE_WIDTH,
      height: WORKSPACE_HEIGHT,
      fit: fit.value,
      crop: cropRect.value ?? undefined,
      maxDurationSeconds: DRAW_TOOL_VIDEO_MAX_DURATION_SECONDS,
      signal: controller.signal,
      onProgress: (done, total) => {
        extractionDone.value = done;
        extractionTotal.value = total;
      }
    });

    if (controller.signal.aborted) {
      return;
    }

    animation.value = createAnimationFromFrames(result.frames, fps.value);
    isTruncated.value = result.truncated;
  } catch (error) {
    if (controller.signal.aborted) {
      return;
    }

    resetAnimation();
    errorMessage.value = error instanceof Error ? error.message : String(error);
  } finally {
    if (extractionAbortController.value === controller) {
      extractionAbortController.value = null;
      isExtracting.value = false;
    }
  }
}

async function loadVideo (file: File) {
  abortExtraction();
  resetAnimation();
  videoSource.value?.release();
  videoSource.value = null;

  try {
    const source = await loadVideoSource(file);

    if (videoFile.value !== file) {
      source.release();
      return;
    }

    videoSource.value = source;
    await runExtraction();
  } catch (error) {
    errorMessage.value = error instanceof Error ? error.message : String(error);
  }
}

function handleCropPointerDown (event: PointerEvent) {
  if (!cropContainerRef.value || isExtracting.value) {
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
  const source = videoSource.value;
  const rect = cropRect.value;

  if (!drag || drag.pointerId !== event.pointerId || !container || !source || !rect) {
    return;
  }

  const bounds = container.getBoundingClientRect();
  const freeWidth = (source.width - rect.width) / source.width * bounds.width;
  const freeHeight = (source.height - rect.height) / source.height * bounds.height;
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
    scheduleExtraction();
  }
}

function insertVideo () {
  if (!animation.value || !videoFile.value) {
    return;
  }

  es.addVideoShape(animation.value.frames.map(frame => frame.imageData), animation.value.fps, videoFile.value.name);
  close();
}

watch(videoFile, file => {
  if (file) {
    loadVideo(file);
  }
});

watch([fps, fit], scheduleExtraction);

watch(isOpen, open => {
  if (!open) {
    resetVideo();
  }
});

onBeforeUnmount(resetVideo);
</script>
