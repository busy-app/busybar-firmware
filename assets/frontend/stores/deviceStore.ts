import { defineStore } from 'pinia';
import { BusyBar } from '@busy-app/busy-lib';
import type {
  VersionInfo,
  Status as DeviceStatus,
  HttpAccessInfo,
  HttpAccessParams
} from '@busy-app/busy-lib';

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
        if (autoUpdate.value.stage === UpdateStage.UPDATING) {
          autoUpdate.value.stage = UpdateStage.SUCCESS;
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
        autoUpdate.value.stage !== UpdateStage.UPDATING
        && !(autoUpdate.value.stage === UpdateStage.SUCCESS && wifiStore.wifi?.state !== 'connected')
        && (fileUpdate.value.stage === 'idle' || fileUpdate.value.stage === 'error')
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
  }

  const refreshInterval = ref<NodeJS.Timeout>();
  async function refreshDeviceData () {
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
      updating: false
    },
    changelog: null as string | null,
    stage: UpdateStage.IDLE as UpdateStage,
    progress: 0,
    progressPollingInterval: null as NodeJS.Timeout | null,
    error: {
      stage: UpdateStage.IDLE as UpdateStage,
      message: null as string | null
    }
  });
  function resetAutoUpdateState () {
    autoUpdate.value.status = null;
    autoUpdate.value.availableVersion = null;
    autoUpdate.value.isAllowed = null;
    autoUpdate.value.changelog = null;
    autoUpdate.value.progress = 0;
    if (autoUpdate.value.progressPollingInterval) {
      clearInterval(autoUpdate.value.progressPollingInterval);
      autoUpdate.value.progressPollingInterval = null;
    }
    autoUpdate.value.error.stage = UpdateStage.IDLE;
    autoUpdate.value.error.message = null;
  }
  async function fetchAutoUpdateStatus (): Promise<void> {
    return apiRequest<UpdateStatus>('/api/update/status', { timeout: 10000 })
      .then(async status => {
        if (status.check.event === 'stop' && status.check.status === 'failure') {
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
    autoUpdate.value.isChecking = true;

    return apiRequest('/api/update/check', { method: 'POST', timeout: 10000 })
      .then(async () => {
        console.debug('Auto-update check requested');
        await new Promise(resolve => setTimeout(resolve, 1000));
        return fetchAutoUpdateStatus();
      })
      .catch(async error => {
        autoUpdate.value.isChecking = false;
        await handleHTTPError(error, 'Update check request failed');
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

  async function requestAutoUpdateInstallation () {
    if (!autoUpdate.value.availableVersion) {
      console.error('No available version to install');
      return;
    }
    console.debug('Requesting auto-update installation');

    return apiRequest(`/api/update/install?version=${autoUpdate.value.availableVersion}`, { method: 'POST', timeout: 10000 });
  }
  async function abortAutoUpdateDownload () {
    await apiRequest('/api/update/abort_download', { method: 'POST' })
      .then(() => {
        console.debug('Auto-update download abort requested');
        autoUpdate.value.modals.updating = false;
        autoUpdate.value.stage = UpdateStage.IDLE;
        autoUpdate.value.progress = 0;
      })
      .catch(async error => {
        await handleHTTPError(error, 'Couldn\'t abort update download');
      });
  }
  async function startAutoUpdate () {
    console.debug('Starting auto-update process');

    // enable loading before sending the request
    autoUpdate.value.stage = UpdateStage.UPLOADING;

    await requestAutoUpdateInstallation()
      .catch(async error => {
        await handleHTTPError(error, 'Update failed');
        return;
      });

    autoUpdate.value.progressPollingInterval = setInterval(async () => {
      await apiRequest<UpdateStatus>('/api/update/status', { timeout: 10000 })
        .then(status => {
          // handle session stop event
          if (status.install.event === UpdateEvent.SESSION_STOP) {
            if (autoUpdate.value.progressPollingInterval) {
              clearInterval(autoUpdate.value.progressPollingInterval);
              autoUpdate.value.progressPollingInterval = null;
            }

            if (status.install.status === UpdateStatusCode.OK) {
              // wait for device to complete the installation and reboot
              return;
            } else if (status.install.status === UpdateStatusCode.BUSY) {
              console.warn('Received session_stop event with status busy. Is this a firmware bug?');
              // ignore and wait for the device to reboot
              return;
            }

            // all other status codes indicate a failure
            autoUpdate.value.error.stage = autoUpdate.value.stage;
            autoUpdate.value.error.message = `Update failed with status: ${status.install.status}`;
            autoUpdate.value.stage = UpdateStage.ERROR;
            return;
          }

          if (status.install.status !== UpdateStatusCode.OK && status.install.status !== UpdateStatusCode.BUSY) {
            console.error('Update failed with status:', status);
            autoUpdate.value.error.stage = autoUpdate.value.stage;
            autoUpdate.value.error.message = `Update failed: ${status.install.status}`;
            autoUpdate.value.stage = UpdateStage.ERROR;
            clearInterval(autoUpdate.value.progressPollingInterval!);
            return;
          }

          if (status.install.action === UpdateAction.DOWNLOAD) {
            autoUpdate.value.stage = UpdateStage.UPLOADING;
            if (status.install.download.total_bytes > 0) {
              autoUpdate.value.progress = Math.round((status.install.download.received_bytes / status.install.download.total_bytes) * 100);
            }
          } else if (status.install.action !== UpdateAction.NONE) {
            autoUpdate.value.stage = UpdateStage.UPDATING;
            autoUpdate.value.progress = 0;
            clearInterval(autoUpdate.value.progressPollingInterval!);
          }
        })
        .catch(async error => {
          await handleHTTPError(error, 'Couldn\'t fetch update status');
          autoUpdate.value.stage = UpdateStage.ERROR;
          clearInterval(autoUpdate.value.progressPollingInterval!);
        });
    }, 1000);

    autoUpdate.value.modals.changelog = false;
    autoUpdate.value.modals.updating = true;
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
        // temp
        autoUpdate.value.progress = fileUpdate.value.progress;

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

    deviceStatus,
    fetchDeviceStatus,

    deviceName,
    fetchDeviceName,
    setDeviceName,

    httpAPIAccess,
    fetchHttpAPIAccess,
    setHttpAPIAccess,

    autoUpdate,
    resetAutoUpdateState,
    fetchAutoUpdateStatus,
    requestAutoUpdateCheck,
    setAutoUpdateBackgroundCheckInterval,
    clearAutoUpdateBackgroundCheckInterval,
    fetchAutoUpdateChangelog,
    abortAutoUpdateDownload,
    startAutoUpdate,

    fileUpdate,
    uploadFirmware
  };
});
