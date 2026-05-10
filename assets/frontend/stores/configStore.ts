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
  const pinPopover = ref(false);

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
      label: 'Notification duration (ms). Error notifications never close automatically.',
      type: 'number',
      value: 10000,
      default: 10000
    },
    {
      name: 'httpRequestTimeout',
      label: 'Default HTTP request timeout (ms). Long requests like file upload don\'t have a timeout (browser default applies).',
      type: 'number',
      value: 3000,
      default: 3000
    },
    {
      name: 'screenStreamCanvasBaseResolutionWidth',
      label: 'Canvas resolution width. Gets multiplied by screen DPR for final resolution. Height is calculated based on screen aspect ratio.',
      type: 'number',
      value: 720,
      default: 720
    },
    {
      name: 'sliderDebounceDelay',
      label: 'Debounce delay for slider inputs (ms)',
      type: 'number',
      value: 250,
      default: 250
    }
  ]);

  function get (key: string): ConfigStoreItem['value'] | undefined {
    const item = items.value.find(i => i.name === key);
    return item ? item.value : undefined;
  }

  return {
    showConfigUI,
    pinPopover,

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
