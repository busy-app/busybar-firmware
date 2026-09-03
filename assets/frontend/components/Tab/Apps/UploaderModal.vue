<template>
  <ModalGeneric
    v-model:open="open"
    data-id="modal-uploader-app"
    title="Add your file"
    dismissible
    wide
    :show-close-button="!file"
    :no-actions="!file"
    :description="file ? 'This file will be added to your BUSY Bar and the local web interface.' : undefined"
    :primary-action-props="{
      label: 'Add',
      onClick: addFile
    }"
    :secondary-action-props="{
      label: 'Cancel',
      onClick: () => { open = false; }
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
  (e: 'select', file: File): void;
}>();

const open = defineModel<boolean>('open', { default: false });

const APP_FILE_EXTENSION = '.tgz';

const toast = useToast();

const file = ref<File | null>(null);
const fileInput = useTemplateRef<HTMLInputElement>('fileInput');

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
  }
});

function addFile () {
  if (!file.value) {
    return;
  }

  emit('select', file.value);
  open.value = false;
}
</script>
