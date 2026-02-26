import { defineStore } from 'pinia';
import { BusyBar } from '@busy-app/busy-lib';
import type {
  VersionInfo,
  Status as DeviceStatus,
  HttpAccessInfo,
  HttpAccessParams
} from '@busy-app/busy-lib';

export const useDeviceStore = defineStore('device', () => {
  const apiRequest = useApiStore().apiRequest;
  const wifiStore = useWifiStore();
  const firmwareStore = useFirmwareStore();

  const busyBar = shallowRef(new BusyBar({
    addr: useRuntimeConfig().public.barUrl || window.location.origin
  }));

  // Assume device is connected unless the screenstream stops.
  // Upon stream failure, a probing HTTP request is sent. If it fails too, set isConnected to false.
  const isConnected = ref<boolean>(true);
  const checkingConnection = ref<boolean>(false);
  async function checkConnection () {
    if (checkingConnection.value) {
      return;
    }
    checkingConnection.value = true;
    try {
      await apiRequest('/api/name', { timeout: 3000 });
      if (!isConnected.value) {
        window.dispatchEvent(new Event('device-reconnected'));
        if (firmwareStore.autoUpdate.stage === UpdateStage.UPDATING) {
          firmwareStore.autoUpdate.stage = UpdateStage.SUCCESS;
        }
      }
      isConnected.value = true;

      toast.remove('device-disconnected');
    } catch (error) {
      // if the request was aborted/cancelled, don't treat it as disconnection
      if (!refreshInterval.value) {
        console.debug('conncheck request aborted, ignoring because refresh interval is cleared');
        // eslint-disable-next-line @typescript-eslint/no-explicit-any
        const e = error as any;
        if (e?.name === 'AbortError' || e?.message?.toLowerCase().includes('abort') || e?.code === 'ECONNABORTED') {
          checkingConnection.value = false;
          return;
        }
      }

      isConnected.value = false;
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
          color: 'warning',
          duration: 0,
          close: true,
          closeIcon: 'i-bi-cross'
        });
      }
    }
    checkingConnection.value = false;
  }

  const refreshInterval = ref<NodeJS.Timeout>();
  async function refreshDeviceData () {
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
    refreshInterval.value = setInterval(refreshDeviceData, 5000);
  }
  function clearRefreshInterval () {
    if (refreshInterval.value) {
      clearInterval(refreshInterval.value);
      refreshInterval.value = undefined;
    }
  }

  // Connection type
  const connectionType = ref<'usb' | 'wifi'>('wifi');
  async function detectConnectionType () {
    try {
      await $fetch('/api/name', {
        baseURL: useRuntimeConfig().public.barUrl
      });
      connectionType.value = 'usb';
    } catch {
      connectionType.value = 'wifi';
    }
    return connectionType.value;
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

  return {
    busyBar,

    isConnected,
    checkConnection,
    connectionType,
    detectConnectionType,
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
    setHttpAPIAccess
  };
});
