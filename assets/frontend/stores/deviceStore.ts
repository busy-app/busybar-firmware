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

// export type FileUpdateStage = 'idle' | 'uploading' | 'unpacking' | 'updating' | 'success' | 'error';
enum UpdateEvent {
  SESSION_START = 'session_start',
  SESSION_STOP = 'session_stop',
  ACTION_BEGIN = 'action_begin',
  ACTION_DONE = 'action_done',
  DETAIL_CHANGE = 'detail_change',
  ACTION_PROGRESS = 'action_progress',
  NONE = 'none'
}
enum UpdateAction {
  DOWNLOAD = 'download',
  SHA_VERIFICATION = 'sha_verification',
  UNPACK = 'unpack',
  PREPARE = 'prepare',
  APPLY = 'apply',
  NONE = 'none'
}
enum UpdateStatusCode {
  OK = 'ok',
  BATTERY_LOW = 'battery_low',
  BUSY = 'busy',
  DOWNLOAD_FAILURE = 'download_failure',
  DOWNLOAD_ABORT = 'download_abort',
  SHA_MISMATCH = 'sha_mismatch',
  UNPACK_STAGING_DIR_FAILURE = 'unpack_staging_dir_failure',
  UNPACK_ARCHIVE_OPEN_FAILURE = 'unpack_archive_open_failure',
  UNPACK_ARCHIVE_UNPACK_FAILURE = 'unpack_archive_unpack_failure',
  INSTALL_MANIFEST_NOT_FOUND = 'install_manifest_not_found',
  INSTALL_MANIFEST_INVALID = 'install_manifest_invalid',
  INSTALL_SESSION_CONFIG_FAILURE = 'install_session_config_failure',
  INSTALL_POINTER_SETUP_FAILURE = 'install_pointer_setup_failure',
  UNKNOWN_FAILURE = 'unknown_failure'
}
export enum UpdateStage {
  IDLE = 'idle',
  UPLOADING = 'uploading',
  UNPACKING = 'unpacking',
  UPDATING = 'updating',
  ERROR = 'error',
  // ! Adding success for file update purposes
  SUCCESS = 'success'
}
interface UpdateDownloadStatus {
  speed_bytes_per_sec: number;
  received_bytes: number;
  total_bytes: number;
}
interface UpdateInstallStatus {
  is_allowed: boolean;
  event: UpdateEvent;
  action: UpdateAction;
  status: UpdateStatusCode;
  detail: string;
  download: UpdateDownloadStatus;
}
export type UpdateCheckResult = 'available' | 'not_available' | 'failure' | 'none';
interface UpdateCheckStatus {
  available_version: string;
  event: 'start' | 'stop' | 'none';
  status: UpdateCheckResult;
}
export interface UpdateStatus {
  install: UpdateInstallStatus;
  check: UpdateCheckStatus;
}

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
      if (!isConnected.value) {
        window.dispatchEvent(new Event('device-reconnected'));
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
      if (fileUpdate.value.stage === 'idle' || fileUpdate.value.stage === 'error') {
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
    const version = await busyBar.SystemVersion()
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
    const status = await busyBar.SystemStatus()
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
    const systemStatus = await busyBar.SystemInfo()
      .catch(async error => {
        await handleHTTPError(error, 'Couldn\'t get system status');
        return undefined;
      });

    return systemStatus;
  }
  async function fetchPowerStatus (): Promise<StatusPower | undefined> {
    const powerStatus = await busyBar.SystemStatusPower()
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
    const name = await busyBar.SettingsName()
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
    return await busyBar.SettingsNameSet({ name })
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
    const access = await busyBar.SettingsAccess()
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

    return await busyBar.SettingsAccessSet(payload as HttpAccessParams)
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
    const brightness = await busyBar.DisplayBrightness()
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
    return await busyBar.DisplayBrightnessSet(brightness)
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
    const volume = await busyBar.AudioVolume()
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
    return await busyBar.AudioVolumeSet({ volume })
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
  const BACKGROUND_AUTO_UPDATE_CHECK_INTERVAL_MS = 30 * 60 * 1000; // 30 minutes
  const autoUpdate = ref({
    status: null as UpdateCheckResult | null,
    availableVersion: null as string | null,
    isAllowed: null as boolean | null,

    isChecking: false,
    isManualCheck: false,
    backgroundCheckInterval: null as NodeJS.Timeout | null,

    modals: {
      changelog: false,
      batteryLow: false,
      updating: false,
      error: false
    },
    changelog: null as string | null,
    step: UpdateStage.IDLE as UpdateStage,
    progress: 0,
    error: {
      step: UpdateStage.IDLE as UpdateStage,
      message: null as string | null
    }
  });
  function resetAutoUpdateState () {
    autoUpdate.value.status = null;
    autoUpdate.value.availableVersion = null;
    autoUpdate.value.isAllowed = null;
    autoUpdate.value.isChecking = false;
    autoUpdate.value.isManualCheck = false;
    for (const key in autoUpdate.value.modals) {
      autoUpdate.value.modals[key as keyof typeof autoUpdate.value.modals] = false;
    }
    autoUpdate.value.changelog = null;
    autoUpdate.value.step = UpdateStage.IDLE;
    autoUpdate.value.progress = 0;
    autoUpdate.value.error.step = UpdateStage.IDLE;
    autoUpdate.value.error.message = null;
  }
  async function fetchAutoUpdateStatus (): Promise<void> {
    return apiRequest<UpdateStatus>('/api/update/status', { timeout: 10000 })
      .then(async status => {
        if (status.check.status === 'failure') {
          // auto-update check failed (e.g. no internet connection)
          console.warn('Auto-update check failed', status);
          autoUpdate.value.isChecking = false;
          if (autoUpdate.value.isManualCheck) {
            autoUpdate.value.isManualCheck = false;
            toast.add({
              title: 'Update check failed',
              description: 'Check your internet connection and try again.',
              icon: 'i-bi-alert',
              color: 'error',
              duration: 10000
            });
          }
          return;
        }

        if (status.check.event !== 'stop' && status.check.status === 'none') {
          if (status.check.event === 'none') {
            // update check never started
            console.debug('Empty auto update status, requesting update check');
            return requestAutoUpdateCheck();
          }

          // update check is still in progress
          console.debug('Auto-update check still in progress, fetching status again');
          await new Promise(resolve => {
            setTimeout(resolve, 3000);
          });
          return fetchAutoUpdateStatus();
        }
        autoUpdate.value.isChecking = false;

        autoUpdate.value.status = status.check.status || null;
        autoUpdate.value.availableVersion = status.check.available_version || null;
        autoUpdate.value.isAllowed = status.install.is_allowed;

        if (autoUpdate.value.isManualCheck && status.check.status === 'not_available') {
          autoUpdate.value.isManualCheck = false;
          toast.add({
            title: 'Your firmware version is up to date',
            icon: 'i-bi-checkmark-circle-fill',
            color: 'success',
            duration: 10000
          });
        }

        console.debug('Auto-update check completed', status);

        if (autoUpdate.value.availableVersion) {
          return fetchAutoUpdateChangelog(autoUpdate.value.availableVersion);
        }
      })
      .catch(async error => {
        autoUpdate.value.isChecking = false;
        await handleHTTPError(error, 'Couldn\'t check for updates');
      });
  }
  async function requestAutoUpdateCheck () {
    if (autoUpdate.value.isChecking) {
      console.debug('Already checking for updates, ignoring request');
      return;
    }
    resetAutoUpdateState();

    return apiRequest('/api/update/check', { method: 'POST', timeout: 10000 })
      .then(async () => {
        console.debug('Auto-update check requested');
        await new Promise(resolve => setTimeout(resolve, 1000));
        return fetchAutoUpdateStatus();
      })
      .catch(async error => {
        autoUpdate.value.isChecking = false;
        await handleHTTPError(error, 'Couldn\'t initiate update check');
      });
  }
  function setAutoUpdateBackgroundCheckInterval () {
    autoUpdate.value.backgroundCheckInterval = setInterval(() => {
      console.debug(`Performing background auto-update check (${new Date().toISOString()})`);
      requestAutoUpdateCheck();
    }, BACKGROUND_AUTO_UPDATE_CHECK_INTERVAL_MS);
  }
  function clearAutoUpdateBackgroundCheckInterval () {
    if (autoUpdate.value.backgroundCheckInterval) {
      clearInterval(autoUpdate.value.backgroundCheckInterval);
      autoUpdate.value.backgroundCheckInterval = null;
    }
  }

  async function fetchAutoUpdateChangelog (version: string) {
    await apiRequest<{ changelog: string }>(`/api/update/changelog?version=${version}`)
      .then(response => {
        autoUpdate.value.changelog = response.changelog;
      })
      .catch(async error => {
        await handleHTTPError(error, 'Couldn\'t fetch update changelog');
        return null;
      });
  }

  function startAutoUpdate () {
    console.debug('Starting auto-update process');
  }

  const fileUpdate = ref({
    firmwareBundleName: 'firmware',
    firmwareFile: null as File | null,
    stage: UpdateStage.IDLE as UpdateStage,
    progress: 0,
    error: null as string | null
  });
  async function uploadFirmware () {
    const xhr = new XMLHttpRequest();
    xhr.open('POST', `${useRuntimeConfig().public.barUrl || window.location.origin}/api/update?name=${fileUpdate.value.firmwareBundleName}`);
    xhr.setRequestHeader('Content-Type', 'application/octet-stream');
    if (useApiStore().apiKey) {
      xhr.setRequestHeader('X-API-Key', useApiStore().apiKey!);
    }

    xhr.upload.onprogress = event => {
      if (event.lengthComputable) {
        fileUpdate.value.progress = Math.round((event.loaded / event.total) * 100);

        if (fileUpdate.value.progress === 100) {
          fileUpdate.value.stage = UpdateStage.UNPACKING;
        }
      }
    };

    xhr.onload = () => {
      if (xhr.status >= 200 && xhr.status < 400) {
        fileUpdate.value.stage = UpdateStage.UPDATING;
        toast.add({
          title: 'Update initiated',
          description: 'The device will reboot to apply the update. Pay attention to the front screen.',
          icon: 'i-ri-check-line',
          color: 'success',
          duration: 10000
        });
      } else {
        console.error('Upload failed:', xhr.status, xhr.responseText);
        fileUpdate.value.stage = UpdateStage.ERROR;
        toast.add({
          title: 'Update failed',
          description: `Error ${xhr.status}: ${xhr.responseText}`,
          icon: 'i-bi-alert',
          color: 'error',
          duration: 10000
        });
        fileUpdate.value.error = `Error ${xhr.status}: ${xhr.responseText}`;
      }
    };

    xhr.onerror = () => {
      console.error('Upload error');
      fileUpdate.value.stage = UpdateStage.ERROR;
      toast.add({
        title: 'Update failed',
        description: 'An error occurred during the upload.',
        icon: 'i-bi-alert',
        color: 'error',
        duration: 10000
      });
      fileUpdate.value.error = 'An error occurred during the upload.';
    };

    fileUpdate.value.stage = UpdateStage.UPLOADING;
    fileUpdate.value.progress = 0;
    xhr.send(fileUpdate.value.firmwareFile);

    await new Promise<void>(resolve => {
      xhr.onloadend = () => {
        resolve();
      };
    });

    fileUpdate.value.firmwareFile = null;
    if (fileUpdate.value.stage as UpdateStage !== UpdateStage.ERROR) {
      fileUpdate.value.stage = UpdateStage.UPDATING;
      fileUpdate.value.progress = 0;
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

    autoUpdate,
    resetAutoUpdateState,
    fetchAutoUpdateStatus,
    requestAutoUpdateCheck,
    setAutoUpdateBackgroundCheckInterval,
    clearAutoUpdateBackgroundCheckInterval,
    fetchAutoUpdateChangelog,
    startAutoUpdate,

    fileUpdate,
    uploadFirmware
  };
});
