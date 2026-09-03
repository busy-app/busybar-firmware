<template>
  <ModalGeneric
    v-model:open="open"
    data-id="modal-uploader-app"
    title="Add your file"
    wide
    :dismissible="!uploading"
    :show-close-button="!file || failed"
    :description="file && !uploading && !failed ? 'This file will be added to your BUSY Bar and the local web interface.' : undefined"
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
        v-else-if="failed"
        data-id="modal-uploader-app-error"
        class="flex flex-col gap-6"
      >
        <div class="flex items-start gap-2">
          <UIcon
            name="i-bi-error-fill"
            class="size-6 shrink-0 text-error"
          />

          <div class="min-w-0 flex-1">
            Unable to upload this file. Try these steps until the issue is resolved:
          </div>
        </div>

        <div class="rounded-xl bg-accented/25 p-4">
          <ul class="list-disc space-y-5 ps-5 text-sm text-highlighted/90">
            <li>Check the file format — only .tgz files are supported</li>
            <li>Check the API version — the app must use a supported version</li>
            <li>Try again later — the server may be unavailable right now</li>
          </ul>
        </div>
      </div>

      <div
        v-else-if="uploading"
        data-id="modal-uploader-app-progress"
        class="flex flex-col gap-6 pt-4"
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

    <template #actions>
      <div
        v-if="file && !failed"
        class="mt-2 flex justify-end gap-2"
      >
        <UButton
          data-id="modal-uploader-app-cancel-button"
          label="Cancel"
          color="neutral"
          variant="ghost"
          size="lg"
          class="min-w-20 justify-center"
          @click="cancel"
        />

        <UButton
          v-if="!uploading"
          data-id="modal-uploader-app-add-button"
          label="Add"
          color="neutral"
          size="lg"
          class="min-w-20 justify-center"
          @click="addFile"
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
const failed = ref(false);

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
    failed.value = false;
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
  failed.value = false;

  try {
    await uploadAppPackage(uploadedFile, onUploadProgress, uploadController.signal);

    emit('uploaded', uploadedFile);
    open.value = false;
  } catch {
    failed.value = !uploadController.signal.aborted;
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
