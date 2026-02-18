<template>
  <div
    data-id="layout-default-preview"
    class="w-full flex flex-col items-center"
  >
    <ScreenStream class="py-6" />
  </div>
</template>

<script setup lang="ts">
const deviceStore = useDeviceStore();

async function init () {
  await deviceStore.fetchDeviceStatus();
}

onMounted(async () => {
  await init();
  window.addEventListener('device-reconnected', init);
});
onBeforeUnmount(() => window.removeEventListener('device-reconnected', init));
</script>
