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
        <AutoUpdateSuccessModal />
        <FileUpdateUploadModal />

        <div class="w-full relative flex flex-col items-center xl:items-start gap-4 xl:grid xl:grid-cols-[160px_auto_160px] xl:gap-0">
          <DefaultLayoutTabs />
          <div class="w-full max-w-[688px] flex flex-col gap-4 mx-auto">
            <!-- <DevtoolsPalette /> -->

            <DefaultLayoutUpdateBanner />
            <DefaultLayoutStateStreamFailBanner />
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
const stateStreamStore = useStateStreamStore();

const shouldLoadDefaultPage = ref(false);
async function init () {
  try {
    await deviceStore.fetchDeviceName(true);

    if (localStorage.getItem('successfulUpdate') === 'true') {
      localStorage.removeItem('successfulUpdate');
      firmwareStore.autoUpdate.modals.success = true;
    }

    // initialize device and wifi state
    await deviceStore.fetchDeviceStatus();
    await wifiStore.fetchWifiState();

    // clear auto-update state (stale after fw update)
    firmwareStore.resetAutoUpdateState();

    // request fresh auto-update status
    if (wifiStore.wifi?.state === 'connected') {
      await firmwareStore.fetchAutoUpdateStatus();
    }

    await initStateStream();
  } catch (error) {
    if ((error as { status: number })?.status === 403) {
      return await navigateTo('/login');
    }
  }
  shouldLoadDefaultPage.value = true;
}

const lastMessageTimestamp = ref(0);
function logStateUpdates (message: StateMessage) {
  const currentMessageTimestamp = Number(message.timestamp);
  for (const update of message.updates) {
    console.debug(`[${currentMessageTimestamp}] (Δ${String(currentMessageTimestamp - lastMessageTimestamp.value).padStart(4, ' ')}ms)`, update);
  }
  if (message.updates.length === 0) {
    console.debug(`[${currentMessageTimestamp}] (Δ${String(currentMessageTimestamp - lastMessageTimestamp.value).padStart(4, ' ')}ms) heartbeat (no updates)`);
  }
  lastMessageTimestamp.value = currentMessageTimestamp;
}

async function initStateStream () {
  try {
    stateStreamStore.showStateStreamFailBanner = false;

    const now = Date.now();
    console.debug('Starting state stream');
    await stateStreamStore.startStateStream(logStateUpdates, console.error);
    console.debug(`State stream started (took ${Date.now() - now}ms)`);

    // if polling was active, stop it since we now have a successful state stream connection
    deviceStore.clearRefreshInterval();
    firmwareStore.clearAutoUpdateBackgroundCheckInterval();
  } catch (error) {
    console.error('Error starting state websocket:', error);

    stateStreamStore.showStateStreamFailBanner = true;

    console.log('Falling back to polling for device state updates');
    deviceStore.setRefreshInterval();
    firmwareStore.setAutoUpdateBackgroundCheckInterval();
  }
}

async function handleDeviceReconnected () {
  await init();
}

async function handleStateStreamRestart () {
  await initStateStream();
}

onMounted(async () => {
  await init();
  window.addEventListener('device-reconnected', handleDeviceReconnected);
  window.addEventListener('protobuf-websocket-restart', handleStateStreamRestart);
  window.addEventListener('wifi-reconnected', firmwareStore.requestAutoUpdateCheck);
});

onBeforeUnmount(() => {
  deviceStore.clearRefreshInterval();
  firmwareStore.clearAutoUpdateBackgroundCheckInterval();
  stateStreamStore.stopStateStream();
  window.removeEventListener('device-reconnected', handleDeviceReconnected);
  window.removeEventListener('protobuf-websocket-restart', handleStateStreamRestart);
  window.removeEventListener('wifi-reconnected', firmwareStore.requestAutoUpdateCheck);
});
</script>
