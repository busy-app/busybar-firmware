import { defineStore } from 'pinia';
import type { AppInfo, AppSettingsDocument, AppStageResult } from '@busy-app/busy-lib';

export interface AppSettingsGeolocationValue {
  mode: 'auto' | 'fixed';
  name: string;
  lat?: number;
  lon?: number;
}

export type AppSettingsNode = {
  label: string;
  description?: string;
} & ({
  type: 'boolean';
  default: boolean;
} | {
  type: 'integer';
  default: number;
  min?: number;
  max?: number;
  step?: number;
} | {
  type: 'string';
  default: string;
  sensitive?: boolean;
  min_length?: number;
  max_length?: number;
} | {
  type: 'enum';
  default: string;
  options: { value: string; label: string }[];
} | {
  type: 'color';
  default: string;
} | {
  type: 'time';
  default: string;
} | {
  type: 'geolocation';
  default: AppSettingsGeolocationValue;
} | {
  type: 'group';
  fields: Record<string, AppSettingsNode>;
});

export interface AppSettingsSchema {
  format_version: number;
  version: number;
  fields: Record<string, AppSettingsNode>;
}

const APPS_PATH = '/ext/user_assets';
const SETTINGS_SCHEMA_PATH = 'appmeta/settings.json';
const ICON_EXTENSION = '.png';
const STORAGE_READ_PATH_MAX_LENGTH = 63;

function toDataUrl (blob: Blob): Promise<string> {
  return new Promise((resolve, reject) => {
    const reader = new FileReader();
    reader.onload = () => resolve(reader.result as string);
    reader.onerror = () => reject(reader.error);
    reader.readAsDataURL(blob);
  });
}

export const useAppsStore = defineStore('apps', () => {
  const deviceStore = useDeviceStore();

  const apps = ref<AppInfo[]>([]);
  const icons = ref<Record<string, string>>({});
  const loading = ref(false);
  // Only what this session started itself; the device has no way to report a running app.
  const runningAppId = ref<string>();
  // Keep the saved notice when an app's settings screen is reopened.
  const settingsSavedByApp = ref<Record<string, boolean>>({});

  async function fetchApps () {
    loading.value = true;

    try {
      const result = await deviceStore.busyBar.AppsList();
      const loadedIcons: Record<string, string> = {};

      for (const app of result.apps) {
        const icon = await readIcon(app.icon_path);
        if (icon) {
          loadedIcons[app.id] = icon;
        }
      }

      apps.value = result.apps;
      icons.value = loadedIcons;
    } catch (error) {
      await handleHTTPError(error, 'Couldn\'t load apps', true);
    } finally {
      loading.value = false;
    }
  }

  async function readIcon (iconPath: string) {
    if (!iconPath.endsWith(ICON_EXTENSION) || iconPath.length > STORAGE_READ_PATH_MAX_LENGTH) {
      return undefined;
    }

    try {
      const data = await deviceStore.busyBar.StorageRead({ path: iconPath }, { as_array_buffer: true });
      return await toDataUrl(new Blob([data], { type: 'image/png' }));
    } catch {
      return undefined;
    }
  }

  async function whileDeviceIsBusy<T> (action: () => Promise<T>): Promise<T> {
    const stateStreamStore = useStateStreamStore();
    const checkOnStale = stateStreamStore.doCheckConnectionOnStreamDataStale;

    stateStreamStore.doCheckConnectionOnStreamDataStale = false;
    deviceStore.pauseAvailabilityPolling();

    try {
      return await action();
    } finally {
      stateStreamStore.doCheckConnectionOnStreamDataStale = checkOnStale;
      deviceStore.resumeAvailabilityPolling();
    }
  }

  function stageApp (file: File, signal: AbortSignal, onProgress: (percent: number) => void): Promise<AppStageResult> {
    return whileDeviceIsBusy(() => new Promise<AppStageResult>((resolve, reject) => {
      const apiStore = useApiStore();
      const xhr = new XMLHttpRequest();

      xhr.open('POST', `${useRuntimeConfig().public.barUrl || window.location.origin}/api/apps/stage`);
      xhr.setRequestHeader('Content-Type', 'application/octet-stream');
      if (apiStore.apiKey) {
        xhr.setRequestHeader('X-API-Token', apiStore.apiKey);
      }
      xhr.responseType = 'json';

      xhr.upload.onprogress = event => {
        if (event.lengthComputable) {
          onProgress(Math.round((event.loaded / event.total) * 100));
        }
      };

      xhr.onload = () => {
        if (xhr.status >= 200 && xhr.status < 300) {
          resolve(xhr.response as AppStageResult);
        } else {
          reject(Object.assign(new Error(`App staging failed with status ${xhr.status}`), { status: xhr.status, data: xhr.response }));
        }
      };
      xhr.onerror = () => reject(new Error('App staging failed: network error'));
      xhr.onabort = () => reject(new DOMException('App staging aborted', 'AbortError'));

      signal.addEventListener('abort', () => xhr.abort(), { once: true });

      xhr.send(file);
    }));
  }

  function installApp (installKey: number) {
    return whileDeviceIsBusy(async () => {
      await deviceStore.busyBar.AppsInstall({ install_key: installKey }, { timeout: 0 });
      await fetchApps();
    });
  }

  async function launchApp (appId: string) {
    await useApiStore().apiRequest('/api/apps/launch', {
      method: 'POST',
      query: { app_id: appId }
    });

    if (runningAppId.value) {
      settingsSavedByApp.value[runningAppId.value] = false;
    }
    settingsSavedByApp.value[appId] = false;
    runningAppId.value = appId;
  }

  async function quitApp () {
    try {
      await useApiStore().apiRequest('/api/apps/quit', { method: 'POST' });
    } catch (error) {
      // 409 means nothing was running, which is the state we are after anyway
      if (httpErrorStatus(error) !== 409) {
        throw error;
      }
    }

    if (runningAppId.value) {
      settingsSavedByApp.value[runningAppId.value] = false;
    }
    runningAppId.value = undefined;
  }

  function restartApp (appId: string) {
    return launchApp(appId);
  }

  function removeApp (appId: string) {
    return whileDeviceIsBusy(async () => {
      await deviceStore.busyBar.AppsRemove({ app_id: appId }, { timeout: 0 });
      settingsSavedByApp.value[appId] = false;
      await fetchApps();
    });
  }

  async function readSettingsSchema (appId: string) {
    const data = await deviceStore.busyBar.StorageRead(
      { path: `${APPS_PATH}/${appId}/${SETTINGS_SCHEMA_PATH}` },
      { as_array_buffer: true }
    );

    return JSON.parse(new TextDecoder().decode(data as ArrayBuffer)) as AppSettingsSchema;
  }

  function getSettings (appId: string) {
    return deviceStore.busyBar.AppsSettingsGet({ app_id: appId });
  }

  async function setSettings (appId: string, settings: AppSettingsDocument) {
    const result = await deviceStore.busyBar.AppsSettingsSet({ app_id: appId, settings });
    settingsSavedByApp.value[appId] = true;
    return result;
  }

  return {
    apps,
    icons,
    loading,
    runningAppId,
    settingsSavedByApp,
    fetchApps,
    readIcon,
    stageApp,
    installApp,
    launchApp,
    quitApp,
    restartApp,
    removeApp,
    readSettingsSchema,
    getSettings,
    setSettings
  };
});
