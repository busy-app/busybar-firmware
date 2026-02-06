<template>
  <div
    data-id="layout-default"
    class="w-screen min-h-screen px-4 sm:px-6 py-4"
  >
    <UContainer>
      <template v-if="shouldLoadDefaultPage">
        <DefaultLayoutHeader />
        <DefaultLayoutPreview class="pb-6" />

        <PasswordSetModal />
        <PasswordUpdateModal />
        <PasswordRemoveModal />

        <AutoUpdateChangelogModal />
        <AutoUpdateBatteryLowModal />
        <AutoUpdateFirmwareModal />

        <div class="w-full relative flex flex-col items-center xl:items-start gap-4 xl:grid xl:grid-cols-[160px_auto_160px] xl:gap-0">
          <DefaultLayoutTabs />
          <div class="w-full max-w-[688px] flex flex-col gap-4 mx-auto">
            <DefaultLayoutUpdateBanner />
            <slot />
          </div>
        </div>
      </template>
      <template v-else>
        <div class="w-screen h-screen absolute top-0 left-0 flex items-center justify-center">
          <UIcon
            name="i-busy-loader"
            class="size-12 animate-spin text-muted"
          />
        </div>
      </template>
    </UContainer>
  </div>
</template>

<script setup lang="ts">
const deviceStore = useDeviceStore();
const wifiStore = useWifiStore();

if (!useRuntimeConfig().public.disablePolling) {
  deviceStore.setRefreshInterval();
  deviceStore.setAutoUpdateBackgroundCheckInterval();
} else {
  console.log('Polling disabled');
}

const shouldLoadDefaultPage = ref(false);
async function init () {
  try {
    await deviceStore.fetchDeviceName(true);

    // initialize device and wifi state
    await deviceStore.getDeviceStatus();
    await wifiStore.getWifiState();

    // get auto-update status, non-blocking
    deviceStore.resetAutoUpdateState();
    deviceStore.fetchAutoUpdateStatus()
      .finally(() => {
        // if status is still null after fetching, request a check (handles the case where the device was just powered on and the status is not yet available)
        if (deviceStore.autoUpdate.status === null) {
          deviceStore.requestAutoUpdateCheck();
        }
      });
  } catch (error) {
    if ((error as { status: number }).status === 403) {
      return await navigateTo('/login');
    }
  }
  shouldLoadDefaultPage.value = true;
}

onMounted(async () => {
  await init();
  window.addEventListener('device-reconnected', init);
});

onBeforeUnmount(() => {
  deviceStore.clearRefreshInterval();
  deviceStore.clearAutoUpdateBackgroundCheckInterval();
  window.removeEventListener('device-reconnected', init);
});
</script>
