<template>
  <SectionCard data-id="files-section-primary">
    <div class="w-full grid grid-cols-[32px_1fr_auto] gap-2 items-center pr-1">
      <div>
        <UTooltip
          :delay-duration="0"
          text="To parent directory"
        >
          <UButton
            icon="i-ri-arrow-left-line"
            variant="ghost"
            color="neutral"
            square
            :loading="loading.list"
            :disabled="currentPath === '/ext'"
            @click="toParentDir"
          />
        </UTooltip>
      </div>

      <div
        ref="pathEl"
        class="bg-neutral-200 dark:bg-neutral-900 rounded py-1 px-2 overflow-auto"
      >
        {{ currentPath }}
      </div>

      <div class="flex justify-end gap-4 pl-2">
        <UTooltip
          :delay-duration="0"
          text="Create directory"
        >
          <UButton
            icon="i-ri-folder-add-line"
            variant="ghost"
            color="neutral"
            square
            :loading="loading.mkdir"
            @click="mkdirNameModel = ''; showMkdirModal = true"
          />
        </UTooltip>

        <UTooltip
          :delay-duration="0"
          text="Upload files"
        >
          <UButton
            icon="i-bi-upload"
            variant="ghost"
            color="neutral"
            square
            :loading="loading.list"
            @click="filesModel = null; filesToUpload = []; showUploadModal = true;"
          />
        </UTooltip>

        <UTooltip
          :delay-duration="0"
          text="Refresh"
        >
          <UButton
            icon="i-bi-refresh"
            variant="ghost"
            color="neutral"
            square
            :loading="loading.list"
            @click="list(currentPath)"
          />
        </UTooltip>
      </div>
    </div>

    <div class="w-full flex flex-col">
      <div
        v-for="item in currentDir"
        :key="`${item.name}_${item.type}`"
        class="min-h-10 grid grid-cols-[1fr_auto] gap-2 items-center py-1 pl-2 pr-1 hover:bg-neutral-200 dark:hover:bg-neutral-700 rounded break-all"
      >
        <div
          class="grid grid-cols-[24px_1fr] gap-4 items-center"
          :class="item.type === 'dir' ? 'cursor-pointer hover:underline' : ''"
          @click="item.type === 'dir' ? list(`${currentPath}/${item.name}`) : ''"
        >
          <UIcon
            class="size-5"
            :name="item.type === 'dir' ? 'i-bi-folder' : 'i-bi-file'"
          />

          <div>{{ item.name }}</div>
        </div>

        <div class="flex justify-end gap-2">
          <UTooltip
            v-if="item.type === 'file'"
            :delay-duration="0"
            text="Download"
          >
            <UButton
              icon="i-bi-download"
              variant="ghost"
              color="neutral"
              square
              :loading="loading.read"
              @click="read(item.name)"
            />
          </UTooltip>
          <UTooltip
            :delay-duration="0"
            text="Delete"
          >
            <UButton
              icon="i-bi-trash"
              variant="ghost"
              color="error"
              square
              :loading="loading.read"
              @click="itemToDelete = { ...item, fullPath: `${currentPath}/${item.name}` }; showDeleteModal = true;"
            />
          </UTooltip>
        </div>
      </div>

      <div
        v-if="currentDir.length === 0"
        class="text-sm text-muted"
      >
        Empty directory
      </div>

      <ModalGeneric
        v-model:open="showMkdirModal"
        data-id="modal-storage-mkdir"
        title="Create directory"
        :primary-action-props="{
          label: 'Create',
          loading: loading.mkdir,
          disabled: mkdirNameModel.trim() === '',
          onClick: mkdir
        }"
        :secondary-action-props="{
          label: 'Cancel',
          variant: 'ghost',
          disabled: loading.mkdir,
          onClick: () => { showMkdirModal = false; }
        }"
      >
        <template #body>
          <UInput
            v-model="mkdirNameModel"
            name="mkdir"
            size="xl"
            variant="soft"
            :ui="{ base: 'ring-1 ring-glass' }"
            :disabled="loading.mkdir"
            @keyup.enter="mkdir"
          />
        </template>
      </ModalGeneric>

      <ModalGeneric
        v-model:open="showUploadModal"
        data-id="modal-storage-upload"
        title="Upload files"
        wide
        :primary-action-props="{
          class: allFilesUploaded ? 'hidden' : '',
          label: 'Upload',
          loading: loading.write,
          disabled: isAnythingLoading || !filesModel || !filesModel.length,
          onClick: uploadFiles
        }"
        :secondary-action-props="{
          label: allFilesUploaded ? 'Close' : 'Cancel',
          variant: 'ghost',
          disabled: loading.write,
          onClick: () => { showUploadModal = false; }
        }"
      >
        <template #body>
          <UFileUpload
            v-model="filesModel"
            multiple
            data-id="modal-update-firmware-file-upload"
            layout="list"
            class="w-full rounded-xl"
            description="Drag and drop to upload"
            :disabled="loading.write"
            :ui="{
              base: 'cursor-pointer',
              icon: 'size-6',
              label: 'text-lg',
              description: 'text-sm',
              files: 'max-h-[175px] overflow-y-auto',
              file: 'bg-neutral-200 dark:bg-neutral-900'
            }"
            @update:model-value="removeUpdatedFiles"
          >
            <template #actions>
              <UButton
                :label="filesModel?.length ? 'Add more files' : 'Select files'"
                color="neutral"
                variant="soft"
                :ui="{ base: 'bg-neutral-200/50 dark:bg-neutral-700/50' }"
                class="mt-2"
                :disabled="loading.write"
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
                  label="Clear all"
                  color="neutral"
                  variant="ghost"
                  class="-my-2"
                  :disabled="loading.write"
                  @click="filesModel = null"
                />
              </div>
            </template>

            <template #files>
              <div class="max-h-[175px] flex flex-col gap-1.5 overflow-y-auto">
                <div
                  v-for="(file, index) in filesModel"
                  :key="index"
                  class="min-h-8 flex justify-between gap-2 items-center p-2 rounded bg-neutral-200 dark:bg-neutral-900"
                >
                  <div class="flex items-center gap-2">
                    <UIcon
                      class="size-5 mx-1"
                      name="i-bi-file"
                    />
                    <div>{{ file.name }}</div>
                  </div>

                  <div class="flex items-center gap-2">
                    <div
                      v-if="filesToUpload.find(f => f.blob.name === file.name)?.status === 'uploading'"
                      class="flex items-center gap-2 text-sm"
                    >
                      <UIcon
                        class="size-4 animate-spin"
                        name="i-busy-loader"
                      />
                      Uploading
                    </div>
                    <div
                      v-if="filesToUpload.find(f => f.blob.name === file.name)?.status === 'success'"
                      class="flex items-center gap-2 text-sm text-green-500"
                    >
                      <UIcon
                        class="size-4"
                        name="i-bi-checkmark"
                      />
                      Uploaded
                    </div>
                    <div
                      v-if="filesToUpload.find(f => f.blob.name === file.name)?.status === 'error'"
                      class="flex items-center gap-2 text-sm text-red-500"
                    >
                      <UIcon
                        class="size-4 relative top-[1px]"
                        name="i-bi-alert"
                      />
                      Failed
                    </div>

                    <UButton
                      v-if="!loading.write"
                      icon="i-ri-close-line"
                      variant="ghost"
                      color="neutral"
                      square
                      @click="filesModel = [...filesModel!.slice(0, index), ...filesModel!.slice(index + 1)]"
                    />
                  </div>
                </div>
              </div>
            </template>
          </UFileUpload>
        </template>
      </ModalGeneric>

      <ModalGeneric
        v-model:open="showDeleteModal"
        data-id="modal-storage-delete"
        :title="`Delete ${itemToDelete?.name}?`"
        description="This action is irreversible"
        :primary-action-props="{
          label: 'Delete',
          color: 'error',
          loading: loading.remove,
          disabled: isAnythingLoading || !itemToDelete,
          onClick: remove
        }"
        :secondary-action-props="{
          label: 'Cancel',
          variant: 'ghost',
          disabled: loading.remove,
          onClick: () => { showDeleteModal = false; }
        }"
      />
    </div>
  </SectionCard>
