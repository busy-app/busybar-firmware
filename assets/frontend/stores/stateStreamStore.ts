import { defineStore } from 'pinia';
import { useApiStore } from '@/stores/apiStore';
import {
  LocalStateStream,
  DataStatus,
  BSB_State,
  BSB_Update,
  StreamLifecycle,
  ConnectionStatus
} from '@busy-app/busy-lib';
import type { ProcessedState, StreamStatus, SmartHomePairingInfo } from '@busy-app/busy-lib';

function mapWifiSecurity (value: BSB_State.WifiStateConnected['security']) {
  switch (value) {
    case BSB_State.WifiSecurity.OPEN:
      return 'Open';
    case BSB_State.WifiSecurity.WPA:
      return 'WPA';
    case BSB_State.WifiSecurity.WPA2:
      return 'WPA2';
    case BSB_State.WifiSecurity.WEP:
      return 'WEP';
    case BSB_State.WifiSecurity.WPA_WPA2:
      return 'WPA/WPA2';
    case BSB_State.WifiSecurity.WPA3:
      return 'WPA3';
    case BSB_State.WifiSecurity.WPA2_WPA3:
      return 'WPA2/WPA3';
    default:
      return undefined;
  }
}

function mapWifiMethod (value: BSB_State.IpAddress['method']) {
  switch (value) {
    case BSB_State.IpConfigurationMethod.DHCP:
      return 'dhcp';
    case BSB_State.IpConfigurationMethod.STATIC:
      return 'static';
    default:
      return undefined;
  }
}

function mapWifiProtocol (value: BSB_State.IpAddress['protocol']) {
  switch (value) {
    case BSB_State.IpProtocol.IPV4:
      return 'ipv4';
    case BSB_State.IpProtocol.IPV6:
      return 'ipv6';
    default:
      return undefined;
  }
}

