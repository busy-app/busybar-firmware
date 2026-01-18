import { defineStore } from 'pinia';
import type {
  WifiStatusResponse as WifiState,
  WifiNetwork,
  WifiConnectParams
} from '@busy-app/busy-lib';

export const useWifiStore = defineStore('wifi', () => {
  const busyBar = useDeviceStore().busyBar;

  const wifi = ref<WifiState | undefined>(undefined);

  async function fetchWifiState (): Promise<WifiState | undefined> {
    const state = await busyBar.statusWifi()
      .then(response => {
        wifi.value = response;
        return response;
      })
      .catch(async error => {
        await await handleHTTPError(error, 'Couldn\'t fetch WiFi state', true);
        return wifi.value;
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
      .catch(async error => {
        await handleHTTPError(error, 'Couldn\'t list WiFi networks', false, 0);
        return [];
      });
  }

  async function connectToWifiNetwork (params: WifiConnectParams) {
    return await busyBar.connectWifi(params)
      .catch(async error => {
        await handleHTTPError(error, 'Couldn\'t connect to WiFi network', false, 0);
        return false;
      });
  }

  async function disconnectFromWifiNetwork () {
    return await busyBar.disconnectWifi()
      .catch(async error => {
        await handleHTTPError(error, 'Couldn\'t disconnect from WiFi network', false, 0);
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
