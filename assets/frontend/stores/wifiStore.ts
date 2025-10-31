import { defineStore } from 'pinia';
import { BusyBar } from '@busy-app/busy-lib';
import type { WifiStatusResponse as WifiState, WifiNetwork, WifiConnectParams } from '@busy-app/busy-lib';

export const useWifiStore = defineStore('wifi', () => {
  const toast = useToast();

  const busyBar = new BusyBar({
    host: useRuntimeConfig().public.barUrl
  });

  const wifi = ref<WifiState | undefined>(undefined);

  async function fetchWifiState (): Promise<WifiState | undefined> {
    const state = await busyBar.statusWifi()
      .then(response => {
        if (!response || typeof response !== 'object') {
          throw new Error('Empty response');
        }
        return response;
      })
      .catch(error => {
        console.error('Error fetching WiFi state:', error);
        toast.add({
          id: 'wifi-status-error',
          title: 'Failed to fetch WiFi state',
          description: error.data?.error || genericErrorMessage,
          icon: 'i-ri-alert-line',
          color: 'error',
          duration: 10000
        });
        return undefined;
      });

    return state;
  }

  async function getWifiState (): Promise<WifiState | undefined> {
    if (wifi.value === undefined) {
      wifi.value = await fetchWifiState();
    }
    return wifi.value;
  }

  async function enableWifi () {
    await busyBar.enableWifi()
      .then(() => {
        wifi.value = { state: 'enabled' };
      })
      .catch(error => {
        console.error('Error enabling WiFi:', error);
        toast.add({
          id: 'wifi-enable-error',
          title: 'Failed to enable WiFi',
          description: error.data?.error || genericErrorMessage,
          icon: 'i-ri-alert-line',
          color: 'error',
          duration: 10000
        });
      });
  }

  async function disableWifi () {
    if (wifi.value === undefined || wifi.value.state === 'disabled') {
      return;
    }

    await busyBar.disableWifi()
      .then(() => {
        wifi.value = { state: 'disabled' };
      })
      .catch(error => {
        console.error('Error disabling WiFi:', error);
        toast.add({
          id: 'wifi-disable-error',
          title: 'Failed to disable WiFi',
          description: error.data?.error || genericErrorMessage,
          icon: 'i-ri-alert-line',
          color: 'error',
          duration: 10000
        });
      });
  }

  async function listWifiNetworks () {
    return await busyBar.networksWifi()
      .then(response => {
        if (!response || !Array.isArray(response.networks)) {
          throw new Error('Failed to fetch WiFi networks');
        }
        // dedupe networks by SSID, keeping the one with the highest signal level
        response.networks = response.networks.reduce<WifiNetwork[]>((acc, curr) => {
          const existing = acc.find(n => n.ssid === curr.ssid);
          if (!existing) {
            acc.push(curr);
          } else if (curr.rssi && existing.rssi && curr.rssi > existing.rssi) {
            const index = acc.indexOf(existing);
            acc[index] = curr;
          }
          return acc;
        }, []);
        return response.networks;
      })
      .catch(error => {
        console.error('Error fetching WiFi networks:', error);
        toast.add({
          id: 'wifi-networks-error',
          title: 'Failed to fetch WiFi networks',
          description: error.data?.error || genericErrorMessage,
          icon: 'i-ri-alert-line',
          color: 'error',
          duration: 10000
        });
        return [];
      });
  }

  async function connectToWifiNetwork (params: WifiConnectParams) {
    return await busyBar.connectWifi(params)
      .catch(error => {
        console.error('Error connecting to WiFi:', error);
        toast.add({
          id: 'wifi-connect-error',
          title: 'Failed to connect to WiFi',
          description: error.data?.error || genericErrorMessage,
          icon: 'i-ri-alert-line',
          color: 'error',
          duration: 10000
        });
        return false;
      });
  }

  async function disconnectFromWifiNetwork () {
    return await busyBar.disconnectWifi()
      .catch(error => {
        console.error('Error disconnecting from WiFi:', error);
        toast.add({
          id: 'wifi-disconnect-error',
          title: 'Failed to disconnect from WiFi',
          description: error.data?.error || genericErrorMessage,
          icon: 'i-ri-alert-line',
          color: 'error',
          duration: 10000
        });
        return false;
      });
  }

  async function forgetSavedWifiNetwork () {
    return await busyBar.forgetWifi()
      .catch(error => {
        console.error('Error forgetting WiFi network:', error);
        toast.add({
          id: 'wifi-forget-error',
          title: 'Failed to forget WiFi network',
          description: error.data?.error || genericErrorMessage,
          icon: 'i-ri-alert-line',
          color: 'error',
          duration: 10000
        });
        return false;
      });
  }

  return {
    wifi,
    fetchWifiState,
    getWifiState,
    enableWifi,
    disableWifi,
    listWifiNetworks,
    connectToWifiNetwork,
    disconnectFromWifiNetwork,
    forgetSavedWifiNetwork
  };
});
