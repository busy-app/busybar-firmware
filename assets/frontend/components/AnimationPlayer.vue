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
}>(), {
  playing: true,
  loop: true
});

const canvasRef = ref<HTMLCanvasElement | null>(null);

let subscriber: AnimationTickerSubscriber | null = null;
let unsubscribe: (() => void) | null = null;

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

  subscriber = {
    animation: props.animation,
    context,
    loop: props.loop,
    playing: props.playing && props.animation.frames.length > 1,
    frameIndex: 0,
    elapsedInFrame: 0
  };

  drawAnimationFrame(subscriber, 0);
  unsubscribe = subscribeToAnimationTicker(subscriber);
}

watch(() => props.animation, setup);

watch(() => props.playing, playing => {
  if (!subscriber) {
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
