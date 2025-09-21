import { defineStore } from 'pinia';

export interface TabOption {
  label: string;
  icon: string;
  activeIcon?: string;
  value: string;
}

export const useApiStore = defineStore('apiStore', () => {
  const barUrl = useRuntimeConfig().public.barUrl;
  const apiKey = ref<string | null>(null);

  type FetchArgs = Parameters<typeof $fetch>;
  type FetchRequest = FetchArgs[0];
  type FetchOptions = FetchArgs[1];

  async function apiRequest<T> (path: FetchRequest, opts?: FetchOptions): Promise<T> {
    const headers: Record<string, string> = {
      ...(opts?.headers as Record<string, string> || {})
    };

    if (apiKey.value) {
      headers['X-API-Key'] = apiKey.value;
    }

    return $fetch(path, {
      baseURL: barUrl,
      ...opts,
      headers
    }) as unknown as T;
  }

  return {
    apiKey,
    apiRequest
  };
}, {
  persist: {
    key: 'apiStore',
    storage: piniaPluginPersistedstate.localStorage()
  }
});
