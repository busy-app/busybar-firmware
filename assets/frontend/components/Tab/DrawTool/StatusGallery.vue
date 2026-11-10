<template>
  <SectionCard
    data-id="draw-tool-gallery-section"
    :title="sectionTitle"
    :ui="{
      actionsWrapper: 'sm:items-center'
    }"
  >
    <template #leading-actions>
      <UButton
        v-if="hasSelection"
        data-id="draw-tool-gallery-cancel-selection"
        color="neutral"
        variant="ghost"
        icon="i-bi-cross"
        square
        @click="clearSelection()"
      />
    </template>

    <template #actions>
      <template v-if="hasSelection">
        <UButton
          data-id="draw-tool-gallery-select-all"
          label="Select all"
          icon="i-bi-checkmark"
          color="neutral"
          variant="ghost"
          class="justify-center sm:justify-start"
          :disabled="areAllStatusesSelected"
          @click="selectAllStatuses"
        />

        <USeparator
          v-if="!areAllStatusesSelected"
          orientation="vertical"
          class="hidden md:block my-auto h-6"
        />

        <UButton
          data-id="draw-tool-gallery-download-selected"
          label="Download"
          color="neutral"
          variant="ghost"
          icon="i-bi-download"
          class="justify-center sm:justify-start"
          :loading="isDownloadingSelected"
          @click="downloadSelectedStatuses"
        />
        <UButton
          data-id="draw-tool-gallery-delete-selected"
          label="Delete"
          color="error"
          variant="ghost"
          icon="i-bi-trash"
          class="justify-center sm:justify-start"
          :loading="isDeletingSelected"
          @click="deleteSelectedStatuses"
        />
      </template>

      <template v-else>
        <!-- <UButton
          data-id="draw-tool-gallery-refresh"
          label="Refresh"
          color="neutral"
          variant="soft"
          icon="i-bi-arrow-clockwise"
          :loading="isRefreshingStatusDirectory"
          @click="() => dts.refreshStatusDirectory()"
        /> -->
        <UButton
          data-id="draw-tool-status-off"
          label="Turn status off"
          icon="i-bi-control-stop"
          color="neutral"
          variant="outline"
          class="justify-center sm:justify-start"
          @click="() => dts.clearStatusDisplay()"
        />
        <UButton
          data-id="draw-tool-gallery-new-status"
          label="Create"
          color="neutral"
          icon="i-bi-plus"
          class="justify-center sm:justify-start"
          @click="emit('newStatus')"
        />
      </template>
    </template>

    <template #raw-body>
      <div class="min-h-48 p-4 sm:p-6 ring-1 ring-accented rounded-xl">
        <div
          v-if="!statusGalleryFiles.length"
          class="h-38 flex items-center justify-center text-center text-sm text-muted transition-all"
          :class="isRefreshingStatusDirectory ? 'text-transparent' : ''"
        >
          Your statuses will appear here after you save them to the device.
        </div>

        <div
          v-else
          class="grid grid-cols-1 md:grid-cols-2 gap-4"
        >
          <article
            v-for="status in statusGalleryFiles"
            :key="status.name"
            class="relative aspect-[72/16] md:aspect-auto md:h-16 w-full md:w-72 overflow-hidden rounded-md ring-1 ring-default"
            @mouseenter="hoveredStatusName = status.name"
            @mouseleave="hoveredStatusName = hoveredStatusName === status.name ? null : hoveredStatusName"
          >
            <div class="h-full w-full overflow-hidden rounded-md bg-neutral-950">
              <AnimationPlayer
                v-if="status.animation"
                :animation="status.animation"
              />
              <img
                v-else-if="status.previewUrl"
                :src="status.previewUrl"
                :alt="status.name"
                class="h-full w-full object-cover [image-rendering:pixelated]"
              >
              <div
                v-else
                class="flex h-full w-full items-center justify-center px-3 text-center text-xs text-muted"
              >
                Preview unavailable
              </div>

              <div
                class="absolute inset-0 rounded-md bg-elevated/90 transition-opacity"
                :class="shouldShowStatusHoverOverlaysByDefault || hasSelection || isStatusOverlayActive(status.name) ? 'opacity-100' : 'pointer-events-none opacity-0'"
              >
                <div class="absolute inset-y-0 left-5 z-10 flex items-center">
                  <input
                    :checked="selectedStatusNames.includes(status.name)"
                    type="checkbox"
                    class="pointer-events-auto size-4 rounded border-default accent-primary"
                    @click.stop
                    @change="toggleStatusSelection(status.name)"
                  >
                </div>

                <div
                  v-if="shouldShowStatusHoverOverlaysByDefault || isStatusOverlayActive(status.name)"
                  class="pointer-events-none absolute inset-0 flex items-center justify-center"
                >
                  <UButton
                    data-id="draw-tool-gallery-show-status"
                    label="Show on BUSY Bar"
                    color="neutral"
                    variant="solid"
                    size="sm"
                    class="pointer-events-auto"
                    :icon="shownStatusName === status.name ? 'i-bi-checkmark' : 'i-bi-play-fill'"
                    :loading="showingStatusName === status.name"
                    :ui="{
                      leadingIcon: `${shownStatusName === status.name ? 'text-success' : ''}`
                    }"
                    @click="() => showStatusOnBusyBar(status.name)"
                  />
                </div>

                <div
                  v-if="shouldShowStatusHoverOverlaysByDefault || isStatusOverlayActive(status.name)"
                  class="absolute inset-y-0 right-3 flex items-center"
                >
                  <TabDrawToolActionMenu
                    v-model:open="statusMenuOpenStates[status.name]"
                    :items="getStatusMenuItems(status.name)"
                  >
                    <UButton
                      data-id="draw-tool-gallery-status-menu"
                      color="neutral"
                      variant="ghost"
                      icon="i-bi-more"
                      square
                      size="sm"
                      aria-label="Status actions"
                      :loading="pendingThemeStatusNames.includes(status.name)"
                    />
                  </TabDrawToolActionMenu>
                </div>
              </div>
            </div>
          </article>
        </div>
      </div>

      <ModalGeneric
        v-model:open="isDeleteConfirmOpen"
        data-id="modal-draw-tool-gallery-delete-confirm"
        :title="deleteConfirmTitle"
        :description="deleteConfirmDescription"
        show-close-button
        :primary-action-props="{
          label: 'Delete',
          color: 'error',
          loading: isDeletingSelected,
          onClick: confirmDeleteStatuses
        }"
        :secondary-action-props="{
          label: 'Cancel',
          variant: 'ghost',
          disabled: isDeletingSelected,
          onClick: closeDeleteConfirmation
        }"
      />

      <ModalGeneric
        v-model:open="isRemoveThemeConfirmOpen"
        data-id="modal-draw-tool-gallery-remove-theme-confirm"
        title="Remove from BUSY Bar?"
        description="This removes the status from your BUSY Bar only. It stays in the draw tool, so you can upload it again anytime."
        :primary-action-props="{
          label: 'Remove',
          color: 'error',
          variant: 'soft',
          loading: isRemovingTheme,
          onClick: confirmRemoveStatusTheme
        }"
        :secondary-action-props="{
          label: 'Cancel',
          variant: 'ghost',
          disabled: isRemovingTheme,
          onClick: () => { isRemoveThemeConfirmOpen = false; }
        }"
      />
    </template>
  </SectionCard>
