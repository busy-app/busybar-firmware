<template>
  <ModalIllustrated
    v-model:open="firmwareStore.autoUpdate.modals.batteryLow"
    data-id="modal-auto-update-battery-low"
    title="Charge your BUSY bar"
    description="The battery charge is too low to start the update (must be 40% or higher)."
    :images="{
      light: batteryLowImage,
      dark: batteryLowImageDark
    }"
    :ui="{
      description: 'hidden',
      header: 'hidden',
      body: 'p-0 sm:p-0 overflow-visible',
      close: 'hidden'
    }"
    :primary-action-props="{
      label: 'Got it',
      onClick: () => { firmwareStore.autoUpdate.modals.batteryLow = false }
    }"
  />
</template>

<script lang="ts" setup>
import batteryLowImage from '@/assets/images/battery-low-image.png';
import batteryLowImageDark from '@/assets/images/battery-low-image-dark.png';

const firmwareStore = useFirmwareStore();
</script>
