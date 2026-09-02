import { defineStore } from 'pinia';
import type { UpdateStatus } from '@busy-app/busy-lib';

type UpdateStatusCheckResult = NonNullable<UpdateStatus['check']>['status'];

export enum UpdateStage {
  IDLE,
  LOADING,
  UPDATING,
  ERROR,
  SUCCESS
}
export interface AutoUpdateSelfCheckState {
  is_enabled: boolean;
  interval_start: string; // "HH:mm" format
  interval_end: string; // "HH:mm" format
}

export const useFirmwareStore = defineStore('firmware', () => {
  const deviceStore = useDeviceStore();
  const configStore = useConfigStore();

  const BACKGROUND_AUTO_UPDATE_CHECK_INTERVAL_MS = 30 * 60 * 1000; // 30 minutes
  const autoUpdate = ref({
    status: null as UpdateStatusCheckResult | null,
    availableVersion: null as string | null,
    isAllowed: null as boolean | null,

    isChecking: false,
    isManualCheck: false,
    backgroundCheckInterval: null as NodeJS.Timeout | null,

    modals: {
      changelog: false,
      batteryLow: false,
      updating: false,
      success: false
    },
    changelog: null as string | null,
    isChangelogLoading: false,
    stage: UpdateStage.IDLE as UpdateStage,
    progress: 0,
    progressPollingInterval: null as NodeJS.Timeout | null,
    error: {
      stage: UpdateStage.IDLE as UpdateStage,
      message: null as string | null
    }
  });

  const autoUpdateSelfCheck = ref<AutoUpdateSelfCheckState>({ is_enabled: false, interval_start: '02:00', interval_end: '05:00' });
  async function fetchAutoUpdateSelfCheck () {
    return useApiStore().apiRequest<AutoUpdateSelfCheckState>('/api/update/autoupdate')
      .then(response => {
        autoUpdateSelfCheck.value = {
          is_enabled: response.is_enabled,
          interval_start: response.interval_start,
          interval_end: response.interval_end
        };

        return autoUpdateSelfCheck.value;
      })
      .catch(async error => {
        await handleHTTPError(error, 'Couldn\'t fetch auto-update self-check settings');
      });
  }
  async function setAutoUpdateSelfCheck (payload: AutoUpdateSelfCheckState) {
    return useApiStore().apiRequest('/api/update/autoupdate', {
      method: 'POST',
      body: payload
    })
      .then(() => {
        autoUpdateSelfCheck.value = payload;
      })
      .catch(async error => {
        await handleHTTPError(error, 'Couldn\'t update auto-update self-check settings');
      });
  }

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
  async function fetchAutoUpdateStatus (attempt: number = 0): Promise<void> {
    return deviceStore.busyBar.UpdateStatusGet()
      .then(async status => {
        if (!status.check?.status || !status.check.event) {
          throw new Error('Invalid update status response: missing check info');
        }
        if (!status.install) {
          throw new Error('Invalid update status response: missing install info');
        }

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
              duration: 0,
              close: true,
              closeIcon: 'i-bi-cross'
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
          if (attempt >= 10) {
            throw new Error('Auto-update check is taking too long, please try again later');
          }
          return fetchAutoUpdateStatus(attempt ? attempt + 1 : 1);
        }
        autoUpdate.value.isChecking = false;

        autoUpdate.value.status = status.check.status || null;
        autoUpdate.value.availableVersion = status.check.available_version || null;
        autoUpdate.value.isAllowed = !!status.install.is_allowed;

        if (autoUpdate.value.isManualCheck && status.check.status === 'not_available') {
          autoUpdate.value.isManualCheck = false;
          toast.add({
            title: 'Your firmware version is up to date',
            icon: 'i-bi-checkmark-circle-fill',
            color: 'success',
            duration: Number(configStore.get('notificationDuration'))
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

    return deviceStore.busyBar.UpdateCheck()
      .then(async () => {
        console.debug('Auto-update check requested');
        await new Promise(resolve => setTimeout(resolve, 1000));
        return fetchAutoUpdateStatus();
      })
      .catch(async error => {
        if (error.status === 409) {
          console.debug('Auto-update check already in progress');
          return;
        }
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
    autoUpdate.value.isChangelogLoading = true;
    await deviceStore.busyBar.UpdateChangelogGet({ version })
      .then(response => {
        autoUpdate.value.changelog = response.changelog || null;
      })
      .catch(async error => {
        await handleHTTPError(error, 'Couldn\'t fetch update changelog');
        return null;
      })
      .finally(() => {
        autoUpdate.value.isChangelogLoading = false;
      });
  }

  async function requestAutoUpdateInstallation () {
    if (!autoUpdate.value.availableVersion) {
      console.error('No available version to install');
      return;
    }
    console.debug('Requesting auto-update installation');

    return deviceStore.busyBar.UpdateInstall({ version: autoUpdate.value.availableVersion }, { timeout: 0 });
  }
  async function abortAutoUpdateDownload () {
    await deviceStore.busyBar.UpdateAbort()
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
    autoUpdate.value.progress = 0;
    autoUpdate.value.error.stage = UpdateStage.IDLE;
    autoUpdate.value.error.message = null;

    // enable loading before sending the request
    autoUpdate.value.stage = UpdateStage.LOADING;

    await requestAutoUpdateInstallation()
      .catch(async error => {
        await handleHTTPError(error, 'Update failed');
        return;
      });

    autoUpdate.value.progressPollingInterval = setInterval(async () => {
      await deviceStore.busyBar.UpdateStatusGet()
        .then(status => {
          if (!status.install) {
            throw new Error('Invalid update status response: missing install info');
          }

          // handle session stop event
          if (status.install.event === 'session_stop') {
            if (autoUpdate.value.progressPollingInterval) {
              clearInterval(autoUpdate.value.progressPollingInterval);
              autoUpdate.value.progressPollingInterval = null;
            }

            if (status.install.status === 'ok') {
              // wait for device to complete the installation and reboot
              return;
            } else if (status.install.status === 'busy') {
              console.warn('Received session_stop event with status busy. Is this a firmware bug?');
              // ignore and wait for the device to reboot
              return;
            } else if (status.install.status === 'download_abort') {
              console.warn('Update download was aborted');
              autoUpdate.value.modals.updating = false;
              autoUpdate.value.stage = UpdateStage.IDLE;
              autoUpdate.value.progress = 0;
              toast.add({
                title: 'Update aborted',
                description: 'The update download has been aborted.',
                icon: 'i-bi-alert',
                color: 'error',
                duration: 0,
                close: true,
                closeIcon: 'i-bi-cross'
              });
              return;
            }

            // all other status codes indicate a failure
            autoUpdate.value.error.stage = autoUpdate.value.stage;
            autoUpdate.value.error.message = `Update failed with status: ${status.install.status}`;
            autoUpdate.value.stage = UpdateStage.ERROR;
            return;
          }

          if (status.install.status !== 'ok' && status.install.status !== 'busy') {
            console.error('Update failed with status:', status);
            autoUpdate.value.error.stage = autoUpdate.value.stage;
            autoUpdate.value.error.message = `Update failed: ${status.install.status}`;
            autoUpdate.value.stage = UpdateStage.ERROR;
            clearInterval(autoUpdate.value.progressPollingInterval!);
            return;
          }

          if (status.install.action === 'download') {
            let totalBytes = Number(status.install.download?.total_bytes);
            if (isNaN(totalBytes)) {
              console.warn('Received invalid total_bytes value in update status, defaulting to 0', status.install.download?.total_bytes);
              totalBytes = 0;
            }
            let receivedBytes = Number(status.install.download?.received_bytes);
            if (isNaN(receivedBytes)) {
              console.warn('Received invalid received_bytes value in update status, defaulting to 0', status.install.download?.received_bytes);
              receivedBytes = 0;
            }

            autoUpdate.value.stage = UpdateStage.LOADING;
            if (totalBytes > 0) {
              autoUpdate.value.progress = Math.round((receivedBytes / totalBytes) * 100);
            }
          } else if (status.install.action !== 'none') {
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
    showFileUploadModal: false,
    stage: UpdateStage.IDLE as UpdateStage,
    progress: 0,
    error: null as string | null
  });
  async function uploadFirmware () {
    const xhr = new XMLHttpRequest();
    xhr.open('POST', `${useRuntimeConfig().public.barUrl || window.location.origin}/api/update`);
    xhr.setRequestHeader('Content-Type', 'application/octet-stream');
    if (useApiStore().apiKey) {
      xhr.setRequestHeader('X-API-Token', useApiStore().apiKey!);
    }

    xhr.upload.onprogress = event => {
      if (event.lengthComputable) {
        fileUpdate.value.progress = Math.round((event.loaded / event.total) * 100);
        autoUpdate.value.progress = fileUpdate.value.progress;

        if (fileUpdate.value.progress === 100) {
          console.debug('Firmware file upload completed, waiting for device to unpack');
        }
      }
    };

    xhr.onload = () => {
      if (xhr.status >= 200 && xhr.status < 400) {
        console.debug('Upload and unpacking complete, waiting for device to reboot');

        fileUpdate.value.stage = UpdateStage.UPDATING;
        toast.add({
          title: 'Update initiated',
          description: 'The device will reboot to apply the update. Pay attention to the front screen.',
          icon: 'i-bi-checkmark-circle-fill',
          color: 'success',
          duration: Number(configStore.get('notificationDuration'))
        });
      } else {
        console.error('Upload failed:', xhr.status, xhr.responseText);
        fileUpdate.value.stage = UpdateStage.ERROR;
        toast.add({
          title: 'Update failed',
          description: `Error ${xhr.status}: ${xhr.responseText}`,
          icon: 'i-bi-alert',
          color: 'error',
          duration: 0,
          close: true,
          closeIcon: 'i-bi-cross'
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
        duration: 0,
        close: true,
        closeIcon: 'i-bi-cross'
      });
      fileUpdate.value.error = 'An error occurred during the upload.';
    };

    fileUpdate.value.stage = UpdateStage.LOADING;
    fileUpdate.value.progress = 0;
    xhr.send(fileUpdate.value.firmwareFile);

    await new Promise<void>(resolve => {
      xhr.onloadend = () => {
        resolve();
      };
    });

    fileUpdate.value.firmwareFile = null;
    if (fileUpdate.value.stage as UpdateStage !== UpdateStage.ERROR) {
      fileUpdate.value.progress = 0;
    }
  }

  function completeSuccessfulUpdate () {
    localStorage.setItem('successfulUpdate', 'true');
    window.location.reload();
  }

  watch(
    () => [autoUpdate.value.stage, fileUpdate.value.stage],
    ([autoStage, fileStage], [prevAutoStage, prevFileStage]) => {
      const justSucceeded
        = (autoStage === UpdateStage.SUCCESS && prevAutoStage !== UpdateStage.SUCCESS)
          || (fileStage === UpdateStage.SUCCESS && prevFileStage !== UpdateStage.SUCCESS);

      if (justSucceeded) {
        completeSuccessfulUpdate();
      }
    }
  );

  async function startFirmwareUpdateFromFile () {
    try {
      fileUpdate.value.showFileUploadModal = false;
      autoUpdate.value.modals.updating = true;

      await uploadFirmware();
      if (fileUpdate.value.stage !== UpdateStage.ERROR) {
        fileUpdate.value.stage = UpdateStage.UPDATING;
      }
    } catch (error) {
      console.error('Firmware update failed:', error);
      fileUpdate.value.stage = UpdateStage.ERROR;
      fileUpdate.value.error = error instanceof Error ? error.message : 'Unknown error';
    }
  }

  return {
    autoUpdateSelfCheck,
    fetchAutoUpdateSelfCheck,
    setAutoUpdateSelfCheck,

    autoUpdate,

    resetAutoUpdateState,
    fetchAutoUpdateStatus,
    requestAutoUpdateCheck,
    setAutoUpdateBackgroundCheckInterval,
    clearAutoUpdateBackgroundCheckInterval,
    startAutoUpdate,
    abortAutoUpdateDownload,

    fileUpdate,
    uploadFirmware,
    startFirmwareUpdateFromFile
  };
});
