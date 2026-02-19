import { defineStore } from 'pinia';
import type { AudioVolumeInfo } from '@busy-app/busy-lib';

export const useAudioStore = defineStore('audio', () => {
  const deviceStore = useDeviceStore();

  const audio = ref<AudioVolumeInfo | undefined>(undefined);
  async function fetchAudioVolume (): Promise<AudioVolumeInfo | undefined> {
    const volume = await deviceStore.busyBar.AudioVolumeGet()
      .then(response => {
        audio.value = response;
        return response;
      })
      .catch(async error => {
        await handleHTTPError(error, 'Couldn\'t get audio volume', true);
        return audio.value;
      });

    return volume;
  }
  async function setAudioVolume (volume: number): Promise<boolean> {
    return await deviceStore.busyBar.AudioVolumeSet({ volume })
      .then(() => {
        if (audio.value) {
          audio.value.volume = volume;
        } else {
          audio.value = { volume };
        }
        return true;
      })
      .catch(async error => {
        await handleHTTPError(error, 'Couldn\'t set audio volume');
        return false;
      });
  }

  return {
    audio,
    fetchAudioVolume,
    setAudioVolume
  };
});
