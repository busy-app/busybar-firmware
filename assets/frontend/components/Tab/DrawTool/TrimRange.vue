<template>
  <div
    class="flex w-full touch-none select-none items-center gap-3 rounded-xl bg-elevated/50 py-2 pl-2 pr-4 ring-1 ring-default"
    @keydown="handleRootKeyDown"
  >
    <UButton
      data-id="draw-tool-video-preview-toggle"
      :icon="playing ? 'i-bi-control-pause' : 'i-bi-control-play'"
      :aria-label="playing ? 'Pause preview' : 'Play preview'"
      color="neutral"
      variant="soft"
      size="md"
      class="mt-4 shrink-0"
      @click="emit('togglePlay')"
    />

    <div
      class="min-w-0 flex-1 px-2"
      :class="disabled ? 'opacity-60' : ''"
    >
      <div
        class="relative h-4 cursor-pointer text-[10px] tabular-nums text-dimmed"
        @pointerdown="handleScrubDown"
        @pointermove="handleScrubMove"
        @pointerup="handleScrubUp"
        @pointercancel="handleScrubUp"
      >
        <span
          v-for="tick in ticks"
          :key="tick.time"
          class="pointer-events-none absolute top-0 -translate-x-1/2 whitespace-nowrap"
          :style="{ left: `${tick.percent}%` }"
        >
          {{ tick.label }}
        </span>
      </div>

      <div
        ref="trackRef"
        class="relative h-9"
      >
        <div
          class="absolute inset-x-0 top-1/2 h-6 -translate-y-1/2 cursor-pointer rounded-md bg-accented/30 ring-1 ring-inset ring-default"
          @pointerdown="handleScrubDown"
          @pointermove="handleScrubMove"
          @pointerup="handleScrubUp"
          @pointercancel="handleScrubUp"
        />

        <span
          v-for="tick in ticks"
          :key="`line-${tick.time}`"
          class="pointer-events-none absolute top-0 h-1.5 w-px -translate-x-1/2 bg-accented"
          :style="{ left: `${tick.percent}%` }"
        />

        <div
          class="absolute top-1/2 h-6 -translate-y-1/2 rounded-md bg-accented/70 ring-1 ring-inset ring-inverted/30"
          :class="disabled ? '' : activeMode === 'move' ? 'cursor-grabbing' : 'cursor-grab'"
          :style="{ left: `${startPercent}%`, width: `${Math.max(0, endPercent - startPercent)}%` }"
          @pointerdown="event => handlePointerDown(event, 'move')"
          @pointermove="handlePointerMove"
          @pointerup="handlePointerUp"
          @pointercancel="handlePointerUp"
        >
          <span class="pointer-events-none absolute inset-0 flex items-center justify-center text-[11px] tabular-nums text-default">
            {{ lengthLabel }}
          </span>
        </div>

        <div
          v-if="playheadPercent !== null"
          ref="playheadRef"
          data-id="draw-tool-video-playhead"
          role="slider"
          tabindex="0"
          aria-label="Playhead"
          :aria-valuemin="0"
          :aria-valuemax="Math.round(duration * 100) / 100"
          :aria-valuenow="Math.round((currentTime ?? 0) * 100) / 100"
          class="group absolute inset-y-0 z-[5] flex w-3 -translate-x-1/2 cursor-ew-resize justify-center focus:outline-none"
          :style="{ left: `${playheadPercent}%` }"
          @pointerdown="handleScrubDown"
          @pointermove="handleScrubMove"
          @pointerup="handleScrubUp"
          @pointercancel="handleScrubUp"
          @keydown="handlePlayheadKeyDown"
        >
          <span class="pointer-events-none h-full w-0.5 rounded-full bg-primary" />
          <span class="pointer-events-none absolute -top-0.5 left-1/2 size-2 -translate-x-1/2 rotate-45 rounded-[2px] bg-primary transition-transform group-hover:scale-125 group-focus:scale-150" />
        </div>

        <div
          v-for="handle in handles"
          :key="handle.mode"
          role="slider"
          tabindex="0"
          :aria-label="handle.label"
          :aria-valuemin="0"
          :aria-valuemax="Math.round(duration * 100) / 100"
          :aria-valuenow="Math.round(handle.value * 100) / 100"
          class="group absolute inset-y-0 z-10 flex w-4 -translate-x-1/2 justify-center focus:outline-none"
          :class="disabled ? '' : 'cursor-ew-resize'"
          :style="{ left: `${handle.percent}%` }"
          @pointerdown="event => handlePointerDown(event, handle.mode)"
          @pointermove="handlePointerMove"
          @pointerup="handlePointerUp"
          @pointercancel="handlePointerUp"
          @keydown="event => handleKeyDown(event, handle.mode)"
        >
          <span class="pointer-events-none h-full w-0.5 rounded-full bg-inverted" />
          <span class="pointer-events-none absolute left-1/2 top-0 size-3 -translate-x-1/2 rounded-full bg-inverted ring-2 ring-default transition-transform group-hover:scale-125 group-focus:scale-125" />
          <span
            v-if="activeMode === handle.mode"
            class="pointer-events-none absolute left-1/2 top-full mt-1 -translate-x-1/2 rounded bg-inverted px-1.5 py-0.5 text-[10px] tabular-nums text-inverted"
          >
            {{ formatTrimTime(handle.value) }}
          </span>
        </div>
      </div>
    </div>
  </div>
