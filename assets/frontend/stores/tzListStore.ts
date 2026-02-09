import { defineStore } from 'pinia';

export interface TimezoneOption {
  name: string;
  offset: string;
  abbr: string;
}

export const useTzListStore = defineStore('tzList', () => {
  const apiRequest = useApiStore().apiRequest;
  const timezoneOptions = ref<TimezoneOption[]>();

  async function fetchTimezoneOptions (): Promise<TimezoneOption[] | undefined> {
    return await apiRequest<TimezoneOption[]>('/api/time/tzlist')
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
    storage: piniaPluginPersistedstate.localStorage()
  }
});
