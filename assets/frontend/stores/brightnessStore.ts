import { defineStore } from 'pinia';
import type { DisplayBrightnessParams } from '@busy-app/busy-lib';

export const useBrightnessStore = defineStore('brightness', () => {
  const { busyBar } = useDeviceStore();

  const displayBrightness = ref<DisplayBrightnessParams | undefined>(undefined);
  async function fetchDisplayBrightness (): Promise<DisplayBrightnessParams | undefined> {
    const brightness = await busyBar.DisplayBrightness()
      .then(response => {
        const frontParsed = response.front === 'auto' ? 'auto' : Number(response.front);
        const backParsed = response.back === 'auto' ? 'auto' : Number(response.back);
        const result = { front: frontParsed, back: backParsed } as DisplayBrightnessParams;
        displayBrightness.value = result;
        return result;
      })
      .catch(async error => {
        await handleHTTPError(error, 'Couldn\'t get display brightness', true);
        return displayBrightness.value;
      });

    return brightness;
  }
  async function setDisplayBrightness (brightness: DisplayBrightnessParams): Promise<boolean> {
    return await busyBar.DisplayBrightnessSet(brightness)
      .then(() => {
        displayBrightness.value = brightness;
        return true;
      })
      .catch(async error => {
        await handleHTTPError(error, 'Couldn\'t set display brightness');
        return false;
      });
  }

  return {
    displayBrightness,
    fetchDisplayBrightness,
    setDisplayBrightness
  };
});
