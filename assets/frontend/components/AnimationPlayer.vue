<template>
  <canvas
    ref="canvasRef"
    :width="animation.width"
    :height="animation.height"
    class="block h-full w-full [image-rendering:pixelated]"
  />
</template>

<script setup lang="ts">
import type { DecodedAnimation } from '@/util/anim2seq';
import { drawAnimationFrame, requestAnimationTick, subscribeToAnimationTicker } from '@/util/animationTicker';
import type { AnimationTickerSubscriber } from '@/util/animationTicker';

const props = withDefaults(defineProps<{
  animation: DecodedAnimation;
  playing?: boolean;
  loop?: boolean;
  // When set, the parent drives the frame and the internal ticker stays off.
  frame?: number | null;
}>(), {
  playing: true,
  loop: true,
  frame: null
});

const canvasRef = ref<HTMLCanvasElement | null>(null);

let subscriber: AnimationTickerSubscriber | null = null;
let unsubscribe: (() => void) | null = null;

function clampFrame (index: number) {
  return Math.min(props.animation.frames.length - 1, Math.max(0, Math.floor(index)));
}

function teardown () {
  unsubscribe?.();
  unsubscribe = null;
  subscriber = null;
}

function setup () {
  teardown();

  const context = canvasRef.value?.getContext('2d');

  if (!context) {
    return;
  }

  const frame = props.frame;

  subscriber = {
    animation: props.animation,
    context,
    loop: props.loop,
    playing: frame === null && props.playing && props.animation.frames.length > 1,
    frameIndex: frame === null ? 0 : clampFrame(frame),
    elapsedInFrame: 0
  };

  drawAnimationFrame(subscriber, subscriber.frameIndex);

  if (frame === null) {
    unsubscribe = subscribeToAnimationTicker(subscriber);
  }
}

watch(() => props.animation, setup);

watch(() => props.frame === null, setup);

watch(() => props.frame, frame => {
  if (frame === null || !subscriber) {
    return;
  }

  subscriber.frameIndex = clampFrame(frame);
  drawAnimationFrame(subscriber, subscriber.frameIndex);
});

watch(() => props.playing, playing => {
  if (!subscriber || props.frame !== null) {
    return;
  }

  subscriber.playing = playing && subscriber.animation.frames.length > 1;

  if (subscriber.playing) {
    requestAnimationTick();
  }
});

watch(() => props.loop, loop => {
  if (subscriber) {
    subscriber.loop = loop;
  }
});

onMounted(setup);
onBeforeUnmount(teardown);
</script>
