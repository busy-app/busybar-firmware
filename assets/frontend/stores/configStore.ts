import { defineStore } from 'pinia';

export const useConfigStore = defineStore('config', () => {
  const refreshDeviceDataAbortIfStreamActive = ref<boolean>(true);

  return {
    refreshDeviceDataAbortIfStreamActive
  };
}, {
  persist: {
    key: 'configStore',
    storage: piniaPluginPersistedstate.localStorage()
  }
});
