<template>
  <SectionCard
    data-id="files-section-primary"
    :class="isDragging ? 'border-dashed border-1 border-muted' : ''"
    @dragenter.prevent="onDragEnter"
    @dragleave.prevent="onDragLeave"
    @dragover.prevent
    @drop.prevent="onDrop"
    @mouseup="stopSelection"
  >
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

      <div class="flex-1 min-w-0 relative">
        <UInputMenu
          ref="filesInputMenu"
          v-model="inputMenuModel"
          v-model:search-term="inputMenuSearchTerm"
          :open-on-focus="true"
          :items="inputMenuItems"
          label-key="path"
          trailing-icon=""
          selected-icon=""
          class="w-full"
          @update:search-term="onInputMenuChange"
          @update:model-value="onInputMenuUpdate"
          @keydown.enter.prevent="onInputMenuEnter"
          @keydown.tab.prevent
          @keyup.tab.prevent="onInputMenuTab"
        >
          <template #leading><span class="text-sm">/ext</span></template>
          <template #item="{ item }">
            <div class="flex items-center gap-1.5">
              <UIcon
                v-if="item.icon"
                :name="item.icon"
                class="size-5"
              />
              <span
                :data-id="`files-autocomplete-${item.path}`"
                class="truncate"
              >{{ item.name }}</span>
            </div>
          </template>
          <template #trailing>
            <UKbd>/</UKbd>
          </template>
        </UInputMenu>
        <!-- <div class="absolute text-sm top-1.5 left-2.5 text-red-500">
          <span>/</span><span class="cursor-pointer hover:underline">ext</span>
          <template
            v-for="part in inputMenuSearchTerm.slice(0, inputMenuSearchTerm.lastIndexOf('/')).split('/').filter(p => p.length > 0)"
            :key="part"
          >
            <span>/</span><span class="cursor-pointer hover:underline">{{ part }}</span>
          </template>
        </div> -->
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

    <div
      ref="fileListContainer"
      class="w-full flex flex-col"
    >
      <UContextMenu
        v-for="(item, index) in currentDir"
        :key="`${item.name}_${item.type}`"
        :items="getContextMenuItems(item)"
      >
        <div
          class="fm-item relative min-h-10 grid grid-cols-[1fr_auto] items-center py-1 px-2 gap-2 rounded break-all select-none"
          :class="[
            item.type === 'dir' ? 'cursor-pointer' : '',
            getItemClass(item, index)
          ]"
          @click="onClickItem(item, $event)"
          @mousedown="onMouseDown(item, index, $event)"
          @mouseenter="onMouseEnter(index)"
        >
          <div
            class="grid grid-cols-[24px_1fr] gap-4 items-center"
          >
            <UIcon
              class="size-5"
              :name="item.type === 'dir' ? 'i-bi-folder' : 'i-bi-file'"
            />

            <div>{{ item.name }}</div>
          </div>

          <div class="flex items-center justify-end gap-2">
            <template v-if="item.type === 'file'">
              <span class="text-sm text-muted">{{ bytesToSize((item as StorageFileElement).size) }}</span>
            </template>
          </div>
        </div>
      </UContextMenu>

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
        :title="itemToDelete ? `Delete ${itemToDelete.name}?` : `Delete ${selectedItems.size} items?`"
        description="This action is irreversible"
        :primary-action-props="{
          label: 'Delete',
          color: 'error',
          loading: loading.remove,
          disabled: isAnythingLoading || (!itemToDelete && selectedItems.size === 0),
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
import type { StorageListElement, StorageFileElement } from '@busy-app/busy-lib';
import type { ContextMenuItem } from '@nuxt/ui';

const deviceStore = useDeviceStore();

const loading = ref({
  list: false,
  read: false,
  remove: false,
  write: false,
  mkdir: false,
  suggestions: false
});
const isAnythingLoading = computed(() => !!Object.values(loading.value).some(e => !!e));

const currentPath = ref('/ext');
const currentDir = ref<StorageListElement[]>([]);

