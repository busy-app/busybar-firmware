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
      name: 'httpPollingInterval',
      label: 'HTTP polling interval (ms). Sound and brightness are polled 6x less often',
      type: 'number',
      value: 5000,
      default: 5000
    },
    {
      name: 'httpRequestTimeout',
      label: 'Default HTTP request timeout (ms). Long requests like file upload don\'t have a timeout (browser default applies)',
      type: 'number',
      value: 3000,
      default: 3000
    },
    {
      name: 'notificationDuration',
      label: 'Notification duration (ms). Error notifications never close automatically',
      type: 'number',
      value: 10000,
      default: 10000
    },
    {
      name: 'refreshDeviceDataAbortIfStreamActive',
      label: 'Abort HTTP polling attempt if stream is active',
      type: 'boolean',
      value: true,
      default: true
    },
    {
      name: 'screenStreamCanvasBaseResolutionWidth',
      label: 'Canvas resolution width. Gets multiplied by screen DPR for final resolution. Height is calculated based on screen aspect ratio',
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
    },
    {
      name: 'stateStreamDataTimeout',
      label: 'State stream data timeout (ms)',
      type: 'number',
      value: 1500,
      default: 1500
    },
    {
      name: 'stateStreamLogFrames',
      label: 'Log state stream frames to console (debug level)',
      type: 'boolean',
      value: false,
      default: false
    },
    {
      name: 'stateStreamLogHeartbeats',
      label: 'Log state stream heartbeats to console (debug level)',
      type: 'boolean',
      value: false,
      default: false
    },
    {
      name: 'stateStreamLogStatusUpdates',
      label: 'Log state stream status updates to console (debug level). This includes all status messages (main/connection/auth/data/worker)',
      type: 'boolean',
      value: true,
      default: true
    },
    {
      name: 'stateStreamLogUpdates',
      label: 'Log state stream updates to console (debug level). Disabling removes frames, heartbeats and state updates from console.debug',
      type: 'boolean',
      value: true,
      default: true
    },
    {
      name: 'stateStreamMaxReconnectAttempts',
      label: 'State stream max reconnect attempts',
      type: 'number',
      value: 5,
      default: 5
    },
    {
      name: 'stateStreamReconnectDelay',
      label: 'State stream reconnect delay (ms)',
      type: 'number',
      value: 250,
      default: 250
    },
    {
      name: 'stateStreamTimeout',
      label: 'State stream socket timeout (ms)',
      type: 'number',
      value: 5000,
      default: 5000
    },
    {
      name: 'wifiAbortSimultaneousRequests',
      label: 'Abort heavy WiFi HTTP request (list/connect/disconnect) if the same request is in progress',
      type: 'boolean',
      value: true,
      default: true
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
