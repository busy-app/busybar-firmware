<template>
  <SectionCard
    data-id="files-section-primary"
    class="relative overflow-visible"
    @mouseup="stopSelection"
  >
    <div class="w-full grid grid-cols-[32px_1fr_auto] gap-2 items-center pr-1">
      <div>
        <UTooltip
          :delay-duration="0"
          text="To parent directory"
        >
          <UButton
            icon="i-bi-arrow-back"
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
        <UInput
          ref="filesInputMenu"
          v-model="inputMenuModel"
          class="fim"
          :ui="{
            base: 'rounded-md pl-15.5'
          }"
          @update:model-value="onInputMenuModelUpdate"
          @keydown.enter.prevent="onEnter"
          @keyup.tab.prevent
          @keydown.tab.prevent="onTab"
          @keydown.arrow-down.prevent="onArrowDown"
          @keydown.arrow-up.prevent="onArrowUp"
        >
          <div
            v-if="inputMenuModel.length > 0 && inputMenuItems.length > 0"
            class="fim-focus-visible
              w-full absolute top-8 flex flex-col gap-0.5 p-1.5
              rounded-md text-sm bg-default ring ring-inset ring-accented z-30"
          >
            <div
              v-for="(item, index) in inputMenuItems"
              :key="item.path"
              class="flex items-center gap-1.5 p-1.5 px-2 rounded-md cursor-pointer hover:bg-elevated/50"
              :class="highlightedItemIndex === index ? 'bg-elevated/85' : ''"
              @mousedown="inputMenuList(item)"
            >
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
            <!-- <div
              v-if="inputMenuItems.length === 0"
              class="flex items-center gap-1.5 p-1 rounded"
            >
              <span class="truncate">Nothing found</span>
            </div> -->
          </div>

          <template #leading>
            <div class="flex items-center gap-1.5 text-sm">
              <UIcon
                name="i-bi-busy-bar"
                class="size-5"
              />
              <span>/ext</span>
            </div>
          </template>
          <template #trailing>
            <UKbd class="fim-focus-hidden">/</UKbd>
          </template>
        </UInput>

        <div class="absolute text-sm top-1.5 left-9">
          <span>/</span><span
            class="cursor-pointer hover:underline mr-[0.5px]"
            @click="list('/ext')"
          >ext</span>
          <template
            v-for="part in inputMenuModel.slice(0, inputMenuModel.lastIndexOf('/')).split('/').filter(p => p.length > 0)"
            :key="part"
          >
            <span>/</span><span
              class="cursor-pointer hover:underline"
              @click="list(`/ext/${inputMenuModel.slice(0, inputMenuModel.indexOf(part) + part.length)}`)"
            >{{ part }}</span>
          </template>
        </div>
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
            @click="filesModel = null; filesToUpload = []; uploadDirectory = null; showUploadModal = true;"
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
            @click="() => { list(currentPath, { syncUrl: false }); }"
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
          :data-drop-dir="item.type === 'dir' ? item.name : undefined"
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

          <div
            v-if="dropTargetDir === item.name && item.type === 'dir'"
            class="absolute inset-0 z-10 flex items-center justify-center gap-2 px-2
              rounded-md border-2 border-dashed border-accented bg-elevated text-sm pointer-events-none"
          >
            <UIcon
              class="size-4 shrink-0"
              name="i-bi-upload"
            />
            <span class="truncate">Drop to upload to {{ item.name }}</span>
          </div>
        </div>
      </UContextMenu>

      <div
        v-if="currentDir.length === 0"
        class="text-sm text-muted"
      >
        Empty directory
      </div>

      <div
        v-if="isDragging && isDragOverCard && !showUploadModal && !dropTargetDir"
        data-id="files-drop-overlay"
        class="absolute inset-0 z-40 flex flex-col items-center justify-center gap-3
          rounded-3xl border-2 border-dashed border-accented bg-default/75 pointer-events-none"
      >
        <UIcon
          class="size-10"
          name="i-bi-upload"
        />
        <p class="text-xl">Drop files to upload</p>
        <p class="text-muted break-all px-4 text-center">to {{ currentPath }}</p>
      </div>

      <ModalGeneric
        v-model:open="showMkdirModal"
        data-id="modal-storage-mkdir"
        title="Create directory"
        :dismissible="!loading.mkdir"
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
            :ui="{ base: 'ring-1 ring-glass bg-accented/50' }"
            :disabled="loading.mkdir"
            @keyup.enter="mkdir"
          />
        </template>
      </ModalGeneric>

      <ModalGeneric
        v-model:open="showUploadModal"
        data-id="modal-storage-upload"
        :title="uploadModalTitle"
        wide
        :dismissible="!loading.write"
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
            :dropzone="false"
            :disabled="loading.write"
            :ui="{
              base: ['cursor-pointer border-dashed focus-visible:-outline-offset-2', isDragging ? 'bg-elevated/25' : ''],
              icon: 'size-6',
              label: 'text-lg',
              description: 'text-sm',
              files: 'max-h-[175px] overflow-y-auto'
            }"
            @update:model-value="removeUpdatedFiles"
          >
            <template #actions>
              <UButton
                :label="filesModel?.length ? 'Add more files' : 'Select files'"
                color="neutral"
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
                  icon="i-bi-cross"
                  label="Clear all"
                  color="neutral"
                  variant="ghost"
                  class="-my-2"
                  :disabled="loading.write"
                  @click="() => { filesModel = null; }"
                />
              </div>
            </template>

            <template #files>
              <div class="max-h-[175px] flex flex-col gap-1.5 overflow-y-auto">
                <div
                  v-for="(file, index) in filesModel"
                  :key="index"
                  class="min-h-8 flex justify-between gap-2 items-center p-2 rounded"
                >
                  <div class="flex items-center gap-2">
                    <UIcon
                      class="size-5 mx-1"
                      name="i-bi-file"
                    />
                    <div>{{ getUploadPath(file) }}</div>
                  </div>

                  <div class="flex items-center gap-2">
                    <div
                      v-if="filesToUpload.find(f => f.blob === file)?.status === 'uploading'"
                      class="flex items-center gap-2 text-sm"
                    >
                      <UIcon
                        class="size-4 animate-spin"
                        name="i-busy-loader"
                      />
                      Uploading
                    </div>
                    <div
                      v-if="filesToUpload.find(f => f.blob === file)?.status === 'success'"
                      class="flex items-center gap-2 text-sm text-green-500"
                    >
                      <UIcon
                        class="size-4"
                        name="i-bi-checkmark"
                      />
                      Uploaded
                    </div>
                    <div
                      v-if="filesToUpload.find(f => f.blob === file)?.status === 'error'"
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
                      icon="i-bi-cross"
                      variant="ghost"
                      color="neutral"
                      square
                      @click="() => { filesModel = [...filesModel!.slice(0, index), ...filesModel!.slice(index + 1)]; }"
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
        :dismissible="!loading.remove"
        description="This action is irreversible"
        :primary-action-props="{
          label: 'Delete',
          color: 'error',
          loading: loading.remove,
          disabled: isAnythingLoading || (!itemToDelete && selectedItems.size === 0),
          onClick: () => { remove(); }
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
import type { LocationQueryValue } from 'vue-router';

