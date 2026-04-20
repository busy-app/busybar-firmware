import { defineStore } from 'pinia';
import { useApiStore } from '@/stores/apiStore';
import { decodeStateMessage, type StateMessage } from '@/util/stateStreamMessage';
import { UpdateStage } from '@/stores/firmwareStore';
import {
  stateStreamWebSocketClient,
  type StateStreamSubscription
} from '@/util/state-stream/stateStreamWebSocketClient';

function isProtoMessage (value: unknown): value is Record<string, unknown> {
  return !!value && typeof value === 'object' && !Array.isArray(value) && !(value instanceof Uint8Array);
}

function getString (value: unknown): string | undefined {
  return typeof value === 'string' ? value : undefined;
}

function getNumber (value: unknown): number | undefined {
  return typeof value === 'number' ? value : undefined;
}

function getNumericValue (value: unknown): number | undefined {
  if (typeof value === 'number') {
    return value;
  }

  if (typeof value === 'string') {
    const parsed = Number(value);
    return Number.isFinite(parsed) ? parsed : undefined;
  }

  return undefined;
}

function lowerCaseEnum (value: string | undefined): string | undefined {
  return value?.toLowerCase();
}

function mapWifiSecurity (value: string | undefined) {
  switch (value) {
    case 'OPEN':
      return 'Open';
    case 'WPA':
      return 'WPA';
    case 'WPA2':
      return 'WPA2';
    case 'WEP':
      return 'WEP';
    case 'WPA_WPA2':
      return 'WPA/WPA2';
    case 'WPA3':
      return 'WPA3';
    case 'WPA2_WPA3':
      return 'WPA2/WPA3';
    default:
      return undefined;
  }
}

function mapWifiMethod (value: string | undefined) {
  switch (value) {
    case 'DHCP':
      return 'dhcp';
    case 'STATIC':
      return 'static';
    default:
      return undefined;
  }
}

function mapWifiProtocol (value: string | undefined) {
  switch (value) {
    case 'IPV4':
      return 'ipv4';
    case 'IPV6':
      return 'ipv6';
    default:
      return undefined;
  }
}

function inferWifiProtocol (value: string | undefined) {
  if (!value) {
    return undefined;
  }

  if (value.includes(':')) {
    return 'ipv6';
  }

  if (value.includes('.')) {
    return 'ipv4';
  }

  return undefined;
}

function mapMatterStatus (value: string | undefined) {
  switch (value) {
    case 'NEVER_STARTED':
      return 'never_started';
    case 'STARTED':
      return 'started';
    case 'COMPLETED_SUCCESSFULLY':
      return 'completed_successfully';
    case 'FAILED':
      return 'failed';
    default:
      return undefined;
  }
}