</template>

<script setup lang="ts">
import type { StorageListElement } from '@busy-app/busy-lib';

const deviceStore = useDeviceStore();

const loading = ref({
  list: false,
  read: false,
  remove: false,
  write: false,
  mkdir: false
});
const isAnythingLoading = computed(() => !!Object.values(loading.value).some(e => !!e));

const currentPath = ref('/ext');
const currentDir = ref<StorageListElement[]>([]);

async function list (path: string) {
  loading.value.list = true;
  currentDir.value = await deviceStore.busyBar.readDirectory({ path })
    .then(result => {
      if (!result.list) {
        throw new Error('Empty response');
      }

      updatePath(path);

      const files = result.list.filter(e => e.type === 'file').sort((a, b) => b.name < a.name ? 1 : -1);
      const dirs = result.list.filter(e => e.type === 'dir').sort((a, b) => b.name < a.name ? 1 : -1);
      return [...dirs, ...files];
    })
    .catch(async error => {
      await handleHTTPError(error, `Couldn't list directory ${currentPath.value}`);
      return [];
    })
    .finally(() => loading.value.list = false);
}

const pathEl = ref<HTMLElement | null>(null);
function updatePath (newValue: string) {
  currentPath.value = newValue;
  setTimeout(() => {
    if (pathEl.value && pathEl.value.scrollWidth > pathEl.value.clientWidth) {
      pathEl.value.scroll(pathEl.value.scrollWidth, 0);
    }
  }, 50);
}

