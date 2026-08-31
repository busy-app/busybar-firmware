<template>
  <ModalIllustrated
    v-model:open="firmwareStore.autoUpdate.modals.success"
    data-id="modal-auto-update-success"
    title="Update completed"
    :description="`Your BUSY Bar is now running the latest firmware${version ? ` (${version})` : ''}.`"
    :images="{
      light: updateCompletedImage,
      dark: updateCompletedImageDark
    }"
    :primary-action-props="{
      label: 'Done',
      onClick: dismiss
    }"
  />
</template>

<script lang="ts" setup>
import updateCompletedImage from '@/assets/images/update-completed-image.png';
import updateCompletedImageDark from '@/assets/images/update-completed-image-dark.png';

const firmwareStore = useFirmwareStore();
const deviceStore = useDeviceStore();
const version = computed(() => deviceStore.deviceStatus?.firmware?.version);

function dismiss () {
  firmwareStore.autoUpdate.modals.success = false;
  firmwareStore.autoUpdate.stage = UpdateStage.IDLE;
  firmwareStore.fileUpdate.stage = UpdateStage.IDLE;
}
</script>