export const useStateStreamStore = defineStore('stateStream', () => {
  const apiStore = useApiStore();
  const deviceStore = useDeviceStore();
  const audioStore = useAudioStore();
  const brightnessStore = useBrightnessStore();
  const firmwareStore = useFirmwareStore();
  const matterStore = useMatterStore();
  const screenStreamStore = useScreenStreamStore();
  const timezoneStore = useTimezoneStore();
  const wifiStore = useWifiStore();
  const barUrl = useRuntimeConfig().public.barUrl || window.location.origin;

  const isWebSocketConnected = ref(false);
  const showStateStreamFailBanner = ref(false);

  type DataCallback = (data: StateMessage) => void;
  type StopCallback = () => void;

  let activeSubscription: StateStreamSubscription | null = null;
  let connectionGeneration = 0;

  function releaseSubscription (subscription: StateStreamSubscription | null) {
    if (!subscription) {
      return;
    }

    if (activeSubscription === subscription) {
      activeSubscription = null;
    }

    stateStreamWebSocketClient.disconnect(subscription);
  }

  function applyDeviceNameUpdate (payload: Record<string, unknown>) {
    const name = getString(payload.name);
    if (name) {
      deviceStore.deviceName = name;
    }
  }

  function applyPowerUpdate (payload: Record<string, unknown>) {
    const state = getString(payload.state);
    const known = isProtoMessage(payload.known) ? payload.known : undefined;

    const nextPower = state === 'known' && known
      ? {
        state: lowerCaseEnum(getString(known.batteryStatus)) as 'discharging' | 'charging' | 'charged' | undefined,
        battery_charge: getNumber(known.batteryChargePercent) ?? 0,
        battery_voltage: getNumber(known.batteryVoltageMv) ?? 0,
        battery_current: getNumber(known.batteryCurrentMa) ?? 0,
        usb_voltage: getNumber(known.usbVoltageMv) ?? 0
      }
      : undefined;

    if (!nextPower) {
      return;
    }

    deviceStore.deviceStatus = {
      ...deviceStore.deviceStatus,
      power: nextPower
    } as NonNullable<typeof deviceStore.deviceStatus>;
  }

  function applyBrightnessUpdate (payload: Record<string, unknown>) {
    const setting = getString(payload.setting);
    if (setting === 'automatic') {
      brightnessStore.displayBrightness = { value: 'auto' };
      return;
    }

    const manual = isProtoMessage(payload.manual) ? payload.manual : undefined;
    const brightness = getNumber(manual?.brightness);
    if (brightness !== undefined) {
      brightnessStore.displayBrightness = { value: brightness };
    }
  }

  function applyAudioUpdate (payload: Record<string, unknown>) {
    const volume = getNumber(payload.volume);
    if (volume !== undefined) {
      audioStore.audio = { volume };
    }
  }

  function applyWifiUpdate (payload: Record<string, unknown>) {
    const nextStateKey = getString(payload.wifiState);
    const connected = isProtoMessage(payload.connected) ? payload.connected : undefined;
    const ipAddresses = Array.isArray(payload.ipAddresses) ? payload.ipAddresses : [];
    const primaryIp = ipAddresses.find(isProtoMessage);
    const previousIpConfig = wifiStore.wifi?.ip_config;
    const address = isProtoMessage(primaryIp) ? getString(primaryIp.address) : undefined;

    const nextWifiState = {
      state: lowerCaseEnum(nextStateKey) as 'unknown' | 'disconnected' | 'connected' | 'connecting' | 'disconnecting' | 'reconnecting' | undefined,
      ssid: getString(connected?.ssid),
      bssid: getString(connected?.bssid),
      channel: getNumber(connected?.channel),
      rssi: getNumber(connected?.rssi),
      security: mapWifiSecurity(getString(connected?.security)),
      ip_config: isProtoMessage(primaryIp)
        ? {
          ip_method: mapWifiMethod(getString(primaryIp.method)) ?? previousIpConfig?.ip_method,
          ip_type: mapWifiProtocol(getString(primaryIp.protocol)) ?? inferWifiProtocol(address) ?? previousIpConfig?.ip_type,
          address,
          gateway: getString(primaryIp.gateway),
          mask: getString(primaryIp.netmask)
        }
        : undefined
    };

    const previousState = wifiStore.wifi?.state;
    wifiStore.wifi = nextWifiState as typeof wifiStore.wifi;

    if (previousState !== nextWifiState.state) {
      if (nextWifiState.state === 'connected') {
        window.dispatchEvent(new Event('wifi-reconnected'));
      } else if (previousState === 'connected') {
        window.dispatchEvent(new Event('wifi-disconnected'));
      }
    }
  }

  function applyUpdateCheck (payload: Record<string, unknown>) {
    const status = getString(payload.status);
    firmwareStore.autoUpdate.isChecking = false;

    if (status === 'available') {
      const available = isProtoMessage(payload.available) ? payload.available : undefined;
      firmwareStore.autoUpdate.status = 'available';
      firmwareStore.autoUpdate.availableVersion = getString(available?.version) ?? null;
      firmwareStore.autoUpdate.isAllowed = true;
      return;
    }

    const unavailable = isProtoMessage(payload.unavailable) ? payload.unavailable : undefined;
    const reason = getString(unavailable?.reason);

    firmwareStore.autoUpdate.availableVersion = null;

    if (reason === 'NOT_AVAILABLE') {
      firmwareStore.autoUpdate.status = 'not_available';
      firmwareStore.autoUpdate.isAllowed = true;
      return;
    }

    if (reason === 'FAILURE') {
      firmwareStore.autoUpdate.status = 'failure';
      firmwareStore.autoUpdate.isAllowed = false;
      return;
    }

    firmwareStore.autoUpdate.status = null;
  }

  function applyUpdateState (payload: Record<string, unknown>) {
    const event = getString(payload.event);
    const action = getString(payload.action);
    const status = getString(payload.status);

    firmwareStore.autoUpdate.isChecking = false;

    if (status === 'BATTERY_LOW') {
      firmwareStore.autoUpdate.isAllowed = false;
      return;
    }

    if (status && status !== 'OK' && status !== 'BUSY') {
      firmwareStore.autoUpdate.error.stage = firmwareStore.autoUpdate.stage;
      firmwareStore.autoUpdate.error.message = `Update failed: ${lowerCaseEnum(status)}`;
      firmwareStore.autoUpdate.stage = UpdateStage.ERROR;
      firmwareStore.autoUpdate.isAllowed = false;
      return;
    }

    firmwareStore.autoUpdate.isAllowed = true;

    if (action === 'DOWNLOAD') {
      firmwareStore.autoUpdate.stage = UpdateStage.LOADING;
      return;
    }

    if (action && action !== 'ACTION_NONE') {
      firmwareStore.autoUpdate.stage = UpdateStage.UPDATING;
      return;
    }

    if (event === 'SESSION_STOP' && status === 'OK') {
      firmwareStore.autoUpdate.stage = UpdateStage.UPDATING;
    }
  }

  function applyTimezoneUpdate (payload: Record<string, unknown>) {
    const timezone = getString(payload.name);
    if (timezone) {
      timezoneStore.timezone = timezone;
    }
  }

  function applyMatterUpdate (payload: Record<string, unknown>) {
    const state = isProtoMessage(payload.state) ? payload.state : undefined;

    matterStore.matterCommissioning = {
      fabricCount: getNumber(payload.fabricCount) ?? 0,
      latestStatus: state
        ? {
          value: mapMatterStatus(getString(state.status)),
          timestamp: getNumericValue(state.timestamp)
        }
        : undefined
    };
  }

  function applyStateMessage (data: StateMessage) {
    for (const update of data.updates) {
      switch (update.state) {
        case 'deviceName':
          if (isProtoMessage(update.deviceName)) {
            applyDeviceNameUpdate(update.deviceName);
          }
          break;
        case 'power':
          if (isProtoMessage(update.power)) {
            applyPowerUpdate(update.power);
          }
          break;
        case 'brightness':
          if (isProtoMessage(update.brightness)) {
            applyBrightnessUpdate(update.brightness);
          }
          break;
        case 'audioVolume':
          if (isProtoMessage(update.audioVolume)) {
            applyAudioUpdate(update.audioVolume);
          }
          break;
        case 'wifi':
          if (isProtoMessage(update.wifi)) {
            applyWifiUpdate(update.wifi);
          }
          break;
        case 'updateCheck':
          if (isProtoMessage(update.updateCheck)) {
            applyUpdateCheck(update.updateCheck);
          }
          break;
        case 'updateState':
          if (isProtoMessage(update.updateState)) {
            applyUpdateState(update.updateState);
          }
          break;
        case 'timezone':
          if (isProtoMessage(update.timezone)) {
            applyTimezoneUpdate(update.timezone);
          }
          break;
        case 'matter':
          if (isProtoMessage(update.matter)) {
            applyMatterUpdate(update.matter);
          }
          break;
        case 'frame':
          if (update.frame) {
            screenStreamStore.applyFrameUpdate(update.frame);
          }
          break;
        default:
          break;
      }
    }
  }

  async function openStateWebsocket (
    dataCallback: DataCallback = () => {
    },
    stopCallback: StopCallback | undefined = () => {
    }
  ) {
    const generation = ++connectionGeneration;
    releaseSubscription(activeSubscription);

    const subscription = await stateStreamWebSocketClient.connect(barUrl, apiStore.apiKey, {
      onStatus: (connected, reconnected) => {
        if (generation !== connectionGeneration) {
          return;
        }

        isWebSocketConnected.value = connected;

        if (connected) {
          return;
        }

        if (!reconnected) {
          void deviceStore.checkConnection();
          stopCallback?.();
        }
      },
      onData: data => {
        if (generation !== connectionGeneration) {
          return;
        }

        try {
          const message = decodeStateMessage(data);
          applyStateMessage(message);
          dataCallback(message);
        } catch (error) {
          console.error('State publisher websocket decode error', error);
        }
      },
      onError: message => {
        if (generation !== connectionGeneration) {
          return;
        }

        console.error('State publisher websocket error', message);
      },
      onCheckConnection: async () => {
        console.debug('No state updates received for a while, checking connection...');
        deviceStore.setRefreshInterval();
        await deviceStore.checkConnection();
      }
    });

    if (generation !== connectionGeneration) {
      releaseSubscription(subscription);
      return;
    }

    activeSubscription = subscription;
    isWebSocketConnected.value = true;
  }

  function closeWebsocket (): Promise<void> {
    connectionGeneration++;
    isWebSocketConnected.value = false;
    releaseSubscription(activeSubscription);
    return Promise.resolve();
  }

  function startStateStream (
    dataCallback: DataCallback = () => {
    },
    stopCallback?: StopCallback
  ) {
    return openStateWebsocket(dataCallback, stopCallback);
  }

  function stopStateStream () {
    return closeWebsocket();
  }

  return {
    isWebSocketConnected,
    showStateStreamFailBanner,

    startStateStream,
    stopStateStream
  };
});
