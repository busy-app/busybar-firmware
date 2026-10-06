import { defineStore } from 'pinia';
import type { TimezoneInfo } from '@busy-app/busy-lib';

export const useTimezoneStore = defineStore('timezone', () => {
  const deviceStore = useDeviceStore();

  const timezone = ref<string | undefined>(undefined);
  async function fetchTimezone (): Promise<TimezoneInfo | undefined> {
    const tz = await deviceStore.busyBar.TimeTimezoneGet()
      .then(response => {
        timezone.value = response.name;
        return response;
      })
      .catch(async error => {
        await handleHTTPError(error, 'Couldn\'t get timezone', true);
        return undefined;
      });

    return tz;
  }
  async function setTimezone (tz: string): Promise<boolean> {
    return await deviceStore.busyBar.TimeTimezoneSet({ timezone: tz })
      .then(() => {
        timezone.value = tz;
        return true;
      })
      .catch(async error => {
        await handleHTTPError(error, 'Couldn\'t set timezone');
        return false;
      });
  }

  return {
    timezone,
    fetchTimezone,
    setTimezone
  };
});
