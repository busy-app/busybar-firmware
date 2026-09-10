import { defineStore } from 'pinia';

interface TelemetryStatus {
  enabled: boolean;
}

export const useTelemetryStore = defineStore('telemetry', () => {
  const apiRequest = useApiStore().apiRequest;

  const enabled = ref<boolean | undefined>(undefined);

  async function fetchTelemetry () {
    try {
      const response = await apiRequest<TelemetryStatus>('/api/telemetry');
      enabled.value = response.enabled;

      return response.enabled;
    } catch (error) {
      // eslint-disable-next-line @typescript-eslint/no-explicit-any
      const e = error as any;
      if (e?.status === 404) {
        return undefined;
      }

      await handleHTTPError(e, 'Couldn\'t get telemetry state', true);
      return enabled.value;
    }
  }

  async function setTelemetry (value: boolean) {
    try {
      const response = await apiRequest<TelemetryStatus>('/api/telemetry', {
        method: 'PUT',
        body: { enabled: value }
      });

      enabled.value = response.enabled;
      return true;
    } catch (error) {
      // eslint-disable-next-line @typescript-eslint/no-explicit-any
      const e = error as any;
      await handleHTTPError(e, 'Couldn\'t set telemetry state');
      return false;
    }
  }

  return {
    enabled,
    fetchTelemetry,
    setTelemetry
  };
});
