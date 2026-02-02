<template>
  <div
    data-id="layout-default-preview"
    class="w-full flex flex-col items-center"
  >
    <ScreenStream class="py-6" />

    <div class="hidden max-w-screen overflow-auto">
      <div class="w-fit flex items-center justify-center gap-6 px-4">
        <!-- <UpdateButton /> -->

        <div
          v-if="power"
          class="flex items-center gap-1.5"
        >
          <div class="relative flex">
            <UIcon
              :name="power?.state === 'charging' ? 'i-busy-battery-charging' : batteryIcon()"
              class="size-6"
            />
            <UIcon
              v-if="power?.state === 'charging'"
              name="i-busy-charging-lightning"
              class="absolute size-6"
            />
          </div>
          <div>{{ power?.battery_charge }}%</div>
        </div>

        <!-- <div class="flex items-center gap-1.5">
          <CircularProgress
            :model-value="3.2 / 8 * 100"
            color="#6A7282"
            track-color="#99A1AF26"
          />
          <div class="whitespace-nowrap">3.2 / 8 GB</div>
        </div> -->
      </div>
    </div>
  </div>
</template>

<script setup lang="ts">
const deviceStore = useDeviceStore();

const power = computed(() => deviceStore.deviceStatus?.power);

function batteryIcon (): string {
  const charge = power.value?.battery_charge || 0;
  if (charge >= 75) {
    return 'i-ri-battery-fill';
  }
  if (charge >= 30) {
    return 'i-ri-battery-low-line';
  }
  return 'i-ri-battery-line';
}

async function init () {
  await deviceStore.getDeviceStatus();
}

onMounted(async () => {
  await init();
  window.addEventListener('device-reconnected', init);
});
onBeforeUnmount(() => window.removeEventListener('device-reconnected', init));
</script>
