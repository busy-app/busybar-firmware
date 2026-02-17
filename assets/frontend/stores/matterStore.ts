import { defineStore } from 'pinia';
import type { MatterStatus } from '@busy-app/busy-lib';
import encodeQR from 'qr';

export const useMatterStore = defineStore('matter', () => {
  const deviceStore = useDeviceStore();

  const matterCommissioning = ref({
    fabricCount: 0,
    latestStatus: undefined as MatterStatus['latest_commissioning_status']
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
    await deviceStore.busyBar.MatterStatusGet()
      .then(response => {
        matterCommissioning.value.fabricCount = response.fabric_count || 0;
        matterCommissioning.value.latestStatus = response.latest_commissioning_status;
      })
      .catch(async error => {
        await handleHTTPError(error, 'Couldn\'t get Matter commissioning status', true);
      });
  }
  async function requestMatterLink (): Promise<void> {
    await deviceStore.busyBar.MatterPair()
      .then(response => {
        matterLink.value.manualCode = response.manual_code || '';
        matterLink.value.availableUntil = new Date(Number(response.available_until));

        if (matterLink.value.timeout) {
          clearTimeout(matterLink.value.timeout);
        }
        matterLink.value.expiresInMs = matterLink.value.availableUntil.getTime() - Date.now();

        const svgElement = encodeQR(response.qr_code!, 'svg');
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
    await deviceStore.busyBar.MatterErase()
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
