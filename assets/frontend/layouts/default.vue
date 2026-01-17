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

      <div class="w-full relative flex flex-col gap-4 xl:grid xl:grid-cols-[160px_auto_160px] xl:gap-0">
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
  await deviceStore.checkConnection();
  if (!deviceStore.isConnected) {
    return;
  }
  toast.remove('device-disconnected');

  await deviceStore.fetchDeviceStatus();
  await wifiStore.fetchWifiState();
  await deviceStore.fetchHttpAPIAccess();
}

let refreshInterval: NodeJS.Timeout;
if (!useRuntimeConfig().public.disablePolling) {
  refreshInterval = setInterval(refreshDeviceData, 5000);
} else {
  console.log('Polling disabled');
}

onBeforeUnmount(() => {
  clearInterval(refreshInterval);
});
</script>