const filesInputMenu = useTemplateRef('filesInputMenu');
defineShortcuts({
  '/': () => {
    filesInputMenu.value?.inputRef?.$el?.focus();
  }
});
const inputMenuModel = ref({ path: '', name: '' });
const inputMenuSearchTerm = ref('');
interface InputMenuItem {
  path: string;
  name: string;
  icon?: string;
}
const inputMenuItems = ref<InputMenuItem[]>([]);

const rootItem: InputMenuItem = { path: '/', name: '/ext', icon: 'i-bi-busy-bar' };

async function onInputMenuChange (newValue: string) {
  if (!newValue.length) {
    return;
    /* if (didInputResetAfterSelect.value) {
      newValue = '/';
    } else {
      console.debug('empty input, resetting to:', inputMenuModel.value.path);
      inputMenuSearchTerm.value = inputMenuModel.value.path;
      didInputResetAfterSelect.value = true;
      return;
    } */
  }
  if (!newValue.startsWith('/')) {
    inputMenuSearchTerm.value = `/${newValue}`;
    newValue = `/${newValue}`;
  }

  const dir = newValue.slice(0, newValue.lastIndexOf('/') + 1);
  let filter = newValue.slice(newValue.lastIndexOf('/') + 1);
  if (filter === '/') {
    filter = '';
  }

  const options = await deviceStore.busyBar.readDirectory({ path: `/ext${dir}` })
    .then(result => {
      if (!result.list) {
        throw new Error('Empty response');
      }
      const dirs = result.list.filter(e => e.type === 'dir').sort((a, b) => b.name < a.name ? 1 : -1);
      return dirs
        .map(d => ({ path: `${dir}${d.name}`, name: d.name }))
        .filter(d => d.name.toLowerCase().startsWith(filter.toLowerCase()));
    })
    .catch(async error => {
      await handleHTTPError(error, `Couldn't list directory ${currentPath.value}`);
      return [];
    }) as InputMenuItem[];

  if (dir === '/' && filter === '') {
    options.unshift(rootItem);
  }

  inputMenuItems.value = options;
}

async function onInputMenuUpdate (model: InputMenuItem | null) {
  if (model) {
    await list(`/ext${model.path}`);
    if (!filesInputMenu?.value?.inputRef) {
      return;
    }
    filesInputMenu.value.inputRef.$el.value = model.path;
  }
}
async function onInputMenuEnter () {
  const path = inputMenuSearchTerm.value.startsWith('/') ? inputMenuSearchTerm.value : `/${inputMenuSearchTerm.value}`;
  if (currentPath.value !== `/ext${path}`) {
    await list(`/ext${path}`);
    if (!filesInputMenu?.value?.inputRef) {
      return;
    }
    filesInputMenu.value.inputRef.$el.value = path;
  }
}
async function onInputMenuTab () {
  const option = inputMenuItems.value.find(i => i.path.startsWith(inputMenuSearchTerm.value));
  if (option) {
    if (!filesInputMenu?.value?.inputRef) {
      return;
    }
    filesInputMenu.value.inputRef.$el.value = option.path;
  }
}

const selectedItems = ref<Set<string>>(new Set());
const isSelecting = ref(false);
const selectStartIndex = ref(-1);

async function list (path: string) {
  selectedItems.value.clear();
  loading.value.list = true;
  await deviceStore.busyBar.readDirectory({ path })
    .then(result => {
      if (!result.list) {
        throw new Error('Empty response');
      }
      const files = result.list.filter(e => e.type === 'file').sort((a, b) => b.name < a.name ? 1 : -1);
      const dirs = result.list.filter(e => e.type === 'dir').sort((a, b) => b.name < a.name ? 1 : -1);
      currentDir.value = [...dirs, ...files];
    })
    .then(() => {
      updatePath(path);
    })
    .catch(async error => {
      await handleHTTPError(error, `Couldn't list directory ${currentPath.value}`);
      return [];
    })
    .finally(() => loading.value.list = false);
}

