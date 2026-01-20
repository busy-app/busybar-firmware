import { defineStore } from 'pinia';
import { BusyBar } from '@busy-app/busy-lib';
import type {
  VersionInfo,
  StatusSystem,
  StatusPower,
  Status as DeviceStatus,
  HttpAccessInfo,
  HttpAccessParams,
  DisplayBrightnessParams,
  AudioVolumeInfo
} from '@busy-app/busy-lib';

export type UpdateStage = 'idle' | 'uploading' | 'unpacking' | 'updating' | 'success' | 'error';

export const useDeviceStore = defineStore('device', () => {
  const apiRequest = useApiStore().apiRequest;
  const wifiStore = useWifiStore();

  const busyBar = new BusyBar({
    addr: useRuntimeConfig().public.barUrl || window.location.origin
  });

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
      // todo: dispatch a global event instead
      if (!isConnected.value) {
        window.location.reload();
      }
      isConnected.value = true;

      toast.remove('device-disconnected');
    } catch {
      isConnected.value = false;
      if (firmwareUpdate.value.stage !== 'idle' && firmwareUpdate.value.stage !== 'error') {
        // during firmware update, the device will be disconnected, so don't show the toast
        return;
      }
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
    checkingConnection.value = false;
  }

  const refreshInterval = ref<NodeJS.Timeout>();
  async function refreshDeviceData () {
    await checkConnection();
    if (!isConnected.value) {
      return;
    }
    toast.remove('device-disconnected');

    await fetchDeviceStatus();
    await wifiStore.fetchWifiState();
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
    const version = await busyBar.getApiVersion()
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
  async function getApiVersion (): Promise<VersionInfo | undefined> {
    if (apiVersion.value === undefined) {
      apiVersion.value = await fetchApiVersion();
    }
    return apiVersion.value;
  }

  // Device status
  const deviceStatus = ref<DeviceStatus | undefined>(undefined);
  async function fetchDeviceStatus (): Promise<DeviceStatus | undefined> {
    const status = await busyBar.deviceStatus()
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
  async function getDeviceStatus (): Promise<DeviceStatus | undefined> {
    if (deviceStatus.value === undefined) {
      deviceStatus.value = await fetchDeviceStatus();
    }
    return deviceStatus.value;
  }
  async function fetchSystemStatus (): Promise<StatusSystem | undefined> {
    const systemStatus = await busyBar.systemStatus()
      .catch(async error => {
        await handleHTTPError(error, 'Couldn\'t get system status');
        return undefined;
      });

    return systemStatus;
  }
  async function fetchPowerStatus (): Promise<StatusPower | undefined> {
    const powerStatus = await busyBar.powerStatus()
      .catch(async error => {
        await handleHTTPError(error, 'Couldn\'t get power status');
        return undefined;
      });

    return powerStatus;
  }

  // Device name
  const DEFAULT_DEVICE_NAME = 'BUSY Bar';
  const deviceName = ref<string | undefined>(undefined);
  async function fetchDeviceName (throwError: boolean = false): Promise<string> {
    const name = await busyBar.getName()
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
  async function getDeviceName (): Promise<string> {
    if (deviceName.value === undefined) {
      deviceName.value = await fetchDeviceName();
    }
    return deviceName.value;
  }
  async function setDeviceName (name: string): Promise<boolean> {
    return await busyBar.setName({ name })
      .then(() => {
        deviceName.value = name;
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
    const access = await busyBar.getHttpAccess()
      .then(response => {
        return response;
      })
      .catch(async error => {
        await handleHTTPError(error, 'Couldn\'t get HTTP API access state', true);
        return httpAPIAccess.value; // return old value if available
      });

    return access;
  }
  async function getHttpAPIAccess (): Promise<HttpAccessInfo | undefined> {
    if (httpAPIAccess.value === undefined) {
      httpAPIAccess.value = await fetchHttpAPIAccess();
    }
    return httpAPIAccess.value;
  }
  async function setHttpAPIAccess (mode: 'key' | 'disabled' | 'enabled', key?: string): Promise<boolean> {
    const payload = { mode } as { mode: 'key' | 'disabled' | 'enabled'; key?: string };
    if (mode === 'key') {
      if (!key) {
        throw new Error('Password not provided');
      }
      payload['key'] = key;
    }

    // fixme: temp solution for required key field even when it's not needed
    if (!key) {
      payload.key = '666666';
    }

    return await busyBar.setHttpAccess(payload as HttpAccessParams)
      .then(async () => {
        httpAPIAccess.value = await fetchHttpAPIAccess();
        return true;
      })
      .catch(async error => {
        await handleHTTPError(error, 'Couldn\'t set HTTP API access state');
        return false;
      });
  }

  // Display brightness
  const displayBrightness = ref<DisplayBrightnessParams | undefined>(undefined);
  async function fetchDisplayBrightness (): Promise<DisplayBrightnessParams | undefined> {
    const brightness = await busyBar.getDisplayBrightness()
      .then(response => {
        const frontParsed = response.front === 'auto' ? 'auto' : Number(response.front);
        const backParsed = response.back === 'auto' ? 'auto' : Number(response.back);
        return { front: frontParsed, back: backParsed } as DisplayBrightnessParams;
      })
      .catch(async error => {
        await handleHTTPError(error, 'Couldn\'t get display brightness', true);
        return displayBrightness.value;
      });

    return brightness;
  }
  async function getDisplayBrightness (): Promise<DisplayBrightnessParams | undefined> {
    if (displayBrightness.value === undefined) {
      displayBrightness.value = await fetchDisplayBrightness();
    }
    return displayBrightness.value;
  }
  async function setDisplayBrightness (brightness: DisplayBrightnessParams): Promise<boolean> {
    return await busyBar.setDisplayBrightness(brightness)
      .then(() => {
        displayBrightness.value = brightness;
        return true;
      })
      .catch(async error => {
        await handleHTTPError(error, 'Couldn\'t set display brightness');
        return false;
      });
  }

  // Audio volume
  const audio = ref<AudioVolumeInfo | undefined>(undefined);
  async function fetchAudioVolume (): Promise<AudioVolumeInfo | undefined> {
    const volume = await busyBar.getAudioVolume()
      .then(response => {
        audio.value = response;
        return response;
      })
      .catch(async error => {
        await handleHTTPError(error, 'Couldn\'t get audio volume', true);
        return audio.value;
      });

    return volume;
  }
  async function getAudioVolume (): Promise<AudioVolumeInfo | undefined> {
    if (audio.value === undefined) {
      audio.value = await fetchAudioVolume();
    }
    return audio.value;
  }
  async function setAudioVolume (volume: number): Promise<boolean> {
    return await busyBar.setAudioVolume({ volume })
      .then(() => {
        if (audio.value) {
          audio.value.volume = volume;
        } else {
          audio.value = { volume };
        }
        return true;
      })
      .catch(async error => {
        await handleHTTPError(error, 'Couldn\'t set audio volume');
        return false;
      });
  }

  // Firmware update
  const firmwareUpdate = ref({
    firmwareBundleName: 'firmware',
    firmwareFile: null as File | null,
    stage: 'idle' as UpdateStage,
    progress: 0,
    error: null as string | null
  });
  async function uploadFirmware () {
    const xhr = new XMLHttpRequest();
    xhr.open('POST', `/api/update?name=${firmwareUpdate.value.firmwareBundleName}`);
    xhr.setRequestHeader('Content-Type', 'application/octet-stream');
    if (useApiStore().apiKey) {
      xhr.setRequestHeader('X-API-Key', useApiStore().apiKey!);
    }

    xhr.upload.onprogress = event => {
      if (event.lengthComputable) {
        firmwareUpdate.value.progress = Math.round((event.loaded / event.total) * 100);

        if (firmwareUpdate.value.progress === 100) {
          firmwareUpdate.value.stage = 'unpacking';
        }
      }
    };

    xhr.onload = () => {
      if (xhr.status >= 200 && xhr.status < 400) {
        firmwareUpdate.value.stage = 'updating';
        toast.add({
          title: 'Update initiated',
          description: 'The device will reboot to apply the update. Pay attention to the front screen.',
          icon: 'i-ri-check-line',
          color: 'success',
          duration: 10000
        });
      } else {
        console.error('Upload failed:', xhr.status, xhr.responseText);
        firmwareUpdate.value.stage = 'error';
        toast.add({
          title: 'Update failed',
          description: `Error ${xhr.status}: ${xhr.responseText}`,
          icon: 'i-bi-alert',
          color: 'error',
          duration: 10000
        });
        firmwareUpdate.value.error = `Error ${xhr.status}: ${xhr.responseText}`;
      }
    };

    firmwareUpdate.value.stage = 'uploading' as UpdateStage;
    firmwareUpdate.value.progress = 0;
    xhr.send(firmwareUpdate.value.firmwareFile);

    await new Promise<void>(resolve => {
      xhr.onloadend = () => {
        resolve();
      };
    });

    firmwareUpdate.value.firmwareFile = null;
    if (firmwareUpdate.value.stage !== 'error') {
      firmwareUpdate.value.stage = 'updating' as UpdateStage;
      firmwareUpdate.value.progress = 0;
    }
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
    getApiVersion,

    deviceStatus,
    fetchSystemStatus,
    fetchPowerStatus,
    fetchDeviceStatus,
    getDeviceStatus,

    deviceName,
    fetchDeviceName,
    getDeviceName,
    setDeviceName,

    httpAPIAccess,
    fetchHttpAPIAccess,
    getHttpAPIAccess,
    setHttpAPIAccess,

    displayBrightness,
    fetchDisplayBrightness,
    getDisplayBrightness,
    setDisplayBrightness,

    audio,
    fetchAudioVolume,
    getAudioVolume,
    setAudioVolume,

    firmwareUpdate,
    uploadFirmware
  };
});