interface InputMenuItem {
  path: string;
  name: string;
  icon?: string;
}

interface UploadFile {
  blob: File;
  status: 'idle' | 'uploading' | 'success' | 'error';
}

interface ListOptions {
  syncUrl?: boolean;
  replaceUrl?: boolean;
  errorNotificationTimeout?: number;
}

const fileRelativePaths = new WeakMap<File, string>();

const deviceStore = useDeviceStore();
const route = useRoute();
const router = useRouter();

const filesInputMenu = useTemplateRef('filesInputMenu');

defineShortcuts({
  '/': () => {
    filesInputMenu.value?.inputRef?.focus();
  }
});

const loading = ref({
  list: false,
  read: false,
  remove: false,
  write: false,
  mkdir: false,
  suggestions: false
});
const currentPath = ref('/ext');

const currentDir = ref<StorageListElement[]>([]);

const inputMenuModel = ref('');
const inputMenuItems = ref<InputMenuItem[]>([]);
const highlightedItemIndex = ref(0);

const selectedItems = ref<Set<string>>(new Set());
const isSelecting = ref(false);
const selectStartIndex = ref(-1);

const wasDraggingSelection = ref(false);
const selectionMode = ref<'replace' | 'add'>('replace');
const initialSelection = ref<Set<string>>(new Set());