</template>

<script setup lang="ts">
import { storeToRefs } from 'pinia';
import type { DropdownMenuItem } from '@nuxt/ui';

const emit = defineEmits<{
  newStatus: [];
}>();

const dts = useDrawToolStore();
const { statusGalleryFiles, statusThemeNames, isRefreshingStatusDirectory } = storeToRefs(dts);

const hoveredStatusName = ref<string | null>(null);
const selectedStatusNames = ref<string[]>([]);
const showingStatusName = ref<string | null>(null);
const shownStatusName = ref<string | null>(null);
const statusMenuOpenStates = ref<Record<string, boolean>>({});
const isDownloadingSelected = ref(false);
const isDeletingSelected = ref(false);
const isDeleteConfirmOpen = ref(false);
const pendingDeleteStatusNames = ref<string[]>([]);
const isRemoveThemeConfirmOpen = ref(false);
const isRemovingTheme = ref(false);
const pendingRemoveThemeStatusName = ref<string | null>(null);
const pendingThemeStatusNames = ref<string[]>([]);
const showStatusIconResetTimeout = ref<ReturnType<typeof setTimeout> | null>(null);
const hoverInaccessibleMediaQuery = ref<MediaQueryList | null>(null);
const shouldShowStatusHoverOverlaysByDefault = ref(false);