function inferWifiProtocol (value: BSB_State.IpAddress['address']) {
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

function mapMatterStatus (value: BSB_State.MatterCommissioningState['status']) {
  switch (value) {
    case BSB_State.MatterCommissioningStatus.NEVER_STARTED:
      return 'never_started';
    case BSB_State.MatterCommissioningStatus.STARTED:
      return 'started';
    case BSB_State.MatterCommissioningStatus.COMPLETED_SUCCESSFULLY:
      return 'completed_successfully';
    case BSB_State.MatterCommissioningStatus.FAILED:
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
  const timezoneStore = useTimezoneStore();
  const wifiStore = useWifiStore();
  const screenStreamStore = useScreenStreamStore();
  const barUrl = useRuntimeConfig().public.barUrl || window.location.origin;
  const configStore = useConfigStore();

  const streamNotRestartable = ref(false);
  const showStateStreamFailBanner = ref(false);
  const showResourceLimitErrorBanner = ref(false);

  const stream = shallowRef(new LocalStateStream(
    { addr: barUrl, token: apiStore.apiKey || '' },
    {
      timeout: Number(configStore.get('stateStreamTimeout')),
      dataTimeout: Number(configStore.get('stateStreamDataTimeout')),
      maxReconnectAttempts: Number(configStore.get('stateStreamMaxReconnectAttempts')),
      reconnectDelay: Number(configStore.get('stateStreamReconnectDelay'))
    }
  ));
  const streamStatus = ref<StreamStatus | null>(null);
  const doCheckConnectionOnStreamDataStale = ref(true);

  function stopStream () {
    stream.value.stop();
    if (streamStatus.value?.data.status === DataStatus.STALE) {
      streamStatus.value.data.status = DataStatus.NONE;
    }
    doCheckConnectionOnStreamDataStale.value = true;
  }

  function applyDeviceNameUpdate (payload: BSB_State.DeviceName) {
    const name = payload.name;
    if (name) {
      deviceStore.deviceName = name;
    }
  }

  function applyPowerUpdate (payload: BSB_State.Power) {
    const known = payload.known;

    const nextPower = known
      ? {
        state: known.batteryStatus ? BSB_State.BatteryStatus[known.batteryStatus] : undefined,
        battery_charge: known.batteryChargePercent ?? 0,
        battery_voltage: known.batteryVoltageMv ?? 0,
        battery_current: known.batteryCurrentMa ?? 0,
        usb_voltage: known.usbVoltageMv ?? 0
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

  function applyBrightnessUpdate (payload: BSB_State.Brightness) {
    if (payload.automatic) {
      brightnessStore.displayBrightness = { value: 'auto' };
      return;
    }

    const brightness = payload.manual?.brightness;
    if (brightness !== undefined) {
      brightnessStore.displayBrightness = { value: brightness };
    } else {
      brightnessStore.displayBrightness = { value: 0 };
    }
  }

  function applyAudioUpdate (payload: BSB_State.AudioVolume) {
    const volume = payload.volume;
    if (volume !== undefined) {
      audioStore.audio = { volume: volume ?? 0 };
    } else {
      audioStore.audio = { volume: 0 };
    }
  }

  function applyWifiUpdate (payload: BSB_State.Wifi) {
    const nextStateKey = Object.keys(payload).filter(key => key !== 'ipAddresses')[0] as keyof Omit<BSB_State.Wifi, 'ipAddresses'>;
    const connected = payload.connected;
    const ipAddresses = Array.isArray(payload.ipAddresses) ? payload.ipAddresses : [];
    const primaryIp = ipAddresses.find(e => e.address);
    const previousIpConfig = wifiStore.wifi?.ip_config;
    const address = primaryIp ? primaryIp.address : undefined;

    // FW-883: reconnecting still reports connected state
    let _nextStateKey = nextStateKey as typeof nextStateKey | 'reconnecting';
    if (nextStateKey === 'connected' && connected?.status === BSB_State.WifiConnectionStatus.RECONNECTING) {
      _nextStateKey = 'reconnecting';
    }

    const nextWifiState = {
      state: _nextStateKey,
      ssid: connected?.ssid,
      bssid: connected?.bssid,
      channel: connected?.channel,
      rssi: connected?.rssi,
      security: mapWifiSecurity(connected?.security),
      ip_config: primaryIp
        ? {
          ip_method: mapWifiMethod(primaryIp.method) ?? previousIpConfig?.ip_method,
          ip_type: mapWifiProtocol(primaryIp.protocol) ?? inferWifiProtocol(address) ?? previousIpConfig?.ip_type,
          address,
          gateway: primaryIp.gateway,
          mask: primaryIp.netmask
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

  function applyUpdateCheck (payload: BSB_Update.CheckState) {
    firmwareStore.autoUpdate.isChecking = false;

    if (payload.available) {
      firmwareStore.autoUpdate.status = 'available';
      firmwareStore.autoUpdate.availableVersion = payload.available?.version ?? null;
      firmwareStore.autoUpdate.isAllowed = true;
      return;
    }

    const reason = payload.unavailable?.reason;

    firmwareStore.autoUpdate.availableVersion = null;

    if (reason === BSB_Update.CheckError.NOT_AVAILABLE) {
      firmwareStore.autoUpdate.status = 'not_available';
      firmwareStore.autoUpdate.isAllowed = true;
      return;
    }

    if (reason === BSB_Update.CheckError.FAILURE) {
      firmwareStore.autoUpdate.status = 'failure';
      firmwareStore.autoUpdate.isAllowed = false;
      return;
    }

    firmwareStore.autoUpdate.status = null;
  }

  function applyTimezoneUpdate (payload: BSB_State.Timezone) {
    if (payload.name) {
      timezoneStore.timezone = payload.name;
    }
  }

  function applyMatterUpdate (payload: BSB_State.Matter) {
    const state = payload.state;

    matterStore.matterCommissioning = {
      fabricCount: payload.fabricCount ?? 0,
      latestStatus: state
        ? {
          value: mapMatterStatus(state.status),
          timestamp: (state.timestamp ?? 0) as NonNullable<SmartHomePairingInfo['latest_pairing_status']>['timestamp']
        }
        : undefined
    };
  }

  function applyStateMessage (data: ProcessedState) {
    if (!data.updates) {
      return;
    }
    for (const update of data.updates) {
      switch (update.state) {
        case 'deviceName':
          if (update.deviceName) {
            applyDeviceNameUpdate(update.deviceName);
          }
          break;
        case 'power':
          if (update.power) {
            applyPowerUpdate(update.power);
          }
          break;
        case 'brightness':
          if (update.brightness) {
            applyBrightnessUpdate(update.brightness);
          }
          break;
        case 'audioVolume':
          if (update.audioVolume) {
            applyAudioUpdate(update.audioVolume);
          }
          break;
        case 'wifi':
          if (update.wifi) {
            applyWifiUpdate(update.wifi);
          }
          break;
        case 'updateCheck':
          if (update.updateCheck) {
            applyUpdateCheck(update.updateCheck);
          }
          break;
        case 'timezone':
          if (update.timezone) {
            applyTimezoneUpdate(update.timezone);
          }
          break;
        case 'matter':
          if (update.matter) {
            applyMatterUpdate(update.matter);
          }
          break;
        case 'frame':
          if (update.frame) {
            screenStreamStore.currentFrame = update.frame;
          }
          break;
        default:
          break;
      }
    }
  }

  async function applyStreamStatus (status: StreamStatus) {
    const oldStatus = streamStatus.value;

    if (oldStatus === null) {
      streamStatus.value = status;
      if (configStore.get('stateStreamLogStatusUpdates')) {
        console.debug('[state stream status] Initial stream status:', status);
      }
      return;
    }

    const diff = deepDiff(oldStatus, status) as Partial<StreamStatus> | undefined;
    if (diff) {
      for (const line of flattenDeepDiff(diff)) {
        if (configStore.get('stateStreamLogStatusUpdates')) {
          console.debug('[state stream status]', line, '| full status:', status);
        }
      }
    }

    streamStatus.value = status;

    if (streamStatus.value.data.status === DataStatus.STALE && oldStatus?.data.status !== DataStatus.STALE && doCheckConnectionOnStreamDataStale.value) {
      console.debug('No state messages received for a while, checking connection...');
      const conncheckResult = await deviceStore.checkConnection();
      if (conncheckResult === false) {
        console.debug('Connection check failed after state stream data stale, stopping stream and starting polling');
        stopStream();
        deviceStore.setRefreshInterval();
      }
    }

    if (streamStatus.value.data.status === DataStatus.ACTIVE
      && streamStatus.value.connection.status === ConnectionStatus.CONNECTED
      && streamStatus.value.main.status === StreamLifecycle.RUNNING
    ) {
      if (showResourceLimitErrorBanner.value) {
        console.debug('Stream is active again, hiding resource limit error banner');
        showResourceLimitErrorBanner.value = false;
      }
      if (showStateStreamFailBanner.value) {
        console.debug('Stream is active again, hiding state stream failure banner');
        showStateStreamFailBanner.value = false;
      }
    }
  }

  return {
    streamNotRestartable,
    showStateStreamFailBanner,
    showResourceLimitErrorBanner,

    streamStatus,
    stream,
    doCheckConnectionOnStreamDataStale,
    stopStream,

    applyStateMessage,
    applyStreamStatus
  };
});
