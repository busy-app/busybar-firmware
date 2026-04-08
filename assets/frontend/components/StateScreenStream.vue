<template>
  <div
    data-id="state-screen-stream"
    class="screen-stream-container"
  >
    <div class="device-image">
      <img
        src="~/assets/images/busybar-device.png"
        class="w-[310px] sm:w-[385px]"
      >
    </div>
    <div class="canvas-container">
      <canvas
        ref="canvasRef"
        data-id="state-screen-stream-canvas"
        :width="canvasWidth"
        :height="canvasHeight"
        class="aspect-[72/16]"
      />
    </div>
  </div>
</template>

<script lang="ts" setup>
import { DeviceScreen } from '@busy-app/busy-lib';
import { decodeFramePayload } from '@/util/stateFrameData';
import type { StateFrameMessage } from '@/util/stateStreamMessage';

const screenStreamStore = useScreenStreamStore();
const deviceStore = useDeviceStore();
const canvasRef = ref<HTMLCanvasElement | null>(null);
const canvasCtx = ref<CanvasRenderingContext2D | null>(null);

const currentFrame = computed(() => screenStreamStore.currentFrame);
const originalDimensions = computed(() => {
  if (currentFrame.value?.width && currentFrame.value?.height) {
    return {
      width: currentFrame.value.width,
      height: currentFrame.value.height
    };
  }

  return screenStreamStore.currentScreen === DeviceScreen.FRONT
    ? { width: 72, height: 16 }
    : { width: 160, height: 80 };
});

const windowWidth = ref(window.innerWidth);
function handleResize () {
  windowWidth.value = window.innerWidth;
  if (windowWidth.value > 640 && scaleFactor.value !== 5) {
    scaleFactor.value = 5;
  } else if (windowWidth.value <= 640 && scaleFactor.value !== 4) {
    scaleFactor.value = 4;
  }
}

const scaleFactor = ref(windowWidth.value > 640 ? 5 : 4);
const canvasWidth = computed(() => originalDimensions.value.width * scaleFactor.value);
const canvasHeight = computed(() => originalDimensions.value.height * scaleFactor.value);

function generateGrayscalePalette (): Array<[number, number, number]> {
  const palette: Array<[number, number, number]> = [];
  for (let value = 0; value < 16; value++) {
    const brightness = (value / 15) * 255;
    palette.push([brightness, brightness, brightness]);
  }
  return palette;
}

const backColorPalette = generateGrayscalePalette();
let renderSequence = 0;

function clearCanvas () {
  if (canvasCtx.value) {
    canvasCtx.value.clearRect(0, 0, canvasWidth.value, canvasHeight.value);
  }
}

async function renderFrame (frame: StateFrameMessage) {
  if (!canvasCtx.value) {
    return;
  }

  const sequence = ++renderSequence;
  let frameData: Uint8Array;

  try {
    frameData = await decodeFramePayload(frame);
  } catch (error) {
    console.error('Could not decode streamed frame', error);
    return;
  }

  if (sequence !== renderSequence || !canvasCtx.value) {
    return;
  }

  const ctx = canvasCtx.value;
  const { width, height } = originalDimensions.value;
  const pixelFormat = frame.pixelFormat ?? 'RGB888';

  ctx.clearRect(0, 0, canvasWidth.value, canvasHeight.value);

  const imageData = ctx.createImageData(width, height);

  if (pixelFormat === 'RGB888') {
    for (let index = 0; index < frameData.length; index += 3) {
      const offset = index / 3 * 4;
      imageData.data[offset] = frameData[index + 2];
      imageData.data[offset + 1] = frameData[index + 1];
      imageData.data[offset + 2] = frameData[index];
      imageData.data[offset + 3] = frameData[index + 2] === 0 && frameData[index + 1] === 0 && frameData[index] === 0 ? 0 : 255;
    }
  } else {
    for (let index = 0; index < frameData.length; index++) {
      const offset = index * 4;
      const pixelValue = pixelFormat === 'L8' ? Math.round((frameData[index] / 255) * 15) : frameData[index];
      const [red, green, blue] = backColorPalette[Math.max(0, Math.min(15, pixelValue))];
      imageData.data[offset] = red;
      imageData.data[offset + 1] = green;
      imageData.data[offset + 2] = blue;
      imageData.data[offset + 3] = 255;
    }
  }

  const tempCanvas = document.createElement('canvas');
  tempCanvas.width = width;
  tempCanvas.height = height;
  const tempCtx = tempCanvas.getContext('2d');

  if (!tempCtx) {
    return;
  }

  tempCtx.putImageData(imageData, 0, 0);

  ctx.imageSmoothingEnabled = false;
  ctx.drawImage(tempCanvas, 0, 0, width, height, 0, 0, canvasWidth.value, canvasHeight.value);

  const gap = scaleFactor.value;
  ctx.save();
  ctx.strokeStyle = 'black';
  ctx.lineWidth = 1;

  for (let x = 0; x <= canvasWidth.value; x += gap) {
    ctx.beginPath();
    ctx.moveTo(x + 0.5, 0);
    ctx.lineTo(x + 0.5, canvasHeight.value);
    ctx.stroke();
  }

  for (let y = 0; y <= canvasHeight.value; y += gap) {
    ctx.beginPath();
    ctx.moveTo(0, y + 0.5);
    ctx.lineTo(canvasWidth.value, y + 0.5);
    ctx.stroke();
  }

  const scaledImageData = ctx.getImageData(0, 0, canvasWidth.value, canvasHeight.value);
  const pixels = scaledImageData.data;

  for (let index = 0; index < pixels.length; index += 4) {
    const brightness = 0.299 * pixels[index] + 0.587 * pixels[index + 1] + 0.114 * pixels[index + 2];
    if (brightness <= 10) {
      pixels[index + 3] = Math.round(255 * (brightness / 51) * 0.2);
    }
  }

  ctx.putImageData(scaledImageData, 0, 0);
  ctx.restore();
}

