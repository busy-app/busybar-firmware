<template>
  <div
    v-if="updateAvailable"
    class="w-full flex items-center justify-between p-4 pr-6 ring-1 ring-glass rounded-3xl bg-linear-to-r from-success-50 to-success-50 dark:from-success-950 dark:to-success-800"
  >
    <div class="flex items-start gap-2.5">
      <UIcon
        name="i-bi-firmware-download"
        class="size-6 text-success"
      />
      <div class="flex flex-col gap-1">
        <span class="text-success font-medium">Update available</span>
        <span class="text-sm text-highlighted/90">Get all the latest features and improvements</span>
      </div>
    </div>

    <div>
      <UButton
        data-id="update-banner-update-button"
        color="neutral"
        label="Update"
        class="justify-center w-20 h-9"
        @click="handleUpdateClick"
      />
    </div>
  </div>
</template>

<script setup lang="ts">
const deviceStore = useDeviceStore();
const firmwareStore = useFirmwareStore();

const updateAvailable = computed(() => {
  return firmwareStore.autoUpdate.status === 'available';
});

async function handleUpdateClick () {
  await deviceStore.fetchDeviceStatus();
  const charge = deviceStore.deviceStatus?.power?.battery_charge;

  if (charge !== undefined && charge < 40) {
    firmwareStore.autoUpdate.modals.batteryLow = true;
  } else {
    firmwareStore.autoUpdate.modals.changelog = true;
  }
}
</script>
