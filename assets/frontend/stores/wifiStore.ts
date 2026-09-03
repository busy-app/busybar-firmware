import { defineStore } from 'pinia';
import type {
  WifiStatusResponse as WifiState,
  WifiNetwork,
  WifiConnectParams
} from '@busy-app/busy-lib';

export const useWifiStore = defineStore('wifi', () => {
  const deviceStore = useDeviceStore();
  const stateStreamStore = useStateStreamStore();
  const configStore = useConfigStore();
  const wifi = ref<WifiState | undefined>(undefined);

  async function fetchWifiState (): Promise<WifiState | undefined> {
    const state = await deviceStore.busyBar.WifiStatusGet()
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

  const isListWifiNetworksLoading = ref(false);
  async function listWifiNetworks () {
    if (isListWifiNetworksLoading.value) {
      if (configStore.get('wifiAbortSimultaneousRequests')) {
        console.warn('wifiStore.listWifiNetworks: already loading, skipping');
        return [];
      } else {
        console.debug('wifiStore.listWifiNetworks: already loading, but wifiAbortSimultaneousRequests is false, allowing simultaneous request');
      }
    }

    isListWifiNetworksLoading.value = true;
    const wasPolling = deviceStore.refreshInterval;
    if (wasPolling) {
      deviceStore.clearRefreshInterval();
    }
    stateStreamStore.doCheckConnectionOnStreamDataStale = false;

    return await deviceStore.busyBar.WifiNetworksGet({ timeout: 0 })
      .then(response => {
        if (!response || !Array.isArray(response.networks)) {
          throw new Error('Failed to fetch WiFi networks');
        }
        // dedupe networks by SSID, keeping the one with the highest signal level
        response.networks = response.networks.reduce<WifiNetwork[]>((acc, curr) => {
          const existing = acc.find(n => n.ssid === curr.ssid);
          if (!existing) {
            acc.push(curr);
          } else if (curr.rssi && existing.rssi && curr.rssi < existing.rssi) {
            const index = acc.indexOf(existing);
            acc[index] = curr;
          }
          return acc;
        }, []);
        return response.networks;
      })
      .catch(async error => {
        if (wifi.value?.state === 'connected') {
          return;
        }
        await handleHTTPError(error, 'Couldn\'t list WiFi networks', false, 0);
        return [];
      })
      .finally(() => {
        stateStreamStore.doCheckConnectionOnStreamDataStale = true;
        if (wasPolling) {
          console.debug('wifiStore.listWifiNetworks: was polling before, resuming polling');
          deviceStore.setRefreshInterval();
        }
        isListWifiNetworksLoading.value = false;
      });
  }

  const isConnectToWifiNetworkLoading = ref(false);
  async function connectToWifiNetwork (params: WifiConnectParams) {
    if (isConnectToWifiNetworkLoading.value) {
      if (configStore.get('wifiAbortSimultaneousRequests')) {
        console.warn('wifiStore.connectToWifiNetwork: already connecting, skipping');
        return false;
      } else {
        console.debug('wifiStore.connectToWifiNetwork: already connecting, but wifiAbortSimultaneousRequests is false, allowing simultaneous request');
      }
    }
    isConnectToWifiNetworkLoading.value = true;

    const wasPolling = deviceStore.refreshInterval;
    if (wasPolling) {
      deviceStore.clearRefreshInterval();
    }
    stateStreamStore.doCheckConnectionOnStreamDataStale = false;
    return await deviceStore.busyBar.WifiConnect(params, { timeout: 0 })
      .catch(async error => {
        await handleHTTPError(error, 'Couldn\'t connect to WiFi network', false, 0);
        return false;
      })
      .finally(() => {
        stateStreamStore.doCheckConnectionOnStreamDataStale = true;
        if (wasPolling) {
          console.debug('wifiStore.connectToWifiNetwork: was polling before, resuming polling');
          deviceStore.setRefreshInterval();
        }
        isConnectToWifiNetworkLoading.value = false;
      });
  }

  const isDisconnectFromWifiNetworkLoading = ref(false);
  async function disconnectFromWifiNetwork () {
    if (isDisconnectFromWifiNetworkLoading.value) {
      if (configStore.get('wifiAbortSimultaneousRequests')) {
        console.warn('wifiStore.disconnectFromWifiNetwork: already disconnecting, skipping');
        return false;
      } else {
        console.debug('wifiStore.disconnectFromWifiNetwork: already disconnecting, but wifiAbortSimultaneousRequests is false, allowing simultaneous request');
      }
    }
    isDisconnectFromWifiNetworkLoading.value = true;

    const wasPolling = deviceStore.refreshInterval;
    if (wasPolling) {
      deviceStore.clearRefreshInterval();
    }
    stateStreamStore.doCheckConnectionOnStreamDataStale = false;
    return await deviceStore.busyBar.WifiDisconnect({ timeout: 0 })
      .catch(async error => {
        await handleHTTPError(error, 'Couldn\'t disconnect from WiFi network', false, 0);
        return false;
      })
      .finally(() => {
        stateStreamStore.doCheckConnectionOnStreamDataStale = true;
        if (wasPolling) {
          console.debug('wifiStore.disconnectFromWifiNetwork: was polling before, resuming polling');
          deviceStore.setRefreshInterval();
        }
        isDisconnectFromWifiNetworkLoading.value = false;
      });
  }

  return {
    wifi,
    fetchWifiState,
    listWifiNetworks,
    connectToWifiNetwork,
    disconnectFromWifiNetwork
  };
});
