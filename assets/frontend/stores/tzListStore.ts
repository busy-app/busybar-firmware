import { defineStore } from 'pinia';
import type { TimezoneItem } from '@busy-app/busy-lib';

export const useTzListStore = defineStore('tzList', () => {
  const deviceStore = useDeviceStore();
  const timezoneOptions = ref<TimezoneItem[]>();

  async function fetchTimezoneOptions (): Promise<TimezoneItem[] | undefined> {
    return await deviceStore.busyBar.TimeTzListGet()
      .then(data => {
        timezoneOptions.value = data;
        return data;
      })
      .catch(async error => {
        await handleHTTPError(error, 'Couldn\'t get timezone options', true);
        return timezoneOptions.value;
      });
  }

  return {
    timezoneOptions,
    fetchTimezoneOptions
  };
}, {
  persist: {
    key: 'tzListStore',
    storage: piniaPluginPersistedstate.sessionStorage()
  }
});
