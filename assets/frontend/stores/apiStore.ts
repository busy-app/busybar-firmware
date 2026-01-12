import type { Toast } from '@nuxt/ui/runtime/composables/useToast.js';
import { defineStore } from 'pinia';

export const useApiStore = defineStore('apiStore', () => {
  const barUrl = useRuntimeConfig().public.barUrl;
  const apiKey = ref<string | null>(null);
  const _toast = useToast();

  type FetchArgs = Parameters<typeof $fetch>;
  type FetchRequest = FetchArgs[0];
  type FetchOptions = FetchArgs[1];

  async function apiRequest<T> (path: FetchRequest, opts?: FetchOptions): Promise<T> {
    const headers: Record<string, string> = {
      ...(opts?.headers as Record<string, string> || {})
    };

    if (apiKey.value) {
      headers['X-API-Token'] = apiKey.value;
    }

    return $fetch(path, {
      baseURL: barUrl,
      ...opts,
      headers
    }) as unknown as T;
  }

  // drop-in replacement for toast.add that updates existing toasts with the same ID instead of adding a new one
  const toast = {
    ..._toast,
    add: (params: Parameters<typeof _toast.add>[0]): Toast => {
      const existingToast = _toast.toasts.value.find(t => t.id === params.id);
      if (existingToast) {
        _toast.update(params.id as string, params);
        return existingToast;
      } else {
        return _toast.add(params);
      }
    }
  };

  return {
    apiKey,
    apiRequest,

    toast
  };
}, {
  persist: {
    key: 'apiStore',
    storage: piniaPluginPersistedstate.localStorage()
  }
});
