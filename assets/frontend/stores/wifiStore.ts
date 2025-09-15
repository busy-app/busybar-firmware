import { defineStore } from 'pinia';

type WifiSecurity =
  | 'Open'
  | 'WPA'
  | 'WPA2'
  | 'WEP'
  | 'WPA Enterprise'
  | 'WPA2 Enterprise'
  | 'WPA WPA2 Mixed'
  | 'WPA3'
  | 'WPA3 Transition'
  | 'WPA3 Enterprise'
  | 'WPA3 Transition Enterprise';

export interface wifiState {
  state: 'enabled' | 'disabled' | 'connected';
  ssid?: string;
  security?: WifiSecurity;
  ip_config?: {
    ip_method: 'dhcp' | 'static';
    ip_type: 'ipv4' | 'ipv6';
    address?: string;
  };
}

export type WifiNetwork = {
  ssid: string;
  security: WifiSecurity;
  rssi: number;
};

export type WifiConnectOptions = {
  ssid: string;
  password?: string;
  security: WifiSecurity;
  ip_config: {
    ip_method: 'dhcp' | 'static';
    ip_type: 'ipv4' | 'ipv6';
    address?: string;
    mask?: string;
    gateway?: string;
  };
};

export const useWifiStore = defineStore('wifi', () => {
  const barUrl = useRuntimeConfig().public.barUrl;
  const toast = useToast();

  const wifi = ref<wifiState>({
    state: 'disabled'
  });

  async function updateWifiState () {
    const state = await $fetch<wifiState>(`${barUrl}/api/wifi/status`)
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
          description: error.message || genericErrorMessage,
          icon: 'i-ri-alert-line',
          color: 'error',
          duration: 10000
        });
        return wifi.value;
      });

    wifi.value = state;
  }

  async function enableWifi () {
    await $fetch(`${barUrl}/api/wifi/enable`, {
      method: 'POST'
    })
      .then(() => {
        wifi.value.state = 'enabled';
      })
      .catch(error => {
        console.error('Error enabling WiFi:', error);
        toast.add({
          id: 'wifi-enable-error',
          title: 'Failed to enable WiFi',
          description: error.message || genericErrorMessage,
          icon: 'i-ri-alert-line',
          color: 'error',
          duration: 10000
        });
      });
  }

  async function disableWifi () {
    await $fetch(`${barUrl}/api/wifi/disable`, {
      method: 'POST'
    })
      .then(() => {
        wifi.value.state = 'disabled';
      })
      .catch(error => {
        console.error('Error disabling WiFi:', error);
        toast.add({
          id: 'wifi-disable-error',
          title: 'Failed to disable WiFi',
          description: error.message || genericErrorMessage,
          icon: 'i-ri-alert-line',
          color: 'error',
          duration: 10000
        });
      });
  }

  async function listWifiNetworks () {
    interface WifiNetworkListResponse {
      count: number;
      networks: WifiNetwork[];
    }

    return await $fetch<WifiNetworkListResponse>(`${barUrl}/api/wifi/networks`)
      .then(response => {
        if (!response || !Array.isArray(response.networks)) {
          throw new Error('Failed to fetch WiFi networks');
        }
        return response.networks;
      })
      .catch(error => {
        console.error('Error fetching WiFi networks:', error);
        toast.add({
          id: 'wifi-networks-error',
          title: 'Failed to fetch WiFi networks',
          description: error.message || genericErrorMessage,
          icon: 'i-ri-alert-line',
          color: 'error',
          duration: 10000
        });
        return [];
      });
  }

  async function connectToWifiNetwork (options: WifiConnectOptions) {
    return await $fetch(`${barUrl}/api/wifi/connect`, {
      method: 'POST',
      body: options
    })
      .catch(error => {
        console.error('Error connecting to WiFi:', error);
        toast.add({
          id: 'wifi-connect-error',
          title: 'Failed to connect to WiFi',
          description: error.message || genericErrorMessage,
          icon: 'i-ri-alert-line',
          color: 'error',
          duration: 10000
        });
        return false;
      });
  }

  async function disconnectFromWifiNetwork () {
    return await $fetch(`${barUrl}/api/wifi/disconnect`, {
      method: 'POST'
    })
      .catch(error => {
        console.error('Error disconnecting from WiFi:', error);
        toast.add({
          id: 'wifi-disconnect-error',
          title: 'Failed to disconnect from WiFi',
          description: error.message || genericErrorMessage,
          icon: 'i-ri-alert-line',
          color: 'error',
          duration: 10000
        });
        return false;
      });
  }

  async function forgetSavedWifiNetwork () {
    return await $fetch(`${barUrl}/api/wifi/forget`, {
      method: 'POST'
    })
      .catch(error => {
        console.error('Error forgetting WiFi network:', error);
        toast.add({
          id: 'wifi-forget-error',
          title: 'Failed to forget WiFi network',
          description: error.message || genericErrorMessage,
          icon: 'i-ri-alert-line',
          color: 'error',
          duration: 10000
        });
        return false;
      });
  }

  return {
    wifi,
    updateWifiState,
    enableWifi,
    disableWifi,
    listWifiNetworks,
    connectToWifiNetwork,
    disconnectFromWifiNetwork,
    forgetSavedWifiNetwork
  };
});
