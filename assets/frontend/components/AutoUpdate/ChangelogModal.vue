<template>
  <ModalGeneric
    v-model:open="firmwareStore.autoUpdate.modals.changelog"
    data-id="modal-auto-update-changelog"
    :dismissible="true"
    :show-close-button="true"
    :title="`Firmware update available (${firmwareStore.autoUpdate.availableVersion})`"
    wide
    sticky-actions
    :primary-action-props="{
      label: 'Start update',
      loading: firmwareStore.autoUpdate.stage === UpdateStage.LOADING,
      onClick: firmwareStore.startAutoUpdate
    }"
  >
    <template #body>
      <div
        v-if="firmwareStore.autoUpdate.isChangelogLoading"
        data-id="modal-auto-update-changelog-loading"
        aria-label="Loading changelog"
        class="flex flex-col gap-3 *:bg-accented"
      >
        <USkeleton class="h-6 w-32 rounded-md" />
        <USkeleton class="h-4 w-full rounded-full" />
        <USkeleton class="h-4 w-4/5 rounded-full" />
      </div>
      <MDC
        v-else-if="firmwareStore.autoUpdate.changelog"
        :value="firmwareStore.autoUpdate.changelog"
        tag="article"
      />
    </template>
  </ModalGeneric>
</template>

<script lang="ts" setup>
const firmwareStore = useFirmwareStore();
</script>
