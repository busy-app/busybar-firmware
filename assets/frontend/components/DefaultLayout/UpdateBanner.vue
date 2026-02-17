<template>
  <div
    v-if="updateAvailable"
    class="w-full flex items-center justify-between p-4 pr-6 ring-1 ring-glass rounded-3xl"
    :class="backgroundGradientClass"
  >
    <template v-if="canUpdate">
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
    </template>

    <template v-else-if="notAllowed">
      <div class="flex items-start gap-2.5">
        <UIcon
          name="i-bi-alert-fill"
          class="size-7.5 text-warning"
        />
        <div class="flex flex-col gap-1">
          <span class="text-warning font-medium">System update paused</span>
          <span class="text-sm text-highlighted/90">The battery charge is too low to start the update. Plug in the charger and keep it connected until the update is complete.</span>
        </div>
      </div>
    </template>
  </div>
</template>

<script setup lang="ts">
const deviceStore = useDeviceStore();
const firmwareStore = useFirmwareStore();

const updateAvailable = computed(() => {
  return firmwareStore.autoUpdate.status === 'available';
});

const canUpdate = computed(() => {
  return updateAvailable.value && firmwareStore.autoUpdate.isAllowed && firmwareStore.autoUpdate.availableVersion !== null;
});

// battery too low
const notAllowed = computed(() => {
  return updateAvailable.value && !firmwareStore.autoUpdate.isAllowed;
});

const backgroundGradientClass = computed(() => {
  if (canUpdate.value) {
    return 'can-update';
  } else if (notAllowed.value) {
    return 'not-allowed';
  }

  return '';
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

<style scoped>
.can-update {
  background: var(--color-success-50);
}
.dark .can-update {
  background: linear-gradient(90deg, var(--color-success-950) 29.96%, var(--color-success-800) 100%);
}

.not-allowed {
  background: var(--color-warning-50);
}
.dark .not-allowed {
  background: linear-gradient(90deg, var(--color-warning-800) 29.96%, var(--color-warning-500) 100%);
}
</style>
