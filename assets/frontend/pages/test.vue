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
        description: 'text-sm'
      }"
    >
      <template #actions>
        <UButton
          label="Select files"
          color="neutral"
          variant="soft"
          :ui="{ base: 'bg-neutral-200/50 dark:bg-neutral-700/50' }"
          class="mt-2"
        />
      </template>

      <template #files-top="{ files }">
        <div
          v-if="files?.length"
          class="w-full mb-2 flex items-center justify-between"
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
        <div class="flex flex-wrap justify-start gap-1.5 overflow-y-scroll">
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
        </div>
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

      <div>
        <UButton
          icon="i-bi-download"
          label="Download animation"
          color="primary"
          variant="solid"
          @click="handleComposeAnimation"
        />
      </div>
    </div>

    <div class="mt-4">
      Upload file to the device:
      <CopyButton
        variant="link"
        color="neutral"
        class="ml-2 text-left text-xs"
        :text="`python3 assets/frontend/util/upload_anim.py -d /ext/animations ~/Downloads/temp.anim`"
        style="font-family: monospace;"
      />
    </div>
  </div>
</template>

<script setup lang="ts">
import { composeAnimation } from '@/util/seq2anim';

useHead({
  title: 'BUSY Bar Virtual LAN',
  meta: [
    {
      name: 'description',
      content: 'Control your BUSY Bar in the browser'
    }
  ]
});

const filesModel = ref<File[] | null>(null);
const fpsModel = ref<number>(60);

const animationOutput = ref<Blob | null>(null);

async function handleComposeAnimation () {
  if (!filesModel.value || filesModel.value.length === 0) {
    console.warn('No files selected for animation composition');
    return;
  }

  try {
    const animation = await composeAnimation(filesModel.value, fpsModel.value);
    animationOutput.value = animation;
    console.log('Composed animation:', animationOutput.value);

    // download animation for testing
    const url = createObjectUrl(animation);
    const a = document.createElement('a');
    a.href = url;
    a.download = 'temp.anim';
    document.body.appendChild(a);
    a.click();
    document.body.removeChild(a);
    URL.revokeObjectURL(url);
  } catch (error) {
    console.error('Error composing animation:', error);
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