async function toParentDir () {
  const newPath = currentPath.value.slice(0, currentPath.value.lastIndexOf('/'));
  await list(newPath);
}

async function read (fileName: string) {
  loading.value.read = true;

  const path = `${currentPath.value}/${fileName}`;
  const file = await deviceStore.busyBar.downloadFile({ path })
    .catch(async error => {
      await handleHTTPError(error, `Couldn't read file ${path}`, false, 0);
      return null;
    });

  if (file !== null) {
    const url = createObjectUrl(file as Blob);
    const a = document.createElement('a');
    a.href = url;
    a.download = fileName;
    document.body.appendChild(a);
    a.click();
    document.body.removeChild(a);
    URL.revokeObjectURL(url);
  }

  loading.value.read = false;
}

function createObjectUrl (file: File | Blob): string {
  return URL.createObjectURL(file);
}

const showDeleteModal = ref(false);
const itemToDelete = ref<StorageListElement & { fullPath: string } | null>(null);
async function remove () {
  if (!itemToDelete.value?.fullPath) {
    toast.add({
      id: 'storage-delete-error',
      title: 'Couldn\'t delete: resource not provided',
      icon: 'i-bi-alert',
      color: 'error',
      duration: 5000
    });
    return;
  }

  const fullPath = itemToDelete.value.fullPath;

  loading.value.remove = true;

  await deviceStore.busyBar.removeResource({ path: fullPath })
    .then(() => {
      itemToDelete.value = null;
      showDeleteModal.value = false;
    })
    .catch(async error => {
      await handleHTTPError(error, `Couldn't delete ${fullPath}`, false, 0);
    })
    .finally(() => loading.value.remove = false);

  return list(currentPath.value);
}

const showUploadModal = ref(false);
const filesModel = ref<File[] | null>(null);
const filesToUpload = ref<{ blob: File; status: 'idle' | 'uploading' | 'success' | 'error' }[]>([]);
const allFilesUploaded = computed(() => filesModel.value
  && filesModel.value.length > 0
  && filesToUpload.value.length === filesModel.value.length
  && filesToUpload.value.filter(f => f.status !== 'success').length === 0);
function prepareFiles () {
  if (!filesModel.value) {
    return;
  }

  for (const file of filesModel.value) {
    filesToUpload.value.push({ blob: file, status: 'idle' });
  }
}
function removeUpdatedFiles () {
  if (filesModel.value && filesModel.value.length) {
    filesModel.value = filesModel.value.filter(f => !(filesToUpload.value.find(fu => fu.blob.name === f.name)?.status === 'success'));
    filesToUpload.value = filesToUpload.value.filter(fu => fu.status !== 'success');
  }
}

async function uploadFiles () {
  if (!filesModel.value) {
    showUploadModal.value = false;
    toast.add({
      id: 'storage-write-error',
      title: 'Couldn\'t upload: files not provided',
      icon: 'i-bi-alert',
      color: 'error',
      duration: 5000
    });
    return;
  }

  loading.value.write = true;

  prepareFiles();
  const _currentPath = currentPath.value;

  for (const file of filesToUpload.value) {
    try {
      file.status = 'uploading';
      await deviceStore.busyBar.uploadFile({
        path: `${_currentPath}/${file.blob.name}`,
        file: file.blob
      });
      file.status = 'success';
    } catch (error) {
      file.status = 'error';
      await handleHTTPError(error, `Couldn't upload ${file.blob.name}`, false, 0);
    }
  }

  await list(currentPath.value);

  loading.value.write = false;
}

const showMkdirModal = ref(false);
const mkdirNameModel = ref('');

async function mkdir () {
  if (!mkdirNameModel.value.trim().length) {
    toast.add({
      id: 'storage-mkdir-error',
      title: 'Couldn\'t create directory: name not provided',
      icon: 'i-bi-alert',
      color: 'error',
      duration: 5000
    });
    return;
  }

  loading.value.mkdir = true;

  const fullPath = `${currentPath.value}/${mkdirNameModel.value}`;
  await deviceStore.busyBar.createDirectory({ path: fullPath })
    .then(() => {
      showMkdirModal.value = false;
    })
    .catch(async error => {
      await handleHTTPError(error, `Couldn't create ${fullPath}`, false, 0);
    })
    .finally(() => loading.value.mkdir = false);

  await list(currentPath.value);
}

onMounted(async () => {
  await list(currentPath.value);
});
</script>
