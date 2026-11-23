import { defineStore } from 'pinia';

export const useApiStore = defineStore('apiStore', () => {
  const apiKey = ref<string | null>(null);

  return {
    apiKey
  };
}, {
  persist: {
    key: 'apiStore',
    storage: piniaPluginPersistedstate.sessionStorage()
  }
});
