<template>
  <div
    data-id="state-screen-stream"
    class="relative w-fit mx-auto"
  >
    <div class="relative z-1">
      <img
        src="~/assets/images/busybar-device.png"
        class="w-[310px] sm:w-[385px]"
      >
    </div>
    <div
      class="w-[288px] sm:w-[360px]
        absolute top-[calc(50%+8.5px)] left-[calc(50%-0.5px)] transform -translate-x-1/2 -translate-y-1/2 z-2"
    >
      <canvas
        ref="canvasRef"
        data-id="state-screen-stream-canvas"
        :width="Number(configStore.get('screenStreamCanvasBaseResolutionWidth')) * dpr"
        :height="(Number(configStore.get('screenStreamCanvasBaseResolutionWidth')) / FRONT_SCREEN_WIDTH * FRONT_SCREEN_HEIGHT) * dpr"
        class="w-full bg-transparent rounded-sm aspect-[72/16]"
      />
    </div>
  </div>
</template>

<script lang="ts" setup>
import { BSB_Frame, Display, ScreenRenderer } from '@busy-app/busy-lib';
import type { ProcessedFrame } from '@busy-app/busy-lib';

const screenStreamStore = useScreenStreamStore();
const deviceStore = useDeviceStore();
const configStore = useConfigStore();

const canvasRef = useTemplateRef('canvasRef');
const dpr = window.devicePixelRatio || 1;
const FRONT_SCREEN_WIDTH = 72;
const FRONT_SCREEN_HEIGHT = 16;
const FRONT_SCREEN_PIXEL_COUNT = FRONT_SCREEN_WIDTH * FRONT_SCREEN_HEIGHT;

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

function setOpaqueAlphaChannel (source: Uint8Array): Uint8Array {
  const data = source.slice();

  for (let index = 3; index < data.length; index += 4) {
    data[index] = 0xFF;
  }

  return data;
}

function expandRgbFramePayload (source: Uint8Array): Uint8Array {
  const data = new Uint8Array((source.length / 3) * 4);

  for (let sourceIndex = 0, targetIndex = 0; sourceIndex < source.length; sourceIndex += 3, targetIndex += 4) {
    data[targetIndex] = source[sourceIndex + 2];
    data[targetIndex + 1] = source[sourceIndex + 1];
    data[targetIndex + 2] = source[sourceIndex];
    data[targetIndex + 3] = 0xFF;
  }

  return data;
}

function normalizeInitialFramePixelData (source: Uint8Array): Uint8Array {
  const expectedRgbLength = FRONT_SCREEN_PIXEL_COUNT * 3;
  const expectedRgbaLength = FRONT_SCREEN_PIXEL_COUNT * 4;

  if (source.length === expectedRgbLength) {
    return expandRgbFramePayload(source);
  }

  if (source.length === expectedRgbaLength) {
    return setOpaqueAlphaChannel(source);
  }

  return source;
}

async function normalizeInitialFramePayload (source: Blob | ArrayBuffer | Uint8Array | string): Promise<Uint8Array> {
  if (typeof source === 'string') {
    const data = isBase64FramePayload(source) ? decodeBase64FramePayload(source) : new TextEncoder().encode(source);

    return normalizeInitialFramePixelData(data);
  }

  if (source instanceof Blob) {
    const text = await source.text();

    if (isBase64FramePayload(text)) {
      return normalizeInitialFramePixelData(decodeBase64FramePayload(text));
    }

    return normalizeInitialFramePixelData(new Uint8Array(await source.arrayBuffer()));
  }

  const data = source instanceof Uint8Array ? source : new Uint8Array(source);
  const text = new TextDecoder().decode(data);

  return normalizeInitialFramePixelData(isBase64FramePayload(text) ? decodeBase64FramePayload(text) : data);
}

async function convertInitialFrameToStateFrame (source: Blob | ArrayBuffer | Uint8Array | string): Promise<ProcessedFrame> {
  const data = await normalizeInitialFramePayload(source);

  return {
    screen: BSB_Frame.Screen.FRONT,
    width: FRONT_SCREEN_WIDTH,
    height: FRONT_SCREEN_HEIGHT,
    encoding: BSB_Frame.Encoding.PLAIN,
    pixelFormat: BSB_Frame.PixelFormat.RGB888,
    data
  };
}

async function getInitialFrame (): Promise<ProcessedFrame> {
  const busyBar = deviceStore.busyBar;
  const screen = await busyBar.DisplayScreenFrameGet({ display: BSB_Frame.Screen.FRONT });
  const initialFrame = await convertInitialFrameToStateFrame(screen as Blob | ArrayBuffer | Uint8Array | string);

  return initialFrame;
}

function renderFrame (frame: ProcessedFrame) {
  if (canvasRef.value && frame?.data && frame.width && frame.height) {
    ScreenRenderer.renderFrame(Display.FRONT, {
      canvas: canvasRef.value,
      data: frame.data,
      width: frame.width,
      height: frame.height
    });
  }
}

watch(() => screenStreamStore.currentFrame, currentFrame => {
  if (currentFrame) {
    renderFrame(currentFrame);
  }
}, { flush: 'post' });

onMounted(async () => {
  if (screenStreamStore.currentFrame) {
    renderFrame(screenStreamStore.currentFrame);
  } else {
    const initialFrame = await getInitialFrame();
    if (initialFrame) {
      screenStreamStore.currentFrame = initialFrame;
    }
  }
});
</script>

<style scoped>
.canvas-container {
  max-width: 360px;
  width: 100%;
  position: absolute;
  top: calc(50% + 8px);
  left: calc(50% - 1px);
  transform: translate(-50%, -50%);
  z-index: 2;
}
</style>
