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

export const useAppsStore = defineStore('apps', () => {
  const deviceStore = useDeviceStore();

  const apps = ref<AppInfo[]>([]);
  const icons = ref<Record<string, string>>({});
  const loading = ref(false);

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
    await deviceStore.busyBar.AppsInstall({ install_key: installKey }, { timeout: 0 });
    await fetchApps();
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
    fetchApps,
    readIcon,
    stageApp,
    installApp,
    removeApp,
    readSettingsSchema,
    getSettings,
    setSettings
  };
});

function toDataUrl (blob: Blob): Promise<string> {
  return new Promise((resolve, reject) => {
    const reader = new FileReader();
    reader.onload = () => resolve(reader.result as string);
    reader.onerror = () => reject(reader.error);
    reader.readAsDataURL(blob);
  });
}
