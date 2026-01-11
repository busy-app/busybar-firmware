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
  const toast = useToast();

  const busyBar = new BusyBar({
    addr: useRuntimeConfig().public.barUrl
  });

  // Connection type
  const connectionType = ref<'usb' | 'wifi'>('wifi');
  async function detectConnectionType () {
    try {
      await $fetch('/api/version', {
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
        if (!response || typeof response !== 'object') {
          throw new Error('Empty response');
        }
        apiVersion.value = response;
        return response;
      })
      .catch(async error => {
        if (error.data?.error === 'Forbidden') {
          await navigateTo('/login');
          return undefined;
        }
        console.error('Error fetching API version:', error);
        toast.add({
          id: 'api-version-error',
          title: 'Failed to fetch API version',
          description: error.data?.error || genericErrorMessage,
          icon: 'i-bi-alert',
          color: 'error',
          duration: 10000
        });
        return undefined;
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
        if (!response || typeof response !== 'object') {
          throw new Error('Empty response');
        }
        deviceStatus.value = response;
        return response;
      })
      .catch(async error => {
        if (error.data?.error === 'Forbidden') {
          await navigateTo('/login');
          return undefined;
        }
        console.error('Error fetching device status:', error);
        toast.add({
          id: 'device-status-error',
          title: 'Failed to fetch device status',
          description: error.data?.error || genericErrorMessage,
          icon: 'i-bi-alert',
          color: 'error',
          duration: 10000
        });
        return undefined;
      });

    return status;
  }
  async function getDeviceStatus (): Promise<DeviceStatus | undefined> {
    if (deviceStatus.value === undefined) {
      deviceStatus.value = await fetchDeviceStatus();
    }
    return deviceStatus.value;
  }
  async function fetchSystemStatus (throwError: boolean = false): Promise<StatusSystem | undefined> {
    const systemStatus = await busyBar.systemStatus()
      .then(response => {
        if (!response || typeof response !== 'object') {
          throw new Error('Empty response');
        }
        return response;
      })
      .catch(async error => {
        if (throwError) {
          throw error;
        }
        if (error.data?.error === 'Forbidden') {
          await navigateTo('/login');
          return undefined;
        }
        console.error('Error fetching system status:', error);
        toast.add({
          id: 'system-status-error',
          title: 'Failed to fetch system status',
          description: error.data?.error || genericErrorMessage,
          icon: 'i-bi-alert',
          color: 'error',
          duration: 10000
        });
        return undefined;
      });

    return systemStatus;
  }
  async function fetchPowerStatus (): Promise<StatusPower | undefined> {
    const powerStatus = await busyBar.powerStatus()
      .then(response => {
        if (!response || typeof response !== 'object') {
          throw new Error('Empty response');
        }
        return response;
      })
      .catch(async error => {
        if (error.data?.error === 'Forbidden') {
          await navigateTo('/login');
          return undefined;
        }
        console.error('Error fetching device power status:', error);
        toast.add({
          id: 'device-power-error',
          title: 'Failed to fetch device power status',
          description: error.data?.error || genericErrorMessage,
          icon: 'i-bi-alert',
          color: 'error',
          duration: 10000
        });
        return undefined;
      });

    return powerStatus;
  }

  // Device name
  const DEFAULT_DEVICE_NAME = 'BUSY Bar';
  const deviceName = ref<string | undefined>(undefined);
  async function fetchDeviceName (): Promise<string> {
    const name = await busyBar.getName()
      .then(response => {
        if (typeof response !== 'string') {
          throw new Error('Empty response');
        }
        deviceName.value = response;
        return response;
      })
      .catch(async error => {
        if (error.data?.error === 'Forbidden') {
          await navigateTo('/login');
          return DEFAULT_DEVICE_NAME;
        }
        console.error('Error fetching device name:', error);
        toast.add({
          id: 'device-name-error',
          title: 'Failed to fetch device name',
          description: error.data?.error || genericErrorMessage,
          icon: 'i-bi-alert',
          color: 'error',
          duration: 10000
        });
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
      .catch(error => {
        console.error('Error setting device name:', error);
        toast.add({
          id: 'device-name-set-error',
          title: 'Failed to set device name',
          description: error.data?.error || genericErrorMessage,
          icon: 'i-bi-alert',
          color: 'error',
          duration: 10000
        });
        return false;
      });
  }

  // HTTP API
  const httpAPIAccess = ref<HttpAccessInfo | undefined>(undefined);
  async function fetchHttpAPIAccess (): Promise<HttpAccessInfo | undefined> {
    const access = await busyBar.getHttpAccess()
      .then(response => {
        if (!response || typeof response !== 'object') {
          throw new Error('Empty response');
        }
        httpAPIAccess.value = response;
        return response;
      })
      .catch(async error => {
        if (error.data?.error === 'Forbidden') {
          await navigateTo('/login');
          return undefined;
        }
        console.error('Error fetching HTTP API access:', error);
        toast.add({
          id: 'http-api-access-error',
          title: 'Failed to fetch HTTP API access',
          description: error.data?.error || genericErrorMessage,
          icon: 'i-bi-alert',
          color: 'error',
          duration: 10000
        });
        return undefined;
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

    return await busyBar.setHttpAccess(payload as HttpAccessParams)
      .then(async () => {
        httpAPIAccess.value = await fetchHttpAPIAccess();
        return true;
      })
      .catch(error => {
        console.error('Error setting HTTP API access:', error);
        toast.add({
          id: 'http-api-access-set-error',
          title: 'Failed to set HTTP API access',
          description: error.data?.error || genericErrorMessage,
          icon: 'i-bi-alert',
          color: 'error',
          duration: 10000
        });
        return false;
      });
  }

  // Display brightness
  const displayBrightness = ref<DisplayBrightnessParams | undefined>(undefined);
  async function fetchDisplayBrightness (): Promise<DisplayBrightnessParams | undefined> {
    const brightness = await busyBar.getDisplayBrightness()
      .then(response => {
        if (!response || typeof response !== 'object') {
          throw new Error('Empty response');
        }
        const frontParsed = response.front === 'auto' ? 'auto' : Number(response.front);
        const backParsed = response.back === 'auto' ? 'auto' : Number(response.back);
        return { front: frontParsed, back: backParsed } as DisplayBrightnessParams;
      })
      .catch(async error => {
        if (error.data?.error === 'Forbidden') {
          await navigateTo('/login');
          return undefined;
        }
        console.error('Error fetching display brightness:', error);
        toast.add({
          id: 'display-brightness-error',
          title: 'Failed to fetch display brightness',
          description: error.data?.error || genericErrorMessage,
          icon: 'i-bi-alert',
          color: 'error',
          duration: 10000
        });
        return undefined;
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
      .catch(error => {
        console.error('Error setting display brightness:', error);
        toast.add({
          id: 'display-brightness-set-error',
          title: 'Failed to set display brightness',
          description: error.data?.error || genericErrorMessage,
          icon: 'i-bi-alert',
          color: 'error',
          duration: 10000
        });
        return false;
      });
  }

  // Audio volume
  const audio = ref<AudioVolumeInfo | undefined>(undefined);
  async function fetchAudioVolume (): Promise<AudioVolumeInfo | undefined> {
    const volume = await busyBar.getAudioVolume()
      .then(response => {
        if (!response || typeof response !== 'object') {
          throw new Error('Empty response');
        }
        audio.value = response;
        return response;
      })
      .catch(async error => {
        if (error.data?.error === 'Forbidden') {
          await navigateTo('/login');
          return undefined;
        }
        console.error('Error fetching audio volume:', error);
        toast.add({
          id: 'audio-volume-error',
          title: 'Failed to fetch audio volume',
          description: error.data?.error || genericErrorMessage,
          icon: 'i-bi-alert',
          color: 'error',
          duration: 10000
        });
        return undefined;
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
      .catch(error => {
        console.error('Error setting audio volume:', error);
        toast.add({
          id: 'audio-volume-set-error',
          title: 'Failed to set audio volume',
          description: error.data?.error || genericErrorMessage,
          icon: 'i-bi-alert',
          color: 'error',
          duration: 10000
        });
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

    connectionType,
    detectConnectionType,

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
