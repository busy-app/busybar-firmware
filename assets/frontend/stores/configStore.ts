import { defineStore } from 'pinia';

export type ConfigStoreItem = {
  name: string;
  label: string;
  type: 'boolean';
  value: boolean;
  default: boolean;
} | {
  name: string;
  label: string;
  type: 'string';
  value: string;
  default: string;
} | {
  name: string;
  label: string;
  type: 'number';
  value: number;
  default: number;
};

export const useConfigStore = defineStore('config', () => {
  const showConfigUI = ref<boolean | undefined>(false);
  const openPopoverOnPageLoad = ref(false);

  const items = ref<ConfigStoreItem[]>([
    {
      name: 'refreshDeviceDataAbortIfStreamActive',
      label: 'Abort device data refresh if stream is active',
      type: 'boolean',
      value: true,
      default: true
    },
    {
      name: 'httpPollingInterval',
      label: 'HTTP polling interval (ms)',
      type: 'number',
      value: 5000,
      default: 5000
    },
    {
      name: 'notificationDuration',
      label: 'Default notification duration (ms)',
      type: 'number',
      value: 10000,
      default: 10000
    },
    {
      name: 'httpRequestTimeout',
      label: 'Default HTTP request timeout (ms)',
      type: 'number',
      value: 3000,
      default: 3000
    }
  ]);

  function get (key: string): ConfigStoreItem['value'] | undefined {
    const item = items.value.find(i => i.name === key);
    return item ? item.value : undefined;
  }

  return {
    showConfigUI,
    openPopoverOnPageLoad,

    items,
    get
  };
}, {
  persist: {
    key: 'configStore',
    storage: piniaPluginPersistedstate.localStorage(),
    omit: ['items'] // items are saved manually in ConfigStoreCard
  }
});