const showMkdirModal = ref(false);
const mkdirNameModel = ref('');

const isDragging = ref(false);
const dragCounter = ref(0);
const isDragOverCard = ref(false);
const dropTargetDir = ref<string | null>(null);
const uploadDirectory = ref<string | null>(null);

const fileListContainer = ref<HTMLElement | null>(null);

const showUploadModal = ref(false);
const filesModel = ref<File[] | null>(null);
const filesToUpload = ref<UploadFile[]>([]);

const showDeleteModal = ref(false);
const itemToDelete = ref<StorageListElement & { fullPath: string } | null>(null);

const isAnythingLoading = computed(() => !!Object.values(loading.value).some(e => !!e));

const allFilesUploaded = computed(() => filesModel.value
  && filesModel.value.length > 0
  && filesToUpload.value.length === filesModel.value.length
  && filesToUpload.value.filter(f => f.status !== 'success').length === 0);

const uploadModalTitle = computed(() => (uploadDirectory.value
  ? `Upload files to ${uploadDirectory.value.slice(uploadDirectory.value.lastIndexOf('/') + 1)}`
  : 'Upload files'));

function getQueryPath (queryPath: LocationQueryValue | LocationQueryValue[] | undefined): string | null {
  if (Array.isArray(queryPath)) {
    return queryPath[0] || null;
  }

  return typeof queryPath === 'string' && queryPath.length > 0 ? queryPath : null;
}

function resolveStoragePath (path: string | null | undefined): string {
  const trimmedPath = path?.trim() || '';
  if (trimmedPath === '') {
    return '/ext';
  }

  let normalizedPath = trimmedPath;
  if (!normalizedPath.startsWith('/ext')) {
    normalizedPath = normalizedPath.startsWith('/') ? `/ext${normalizedPath}` : `/ext/${normalizedPath}`;
  }

  normalizedPath = normalizedPath.replace(/\/+$/, '');
  return normalizedPath.length > 0 ? normalizedPath : '/ext';
}

function normalizePath (path: string): string {
  if (path.startsWith('/ext')) {
    path = path.slice(4);
  }
  if (path.length > 1 && !path.startsWith('/')) {
    path = `/${path}`;
  }
  return path.replaceAll(/^\/+|\/+$/g, '/');
}

async function onInputMenuModelUpdate (newValue: string) {
  inputMenuModel.value = normalizePath(newValue);

  const dir = newValue.slice(0, newValue.lastIndexOf('/') + 1);
  let filter = newValue.slice(newValue.lastIndexOf('/') + 1);
  if (filter === '/') {
    filter = '';
  }

  const options = await deviceStore.busyBar.StorageListGet({ path: `/ext${dir}` })
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
      await handleHTTPError(error, `Couldn't list directory /ext${dir}`);
      return [];
    }) as InputMenuItem[];

  inputMenuItems.value = options;
}

async function inputMenuList (model: InputMenuItem | null) {
  await list(`/ext${normalizePath(model?.path || inputMenuModel.value)}`);
}

function onArrowUp () {
  highlightedItemIndex.value = (highlightedItemIndex.value - 1 + inputMenuItems.value.length) % inputMenuItems.value.length;
}

function onArrowDown () {
  highlightedItemIndex.value = (highlightedItemIndex.value + 1) % inputMenuItems.value.length;
}

async function onTab () {
  if (inputMenuItems.value.length === 0) {
    return;
  } else if (inputMenuItems.value.length === 1) {
    await inputMenuList(inputMenuItems.value[0]);
  }
  return onArrowDown();
}

