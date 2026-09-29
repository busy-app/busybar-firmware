import { computed, ref } from 'vue';
import type { Ref } from 'vue';

export type TrimDragMode = 'start' | 'end' | 'move';

export interface TrimRangeOptions {
  duration: number;
  minLength: number;
  maxLength: number;
  step: number;
  disabled: boolean;
}

export function formatTrimTime (seconds: number) {
  const safe = Math.max(0, seconds);
  const minutes = Math.floor(safe / 60);
  const rest = (safe - minutes * 60).toFixed(1).padStart(4, '0');

  return `${minutes}:${rest}`;
}

export function useTrimRange (
  props: Readonly<TrimRangeOptions>,
  start: Ref<number>,
  end: Ref<number>,
  trackRef: Ref<HTMLElement | null>,
  onChange: () => void
) {
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

    const target = event.currentTarget as HTMLElement;

    target.setPointerCapture(event.pointerId);
    // preventDefault above also cancels focus-on-click; restore it so arrow keys work.
    target.focus();

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
      onChange();
    }
  }

  function timeFromClientX (clientX: number) {
    const track = trackRef.value;

    if (!track || props.duration <= 0) {
      return null;
    }

    const rect = track.getBoundingClientRect();

    if (rect.width <= 0) {
      return null;
    }

    return clamp((clientX - rect.left) / rect.width, 0, 1) * props.duration;
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
      onChange();
    }
  }

  return {
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
  };
}
