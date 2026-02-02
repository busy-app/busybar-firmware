import type { Toast } from '@nuxt/ui/runtime/composables/useToast.js';

const _toast = useToast();
export const toast = {
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
