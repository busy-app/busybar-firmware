import { defineStore } from 'pinia';

interface TelemetryState {
  enabled: boolean;
  asked: boolean;
}

const TELEMETRY_API_PATH = '/api/telemetry';

function isTelemetryState (value: unknown): value is TelemetryState {
  return typeof value === 'object' && value !== null
    && typeof (value as TelemetryState).enabled === 'boolean'
    && typeof (value as TelemetryState).asked === 'boolean';
}

export const useTelemetryStore = defineStore('telemetry', () => {
  const { apiRequest } = useApiStore();
  const configStore = useConfigStore();
  const wifiStore = useWifiStore();

  const isSupported = ref(false);
  const enabled = ref(false);
  const asked = ref(false);
  const saving = ref(false);
  const showConsentModal = ref(false);

  const isWifiConnected = computed(() => wifiStore.wifi?.state === 'connected');

  function applyState (state: TelemetryState) {
    enabled.value = state.enabled;
    asked.value = state.asked;
  }

  function showConsentIfNeeded () {
    if (isSupported.value && !asked.value && isWifiConnected.value) {
      showConsentModal.value = true;
    }
  }

  async function fetchTelemetry () {
    try {
      const response = await apiRequest<unknown>(TELEMETRY_API_PATH, {
        timeout: Number(configStore.get('httpRequestTimeout'))
      });

      if (!isTelemetryState(response)) {
        throw new Error('Unexpected telemetry response');
      }

      applyState(response);
      isSupported.value = true;
    } catch (error) {
      // Firmware without telemetry answers 404: hide the feature instead of showing an error
      isSupported.value = false;
      console.warn('Telemetry is not available:', error);
      return;
    }

    showConsentIfNeeded();
  }

  async function saveTelemetry (state: TelemetryState) {
    saving.value = true;

    try {
      const response = await apiRequest<unknown>(TELEMETRY_API_PATH, {
        method: 'PUT',
        body: state,
        timeout: Number(configStore.get('httpRequestTimeout'))
      });
      applyState(isTelemetryState(response) ? response : state);

      return true;
    } catch (error) {
      await handleHTTPError(error, 'Couldn\'t save device data sharing setting');

      return false;
    } finally {
      saving.value = false;
    }
  }

  function requestEnable () {
    showConsentModal.value = true;
  }

  async function answerConsent (share: boolean) {
    await saveTelemetry({ enabled: share, asked: true });
    showConsentModal.value = false;
  }

  async function disable () {
    await saveTelemetry({ enabled: false, asked: true });
  }

  watch(isWifiConnected, isConnected => {
    if (isConnected) {
      showConsentIfNeeded();
    }
  });

  return {
    isSupported,
    enabled,
    asked,
    saving,
    showConsentModal,
    fetchTelemetry,
    requestEnable,
    answerConsent,
    disable
  };
});
