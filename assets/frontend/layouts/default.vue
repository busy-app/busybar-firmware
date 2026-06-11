<template>
  <div
    data-id="layout-default"
    class="min-h-screen px-4 sm:px-6 py-4"
  >
    <ConfigStoreCard v-if="configStore.showConfigUI === true" />

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
        <TabDrawToolLeaveEditorModal />

        <div class="w-full relative flex flex-col items-center xl:items-start gap-4 xl:grid xl:grid-cols-[160px_auto_160px] xl:gap-0">
          <DefaultLayoutTabs />
          <div class="w-full max-w-[688px] flex flex-col gap-4 mx-auto">
            <DefaultLayoutUpdateBanner />
            <DefaultLayoutStateStreamFailBanner />
            <DefaultLayoutStateStreamResourceLimitBanner />

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
import type { ProcessedState } from '@busy-app/busy-lib';
import { BSB_Error, StateStreamErrorCode, StreamLifecycle } from '@busy-app/busy-lib';

const deviceStore = useDeviceStore();
const firmwareStore = useFirmwareStore();
const wifiStore = useWifiStore();
const stateStreamStore = useStateStreamStore();
const configStore = useConfigStore();

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

function logStateUpdates (message: ProcessedState) {
  if (!message.updates) {
    message.updates = [];
  }
  for (const update of message.updates) {
    if (update.state === 'frame' && configStore.get('stateStreamLogFrames') === false) {
      continue;
    }
    console.debug(`[state stream message] (${Number(message.timestamp)})`, update);
  }
  if (message.updates.length === 0 && configStore.get('stateStreamLogHeartbeats')) {
    console.debug(`[state stream message] (${Number(message.timestamp)}) heartbeat (no updates)`);
  }
}

function handleStateStreamFailure () {
  console.warn('Falling back to polling for device state updates');
  deviceStore.setRefreshInterval();
  firmwareStore.setAutoUpdateBackgroundCheckInterval();
}
function initStateStream () {
  try {
    stateStreamStore.showStateStreamFailBanner = false;

    const now = Date.now();
    console.debug('Starting state stream');
    stateStreamStore.stream.start({
      dataCallback: message => {
        stateStreamStore.applyStateMessage(message);
        if (configStore.get('stateStreamLogUpdates')) {
          logStateUpdates(message);
        }
      },
      statusCallback: stateStreamStore.applyStreamStatus,
      errorCallback: error => {
        function stop () {
          try {
            stateStreamStore.stopStream();
          } catch (stopError) {
            console.warn('Failed to stop state stream after error:', stopError);
          }
          handleStateStreamFailure();
        }

        if (error.data?.severity === BSB_Error.Severity.WARNING) {
          console.warn(`[state stream warning] ${error.code}: ${error.message}`);
          return;
        }
        console.error(`[state stream error] ${error.code}: ${error.message}`);
        if (error.code === StateStreamErrorCode.DEVICE_ERROR && error.data?.cause === BSB_Error.Cause.RESOURCE_LIMIT) {
          if (stateStreamStore.showStateStreamFailBanner) {
            stateStreamStore.showStateStreamFailBanner = false;
          }
          stateStreamStore.showResourceLimitErrorBanner = true;
          stop();
        } else {
          if (error.code === StateStreamErrorCode.CONNECTION_TIMEOUT || error.code === StateStreamErrorCode.CONNECTION_LOST || error.data?.severity === BSB_Error.Severity.FATAL) {
            if (stateStreamStore.showResourceLimitErrorBanner) {
              stateStreamStore.showResourceLimitErrorBanner = false;
            }
            stateStreamStore.showStateStreamFailBanner = true;
            stop();
          }
        }
      }
    });
    console.debug(`State stream started (took ${Date.now() - now}ms)`);

    // if polling was active, stop it since we now have a successful state stream connection
    deviceStore.clearRefreshInterval();
    firmwareStore.clearAutoUpdateBackgroundCheckInterval();
  } catch (error) {
    console.error('Error starting state websocket:', error);
    stateStreamStore.showStateStreamFailBanner = true;
    handleStateStreamFailure();
  }
}

async function handleDeviceReconnected () {
  await waitForStateStreamRestartableState();

  await init();
}

const STATE_STREAM_RESTARTABLE_STATE_TIMEOUT_MS = 7500;
async function waitForStateStreamRestartableState (): Promise<void> {
  if (stateStreamStore.streamStatus?.main.status !== StreamLifecycle.IDLE && stateStreamStore.streamStatus?.main.status !== StreamLifecycle.STOPPED) {
    await new Promise((resolve, reject) => {
      const restartableStateTimeout = setTimeout(() => {
        clearInterval(restartableStateInterval);
        stateStreamStore.streamNotRestartable = true;
        if (stateStreamStore.showResourceLimitErrorBanner) {
          stateStreamStore.showResourceLimitErrorBanner = false;
        }
        stateStreamStore.showStateStreamFailBanner = true;
        reject(new Error('State stream is not in a restartable state'));
      }, STATE_STREAM_RESTARTABLE_STATE_TIMEOUT_MS);

      const restartableStateInterval = setInterval(() => {
        if (stateStreamStore.streamStatus?.main.status === StreamLifecycle.IDLE || stateStreamStore.streamStatus?.main.status === StreamLifecycle.STOPPED) {
          clearTimeout(restartableStateTimeout);
          clearInterval(restartableStateInterval);
          resolve(true);
        }
      }, 100);
    });
  }
}
async function handleStateStreamRestart () {
  console.debug('Trying to restart state stream...');
  if (stateStreamStore.streamStatus?.main.status !== StreamLifecycle.IDLE && stateStreamStore.streamStatus?.main.status !== StreamLifecycle.STOPPED) {
    await waitForStateStreamRestartableState();
  }

  await initStateStream();
}

onMounted(async () => {
  await init();
  window.addEventListener('device-reconnected', handleDeviceReconnected);
  window.addEventListener('protobuf-websocket-restart', handleStateStreamRestart);
  window.addEventListener('wifi-reconnected', firmwareStore.requestAutoUpdateCheck);

  // check if config store is cached in localStorage
  if (localStorage.getItem('configStore') === null) {
    // make it cache showConfigUI for external access
    const showUI = configStore.showConfigUI;
    configStore.showConfigUI = undefined;
    configStore.showConfigUI = showUI;
  }
});

onBeforeUnmount(() => {
  deviceStore.clearRefreshInterval();
  firmwareStore.clearAutoUpdateBackgroundCheckInterval();
  stateStreamStore.stopStream();
  stateStreamStore.streamStatus = null;
  window.removeEventListener('device-reconnected', handleDeviceReconnected);
  window.removeEventListener('protobuf-websocket-restart', handleStateStreamRestart);
  window.removeEventListener('wifi-reconnected', firmwareStore.requestAutoUpdateCheck);
});
</script>
