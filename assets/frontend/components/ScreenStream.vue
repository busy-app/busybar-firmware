<template>
  <div
    data-id="screen-stream"
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
        data-id="screen-stream-canvas"
        :width="canvasWidth"
        :height="canvasHeight"
        class="aspect-[72/16]"
      />
    </div>
  </div>
</template>

<script lang="ts" setup>
import { DeviceScreen } from '@busy-app/busy-lib';

const deviceScreenStreamStore = useDeviceScreenStreamStore();
const canvasRef = ref<HTMLCanvasElement | null>(null);
const canvasCtx = ref<CanvasRenderingContext2D | null>(null);

const originalDimensions = computed(() => {
  return deviceScreenStreamStore.currentScreen === DeviceScreen.FRONT
    ? { width: 72, height: 16 } // front screen
    : { width: 160, height: 80 }; // back screen
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

// scale the canvas
const scaleFactor = ref(windowWidth.value > 640 ? 5 : 4);
const canvasWidth = computed(() => originalDimensions.value.width * scaleFactor.value);
const canvasHeight = computed(() => originalDimensions.value.height * scaleFactor.value);

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
      // If r, g, and b are all 0, set alpha to 0 (transparent), else 255 (opaque)
      if (data[i + 2] === 0 && data[i + 1] === 0 && data[i] === 0) {
        imageData.data[offset + 3] = 0; // transparent
      } else {
        imageData.data[offset + 3] = 255; // opaque
      }
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

    // Draw grid
    const gap = scaleFactor.value;
    ctx.save();
    ctx.strokeStyle = 'black';
    ctx.lineWidth = 1;

    // Vertical lines
    for (let x = 0; x <= canvasWidth.value; x += gap) {
      ctx.beginPath();
      ctx.moveTo(x + 0.5, 0);
      ctx.lineTo(x + 0.5, canvasHeight.value);
      ctx.stroke();
    }

    // Horizontal lines
    for (let y = 0; y <= canvasHeight.value; y += gap) {
      ctx.beginPath();
      ctx.moveTo(0, y + 0.5);
      ctx.lineTo(canvasWidth.value, y + 0.5);
      ctx.stroke();
    }

    // Apply darkening to very dark pixels
    const imgData = ctx.getImageData(0, 0, canvasWidth.value, canvasHeight.value);
    const d = imgData.data;
    for (let i = 0; i < d.length; i += 4) {
      // Calculate perceived brightness (0=black, 255=white)
      const brightness = 0.299 * d[i] + 0.587 * d[i + 1] + 0.114 * d[i + 2];
      // For very dark pixels (brightness <= 10), apply a fade to black
      if (brightness <= 10) {
        // Linear fade: black (0) = 0 alpha, 51 = 0.2 alpha, up to 255 = 1 alpha
        d[i + 3] = Math.round(255 * ((brightness - 0) / (51 - 0)) * 0.2);
      }
    }
    ctx.putImageData(imgData, 0, 0);

    ctx.restore();
  }
}

function stopCallback () {
  if (canvasCtx.value) {
    canvasCtx.value.clearRect(0, 0, canvasWidth.value, canvasHeight.value);
  }
}

async function init () {
  if (deviceScreenStreamStore.isConnected) {
    await deviceScreenStreamStore.stopScreenStream();
  }

  if (canvasRef.value) {
    canvasCtx.value = canvasRef.value.getContext('2d', { willReadFrequently: true });

    if (canvasCtx.value) {
      canvasCtx.value.imageSmoothingEnabled = false;
    }
  }

  deviceScreenStreamStore.startScreenStream(dataCallback, stopCallback)
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
}

onMounted(async () => {
  window.addEventListener('resize', handleResize);

  await init();
  window.addEventListener('device-reconnected', init);
});
onBeforeUnmount(async () => {
  window.removeEventListener('resize', handleResize);
  await deviceScreenStreamStore.stopScreenStream();
  window.removeEventListener('device-reconnected', init);
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