const hasSelection = computed(() => selectedStatusNames.value.length > 0);
const sectionTitle = computed(() => hasSelection.value ? `${selectedStatusNames.value.length} selected` : 'Your statuses');
const areAllStatusesSelected = computed(() => {
  return !!statusGalleryFiles.value.length && selectedStatusNames.value.length === statusGalleryFiles.value.length;
});
const deleteConfirmTitle = computed(() => pendingDeleteStatusNames.value.length > 1 ? 'Delete selected statuses?' : 'Delete status?');
const deleteConfirmDescription = computed(() => pendingDeleteStatusNames.value.length > 1
  ? `${pendingDeleteStatusNames.value.length} statuses will be deleted. This action cannot be undone.`
  : 'Status will be deleted. This action cannot be undone.');

function updateStatusHoverOverlayAccessibility () {
  shouldShowStatusHoverOverlaysByDefault.value = hoverInaccessibleMediaQuery.value?.matches ?? false;
}

function clearSelection () {
  selectedStatusNames.value = [];
  hoveredStatusName.value = null;
  statusMenuOpenStates.value = {};
}

function openDeleteConfirmation (statusNames: string[]) {
  pendingDeleteStatusNames.value = [...new Set(statusNames)];

  if (!pendingDeleteStatusNames.value.length) {
    return;
  }

  isDeleteConfirmOpen.value = true;
}

function closeDeleteConfirmation () {
  if (isDeletingSelected.value) {
    return;
  }

  isDeleteConfirmOpen.value = false;
  pendingDeleteStatusNames.value = [];
}

function toggleStatusSelection (statusName: string) {
  if (selectedStatusNames.value.includes(statusName)) {
    selectedStatusNames.value = selectedStatusNames.value.filter(name => name !== statusName);
    return;
  }

  selectedStatusNames.value = [...selectedStatusNames.value, statusName];
}

function selectAllStatuses () {
  selectedStatusNames.value = statusGalleryFiles.value.map(status => status.name);
}

function isStatusOverlayActive (statusName: string) {
  return hoveredStatusName.value === statusName
    || !!statusMenuOpenStates.value[statusName]
    || pendingThemeStatusNames.value.includes(statusName);
}

function setThemeStatusPending (statusName: string, isPending: boolean) {
  pendingThemeStatusNames.value = isPending
    ? [...pendingThemeStatusNames.value, statusName]
    : pendingThemeStatusNames.value.filter(name => name !== statusName);
}

function getStatusThemeMenuItem (statusName: string): DropdownMenuItem {
  if (statusThemeNames.value[statusName]) {
    return {
      label: 'Remove from BUSY Bar',
      description: 'It won’t appear in BUSY/CUSTOM setup on BUSY Bar.',
      icon: 'i-bi-cross',
      disabled: pendingThemeStatusNames.value.includes(statusName),
      onClick: () => openRemoveThemeConfirmation(statusName)
    };
  }

  return {
    label: 'Save to BUSY Bar',
    description: 'Find it in BUSY/CUSTOM setup under Theme.',
    icon: 'i-bi-checkmark',
    disabled: pendingThemeStatusNames.value.includes(statusName),
    onClick: () => saveStatusToBusyBarTheme(statusName)
  };
}

function getStatusMenuItems (statusName: string): DropdownMenuItem[] {
  const isAnimation = getStatusFileKind(statusName) === 'animation';

  return [
    getStatusThemeMenuItem(statusName),
    {
      label: isAnimation ? 'Download animation' : 'Download PNG',
      description: isAnimation ? 'Save the animation to your computer.' : 'Save the image to your computer.',
      icon: 'i-bi-download',
      onClick: () => dts.downloadStatusFile(statusName)
    },
    ...(isAnimation
      ? [{
        label: 'Download GIF',
        description: 'Save the GIF to your computer.',
        icon: 'i-bi-download',
        onClick: () => dts.downloadStatusAsGif(statusName)
      }]
      : []),
    {
      label: 'Delete',
      icon: 'i-bi-trash',
      color: 'error',
      onClick: () => {
        statusMenuOpenStates.value[statusName] = false;
        openDeleteConfirmation([statusName]);
      }
    }
  ];
}

