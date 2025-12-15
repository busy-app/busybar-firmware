import { defineStore } from 'pinia';
import type {
  WifiStatusResponse as WifiState,
  WifiNetwork,
  WifiConnectParams
} from '@busy-app/busy-lib';

export const useWifiStore = defineStore('wifi', () => {
  const toast = useToast();

  const busyBar = useDeviceStore().busyBar;

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

  return {
    wifi,
    fetchWifiState,
    getWifiState,
    listWifiNetworks,
    connectToWifiNetwork,
    disconnectFromWifiNetwork
  };
});
