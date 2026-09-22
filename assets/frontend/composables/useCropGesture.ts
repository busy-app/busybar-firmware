import type { Ref } from 'vue';
import { VIDEO_CROP_MIN_SCALE } from '@/util/videoLimits';
import type { VideoCropRect, VideoCropState } from '@/util/videoFrames';

interface CropGestureOptions {
  crop: Ref<VideoCropState>;
  cropRect: Ref<VideoCropRect | null>;
  containerRef: Ref<HTMLElement | null>;
  disabled: Ref<boolean>;
  onCommit: () => void;
}

export function useCropGesture (options: CropGestureOptions) {
  const moveDrag = ref<{ pointerId: number; startX: number; startY: number; startOffsetX: number; startOffsetY: number } | null>(null);
  const zoomDrag = ref<{ pointerId: number; startX: number; startY: number; startScale: number; startWidth: number } | null>(null);

  const isMoving = computed(() => !!moveDrag.value);

  function onMoveDown (event: PointerEvent) {
    if (!options.containerRef.value || options.disabled.value) {
      return;
    }

    event.preventDefault();
    (event.currentTarget as HTMLElement).setPointerCapture(event.pointerId);
    moveDrag.value = {
      pointerId: event.pointerId,
      startX: event.clientX,
      startY: event.clientY,
      startOffsetX: options.crop.value.offsetX,
      startOffsetY: options.crop.value.offsetY
    };
  }

  function onMoveMove (event: PointerEvent) {
    const drag = moveDrag.value;
    const container = options.containerRef.value;
    const rect = options.cropRect.value;

    if (!drag || drag.pointerId !== event.pointerId || !container || !rect) {
      return;
    }

    const bounds = container.getBoundingClientRect();
    const freeWidth = (1 - rect.width) * bounds.width;
    const freeHeight = (1 - rect.height) * bounds.height;
    const nextOffsetX = freeWidth > 0 ? drag.startOffsetX + (event.clientX - drag.startX) / freeWidth : drag.startOffsetX;
    const nextOffsetY = freeHeight > 0 ? drag.startOffsetY + (event.clientY - drag.startY) / freeHeight : drag.startOffsetY;

    options.crop.value = {
      ...options.crop.value,
      offsetX: Math.min(1, Math.max(0, nextOffsetX)),
      offsetY: Math.min(1, Math.max(0, nextOffsetY))
    };
  }

  function onMoveUp (event: PointerEvent) {
    const drag = moveDrag.value;

    if (!drag || drag.pointerId !== event.pointerId) {
      return;
    }

    (event.currentTarget as HTMLElement).releasePointerCapture(event.pointerId);
    moveDrag.value = null;

    if (drag.startOffsetX !== options.crop.value.offsetX || drag.startOffsetY !== options.crop.value.offsetY) {
      options.onCommit();
    }
  }

  function onZoomDown (event: PointerEvent) {
    const rect = options.cropRect.value;

    if (!rect || options.disabled.value) {
      return;
    }

    event.preventDefault();
    event.stopPropagation();
    (event.currentTarget as HTMLElement).setPointerCapture(event.pointerId);
    zoomDrag.value = {
      pointerId: event.pointerId,
      startX: event.clientX,
      startY: event.clientY,
      startScale: options.crop.value.scale,
      startWidth: rect.width
    };
  }

  function onZoomMove (event: PointerEvent) {
    const drag = zoomDrag.value;
    const container = options.containerRef.value;

    if (!drag || drag.pointerId !== event.pointerId || !container) {
      return;
    }

    event.stopPropagation();

    const bounds = container.getBoundingClientRect();

    if (bounds.width <= 0 || bounds.height <= 0) {
      return;
    }

    const delta = (((event.clientX - drag.startX) / bounds.width) + ((event.clientY - drag.startY) / bounds.height)) / 2;
    const widthPerScale = drag.startWidth / Math.max(0.001, drag.startScale);
    const nextScale = drag.startScale + (delta / widthPerScale);

    options.crop.value = {
      ...options.crop.value,
      scale: Math.min(1, Math.max(VIDEO_CROP_MIN_SCALE, nextScale))
    };
  }

  function onZoomUp (event: PointerEvent) {
    const drag = zoomDrag.value;

    if (!drag || drag.pointerId !== event.pointerId) {
      return;
    }

    event.stopPropagation();
    (event.currentTarget as HTMLElement).releasePointerCapture(event.pointerId);
    zoomDrag.value = null;

    if (drag.startScale !== options.crop.value.scale) {
      options.onCommit();
    }
  }

  function reset () {
    moveDrag.value = null;
    zoomDrag.value = null;
  }

  return { isMoving, reset, onMoveDown, onMoveMove, onMoveUp, onZoomDown, onZoomMove, onZoomUp };
}
