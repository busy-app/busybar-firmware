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
const PROBE_APP_ID = 'app.probe.none';
const PROBE_ELEMENT_ID = 'probe.absent';
const SHOW_GRACE_MS = 4000;

export const useAppsStore = defineStore('apps', () => {
  const deviceStore = useDeviceStore();

  const apps = ref<AppInfo[]>([]);
  const icons = ref<Record<string, string>>({});
  const loading = ref(false);
  const runningAppId = ref<string>();
  let runningAppEpoch = 0;
  let showGraceUntil = 0;
  let refreshInFlight = false;

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

  function stageApp (file: File, signal: AbortSignal, onProgress: (percent: number) => void): Promise<AppStageResult> {
    return new Promise((resolve, reject) => {
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
    });
  }

  async function installApp (installKey: number) {
    const stateStreamStore = useStateStreamStore();
    const checkOnStale = stateStreamStore.doCheckConnectionOnStreamDataStale;
    stateStreamStore.doCheckConnectionOnStreamDataStale = false;
    deviceStore.pauseAvailabilityPolling();

    try {
      await deviceStore.busyBar.AppsInstall({ install_key: installKey }, { timeout: 0 });
      await fetchApps();
    } finally {
      stateStreamStore.doCheckConnectionOnStreamDataStale = checkOnStale;
      deviceStore.resumeAvailabilityPolling();
    }
  }

  async function refreshRunningApp (appId: string) {
    if (refreshInFlight) {
      return;
    }

    refreshInFlight = true;
    const epoch = runningAppEpoch;

    try {
      const showing = await isAppOnDisplay(appId);

      if (epoch !== runningAppEpoch || showing === null) {
        return;
      }

      if (showing) {
        showGraceUntil = 0;
        runningAppId.value = appId;
        return;
      }

      if (runningAppId.value === appId && Date.now() < showGraceUntil) {
        return;
      }

      runningAppId.value = undefined;
    } finally {
      refreshInFlight = false;
    }
  }

  async function launchApp (appId: string) {
    runningAppEpoch += 1;

    await useApiStore().apiRequest('/api/apps/launch', {
      method: 'POST',
      query: { app_id: appId }
    });
    runningAppId.value = appId;
    showGraceUntil = Date.now() + SHOW_GRACE_MS;
  }

  async function quitApp () {
    runningAppEpoch += 1;

    try {
      await useApiStore().apiRequest('/api/apps/quit', { method: 'POST' });
    } catch (error) {
      // 409 means nothing was running, which is the state we are after anyway
      if (httpErrorStatus(error) !== 409) {
        throw error;
      }
    }

    runningAppId.value = undefined;
    showGraceUntil = 0;
  }

  async function restartApp (appId: string) {
    await quitApp();
    await launchApp(appId);
  }

  async function removeApp (appId: string) {
    await deviceStore.busyBar.AppsRemove({ app_id: appId }, { timeout: 0 });
    await fetchApps();
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

  function setSettings (appId: string, settings: AppSettingsDocument) {
    return deviceStore.busyBar.AppsSettingsSet({ app_id: appId, settings });
  }

  return {
    apps,
    icons,
    loading,
    runningAppId,
    fetchApps,
    readIcon,
    stageApp,
    installApp,
    refreshRunningApp,
    launchApp,
    quitApp,
    restartApp,
    removeApp,
    readSettingsSchema,
    getSettings,
    setSettings
  };
});

async function isAppOnDisplay (appId: string) {
  const canvas = await probeCanvas(PROBE_APP_ID);

  if (canvas === null) {
    return null;
  }

  if (canvas === 'idle') {
    return false;
  }

  const app = await probeCanvas(appId);
  return app === null ? null : app === 'idle';
}

async function probeCanvas (applicationName: string) {
  try {
    await useDeviceStore().busyBar.DisplayClear({
      application_name: applicationName,
      element_ids: [PROBE_ELEMENT_ID]
    });
    return 'idle';
  } catch (error) {
    const message = error instanceof Error ? error.message : '';

    if (message.includes('non-existent')) {
      return 'idle';
    }

    if (message.includes('not displaying anything')) {
      return 'foreign';
    }

    return null;
  }
}

function toDataUrl (blob: Blob): Promise<string> {
  return new Promise((resolve, reject) => {
    const reader = new FileReader();
    reader.onload = () => resolve(reader.result as string);
    reader.onerror = () => reject(reader.error);
    reader.readAsDataURL(blob);
  });
}
