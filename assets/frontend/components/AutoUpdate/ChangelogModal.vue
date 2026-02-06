<template>
  <ModalGeneric
    v-model:open="deviceStore.autoUpdate.modals.changelog"
    data-id="modal-auto-update-changelog"
    :dismissible="true"
    :show-close-button="true"
    :title="`Firmware update available (${deviceStore.autoUpdate.availableVersion})`"
    wide
    :primary-action-props="{
      label: 'Start update',
      onClick: deviceStore.startAutoUpdate
    }"
  >
    <template #body>
      <MDC
        v-if="deviceStore.autoUpdate.changelog"
        :value="deviceStore.autoUpdate.changelog"
        tag="div"
      />
    </template>
  </ModalGeneric>
</template>

<script lang="ts" setup>
const deviceStore = useDeviceStore();
</script>
