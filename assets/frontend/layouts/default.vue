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
        <FileUpdateUploadModal />

        <div class="w-full relative flex flex-col items-center xl:items-start gap-4 xl:grid xl:grid-cols-[160px_auto_160px] xl:gap-0">
          <DefaultLayoutTabs />
          <div class="w-full max-w-[688px] flex flex-col gap-4 mx-auto">
            <!-- <DevtoolsPalette /> -->

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
const firmwareStore = useFirmwareStore();
const wifiStore = useWifiStore();

if (!useRuntimeConfig().public.disablePolling) {
  deviceStore.setRefreshInterval();
  firmwareStore.setAutoUpdateBackgroundCheckInterval();
} else {
  console.log('Polling disabled');
}

const shouldLoadDefaultPage = ref(false);
async function init () {
  try {
    await deviceStore.fetchDeviceName(true);

    // initialize device and wifi state
    await deviceStore.fetchDeviceStatus();
    await wifiStore.fetchWifiState();

    // clear auto-update state (stale after fw update)
    firmwareStore.resetAutoUpdateState();

    // request fresh auto-update status, non-blocking
    if (wifiStore.wifi?.state === 'connected') {
      await firmwareStore.fetchAutoUpdateStatus();
    }
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
  window.addEventListener('wifi-reconnected', firmwareStore.requestAutoUpdateCheck);
});

onBeforeUnmount(() => {
  deviceStore.clearRefreshInterval();
  firmwareStore.clearAutoUpdateBackgroundCheckInterval();
  window.removeEventListener('device-reconnected', init);
  window.removeEventListener('wifi-reconnected', firmwareStore.requestAutoUpdateCheck);
});
</script>
