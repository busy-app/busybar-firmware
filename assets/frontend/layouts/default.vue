<template>
  <div
    data-id="layout-default"
    class="w-screen min-h-screen px-4 sm:px-6 py-4"
  >
    <UContainer>
      <DefaultLayoutHeader />
      <DefaultLayoutPreview class="pb-10" />

      <PasswordSetModal />
      <PasswordUpdateModal />
      <PasswordRemoveModal />

      <div class="w-full relative flex flex-col items-center gap-4 xl:grid xl:grid-cols-[160px_auto_160px] xl:gap-0">
        <DefaultLayoutTabs />
        <div class="w-full max-w-[688px] mx-auto">
          <slot />
        </div>
      </div>
    </UContainer>
  </div>
</template>

<script setup lang="ts">
const deviceStore = useDeviceStore();
const wifiStore = useWifiStore();

async function refreshDeviceData () {
  if (!deviceStore.isConnected) {
    await deviceStore.checkConnection();
    if (!deviceStore.isConnected) {
      return;
    }
    toast.remove('device-disconnected');
  }
  await deviceStore.fetchDeviceStatus();
  await wifiStore.fetchWifiState();
  await deviceStore.fetchHttpAPIAccess();
}

const refreshInterval = setInterval(refreshDeviceData, 5000);

onBeforeUnmount(() => {
  clearInterval(refreshInterval);
});
</script>
