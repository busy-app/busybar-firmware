import { defineStore } from 'pinia';

export const useTimezoneStore = defineStore('timezone', () => {
  const { apiRequest } = useApiStore();

  const timezone = ref<string | undefined>(undefined);
  async function fetchTimezone (): Promise<string | undefined> {
    const tz = await apiRequest<{ timezone: string }>('/api/time/timezone')
      .then(response => {
        timezone.value = response.timezone;
        return response.timezone;
      })
      .catch(async error => {
        await handleHTTPError(error, 'Couldn\'t get timezone', true);
        return timezone.value;
      });

    return tz;
  }
  async function setTimezone (tz: string): Promise<boolean> {
    return await apiRequest('/api/time/timezone', { method: 'POST', query: { timezone: tz } })
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
