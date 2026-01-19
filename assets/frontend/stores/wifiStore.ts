import { defineStore } from 'pinia';
import type {
  WifiStatusResponse as WifiState,
  WifiNetwork,
  WifiConnectParams
} from '@busy-app/busy-lib';

export const useWifiStore = defineStore('wifi', () => {
  const deviceStore = useDeviceStore();
  const wifi = ref<WifiState | undefined>(undefined);

  async function fetchWifiState (): Promise<WifiState | undefined> {
    const state = await deviceStore.busyBar.statusWifi()
      .then(response => {
        wifi.value = response;
        return response;
      })
      .catch(async error => {
        await handleHTTPError(error, 'Couldn\'t fetch WiFi state', true);
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
    deviceStore.clearRefreshInterval();
    return await deviceStore.busyBar.networksWifi()
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
      })
      .finally(() => {
        deviceStore.setRefreshInterval();
      });
  }

  async function connectToWifiNetwork (params: WifiConnectParams) {
    deviceStore.clearRefreshInterval();
    return await deviceStore.busyBar.connectWifi(params)
      .catch(async error => {
        await handleHTTPError(error, 'Couldn\'t connect to WiFi network', false, 0);
        return false;
      })
      .finally(() => {
        deviceStore.setRefreshInterval();
      });
  }

  async function disconnectFromWifiNetwork () {
    deviceStore.clearRefreshInterval();
    return await deviceStore.busyBar.disconnectWifi()
      .catch(async error => {
        await handleHTTPError(error, 'Couldn\'t disconnect from WiFi network', false, 0);
        return false;
      })
      .finally(() => {
        deviceStore.setRefreshInterval();
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
