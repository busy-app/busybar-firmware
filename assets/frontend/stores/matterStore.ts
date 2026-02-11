import { defineStore } from 'pinia';
import encodeQR from 'qr';

export const useMatterStore = defineStore('matter', () => {
  const { apiRequest } = useApiStore();

  const matterCommissioning = ref({
    fabricCount: 0,
    latestStatus: ''
  });
  const matterLink = ref({
    qrCode: '',
    manualCode: '',
    availableUntil: null as Date | null,

    showModal: false,
    expiresInMs: 0,
    timeout: null as NodeJS.Timeout | null
  });
  async function fetchMatterCommissioning (): Promise<void> {
    await apiRequest<{ fabric_count: number; latest_status: string }>('/api/matter/commissioning')
      .then(response => {
        matterCommissioning.value.fabricCount = response.fabric_count;
        matterCommissioning.value.latestStatus = response.latest_status;
      })
      .catch(async error => {
        await handleHTTPError(error, 'Couldn\'t get Matter commissioning status', true);
      });
  }
  async function requestMatterLink (): Promise<void> {
    await apiRequest<{ qr_code: string; manual_code: string; available_until: number }>('/api/matter/commissioning', { method: 'POST' })
      .then(response => {
        matterLink.value.manualCode = response.manual_code;
        matterLink.value.availableUntil = new Date(Number(response.available_until));

        if (matterLink.value.timeout) {
          clearTimeout(matterLink.value.timeout);
        }
        matterLink.value.expiresInMs = matterLink.value.availableUntil.getTime() - Date.now();

        const svgElement = encodeQR(matterLink.value.qrCode, 'svg');
        matterLink.value.qrCode = svgElement;

        matterLink.value.timeout = setTimeout(() => {
          matterLink.value.showModal = false;
          matterLink.value.qrCode = '';
          matterLink.value.manualCode = '';
          matterLink.value.availableUntil = null;
          matterLink.value.timeout = null;
          console.debug('Matter commissioning link expired');
        }, matterLink.value.expiresInMs);

        matterLink.value.showModal = true;
      })
      .catch(async error => {
        await handleHTTPError(error, 'Couldn\'t request Matter commissioning link');
      });
  }
  async function deleteAllPairings (): Promise<void> {
    await apiRequest('/api/matter/commissioning', { method: 'DELETE' })
      .then(() => {
        console.debug('All Matter pairings deleted, waiting for device to reboot');
      })
      .catch(async error => {
        await handleHTTPError(error, 'Couldn\'t delete pairings');
      });
  }

  return {
    matterCommissioning,
    matterLink,
    fetchMatterCommissioning,
    requestMatterLink,
    deleteAllPairings
  };
});