async function saveStatusToBusyBarTheme (statusName: string) {
  if (pendingThemeStatusNames.value.includes(statusName)) {
    return;
  }

  setThemeStatusPending(statusName, true);

  try {
    await dts.saveSavedStatusTheme(statusName);
    showDrawToolSuccessToast('Saved to BUSY Bar. Find it in BUSY/CUSTOM → Setup → Theme.');
  } catch (error) {
    await showDrawToolErrorToast('Couldn’t save to BUSY Bar. Check the connection and try again.', error);
  } finally {
    setThemeStatusPending(statusName, false);
  }
}

function openRemoveThemeConfirmation (statusName: string) {
  statusMenuOpenStates.value[statusName] = false;
  pendingRemoveThemeStatusName.value = statusName;
  isRemoveThemeConfirmOpen.value = true;
}

async function confirmRemoveStatusTheme () {
  const statusName = pendingRemoveThemeStatusName.value;

  if (!statusName) {
    return;
  }

  isRemovingTheme.value = true;
  setThemeStatusPending(statusName, true);

  try {
    await dts.removeStatusTheme(statusName);
    showDrawToolSuccessToast('Removed from BUSY Bar. It won’t appear in BUSY/CUSTOM setup.');
  } catch (error) {
    await showDrawToolErrorToast('Couldn’t remove from BUSY Bar. Check the connection and try again.', error);
  } finally {
    setThemeStatusPending(statusName, false);
    isRemovingTheme.value = false;
    isRemoveThemeConfirmOpen.value = false;
    pendingRemoveThemeStatusName.value = null;
  }
}

async function showStatusOnBusyBar (statusName: string) {
  showingStatusName.value = statusName;

  try {
    await dts.showSavedStatusOnBusyBar(statusName);
    shownStatusName.value = statusName;

    if (showStatusIconResetTimeout.value) {
      clearTimeout(showStatusIconResetTimeout.value);
    }

    showStatusIconResetTimeout.value = setTimeout(() => {
      shownStatusName.value = null;
    }, 3000);
  } catch {
    // Request errors are already reported by the store
  } finally {
    showingStatusName.value = null;
  }
}

async function downloadSelectedStatuses () {
  const selectedNames = [...selectedStatusNames.value];

  if (!selectedNames.length) {
    return;
  }

  isDownloadingSelected.value = true;

  try {
    for (const statusName of selectedNames) {
      await dts.downloadStatusFile(statusName);
    }
  } finally {
    isDownloadingSelected.value = false;
    clearSelection();
  }
}

async function deleteSelectedStatuses () {
  const selectedNames = [...selectedStatusNames.value];

  if (!selectedNames.length) {
    return;
  }

  openDeleteConfirmation(selectedNames);
}

async function confirmDeleteStatuses () {
  const statusNamesToDelete = [...pendingDeleteStatusNames.value];

  if (!statusNamesToDelete.length) {
    return;
  }

  isDeletingSelected.value = true;

  try {
    await dts.deleteStatusFiles(statusNamesToDelete);
  } finally {
    isDeletingSelected.value = false;
    closeDeleteConfirmation();
    clearSelection();
  }
}

watch(statusGalleryFiles, files => {
  const availableNames = new Set(files.map(file => file.name));
  selectedStatusNames.value = selectedStatusNames.value.filter(name => availableNames.has(name));
  statusMenuOpenStates.value = Object.fromEntries(Object.entries(statusMenuOpenStates.value).filter(([name]) => availableNames.has(name)));

  if (hoveredStatusName.value && !availableNames.has(hoveredStatusName.value)) {
    hoveredStatusName.value = null;
  }
}, { immediate: true });

onMounted(() => {
  if (typeof window === 'undefined' || typeof window.matchMedia !== 'function') {
    return;
  }

  hoverInaccessibleMediaQuery.value = window.matchMedia('(hover: none), (any-hover: none), (pointer: coarse), (any-pointer: coarse), (pointer: none), (any-pointer: none)');
  updateStatusHoverOverlayAccessibility();
  hoverInaccessibleMediaQuery.value.addEventListener('change', updateStatusHoverOverlayAccessibility);
});

onBeforeUnmount(() => {
  hoverInaccessibleMediaQuery.value?.removeEventListener('change', updateStatusHoverOverlayAccessibility);

  if (showStatusIconResetTimeout.value) {
    clearTimeout(showStatusIconResetTimeout.value);
  }
});
</script>
