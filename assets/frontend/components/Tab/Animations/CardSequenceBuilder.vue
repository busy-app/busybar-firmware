<template>
  <SectionCard data-id="animations-section-primary">
    <div class="w-full flex flex-col gap-4">
      <UFileUpload
        v-model="filesModel"
        multiple
        data-id="modal-update-firmware-file-upload"
        accept=".png"
        layout="list"
        class="w-full rounded-xl"
        label="Upload sequence files (.png)"
        description="Drag and drop to upload"
        :ui="{
          base: 'cursor-pointer',
          icon: 'size-6',
          label: 'text-lg',
          description: 'text-sm',
          files: 'max-h-[175px] overflow-y-auto',
          file: 'bg-elevated/50',
          fileLeadingAvatar: 'w-24 h-auto rounded-none mr-1'
        }"
      >
        <template #actions>
          <UButton
            :label="filesModel?.length ? 'Add more files' : 'Select files'"
            color="neutral"
            class="mt-2"
          />
        </template>

        <template #files-top="{ files }">
          <div
            v-if="files?.length"
            class="w-full my-2 flex items-center justify-between"
          >
            <p class="font-bold">Files ({{ files?.length }})</p>

            <UButton
              icon="i-bi-cross"
              label="Clear"
              color="error"
              variant="ghost"
              class="-my-2"
              @click="filesModel = null"
            />
          </div>
        </template>
      </UFileUpload>

      <div class="w-full flex items-end justify-between gap-6">
        <div class="flex gap-4">
          <UFormField label="FPS">
            <UInput
              v-model="fpsModel"
              type="number"
              :min="1"
              @update:model-value="saveFps"
            />
          </UFormField>

          <UFormField label="Color Mode">
            <USelect
              v-model="colorModeModel"
              :items="colorModeOptions"
              @update:model-value="saveColorMode"
            />
          </UFormField>
        </div>

        <div class="flex gap-2">
          <UButton
            icon="i-bi-download"
            label="Save animation file"
            color="neutral"
            variant="ghost"
            @click="composeAndDownload"
          />
          <UButton
            icon="i-bi-control-play"
            label="Play on device"
            color="neutral"
            variant="solid"
            @click="composeAndUpload"
          />
        </div>
      </div>
    </div>
  </SectionCard>
</template>

<script setup lang="ts">
import { composeAnimation } from '@/util/seq2anim';
import type { ColorMode } from '@/util/seq2anim';
import type { DisplayDrawParams } from '@busy-app/busy-lib';

const deviceStore = useDeviceStore();

const filesModel = ref<File[] | null>(null);
const fpsModel = ref<number>(30);
const colorModeModel = ref<ColorMode>('rgb888');

const colorModeOptions = [
  { label: 'RGB888 (Front)', value: 'rgb888' },
  { label: 'Gray4 (Back)', value: 'gray4' }
];

const animationOutput = ref<Blob | null>(null);

async function handleComposeAnimation () {
  if (!filesModel.value || filesModel.value.length === 0) {
    console.warn('No files selected for animation composition');
    return;
  }

  try {
    toast.remove('animation-compose-error');
    const animation = await composeAnimation(filesModel.value, {
      fps: fpsModel.value,
      colorMode: colorModeModel.value
    });
    animationOutput.value = animation;
    console.log('Composed animation:', animationOutput.value);
  } catch (error) {
    console.error('Error composing animation:', error);
    toast.add({
      id: 'animation-compose-error',
      title: 'Animation Composition Error',
      description: String(error),
      icon: 'i-bi-alert',
      color: 'error',
      duration: 0,
      close: true,
      closeIcon: 'i-bi-cross'
    });
  }
}

async function composeAndDownload () {
  await handleComposeAnimation();
  if (animationOutput.value) {
    const url = createObjectUrl(animationOutput.value);
    const a = document.createElement('a');
    a.href = url;
    a.download = 'test.anim';
    document.body.appendChild(a);
    a.click();
    document.body.removeChild(a);
    URL.revokeObjectURL(url);
  }
}

async function composeAndUpload () {
  await handleComposeAnimation();
  if (animationOutput.value) {
    try {
      await deleteAssets();

      await uploadAnimation(animationOutput.value);

      await drawAnimation();

      toast.add({
        title: 'Animation uploaded',
        description: 'Check the front display to view it',
        icon: 'i-bi-checkmark-circle',
        color: 'success',
        duration: 10000,
        close: true,
        closeIcon: 'i-bi-cross'
      });
    } catch {
      // request errors are already handled
    }
  }
}

async function deleteAssets () {
  return deviceStore.busyBar.AssetsDelete({
    appId: 'virtual-lan-animation-test'
  })
    .catch(async error => {
      if (String(error).startsWith('Error: Assets missing')) {
        // if there are no existing assets, we can ignore the error and proceed with upload
        return;
      }

      await handleHTTPError(error, 'Couldn\'t delete existing animation assets', true);
      throw error;
    });
}

async function uploadAnimation (animation: Blob) {
  return deviceStore.busyBar.AssetsUpload({
    appId: 'virtual-lan-animation-test',
    file: animation,
    fileName: 'test.anim'
  })
    .catch(async error => {
      await handleHTTPError(error, 'Couldn\'t upload animation file', true);
      throw error;
    });
}

async function drawAnimation () {
  return deviceStore.busyBar.DisplayDraw({
    appId: 'virtual-lan-animation-test',
    elements: [
      {
        id: '0',
        timeout: 0,
        align: 'top_left',
        display: 'front',
        x: 0,
        y: 0,
        type: 'anim',
        path: 'test.anim',
        section_name: 'loop',
        loop: true
      }
    ]
  } as unknown as DisplayDrawParams)
    .catch(async error => {
      await handleHTTPError(error, 'Display draw command failed', true);
      throw error;
    });
}

function createObjectUrl (file: File | Blob): string {
  return URL.createObjectURL(file);
}

function saveFps () {
  localStorage.setItem('animation-fps', fpsModel.value.toString());
}

function saveColorMode () {
  localStorage.setItem('animation-color-mode', colorModeModel.value);
}

function init () {
  const savedFps = localStorage.getItem('animation-fps');
  if (savedFps) {
    fpsModel.value = parseInt(savedFps, 10);
  }
  const savedColorMode = localStorage.getItem('animation-color-mode');
  if (savedColorMode) {
    colorModeModel.value = savedColorMode as ColorMode;
  }
}

onMounted(async () => {
  init();
  window.addEventListener('device-reconnected', init);
});
onBeforeUnmount(() => window.removeEventListener('device-reconnected', init));
</script>
