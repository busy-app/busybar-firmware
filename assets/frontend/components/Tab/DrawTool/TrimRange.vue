<template>
  <div
    ref="trackRef"
    class="relative h-10 w-full touch-none select-none overflow-hidden rounded-lg bg-elevated ring-1 ring-accented"
    :class="disabled ? 'opacity-60' : ''"
  >
    <div
      class="absolute inset-y-0 bg-primary/20"
      :style="{ left: '0%', width: `${startPercent}%` }"
    />
    <div
      class="absolute inset-y-0 bg-primary/20"
      :style="{ left: `${endPercent}%`, right: '0%' }"
    />

    <div
      class="absolute inset-y-0 bg-primary/25 ring-1 ring-primary"
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
      v-for="handle in handles"
      :key="handle.mode"
      role="slider"
      tabindex="0"
      :aria-label="handle.label"
      :aria-valuemin="0"
      :aria-valuemax="Math.round(duration * 100) / 100"
      :aria-valuenow="Math.round(handle.value * 100) / 100"
      class="absolute inset-y-0 z-10 flex w-4 -translate-x-1/2 items-center justify-center rounded-md bg-primary focus-visible:outline-2 focus-visible:outline-offset-2 focus-visible:outline-primary"
      :class="disabled ? '' : 'cursor-ew-resize'"
      :style="{ left: `${handle.percent}%` }"
      @pointerdown="event => handlePointerDown(event, handle.mode)"
      @pointermove="handlePointerMove"
      @pointerup="handlePointerUp"
      @pointercancel="handlePointerUp"
      @keydown="event => handleKeyDown(event, handle.mode)"
    >
      <span class="pointer-events-none h-4 w-0.5 rounded-full bg-inverted/70" />
    </div>
  </div>
</template>

<script setup lang="ts">
type TrimDragMode = 'start' | 'end' | 'move';

const props = withDefaults(defineProps<{
  duration: number;
  minLength: number;
  maxLength: number;
  step?: number;
  disabled?: boolean;
}>(), {
  step: 0.05,
  disabled: false
});

const emit = defineEmits<{ change: [] }>();

const start = defineModel<number>('start', { required: true });
const end = defineModel<number>('end', { required: true });

const trackRef = ref<HTMLDivElement | null>(null);
const drag = ref<{ pointerId: number; mode: TrimDragMode; clientX: number; start: number; end: number; moved: boolean } | null>(null);

const activeMode = computed(() => drag.value?.mode ?? null);
const startPercent = computed(() => props.duration > 0 ? (start.value / props.duration) * 100 : 0);
const endPercent = computed(() => props.duration > 0 ? (end.value / props.duration) * 100 : 100);
const lengthLabel = computed(() => `${(end.value - start.value).toFixed(1)} s`);

const handles = computed(() => [
  { mode: 'start' as const, label: 'Trim start', value: start.value, percent: startPercent.value },
  { mode: 'end' as const, label: 'Trim end', value: end.value, percent: endPercent.value }
]);

function snap (value: number) {
  return Math.round(value / props.step) * props.step;
}

function clamp (value: number, min: number, max: number) {
  return Math.min(max, Math.max(min, value));
}

function resolveRange (mode: TrimDragMode, fromStart: number, fromEnd: number, deltaSeconds: number) {
  if (mode === 'move') {
    const length = fromEnd - fromStart;
    const nextStart = clamp(snap(fromStart + deltaSeconds), 0, Math.max(0, props.duration - length));

    return { start: nextStart, end: nextStart + length };
  }

  if (mode === 'start') {
    const lowerBound = Math.max(0, fromEnd - props.maxLength);
    const upperBound = Math.max(lowerBound, fromEnd - props.minLength);

    return { start: clamp(snap(fromStart + deltaSeconds), lowerBound, upperBound), end: fromEnd };
  }

  const upperBound = Math.min(props.duration, fromStart + props.maxLength);
  const lowerBound = Math.min(upperBound, fromStart + props.minLength);

  return { start: fromStart, end: clamp(snap(fromEnd + deltaSeconds), lowerBound, upperBound) };
}

function applyDelta (mode: TrimDragMode, fromStart: number, fromEnd: number, deltaSeconds: number) {
  const next = resolveRange(mode, fromStart, fromEnd, deltaSeconds);

  start.value = next.start;
  end.value = next.end;

  return next.start !== fromStart || next.end !== fromEnd;
}

function handlePointerDown (event: PointerEvent, mode: TrimDragMode) {
  if (props.disabled || props.duration <= 0) {
    return;
  }

  event.preventDefault();
  (event.currentTarget as HTMLElement).setPointerCapture(event.pointerId);
  drag.value = { pointerId: event.pointerId, mode, clientX: event.clientX, start: start.value, end: end.value, moved: false };
}

function handlePointerMove (event: PointerEvent) {
  const state = drag.value;
  const track = trackRef.value;

  if (!state || state.pointerId !== event.pointerId || !track) {
    return;
  }

  const width = track.getBoundingClientRect().width;

  if (width <= 0) {
    return;
  }

  if (applyDelta(state.mode, state.start, state.end, ((event.clientX - state.clientX) / width) * props.duration)) {
    state.moved = true;
  }
}

function handlePointerUp (event: PointerEvent) {
  const state = drag.value;

  if (!state || state.pointerId !== event.pointerId) {
    return;
  }

  (event.currentTarget as HTMLElement).releasePointerCapture(event.pointerId);
  drag.value = null;

  if (state.moved) {
    emit('change');
  }
}

function handleKeyDown (event: KeyboardEvent, mode: TrimDragMode) {
  if (props.disabled) {
    return;
  }

  const direction = event.key === 'ArrowLeft' ? -1 : event.key === 'ArrowRight' ? 1 : 0;

  if (!direction) {
    return;
  }

  event.preventDefault();

  if (applyDelta(mode, start.value, end.value, direction * props.step * (event.shiftKey ? 10 : 1))) {
    emit('change');
  }
}
</script>
