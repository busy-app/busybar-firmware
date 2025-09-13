<template>
  <div class="screen-stream-container">
    <div class="device-image">
      <img
        src="~/assets/images/busybar-device.png"
        class="w-[388px]"
      >
    </div>
    <div class="canvas-container">
      <canvas
        ref="canvasRef"
        :width="canvasWidth"
        :height="canvasHeight"
        class="aspect-[72/16]"
      />
    </div>
    <img
      src="~/assets/images/front-screen-pixel-grid.png"
      class="w-[360px] absolute top-[calc(50%-32px)] left-1/2 transform -translate-x-1/2 z-[3]"
    >
  </div>
</template>

<script lang="ts" setup>
import { DeviceScreen } from '@busy-app/busy-lib';

const deviceScreenStreamStore = useDeviceScreenStreamStore();
const toast = useToast();
const canvasRef = ref<HTMLCanvasElement | null>(null);
const canvasCtx = ref<CanvasRenderingContext2D | null>(null);

const originalDimensions = computed(() => {
  return deviceScreenStreamStore.currentScreen === DeviceScreen.FRONT
    ? { width: 72, height: 16 } // front screen
    : { width: 160, height: 80 }; // back screen
});

// scale the canvas
const scaleFactor = 5;
const canvasWidth = computed(() => originalDimensions.value.width * scaleFactor);
const canvasHeight = computed(() => originalDimensions.value.height * scaleFactor);

function generateGrayscalePalette (): Array<[number, number, number]> {
  const palette: Array<[number, number, number]> = [];
  for (let x = 0; x < 16; x++) {
    const value = ((x - 0) / (16 - 0)) * (255 - 0) + 0;
    palette.push([value, value, value]);
  }
  return palette;
}

const backColorPalette = generateGrayscalePalette();

function dataCallback (data: Uint8Array) {
  if (!canvasCtx.value) {
    return;
  }

  const ctx = canvasCtx.value;
  const currentScreen = deviceScreenStreamStore.currentScreen;
  const { width, height } = originalDimensions.value;

  ctx.clearRect(0, 0, canvasWidth.value, canvasHeight.value);

  const imageData = ctx.createImageData(width, height);

  if (currentScreen === DeviceScreen.FRONT) {
    for (let i = 0; i < data.length; i += 3) {
      const offset = i / 3 * 4;
      imageData.data[offset] = data[i + 2]; // r
      imageData.data[offset + 1] = data[i + 1]; // g
      imageData.data[offset + 2] = data[i]; // b
      imageData.data[offset + 3] = 255; // a
    }
  } else {
    for (let i = 0; i < data.length; i++) {
      const offset = i * 4;
      const pixelValue = data[i]; // 0-15
      const [r, g, b] = backColorPalette[pixelValue];
      imageData.data[offset] = r;
      imageData.data[offset + 1] = g;
      imageData.data[offset + 2] = b;
      imageData.data[offset + 3] = 255; // a
    }
  }

  const tempCanvas = document.createElement('canvas');
  tempCanvas.width = width;
  tempCanvas.height = height;
  const tempCtx = tempCanvas.getContext('2d');

  if (tempCtx) {
    tempCtx.putImageData(imageData, 0, 0);

    ctx.imageSmoothingEnabled = false;
    ctx.drawImage(
      tempCanvas,
      0,
      0,
      width,
      height,
      0,
      0,
      canvasWidth.value,
      canvasHeight.value
    );
  }
}

function stopCallback () {
  console.log('Screen stream stopped');

  if (canvasCtx.value) {
    canvasCtx.value.clearRect(0, 0, canvasWidth.value, canvasHeight.value);
  }
}

onMounted(() => {
  if (canvasRef.value) {
    canvasCtx.value = canvasRef.value.getContext('2d');

    if (canvasCtx.value) {
      canvasCtx.value.imageSmoothingEnabled = false;
    }
  }

  deviceScreenStreamStore.startScreenStream(dataCallback, stopCallback)
    .then(() => {
      console.log('Screen stream started');
    })
    .catch(error => {
      console.error('Error starting screen stream:', error);
      toast.add({
        id: 'screen-stream-error',
        title: 'Error starting screen stream',
        description: error.message || 'An error occurred while starting the screen stream.',
        color: 'error',
        duration: 5000
      });
    });
});

onBeforeUnmount(() => {
  deviceScreenStreamStore.stopScreenStream();
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
  top: calc(50% + 20px);
  left: 50%;
  transform: translate(-50%, -50%);
  z-index: 2;
}

canvas {
  display: block;
  background-color: transparent;
}
</style>
