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
          <UFormField label="Frames per second">
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
              :options="colorModeOptions"
              option-attribute="label"
              value-attribute="value"
              @update:model-value="saveColorMode"
            />
          </UFormField>
        </div>

        <div class="flex gap-2">
          <UButton
            icon="i-bi-download"
            label="Save animation file"
            color="primary"
            variant="ghost"
            @click="composeAndDownload"
          />
          <UButton
            icon="i-bi-upload"
            label="Upload to device"
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
    await deviceStore.busyBar.StorageWrite({
      file: animationOutput.value,
      path: '/ext/animations/temp.anim'
    })
      .then(() => {
        toast.add({
          title: 'Animation uploaded',
          description: 'Open Settings > Debug Apps > Animation Player to view it',
          icon: 'i-bi-checkmark-circle',
          color: 'success',
          duration: 0,
          close: true,
          closeIcon: 'i-bi-cross'
        });
      })
      .catch(error => {
        console.error('Error uploading animation file:', error);
        toast.add({
          title: 'Upload Failed',
          description: `Failed to upload animation file: ${error}`,
          icon: 'i-bi-alert',
          color: 'error',
          duration: 0,
          close: true,
          closeIcon: 'i-bi-cross'
        });
      });
  }
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
