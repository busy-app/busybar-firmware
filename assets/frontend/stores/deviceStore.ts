import { defineStore } from 'pinia';

export interface ApiVersion {
  api_semver: string;
}

export interface SystemStatus {
  branch: string;
  version: string;
  build_date: string;
  commit_hash: string;
  uptime: string;
}

export interface PowerStatus {
  state: string;
  battery_charge: number;
  battery_voltage: number;
  battery_current: number;
  usb_voltage: number;
}

export interface BleStatus {
  state: string;
}

export interface DeviceStatus {
  system: SystemStatus;
  power: PowerStatus;
  ble: BleStatus;
}

export interface HttpAPIAccess {
  mode: 'key' | 'disabled' | 'enabled';
  key_valid: boolean;
}

export interface DisplayBrightness {
  front: 'auto' | number;
  back: 'auto' | number;
}

export interface AudioVolume {
  volume: number;
}

export type UpdateStage = 'idle' | 'uploading' | 'unpacking' | 'updating' | 'error';

export const useDeviceStore = defineStore('device', () => {
  const barUrl = useRuntimeConfig().public.barUrl;
  const toast = useToast();

  // API version
  const apiVersion = ref<ApiVersion | undefined>(undefined);

  async function fetchApiVersion (): Promise<ApiVersion | undefined> {
    const version = await $fetch<ApiVersion>(`${barUrl}/api/version`, { timeout: 3000 })
      .then(response => {
        if (!response || typeof response !== 'object') {
          throw new Error('Empty response');
        }
        console.log('API version fetched:', response);
        apiVersion.value = response;
        return response;
      })
      .catch(error => {
        console.error('Error fetching API version:', error);
        toast.add({
          id: 'api-version-error',
          title: 'Failed to fetch API version',
          description: error.data?.error || genericErrorMessage,
          icon: 'i-ri-alert-line',
          color: 'error',
          duration: 10000
        });
        return undefined;
      });

    return version;
  }

  async function getApiVersion (): Promise<ApiVersion | undefined> {
    if (apiVersion.value === undefined) {
      apiVersion.value = await fetchApiVersion();
    }
    return apiVersion.value;
  }

  // Device status
  const deviceStatus = ref<DeviceStatus | undefined>(undefined);

  async function fetchDeviceStatus (): Promise<DeviceStatus | undefined> {
    const status = await $fetch<DeviceStatus>(`${barUrl}/api/status`, { timeout: 3000 })
      .then(response => {
        if (!response || typeof response !== 'object') {
          throw new Error('Empty response');
        }
        console.log('Device status fetched:', response);
        deviceStatus.value = response;
        return response;
      })
      .catch(error => {
        console.error('Error fetching device status:', error);
        toast.add({
          id: 'device-status-error',
          title: 'Failed to fetch device status',
          description: error.data?.error || genericErrorMessage,
          icon: 'i-ri-alert-line',
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

  async function fetchSystemStatus (throwError: boolean = false): Promise<SystemStatus | undefined> {
    const systemStatus = await $fetch<SystemStatus>(`${barUrl}/api/status/system`, { timeout: 3000 })
      .then(response => {
        if (!response || typeof response !== 'object') {
          throw new Error('Empty response');
        }
        console.log('System status fetched:', response);
        return response;
      })
      .catch(error => {
        if (throwError) {
          throw error;
        }
        console.error('Error fetching system status:', error);
        toast.add({
          id: 'system-status-error',
          title: 'Failed to fetch system status',
          description: error.data?.error || genericErrorMessage,
          icon: 'i-ri-alert-line',
          color: 'error',
          duration: 10000
        });
        return undefined;
      });

    return systemStatus;
  }

  async function fetchPowerStatus (): Promise<PowerStatus | undefined> {
    const powerStatus = await $fetch<PowerStatus>(`${barUrl}/api/status/power`, { timeout: 3000 })
      .then(response => {
        if (!response || typeof response !== 'object') {
          throw new Error('Empty response');
        }
        console.log('Device power status fetched:', response);
        return response;
      })
      .catch(error => {
        console.error('Error fetching device power status:', error);
        toast.add({
          id: 'device-power-error',
          title: 'Failed to fetch device power status',
          description: error.data?.error || genericErrorMessage,
          icon: 'i-ri-alert-line',
          color: 'error',
          duration: 10000
        });
        return undefined;
      });

    return powerStatus;
  }

  // HTTP API
  const httpAPIAccess = ref<HttpAPIAccess | undefined>(undefined);

  async function fetchHttpAPIAccess (): Promise<HttpAPIAccess | undefined> {
    const access = await $fetch<HttpAPIAccess>(`${barUrl}/api/access`, { timeout: 3000 })
      .then(response => {
        if (!response || typeof response !== 'object') {
          throw new Error('Empty response');
        }
        console.log('HTTP API access fetched:', response);
        httpAPIAccess.value = response;
        return response;
      })
      .catch(error => {
        console.error('Error fetching HTTP API access:', error);
        toast.add({
          id: 'http-api-access-error',
          title: 'Failed to fetch HTTP API access',
          description: error.data?.error || genericErrorMessage,
          icon: 'i-ri-alert-line',
          color: 'error',
          duration: 10000
        });
        return undefined;
      });

    return access;
  }

  async function getHttpAPIAccess (): Promise<HttpAPIAccess | undefined> {
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

    return await $fetch(`${barUrl}/api/access`, {
      method: 'POST',
      query: payload
    })
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
          icon: 'i-ri-alert-line',
          color: 'error',
          duration: 10000
        });
        return false;
      });
  }

  // Display brightness
  const displayBrightness = ref<DisplayBrightness | undefined>(undefined);

  async function fetchDisplayBrightness (): Promise<DisplayBrightness | undefined> {
    interface APIDisplayBrightness {
      front: 'auto' | string;
      back: 'auto' | string;
    }
    const brightness = await $fetch<APIDisplayBrightness>(`${barUrl}/api/display/brightness`, { timeout: 3000 })
      .then(response => {
        if (!response || typeof response !== 'object') {
          throw new Error('Empty response');
        }
        console.log('Display brightness fetched:', response);
        const frontParsed = response.front === 'auto' ? 'auto' : Number(response.front);
        const backParsed = response.back === 'auto' ? 'auto' : Number(response.back);
        return { front: frontParsed, back: backParsed } as DisplayBrightness;
      })
      .catch(error => {
        console.error('Error fetching display brightness:', error);
        toast.add({
          id: 'display-brightness-error',
          title: 'Failed to fetch display brightness',
          description: error.data?.error || genericErrorMessage,
          icon: 'i-ri-alert-line',
          color: 'error',
          duration: 10000
        });
        return undefined;
      });

    return brightness;
  }

  async function getDisplayBrightness (): Promise<DisplayBrightness | undefined> {
    if (displayBrightness.value === undefined) {
      displayBrightness.value = await fetchDisplayBrightness();
    }
    return displayBrightness.value;
  }

  async function setDisplayBrightness (brightness: DisplayBrightness): Promise<boolean> {
    return await $fetch(`${barUrl}/api/display/brightness`, {
      method: 'POST',
      query: brightness
    })
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
          icon: 'i-ri-alert-line',
          color: 'error',
          duration: 10000
        });
        return false;
      });
  }

  // Audio volume
  const audio = ref<AudioVolume | undefined>(undefined);

  async function fetchAudioVolume (): Promise<AudioVolume | undefined> {
    const volume = await $fetch<AudioVolume>(`${barUrl}/api/audio/volume`, { timeout: 3000 })
      .then(response => {
        if (!response || typeof response !== 'object') {
          throw new Error('Empty response');
        }
        console.log('Audio volume fetched:', response);
        audio.value = response;
        return response;
      })
      .catch(error => {
        console.error('Error fetching audio volume:', error);
        toast.add({
          id: 'audio-volume-error',
          title: 'Failed to fetch audio volume',
          description: error.data?.error || genericErrorMessage,
          icon: 'i-ri-alert-line',
          color: 'error',
          duration: 10000
        });
        return undefined;
      });

    return volume;
  }

  async function getAudioVolume (): Promise<AudioVolume | undefined> {
    if (audio.value === undefined) {
      audio.value = await fetchAudioVolume();
    }
    return audio.value;
  }

  async function setAudioVolume (volume: number): Promise<boolean> {
    return await $fetch(`${barUrl}/api/audio/volume`, {
      method: 'POST',
      query: { volume }
    })
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
          icon: 'i-ri-alert-line',
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
    xhr.open('POST', `${barUrl}/api/update?name=${firmwareUpdate.value.firmwareBundleName}`);
    xhr.setRequestHeader('Content-Type', 'application/octet-stream');

    xhr.upload.onprogress = event => {
      if (event.lengthComputable) {
        firmwareUpdate.value.progress = Math.round((event.loaded / event.total) * 100);

        if (firmwareUpdate.value.progress === 100) {
          firmwareUpdate.value.stage = 'unpacking';
        }
      } else {
        console.log(`Uploaded ${event.loaded} bytes`);
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
          icon: 'i-ri-alert-line',
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
    apiVersion,
    fetchApiVersion,
    getApiVersion,

    deviceStatus,
    fetchSystemStatus,
    fetchPowerStatus,
    fetchDeviceStatus,
    getDeviceStatus,

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
