import { defineStore } from 'pinia';
import type { AppInfo, AppSettingsDocument, AppStageResult } from '@busy-app/busy-lib';

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
    if (!iconPath.endsWith(ICON_EXTENSION)) {
      return undefined;
    }

    try {
      const data = await deviceStore.busyBar.StorageRead({ path: iconPath }, { as_array_buffer: true });
      return await toDataUrl(new Blob([data], { type: 'image/png' }));
    } catch {
      return undefined;
    }
  }

  function stageApp (file: File, signal: AbortSignal): Promise<AppStageResult> {
    return deviceStore.busyBar.AppsStage({ file }, { signal, timeout: 0 });
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