async function onEnter () {
  if (inputMenuItems.value.length === 0) {
    return;
  } else {
    if (inputMenuModel.value.length > 0) {
      await inputMenuList(inputMenuItems.value[highlightedItemIndex.value] || null);
    } else {
      await inputMenuList(null);
    }
  }
}

async function list (path: string, options: ListOptions = {}) {
  selectedItems.value.clear();
  loading.value.list = true;
  try {
    const result = await deviceStore.busyBar.StorageListGet({ path });
    if (!result.list) {
      throw new Error('Empty response');
    }

    const files = result.list.filter(e => e.type === 'file').sort((a, b) => b.name < a.name ? 1 : -1);
    const dirs = result.list.filter(e => e.type === 'dir').sort((a, b) => b.name < a.name ? 1 : -1);
    currentDir.value = [...dirs, ...files];
    updatePath(path, options);
    return true;
  } catch (error) {
    await handleHTTPError(error, `Couldn't list directory ${path}`, false, options.errorNotificationTimeout);
    return false;
  } finally {
    loading.value.list = false;
  }
}

function updatePath (newValue: string, options: ListOptions = {}) {
  currentPath.value = newValue;

  const slicedPath = normalizePath(newValue);

  inputMenuModel.value = slicedPath;
  inputMenuItems.value = currentDir.value
    .filter(e => e.type === 'dir')
    .map(d => ({ path: `${slicedPath}${slicedPath.endsWith('/') ? '' : '/'}${d.name}`, name: d.name }));

  if (options.syncUrl === false) {
    return;
  }

  const currentQueryPath = getQueryPath(route.query.path);
  if (currentQueryPath === newValue) {
    return;
  }

  const navigation = options.replaceUrl ? router.replace : router.push;
  void navigation({ query: { ...route.query, path: newValue } });
}

async function toParentDir () {
  const newPath = currentPath.value.slice(0, currentPath.value.lastIndexOf('/'));
  await list(newPath);
}

async function read (fileName: string) {
  loading.value.read = true;

  const path = `${currentPath.value}/${fileName}`;
  const file = await deviceStore.busyBar.StorageRead({ path }, { timeout: 0 })
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

  if (selectedItems.value.size > 1 && selectedItems.value.has(item.name)) {
    const target = event.currentTarget as HTMLElement | null;
    if (!target) {
      return;
    }

    target.dispatchEvent(new MouseEvent('contextmenu', {
      bubbles: true,
      cancelable: true,
      button: 2,
      buttons: 2,
      clientX: event.clientX,
      clientY: event.clientY,
      ctrlKey: event.ctrlKey,
      metaKey: event.metaKey,
      shiftKey: event.shiftKey,
      altKey: event.altKey
    }));
    return;
  }

  if (item.type === 'dir') {
    selectedItems.value.clear();
    list(`${currentPath.value}/${item.name}`);
    return;
  }

  if (selectedItems.value.size <= 1 || !selectedItems.value.has(item.name)) {
    selectedItems.value.clear();
    selectedItems.value.add(item.name);
  }

  const target = event.currentTarget as HTMLElement | null;
  if (!target) {
    return;
  }

  target.dispatchEvent(new MouseEvent('contextmenu', {
    bubbles: true,
    cancelable: true,
    button: 2,
    buttons: 2,
    clientX: event.clientX,
    clientY: event.clientY,
    ctrlKey: event.ctrlKey,
    metaKey: event.metaKey,
    shiftKey: event.shiftKey,
    altKey: event.altKey
  }));
}

