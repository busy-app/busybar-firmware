<template>
  <ModalGeneric
    v-model:open="open"
    data-id="modal-uploader-app"
    title="Add your file"
    wide
    :dismissible="!uploading"
    :show-close-button="!file"
    :no-actions="!file"
    :description="file && !uploading ? 'This file will be added to your BUSY Bar and the local web interface.' : undefined"
    :primary-action-props="uploading ? undefined : {
      label: 'Add',
      onClick: addFile
    }"
    :secondary-action-props="{
      label: 'Cancel',
      onClick: cancel
    }"
  >
    <template #body>
      <template v-if="!file">
        <div @click="fileInput?.click()">
          <UFileUpload
            v-model="file"
            data-id="modal-uploader-app-file-upload"
            accept=".tgz"
            :interactive="false"
            class="h-[396px] w-full"
            label="Add app file (.tgz)"
            description="Drag and drop to upload"
            :ui="{
              base: 'bg-transparent border-solid rounded-2xl cursor-pointer transition-colors data-[dragging=true]:bg-transparent data-[dragging=true]:border-accented',
              label: 'text-lg mt-4',
              description: 'text-sm mt-1',
              actions: 'mt-6',
            }"
          >
            <template #leading>
              <div class="flex size-12 items-center justify-center rounded-full bg-accented/25">
                <UIcon
                  name="i-bi-upload"
                  class="size-6"
                />
              </div>
            </template>

            <template #actions>
              <UButton
                data-id="modal-uploader-app-select-file-button"
                label="Select file"
                color="neutral"
              />
            </template>
          </UFileUpload>
        </div>

        <input
          ref="fileInput"
          data-id="modal-uploader-app-file-input"
          type="file"
          accept=".tgz"
          class="sr-only"
          @change="selectFile"
        >
      </template>

      <div
        v-else-if="uploading"
        data-id="modal-uploader-app-progress"
        class="flex flex-col gap-6 py-4"
      >
        <div class="flex items-start gap-2">
          <UIcon
            name="i-bi-upload"
            class="size-6 shrink-0"
          />

          <div class="min-w-0 flex-1">
            Uploading the file. Do not disconnect your BUSY Bar.
          </div>

          <div
            data-id="modal-uploader-app-progress-value"
            class="text-muted"
          >
            {{ progress }}%
          </div>
        </div>

        <UProgress
          v-model="progress"
          color="success"
          size="lg"
        />
      </div>

      <div
        v-else
        data-id="modal-uploader-app-file-selected"
        class="flex items-center gap-2 rounded-xl border border-accented p-3"
      >
        <div class="flex items-center p-2">
          <UIcon
            name="i-bi-archive"
            class="size-6"
          />
        </div>

        <div
          data-id="modal-uploader-app-file-name"
          class="min-w-0 flex-1 truncate font-medium text-toned"
        >
          {{ file.name }}
        </div>

        <UButton
          data-id="modal-uploader-app-remove-file-button"
          icon="i-bi-trash"
          color="neutral"
          variant="soft"
          square
          :ui="{
            base: 'p-2.5 rounded-full bg-accented/50'
          }"
          @click="() => { file = null; }"
        />
      </div>
    </template>
  </ModalGeneric>
</template>

<script setup lang="ts">

const emit = defineEmits<{
  (e: 'uploaded', file: File): void;
}>();

const open = defineModel<boolean>('open', { default: false });

const APP_FILE_EXTENSION = '.tgz';

const toast = useToast();

const file = ref<File | null>(null);
const fileInput = useTemplateRef<HTMLInputElement>('fileInput');

const uploading = ref(false);
const progress = ref(0);

let uploadController: AbortController | null = null;

function selectFile (event: Event) {
  const input = event.target as HTMLInputElement;

  file.value = input.files?.[0] ?? null;
  input.value = '';
}

watch(file, value => {
  if (!value || value.name.toLowerCase().endsWith(APP_FILE_EXTENSION)) {
    return;
  }

  toast.add({
    title: 'Unsupported file',
    description: `Only ${APP_FILE_EXTENSION} app packages can be uploaded.`,
    icon: 'i-bi-alert',
    color: 'error'
  });

  file.value = null;
});

watch(open, value => {
  if (value) {
    file.value = null;
    progress.value = 0;
  }
});

function onUploadProgress (value: number) {
  progress.value = value;
}

async function addFile () {
  if (!file.value) {
    return;
  }

  const uploadedFile = file.value;

  uploadController = new AbortController();
  uploading.value = true;
  progress.value = 0;

  try {
    await uploadAppPackage(uploadedFile, onUploadProgress, uploadController.signal);

    emit('uploaded', uploadedFile);
    open.value = false;
  } catch {
    if (!uploadController.signal.aborted) {
      toast.add({
        title: 'Upload failed',
        description: `${uploadedFile.name} could not be uploaded to your BUSY Bar.`,
        icon: 'i-bi-alert',
        color: 'error'
      });
    }
  } finally {
    uploading.value = false;
    uploadController = null;
  }
}

function cancel () {
  if (uploading.value) {
    uploadController?.abort();
    return;
  }

  open.value = false;
}
</script>