</template>

<script setup lang="ts">
import { formatTrimTime, useTrimRange } from '@/util/trimRange';

const TICK_STEPS_SECONDS = [0.5, 1, 2, 5, 10, 15, 30, 60, 120, 300];
const MAX_TICKS = 6;

const props = withDefaults(defineProps<{
  duration: number;
  minLength: number;
  maxLength: number;
  step?: number;
  fps?: number;
  disabled?: boolean;
  currentTime?: number | null;
  playing?: boolean;
}>(), {
  step: 0.05,
  fps: 0,
  disabled: false,
  currentTime: null,
  playing: false
});

const emit = defineEmits<{
  change: [];
  seek: [time: number];
  togglePlay: [];
  dragging: [value: boolean];
}>();

const start = defineModel<number>('start', { required: true });
const end = defineModel<number>('end', { required: true });

const trackRef = ref<HTMLDivElement | null>(null);
const scrubPointerId = ref<number | null>(null);
const playheadRef = ref<HTMLDivElement | null>(null);

const {
  activeMode,
  startPercent,
  endPercent,
  lengthLabel,
  handles,
  handlePointerDown,
  handlePointerMove,
  handlePointerUp,
  handleKeyDown,
  timeFromClientX
} = useTrimRange(props, start, end, trackRef, () => emit('change'));

watch(activeMode, mode => emit('dragging', mode !== null));

onBeforeUnmount(() => emit('dragging', false));

const playheadPercent = computed(() => {
  if (props.currentTime === null || props.duration <= 0) {
    return null;
  }

  return Math.min(100, Math.max(0, (props.currentTime / props.duration) * 100));
});

const ticks = computed(() => {
  if (props.duration <= 0) {
    return [];
  }

  const tickStep = TICK_STEPS_SECONDS.find(value => props.duration / value <= MAX_TICKS) ?? TICK_STEPS_SECONDS[TICK_STEPS_SECONDS.length - 1];
  const list: Array<{ time: number; percent: number; label: string }> = [];

  for (let time = 0; time <= props.duration + 1e-6; time += tickStep) {
    list.push({
      time,
      percent: (time / props.duration) * 100,
      label: time < 60 ? `${Number(time.toFixed(1))}s` : formatTrimTime(time)
    });
  }

  return list;
});

function focus () {
  playheadRef.value?.focus();
}

defineExpose({ focus });

function handleRootKeyDown (event: KeyboardEvent) {
  if (event.key !== ' ' && event.code !== 'Space') {
    return;
  }

  // Buttons already toggle on Space natively; handling it here too would toggle twice.
  if ((event.target as HTMLElement | null)?.closest('button')) {
    return;
  }

  event.preventDefault();
  emit('togglePlay');
}

function handlePlayheadKeyDown (event: KeyboardEvent) {
  const direction = event.key === 'ArrowLeft' ? -1 : event.key === 'ArrowRight' ? 1 : 0;

  if (!direction || props.duration <= 0 || props.currentTime === null) {
    return;
  }

  event.preventDefault();

  // Pause first, otherwise playback immediately overwrites the stepped frame.
  if (props.playing) {
    emit('togglePlay');
  }

  const frameStep = props.fps > 0 ? 1 / props.fps : props.step;
  const next = props.currentTime + (direction * frameStep * (event.shiftKey ? 10 : 1));

  emit('seek', Math.min(props.duration, Math.max(0, next)));
}

function emitSeek (clientX: number) {
  const time = timeFromClientX(clientX);

  if (time !== null) {
    emit('seek', time);
  }
}

function handleScrubDown (event: PointerEvent) {
  if (props.duration <= 0) {
    return;
  }

  event.preventDefault();

  const target = event.currentTarget as HTMLElement;

  target.setPointerCapture(event.pointerId);
  // preventDefault above also cancels focus-on-click; restore it so arrow keys work.
  target.focus();

  scrubPointerId.value = event.pointerId;
  emitSeek(event.clientX);
}

function handleScrubMove (event: PointerEvent) {
  if (scrubPointerId.value !== event.pointerId) {
    return;
  }

  emitSeek(event.clientX);
}

function handleScrubUp (event: PointerEvent) {
  if (scrubPointerId.value !== event.pointerId) {
    return;
  }

  (event.currentTarget as HTMLElement).releasePointerCapture(event.pointerId);
  scrubPointerId.value = null;
}
</script>