function getItemClass (item: StorageListElement, index: number) {
  if (!selectedItems.value.has(item.name)) {
    return '';
  }

  const isPrevSelected = index > 0 && selectedItems.value.has(currentDir.value[index - 1].name);
  const isNextSelected = index < currentDir.value.length - 1 && selectedItems.value.has(currentDir.value[index + 1].name);

  let classes = 'bg-accented';

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

async function remove () {
  if (selectedItems.value.size > 0 && !itemToDelete.value) {
    loading.value.remove = true;
    const items = new Set(selectedItems.value);
    for (const name of items) {
      const fullPath = `${currentPath.value}/${name}`;
      await deviceStore.busyBar.StorageRemove({ path: fullPath }, { timeout: 0 })
        .catch(async error => {
          await handleHTTPError(error, `Couldn't delete ${fullPath}`, false, 0);
        });
    }
    loading.value.remove = false;
    showDeleteModal.value = false;
    selectedItems.value.clear();
    return list(currentPath.value, { syncUrl: false });
  }

  if (!itemToDelete.value?.fullPath) {
    toast.add({
      id: 'storage-delete-error',
      title: 'Couldn\'t delete: resource not provided',
      icon: 'i-bi-alert',
      color: 'error',
      duration: 0,
      close: true,
      closeIcon: 'i-bi-cross'
    });
    return;
  }

  const fullPath = itemToDelete.value.fullPath;

  loading.value.remove = true;

  await deviceStore.busyBar.StorageRemove({ path: fullPath }, { timeout: 0 })
    .then(() => {
      itemToDelete.value = null;
      showDeleteModal.value = false;
    })
    .catch(async error => {
      await handleHTTPError(error, `Couldn't delete ${fullPath}`, false, 0);
    })
    .finally(() => loading.value.remove = false);

  return list(currentPath.value, { syncUrl: false });
}

function prepareFiles () {
  if (!filesModel.value) {
    filesToUpload.value = [];
    return;
  }

  const prepared = filesToUpload.value;
  filesToUpload.value = filesModel.value.map<UploadFile>(file => prepared.find(f => f.blob === file)
    ?? { blob: file, status: 'idle' });
}
function removeUpdatedFiles () {
  if (filesModel.value && filesModel.value.length) {
    filesModel.value = filesModel.value.filter(f => !(filesToUpload.value.find(fu => fu.blob === f)?.status === 'success'));
    filesToUpload.value = filesToUpload.value.filter(fu => fu.status !== 'success');
  }
}

function getUploadPath (file: File) {
  return fileRelativePaths.get(file) ?? file.name;
}

async function getDroppedUploadFiles (dataTransfer: DataTransfer) {
  const droppedFiles = await getDroppedFiles(dataTransfer);

  for (const { file, relativePath } of droppedFiles) {
    fileRelativePaths.set(file, relativePath);
  }

  return droppedFiles.map(({ file }) => file);
}

async function uploadFiles () {
  if (!filesModel.value) {
    showUploadModal.value = false;
    toast.add({
      id: 'storage-write-error',
      title: 'Couldn\'t upload: files not provided',
      icon: 'i-bi-alert',
      color: 'error',
      duration: 0,
      close: true,
      closeIcon: 'i-bi-cross'
    });
    return;
  }

  loading.value.write = true;

  prepareFiles();
  const _currentPath = uploadDirectory.value ?? currentPath.value;

  for (const file of filesToUpload.value) {
    if (file.status === 'success') {
      continue;
    }

    try {
      file.status = 'uploading';
      await deviceStore.busyBar.StorageWrite({
        path: `${_currentPath}/${getUploadPath(file.blob)}`,
        file: file.blob
      }, { timeout: 0 });
      file.status = 'success';
    } catch (error) {
      file.status = 'error';
      await handleHTTPError(error, `Couldn't upload ${getUploadPath(file.blob)}`, false, 0);
    }
  }

  await list(currentPath.value, { syncUrl: false });

  loading.value.write = false;
}

function isFileDrag (e: DragEvent) {
  return !!e.dataTransfer?.types.includes('Files');
}

function resetDragState () {
  isDragging.value = false;
  dragCounter.value = 0;
  isDragOverCard.value = false;
  dropTargetDir.value = null;
}

function isOverCard (e: DragEvent) {
  const target = e.target as HTMLElement | null;
  return !!target?.closest('[data-id="files-section-primary"]');
}

function getDropTargetDir (e: DragEvent) {
  const target = e.target as HTMLElement | null;
  return target?.closest<HTMLElement>('[data-drop-dir]')?.dataset.dropDir ?? null;
}

function onWindowDragEnter (e: DragEvent) {
  if (!isFileDrag(e)) {
    return;
  }

  dragCounter.value++;
  isDragging.value = true;
}

function onWindowDragLeave (e: DragEvent) {
  if (!isFileDrag(e)) {
    return;
  }

  dragCounter.value--;
  if (dragCounter.value <= 0) {
    resetDragState();
  }
}

function onWindowDragOver (e: DragEvent) {
  if (!isFileDrag(e)) {
    return;
  }

  e.preventDefault();
  isDragOverCard.value = isOverCard(e);
  dropTargetDir.value = showUploadModal.value ? null : getDropTargetDir(e);
}

async function onWindowDrop (e: DragEvent) {
  if (!isFileDrag(e)) {
    return;
  }

  e.preventDefault();
  const isDropOnCard = isOverCard(e);
  const targetDir = getDropTargetDir(e);
  resetDragState();

  if (!e.dataTransfer) {
    return;
  }

  if (showUploadModal.value) {
    if (loading.value.write) {
      return;
    }

    const files = await getDroppedUploadFiles(e.dataTransfer);
    if (files.length) {
      filesModel.value = [...(filesModel.value || []), ...files];
      removeUpdatedFiles();
    }

    return;
  }

  if (!isDropOnCard) {
    return;
  }

  const files = await getDroppedUploadFiles(e.dataTransfer);
  if (files.length) {
    filesModel.value = files;
    filesToUpload.value = [];
    uploadDirectory.value = targetDir ? `${currentPath.value}/${targetDir}` : null;
    showUploadModal.value = true;
  }
}

async function mkdir () {
  if (!mkdirNameModel.value.trim().length) {
    toast.add({
      id: 'storage-mkdir-error',
      title: 'Couldn\'t create directory: name not provided',
      icon: 'i-bi-alert',
      color: 'error',
      duration: 0,
      close: true,
      closeIcon: 'i-bi-cross'
    });
    return;
  }

  loading.value.mkdir = true;

  const fullPath = `${currentPath.value}/${mkdirNameModel.value}`;
  await deviceStore.busyBar.StorageMkdir({ path: fullPath })
    .then(() => {
      showMkdirModal.value = false;
    })
    .catch(async error => {
      await handleHTTPError(error, `Couldn't create ${fullPath}`, false, 0);
    })
    .finally(() => loading.value.mkdir = false);

  await list(currentPath.value, { syncUrl: false });
}

async function refreshCurrentPath () {
  await list(currentPath.value, { syncUrl: false });
}

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

watch(() => route.query.path, async queryPath => {
  const rawPath = getQueryPath(queryPath);

  if (!rawPath) {
    await list('/ext', { syncUrl: false });
    return;
  }

  const requestedPath = resolveStoragePath(rawPath);

  if (requestedPath === '/ext') {
    router.replace({ query: { ...route.query, path: undefined } });
  }

  if (requestedPath === currentPath.value && currentDir.value.length > 0) {
    return;
  }

  const success = await list(requestedPath, { syncUrl: false, errorNotificationTimeout: 0 });
  if (!success && requestedPath !== '/ext') {
    await list('/ext', { syncUrl: true, replaceUrl: true });
  }
}, { immediate: true });

onMounted(() => {
  window.addEventListener('device-reconnected', refreshCurrentPath);
  window.addEventListener('click', onGlobalClick);
  window.addEventListener('dragenter', onWindowDragEnter);
  window.addEventListener('dragleave', onWindowDragLeave);
  window.addEventListener('dragover', onWindowDragOver);
  window.addEventListener('drop', onWindowDrop);
});

onBeforeUnmount(() => {
  window.removeEventListener('device-reconnected', refreshCurrentPath);
  window.removeEventListener('click', onGlobalClick);
  window.removeEventListener('dragenter', onWindowDragEnter);
  window.removeEventListener('dragleave', onWindowDragLeave);
  window.removeEventListener('dragover', onWindowDragOver);
  window.removeEventListener('drop', onWindowDrop);

  router.replace({ query: { ...route.query, path: undefined } });
});
</script>

<style scoped>
.fim-focus-visible {
  display: none;
}
.fim:focus-within .fim-focus-visible {
  display: block;
}
.fim:focus-within .fim-focus-hidden {
  display: none;
}

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