watch(currentFrame, frame => {
  if (!frame) {
    clearCanvas();
    return;
  }

  void renderFrame(frame);
}, { immediate: true });

watch([canvasWidth, canvasHeight], () => {
  if (currentFrame.value) {
    void renderFrame(currentFrame.value);
  }
});

function isBase64FramePayload (value: string): boolean {
  const normalized = value.trim();

  return normalized.length > 0
    && normalized.length % 4 === 0
    && /^[A-Za-z0-9+/]+={0,2}$/.test(normalized);
}

function decodeBase64FramePayload (value: string): Uint8Array {
  const decoded = atob(value.trim());
  const data = new Uint8Array(decoded.length);

  for (let index = 0; index < decoded.length; index++) {
    data[index] = decoded.charCodeAt(index);
  }

  return data;
}

async function normalizeInitialFramePayload (source: Blob | ArrayBuffer | Uint8Array | string): Promise<Uint8Array> {
  if (typeof source === 'string') {
    return isBase64FramePayload(source) ? decodeBase64FramePayload(source) : new TextEncoder().encode(source);
  }

  if (source instanceof Blob) {
    const text = await source.text();

    if (isBase64FramePayload(text)) {
      return decodeBase64FramePayload(text);
    }

    return new Uint8Array(await source.arrayBuffer());
  }

  const data = source instanceof Uint8Array ? source : new Uint8Array(source);
  const text = new TextDecoder().decode(data);

  return isBase64FramePayload(text) ? decodeBase64FramePayload(text) : data;
}

async function convertInitialFrameToStateFrame (source: Blob | ArrayBuffer | Uint8Array | string): Promise<StateFrameMessage> {
  const data = await normalizeInitialFramePayload(source);

  return {
    screen: 'FRONT',
    width: 72,
    height: 16,
    encoding: 'PLAIN',
    pixelFormat: 'RGB888',
    data
  };
}

async function getInitialFrame () {
  const busyBar = deviceStore.busyBar;
  const screen = await busyBar.DisplayScreenFrameGet({ display: DeviceScreen.FRONT });
  const initialFrame = await convertInitialFrameToStateFrame(screen as Blob | ArrayBuffer | Uint8Array | string);

  await renderFrame(initialFrame);
}

onMounted(async () => {
  window.addEventListener('resize', handleResize);

  if (canvasRef.value) {
    canvasCtx.value = canvasRef.value.getContext('2d', { willReadFrequently: true });
    if (canvasCtx.value) {
      canvasCtx.value.imageSmoothingEnabled = false;
    }
  }

  if (currentFrame.value) {
    await renderFrame(currentFrame.value);
  } else {
    await getInitialFrame();
  }
});

onBeforeUnmount(() => {
  window.removeEventListener('resize', handleResize);
});
</script>

<style scoped>
.screen-stream-container {
  position: relative;
  width: fit-content;
  margin: 0 auto;
}

.device-image {
  position: relative;
  z-index: 1;
}

.canvas-container {
  position: absolute;
  top: calc(50% + 8px);
  left: calc(50% - 1px);
  transform: translate(-50%, -50%);
  z-index: 2;
}

canvas {
  display: block;
  background-color: transparent;
  border-radius: 2px;
}
</style>
