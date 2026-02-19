import { defineStore } from 'pinia';
import type { DisplayBrightnessParams } from '@busy-app/busy-lib';

export const useBrightnessStore = defineStore('brightness', () => {
  const deviceStore = useDeviceStore();

  const displayBrightness = ref<DisplayBrightnessParams | undefined>(undefined);
  async function fetchDisplayBrightness (): Promise<DisplayBrightnessParams | undefined> {
    const brightness = await deviceStore.busyBar.DisplayBrightnessGet()
      .then(response => {
        const valueParsed = response.value === 'auto' ? 'auto' : Number(response.value);
        const result = { value: valueParsed } as DisplayBrightnessParams;
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
    return await deviceStore.busyBar.DisplayBrightnessSet(brightness)
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
