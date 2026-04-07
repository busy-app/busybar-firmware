<template>
  <ModalGeneric
    v-model:open="firmwareStore.autoUpdate.modals.changelog"
    data-id="modal-auto-update-changelog"
    :dismissible="true"
    :show-close-button="true"
    :title="`Firmware update available (${firmwareStore.autoUpdate.availableVersion})`"
    wide
    :primary-action-props="{
      label: 'Start update',
      loading: firmwareStore.autoUpdate.stage === UpdateStage.LOADING,
      onClick: firmwareStore.startAutoUpdate
    }"
  >
    <template #body>
      <div v-if="firmwareStore.autoUpdate.isChangelogLoading">Loading changelog...</div>
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
