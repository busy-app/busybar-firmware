import { defineStore } from 'pinia';
import { BusyBar, DataStatus, StreamLifecycle } from '@busy-app/busy-lib';
import type {
  VersionInfo,
  Status as DeviceStatus,
  HttpAccessInfo,
  HttpAccessParams,
  NetworkInterfaceInfo,
  AccountInfo
} from '@busy-app/busy-lib';

export const useDeviceStore = defineStore('device', () => {
  const apiRequest = useApiStore().apiRequest;
  const wifiStore = useWifiStore();
  const firmwareStore = useFirmwareStore();
  const stateStreamStore = useStateStreamStore();
  const configStore = useConfigStore();

  const busyBar = shallowRef(new BusyBar({
    addr: useRuntimeConfig().public.barUrl || window.location.origin,
    timeout: Number(configStore.get('httpRequestTimeout'))
  }));

  // Assume device is connected unless the screenstream stops.
  // Upon stream failure, a probing HTTP request is sent. If it fails too, set isConnected to false.
  const isConnected = ref<boolean>(true);
  const checkingConnection = ref<boolean>(false);
  type ConnCheckResult = true | false | 'aborted';
  const successfulConnchecksWithDataStale = ref(0);
  async function checkConnection (): Promise<ConnCheckResult> {
    if (checkingConnection.value) {
      return 'aborted';
    }

    if (isConnected.value && successfulConnchecksWithDataStale.value >= 3) {
      console.warn('Data has been stale for a while and multiple connection checks have succeeded, restarting state stream as it seems to be in a bad state');
      stateStreamStore.stopStream();
      successfulConnchecksWithDataStale.value = 0;
      window.dispatchEvent(new Event('protobuf-websocket-restart'));
    }

    checkingConnection.value = true;
    const wasConnected = isConnected.value;
    try {
      await apiRequest('/api/name', { timeout: Number(configStore.get('httpRequestTimeout')) });
      if (!isConnected.value) {
        window.dispatchEvent(new Event('device-reconnected'));
        if (firmwareStore.autoUpdate.stage === UpdateStage.UPDATING) {
          firmwareStore.autoUpdate.stage = UpdateStage.SUCCESS;
        }
      }
      isConnected.value = true;
      console.debug('Device is connected');
      toast.remove('device-disconnected');
    } catch (error) {
      // if the request was aborted/cancelled, don't treat it as disconnection
      if (!refreshInterval.value && stateStreamStore.streamStatus?.data.status === DataStatus.ACTIVE) {
        console.debug('conncheck request aborted, ignoring because refresh interval is cleared and stream data is active');
        // eslint-disable-next-line @typescript-eslint/no-explicit-any
        const e = error as any;
        if (e?.name === 'AbortError' || e?.message?.toLowerCase().includes('abort') || e?.code === 'ECONNABORTED') {
          checkingConnection.value = false;
          return 'aborted';
        }
      }

      if (wasConnected) {
        window.dispatchEvent(new Event('device-disconnected'));
      }
      isConnected.value = false;
      console.debug('Device is disconnected');
      if (
        firmwareStore.autoUpdate.stage !== UpdateStage.UPDATING
        && !(firmwareStore.autoUpdate.stage === UpdateStage.SUCCESS && wifiStore.wifi?.state !== 'connected')
        && (firmwareStore.fileUpdate.stage === UpdateStage.IDLE || firmwareStore.fileUpdate.stage === UpdateStage.ERROR)
      ) {
        toast.add({
          id: 'device-disconnected',
          title: 'Device disconnected',
          description: 'Device lost. Please check the connection.',
          icon: 'i-bi-alert',
          color: 'error',
          duration: 0,
          close: true,
          closeIcon: 'i-bi-cross'
        });
      }
    }
    checkingConnection.value = false;

    if (isConnected.value && stateStreamStore.streamStatus?.data.status === DataStatus.STALE) {
      successfulConnchecksWithDataStale.value++;
    } else {
      successfulConnchecksWithDataStale.value = 0;
    }

    return isConnected.value;
  }

  const refreshInterval = ref<NodeJS.Timeout>();
  async function refreshDeviceData () {
    if (configStore.get('refreshDeviceDataAbortIfStreamActive')) {
      console.debug('Checking whether to refresh device data. Stream status:', stateStreamStore.streamStatus);
      if (stateStreamStore.streamStatus?.main.status === StreamLifecycle.RUNNING && stateStreamStore.streamStatus?.data.status === DataStatus.ACTIVE) {
        console.debug('Skipping device data refresh because stream is active and config is set to abort in this case');
        if (refreshInterval.value) {
          clearInterval(refreshInterval.value);
          refreshInterval.value = undefined;
          console.debug('Cleared refresh interval to stop refreshing device data while stream is active');
        }
        return;
      }
    }

    const firmwareStore = useFirmwareStore();
    if (firmwareStore.autoUpdate.stage === UpdateStage.LOADING || firmwareStore.fileUpdate.stage === UpdateStage.LOADING) {
      // During auto update, the device is expected to be unresponsive, so skip connection check and just wait for it to come back
      console.debug('Skipping connection check during auto update');
      return;
    }

    await checkConnection();
    if (!isConnected.value) {
      return;
    }
    toast.remove('device-disconnected');

    await fetchDeviceStatus();
    const oldWifiState = wifiStore.wifi?.state;
    const newState = await wifiStore.fetchWifiState();
    if (oldWifiState !== newState?.state) {
      if (newState?.state === 'connected') {
        window.dispatchEvent(new Event('wifi-reconnected'));
      } else {
        window.dispatchEvent(new Event('wifi-disconnected'));
      }
    }
    await fetchHttpAPIAccess();
  }
  function setRefreshInterval () {
    refreshInterval.value = setInterval(refreshDeviceData, Number(configStore.get('httpPollingInterval')));
  }
  function clearRefreshInterval () {
    if (refreshInterval.value) {
      clearInterval(refreshInterval.value);
      refreshInterval.value = undefined;
    }
  }

  // Connection type
  const connectionType = ref<NetworkInterfaceInfo['type']>('wifi');
  async function detectConnectionType () {
    await busyBar.value.SystemTransportGet()
      .then(response => {
        connectionType.value = response.type;
        console.debug('Detected connection type:', connectionType.value);
      })
      .catch(async error => {
        await handleHTTPError(error, 'Couldn\'t get connection type', true);
        return connectionType.value;
      });
  }

  // API version
  const apiVersion = ref<VersionInfo | undefined>(undefined);
  async function fetchApiVersion (): Promise<VersionInfo | undefined> {
    const version = await busyBar.value.SystemVersionGet()
      .then(response => {
        apiVersion.value = response;
        return response;
      })
      .catch(async error => {
        await handleHTTPError(error, 'Couldn\'t get HTTP API version', true);
        return apiVersion.value;
      });

    return version;
  }

  // Device status
  const deviceStatus = ref<DeviceStatus | undefined>(undefined);
  async function fetchDeviceStatus (): Promise<DeviceStatus | undefined> {
    const status = await busyBar.value.SystemStatusGet()
      .then(response => {
        deviceStatus.value = response;
        return response;
      })
      .catch(async error => {
        await handleHTTPError(error, 'Couldn\'t get device status', true);
        return deviceStatus.value; // return old value if available
      });

    return status;
  }

  // Device name
  const DEFAULT_DEVICE_NAME = 'BUSY Bar';
  const deviceName = ref<string | undefined>(undefined);
  async function fetchDeviceName (throwError: boolean = false): Promise<string> {
    const name = await busyBar.value.SettingsNameGet()
      .then(response => {
        deviceName.value = response.name;
        return response.name;
      })
      .catch(async error => {
        if (throwError) {
          throw error;
        }
        await handleHTTPError(error, 'Couldn\'t get device name');
        return DEFAULT_DEVICE_NAME;
      });

    return name;
  }
  async function setDeviceName (name: string): Promise<boolean> {
    return await busyBar.value.SettingsNameSet({ name })
      .then(() => {
        deviceName.value = name;
        toast.add({
          title: 'Changes saved',
          icon: 'i-bi-checkmark-circle-fill',
          color: 'success'
        });
        return true;
      })
      .catch(async error => {
        await handleHTTPError(error, 'Couldn\'t set device name');
        return false;
      });
  }

  // HTTP API
  const httpAPIAccess = ref<HttpAccessInfo | undefined>(undefined);
  async function fetchHttpAPIAccess (): Promise<HttpAccessInfo | undefined> {
    const access = await busyBar.value.SettingsAccessGet()
      .then(response => {
        httpAPIAccess.value = response;
        return response;
      })
      .catch(async error => {
        await handleHTTPError(error, 'Couldn\'t get HTTP API access state', true);
        return httpAPIAccess.value; // return old value if available
      });

    return access;
  }
  async function setHttpAPIAccess (mode: 'key' | 'disabled' | 'enabled', key?: string): Promise<boolean> {
    const payload = { mode } as { mode: 'key' | 'disabled' | 'enabled'; key?: string };
    if (mode === 'key') {
      if (!key) {
        throw new Error('Password not provided');
      }
      payload['key'] = key;
    }

    return await busyBar.value.SettingsAccessSet(payload as HttpAccessParams)
      .then(async () => {
        httpAPIAccess.value = await fetchHttpAPIAccess();
        toast.add({
          title: mode === 'key' ? 'Password set' : 'Changes saved',
          icon: 'i-bi-checkmark-circle-fill',
          color: 'success'
        });
        return true;
      })
      .catch(async error => {
        await handleHTTPError(error, 'Couldn\'t set HTTP API access state');
        return false;
      });
  }

  // Debug log
  const DEBUG_LOG_FILENAME = 'web-debug-log';
  async function requestDebugLogDump () {
    return await busyBar.value.SystemLogDump({ filename: DEBUG_LOG_FILENAME })
      .then(({ path }) => path)
      .catch(async error => {
        await handleHTTPError(error, 'Failed to generate debug log due to an internal error. Please try again.', true);
        return undefined;
      });
  }
  async function fetchDebugLog (path: string) {
    return await busyBar.value.StorageRead({ path })
      .then(file => file instanceof Blob ? file : new Blob([file], { type: 'text/plain' }))
      .catch(async error => {
        await handleHTTPError(error, 'Unable to download debug log. Please try again.', true);
        return undefined;
      });
  }

  // Account
  const accountInfo = ref<AccountInfo | null>(null);
  async function fetchAccountInfo () {
    const info = await busyBar.value.AccountInfoGet()
      .then(response => {
        accountInfo.value = response;
        return response;
      })
      .catch(async error => {
        await handleHTTPError(error, 'Couldn\'t get account info', true);
        return null;
      });

    return info;
  }

  return {
    busyBar,

    isConnected,
    checkConnection,
    connectionType,
    detectConnectionType,
    refreshInterval,
    setRefreshInterval,
    clearRefreshInterval,

    apiVersion,
    fetchApiVersion,

    deviceStatus,
    fetchDeviceStatus,

    deviceName,
    fetchDeviceName,
    setDeviceName,

    httpAPIAccess,
    fetchHttpAPIAccess,
    setHttpAPIAccess,

    requestDebugLogDump,
    fetchDebugLog,

    accountInfo,
    fetchAccountInfo
  };
});
