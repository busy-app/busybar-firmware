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

const props = withDefaults(defineProps<{
  animation: DecodedAnimation;
  playing?: boolean;
  loop?: boolean;
}>(), {
  playing: true,
  loop: true
});

const canvasRef = ref<HTMLCanvasElement | null>(null);
const frameIndex = ref(0);
const animationFrameHandle = ref<number | null>(null);
const lastTimestamp = ref<number | null>(null);
const elapsedInFrame = ref(0);

function drawFrame (index: number) {
  const context = canvasRef.value?.getContext('2d');
  const frame = props.animation.frames[index];

  if (!context || !frame) {
    return;
  }

  context.putImageData(frame.imageData, 0, 0);
}

function stop () {
  if (animationFrameHandle.value !== null) {
    cancelAnimationFrame(animationFrameHandle.value);
    animationFrameHandle.value = null;
  }

  lastTimestamp.value = null;
}

function tick (timestamp: number) {
  animationFrameHandle.value = null;

  const frameDuration = 1000 / Math.max(1, props.animation.fps);
  const frames = props.animation.frames;

  if (lastTimestamp.value !== null) {
    elapsedInFrame.value += timestamp - lastTimestamp.value;
  }

  lastTimestamp.value = timestamp;

  let advanced = false;

  while (elapsedInFrame.value >= frameDuration * frames[frameIndex.value].duration) {
    elapsedInFrame.value -= frameDuration * frames[frameIndex.value].duration;

    if (frameIndex.value + 1 >= frames.length) {
      if (!props.loop) {
        elapsedInFrame.value = 0;
        stop();
        return;
      }

      frameIndex.value = 0;
    } else {
      frameIndex.value += 1;
    }

    advanced = true;
  }

  if (advanced) {
    drawFrame(frameIndex.value);
  }

  animationFrameHandle.value = requestAnimationFrame(tick);
}

function play () {
  if (animationFrameHandle.value !== null || props.animation.frames.length < 2) {
    return;
  }

  animationFrameHandle.value = requestAnimationFrame(tick);
}

function reset () {
  stop();
  frameIndex.value = 0;
  elapsedInFrame.value = 0;
  drawFrame(0);

  if (props.playing) {
    play();
  }
}

watch(() => props.animation, reset);

watch(() => props.playing, playing => {
  if (playing) {
    play();
  } else {
    stop();
  }
});

onMounted(reset);
onBeforeUnmount(stop);
</script>
