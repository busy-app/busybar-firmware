import { defineStore } from 'pinia';
import type { ProcessedFrame } from '@busy-app/busy-lib';

export const useScreenStreamStore = defineStore('screenStream', () => {
  const currentFrame = ref<ProcessedFrame | null>(null);

  return {
    currentFrame
  };
}, {
  persist: false
});