function updatePath (newValue: string) {
  currentPath.value = newValue;

  let slicedPath = newValue.startsWith('/ext') ? newValue.slice(4) : newValue;
  slicedPath = slicedPath.replaceAll(/^\/+|\/+$/g, '/');
  inputMenuModel.value = { path: slicedPath, name: slicedPath.slice(slicedPath.lastIndexOf('/') + 1) };
  inputMenuSearchTerm.value = slicedPath;
  inputMenuItems.value = currentDir.value
    .filter(e => e.type === 'dir')
    .map(d => ({ path: `${slicedPath.slice(0, slicedPath.lastIndexOf('/') + 1)}${d.name}`, name: d.name }));

  if (slicedPath === '' || slicedPath === '/') {
    inputMenuItems.value.unshift(rootItem);
  }
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

const wasDraggingSelection = ref(false);
const selectionMode = ref<'replace' | 'add'>('replace');
const initialSelection = ref<Set<string>>(new Set());

function onMouseDown (item: StorageListElement, index: number, event: MouseEvent) {
  if (event.button === 0) {
    if (event.shiftKey && selectStartIndex.value !== -1) {
      selectionMode.value = 'replace';
      isSelecting.value = true;
      wasDraggingSelection.value = false;
      initialSelection.value = new Set();

      const start = selectStartIndex.value;
      const end = index;
      const min = Math.min(start, end);
      const max = Math.max(start, end);

      selectedItems.value.clear();
      for (let i = min; i <= max; i++) {
        const it = currentDir.value[i];
        if (it) {
          selectedItems.value.add(it.name);
        }
      }
    } else if (event.ctrlKey || event.metaKey) {
      selectionMode.value = 'add';
      initialSelection.value = new Set(selectedItems.value);
      isSelecting.value = true;
      wasDraggingSelection.value = false;
      selectStartIndex.value = index;

      if (selectedItems.value.has(item.name)) {
        selectedItems.value.delete(item.name);
      } else {
        selectedItems.value.add(item.name);
      }
    } else {
      selectionMode.value = 'replace';
      isSelecting.value = true;
      wasDraggingSelection.value = false;
      selectStartIndex.value = index;
      initialSelection.value = new Set();
    }
  } else if (event.button === 2) {
    if (!selectedItems.value.has(item.name)) {
      selectedItems.value.clear();
      selectedItems.value.add(item.name);
    }
  }
}

function onMouseEnter (index: number) {
  if (isSelecting.value) {
    wasDraggingSelection.value = true;
    const start = selectStartIndex.value;
    const end = index;

    const min = Math.min(start, end);
    const max = Math.max(start, end);

    if (selectionMode.value === 'add') {
      selectedItems.value = new Set(initialSelection.value);
      for (let i = min; i <= max; i++) {
        const it = currentDir.value[i];
        if (it) {
          selectedItems.value.add(it.name);
        }
      }
    } else {
      selectedItems.value.clear();
      for (let i = min; i <= max; i++) {
        const it = currentDir.value[i];
        if (it) {
          selectedItems.value.add(it.name);
        }
      }
    }
  }
}

function stopSelection () {
  isSelecting.value = false;
}

function onClickItem (item: StorageListElement, event: MouseEvent) {
  if (wasDraggingSelection.value || event.shiftKey) {
    return;
  }

  if (event.ctrlKey || event.metaKey) {
    return;
  }

  selectedItems.value.clear();
  if (item.type === 'dir') {
    list(`${currentPath.value}/${item.name}`);
  }
}

function getItemClass (item: StorageListElement, index: number) {
  if (!selectedItems.value.has(item.name)) {
    return '';
  }

  const isPrevSelected = index > 0 && selectedItems.value.has(currentDir.value[index - 1].name);
  const isNextSelected = index < currentDir.value.length - 1 && selectedItems.value.has(currentDir.value[index + 1].name);

  let classes = 'bg-neutral-200 dark:bg-neutral-700';

  if (isPrevSelected && isNextSelected) {
    classes += ' rounded-none';
  } else if (isPrevSelected) {
    classes += ' rounded-b-md rounded-t-none';
  } else if (isNextSelected) {
    classes += ' rounded-t-md rounded-b-none';
  } else {
    classes += ' rounded-md';
  }

  return classes;
}

function getContextMenuItems (item: StorageListElement): ContextMenuItem[] {
  const isSelected = selectedItems.value.has(item.name);
  if (selectedItems.value.size > 1 && isSelected) {
    const allFiles = Array.from(selectedItems.value).every(name => {
      const el = currentDir.value.find(e => e.name === name);
      return el && el.type === 'file';
    });

    const items = [];
    const itemString = allFiles ? 'files' : 'items';
    if (allFiles) {
      items.push({
        label: `Download ${selectedItems.value.size} files`,
        icon: 'i-bi-download',
        onClick: batchDownload
      });
    }

    items.push({
      label: `Delete ${selectedItems.value.size} ${itemString}`,
      icon: 'i-bi-trash',
      color: 'error',
      onClick: confirmBatchDelete
    });

    return items as ContextMenuItem[];
  }

  return [
    {
      label: item.type === 'dir' ? 'Open directory' : 'Download file',
      icon: item.type === 'dir' ? 'i-ri-arrow-right-line' : 'i-bi-download',
      onClick: () => item.type === 'dir' ? list(`${currentPath.value}/${item.name}`) : read(item.name)
    },
    {
      label: 'Delete',
      icon: 'i-bi-trash',
      color: 'error',
      onClick: () => {
        itemToDelete.value = { ...item, fullPath: `${currentPath.value}/${item.name}` };
        showDeleteModal.value = true;
      }
    }
  ];
}

async function batchDownload () {
  const items = new Set(selectedItems.value);
  for (const name of items) {
    const item = currentDir.value.find(e => e.name === name);
    if (item && item.type === 'file') {
      await read(item.name);
    }
  }
}

function confirmBatchDelete () {
  itemToDelete.value = null; // Clear single
  showDeleteModal.value = true;
}

const showDeleteModal = ref(false);
const itemToDelete = ref<StorageListElement & { fullPath: string } | null>(null);
async function remove () {
  if (selectedItems.value.size > 0 && !itemToDelete.value) {
    loading.value.remove = true;
    const items = new Set(selectedItems.value);
    for (const name of items) {
      const fullPath = `${currentPath.value}/${name}`;
      await deviceStore.busyBar.removeResource({ path: fullPath })
        .catch(async error => {
          await handleHTTPError(error, `Couldn't delete ${fullPath}`, false, 0);
        });
    }
    loading.value.remove = false;
    showDeleteModal.value = false;
    selectedItems.value.clear();
    return list(currentPath.value);
  }

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

const isDragging = ref(false);
const dragCounter = ref(0);

function onDragEnter () {
  dragCounter.value++;
  isDragging.value = true;
}

function onDragLeave () {
  dragCounter.value--;
  if (dragCounter.value <= 0) {
    isDragging.value = false;
    dragCounter.value = 0;
  }
}

function onDrop (e: DragEvent) {
  isDragging.value = false;
  dragCounter.value = 0;
  if (e.dataTransfer?.files) {
    const files = Array.from(e.dataTransfer.files);
    if (files.length) {
      filesModel.value = files;
      filesToUpload.value = [];
      showUploadModal.value = true;
    }
  }
}

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

const fileListContainer = ref<HTMLElement | null>(null);

function onGlobalClick (event: MouseEvent) {
  if (selectedItems.value.size === 0) {
    return;
  }

  const target = event.target as HTMLElement;
  // Don't clear if clicking inside the list
  if (fileListContainer.value && fileListContainer.value.contains(target)) {
    return;
  }
  // Don't clear if clicking context menu items (usually in a separate portal with role="menu")
  if (target.closest('[role="menu"]')) {
    return;
  }
  // Don't clear if a modal is open (e.g. delete confirmation)
  if (showDeleteModal.value || showMkdirModal.value || showUploadModal.value) {
    return;
  }

  selectedItems.value.clear();
}

async function init () {
  await list(currentPath.value);
}

onMounted(async () => {
  await init();
  window.addEventListener('device-reconnected', init);
  window.addEventListener('click', onGlobalClick);
});
onBeforeUnmount(() => {
  window.removeEventListener('device-reconnected', init);
  window.removeEventListener('click', onGlobalClick);
});
</script>

<style scoped>
.fm-item:hover:after {
  content: '';
  display: block;
  position: absolute;
  top: 0;
  left: 0;
  width: 100%;
  height: 100%;
  border-radius: 3px;
  backdrop-filter: brightness(1.5);
}
</style>
