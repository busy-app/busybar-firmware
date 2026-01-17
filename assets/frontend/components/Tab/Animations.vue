<template>
  <div
    data-id="page-index"
    class="w-full flex flex-col gap-4"
  >
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
          variant="soft"
          :ui="{ base: 'bg-neutral-200/50 dark:bg-neutral-700/50' }"
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
            icon="i-ri-close-line"
            label="Clear"
            color="error"
            variant="ghost"
            class="-my-2"
            @click="filesModel = null"
          />
        </div>
      </template>

      <template #files>
        <!-- <div class="flex flex-wrap justify-start gap-1.5 overflow-y-scroll">
          <div
            v-for="(file, index) in filesModel"
            :key="index"
            class="ring-1 ring-muted"
          >
            <img
              :src="createObjectUrl(file)"
              alt="Uploaded file preview"
              class="w-full h-full object-cover"
            >
          </div>
        </div> -->
      </template>
    </UFileUpload>

    <div class="w-full flex items-end justify-between gap-6">
      <UFormField label="Frames per second">
        <UInput
          v-model="fpsModel"
          type="number"
          :min="1"
          @update:model-value="saveFps"
        />
      </UFormField>

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
</template>

<script setup lang="ts">
import { composeAnimation } from '@/util/seq2anim';

const deviceStore = useDeviceStore();

const filesModel = ref<File[] | null>(null);
const fpsModel = ref<number>(60);

const animationOutput = ref<Blob | null>(null);

async function handleComposeAnimation () {
  if (!filesModel.value || filesModel.value.length === 0) {
    console.warn('No files selected for animation composition');
    return;
  }

  try {
    toast.remove('animation-compose-error');
    const animation = await composeAnimation(filesModel.value, fpsModel.value);
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
    await deviceStore.busyBar.uploadFile({
      file: animationOutput.value,
      path: '/ext/animations/test.anim'
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
onMounted(() => {
  const savedFps = localStorage.getItem('animation-fps');
  if (savedFps) {
    fpsModel.value = parseInt(savedFps, 10);
  }
});
</script>
