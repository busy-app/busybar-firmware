<template>
  <UModal
    v-model:open="deviceStore.autoUpdate.modals.batteryLow"
    data-id="modal-auto-update-battery-low"
    title="Charge your BUSY bar"
    description="The battery charge is too low to start the update (must be 40% or higher)."
    :ui="{
      content: 'max-w-[480px] divide-none bg-neutral-100/90 dark:bg-neutral-800/75 backdrop-blur-[5px] ring-1 ring-glass',
      description: 'hidden',
      header: 'hidden',
      body: 'p-0 sm:p-0 overflow-visible',
      close: 'hidden',
      overlay: 'bg-neutral-900/20 dark:bg-neutral-900/80'
    }"
  >
    <template #body>
      <div
        class="flex flex-col gap-6 p-6 bg-no-repeat"
        :style="`background-image: url(${batteryLowImage}); background-size: 150%; background-position: center`"
      >
        <div class="text-center text-xl font-medium">Charge your BUSY bar</div>

        <div class="h-50" />
        <div class="text-center">The battery charge is too low to start the update<br>(must be 40% or higher).</div>

        <div class="flex justify-end">
          <UButton
            color="neutral"
            label="Got it"
            class="min-w-20 justify-center"
            @click="deviceStore.autoUpdate.modals.batteryLow = false"
          />
        </div>
      </div>
    </template>
  </UModal>
</template>

<script lang="ts" setup>
import batteryLowImage from '@/assets/images/battery-low.png';

const deviceStore = useDeviceStore();
</script>
