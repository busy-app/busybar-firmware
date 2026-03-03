<template>
  <SectionCard
    data-id="settings-section-primary"
    icon="i-bi-info-fill"
    title="About device"
  >
    <div class="flex flex-col gap-4">
      <div class="flex items-center gap-2.5">
        <UIcon
          name="i-bi-info"
          class="size-6"
        />
        <div class="font-medium">General</div>
      </div>
      <ContentList :items="generalContent" />
    </div>

    <div class="flex flex-col gap-4">
      <div class="flex items-center gap-2.5">
        <UIcon
          name="i-bi-firmware"
          class="size-6"
        />
        <div class="font-medium">Firmware</div>
      </div>
      <ContentList :items="firmwareContent" />
    </div>

    <div class="flex flex-col gap-4">
      <div class="flex items-center gap-2.5">
        <UIcon
          name="i-bi-network"
          class="size-6"
        />
        <div class="font-medium">Network</div>
      </div>
      <ContentList :items="networkContent" />
    </div>
  </SectionCard>
</template>

<script setup lang="ts">
const deviceStore = useDeviceStore();
const wifiStore = useWifiStore();

const firmware = computed(() => deviceStore.deviceStatus?.firmware);
const system = computed(() => deviceStore.deviceStatus?.system);
const device = computed(() => deviceStore.deviceStatus?.device);

const productionDate = computed(() => {
  const timestamp = device.value?.otp_timestamp;
  if (!timestamp) {
    return undefined;
  }

  const date = new Date(timestamp * 1000);
  return date.toLocaleString('en-US', { month: 'short', year: 'numeric' });
});

const generalContent = computed(() => [
  [
    {
      title: 'Serial number',
      value: device.value?.serial_number,
      loading: !device.value,
      class: 'overflow-visible whitespace-normal break-all'
    },
    {
      title: 'Mac address [Bluetooth]',
      value: device.value?.ble_mac,
      loading: !device.value,
      class: 'overflow-visible whitespace-normal break-all'
    },
    {
      title: 'Mac address [Wi-Fi]',
      value: device.value?.wifi_mac,
      loading: !device.value,
      class: 'overflow-visible whitespace-normal break-all'
    },
    {
      title: 'Mac address [USB]',
      value: device.value?.usb_mac,
      loading: !device.value,
      class: 'overflow-visible whitespace-normal break-all'
    },
    {
      title: 'Hardware version',
      value: device.value?.otp_model,
      loading: !device.value
    },
    {
      title: 'Production date',
      value: productionDate.value,
      loading: !device.value
    }
  ],
  [
    {
      title: 'Front display resolution',
      value: '72×16 (LED)'
    },
    {
      title: 'Front display refresh rate',
      value: '60 Hz'
    },
    {
      title: 'Back display resolution',
      value: '160×80 (OLED)'
    },
    {
      title: 'Central MCU',
      value: 'STM32U5M'
    },
    {
      title: 'RAM size',
      value: '2.5 MB'
    }
  ]
]);

const firmwareContent = computed(() => [
  [
    {
      title: 'Version',
      value: firmware.value?.version,
      loading: !firmware.value
    },
    {
      title: 'Branch',
      value: firmware.value?.branch,
      loading: !firmware.value
    },
    {
      title: 'Commit hash',
      value: firmware.value?.commit_hash,
      loading: !firmware.value
    }
  ],
  [
    {
      title: 'Build date',
      value: firmware.value?.build_date,
      loading: !firmware.value
    },
    {
      title: 'API version',
      value: deviceStore.apiVersion?.api_semver,
      loading: !deviceStore.apiVersion
    },
    {
      title: 'Uptime',
      value: system.value?.uptime ? system.value.uptime.slice(0, system.value.uptime.lastIndexOf(' ')) : undefined,
      loading: !system.value
    }
  ]
]);

const networkContent = computed(() => {
  if (wifiStore.wifi?.state === 'connected') {
    return [
      [
        {
          title: 'Wi-Fi status',
          value: wifiStore.wifi?.state,
          loading: !wifiStore.wifi,
          class: 'capitalize'
        },
        {
          title: 'Name',
          value: wifiStore.wifi?.ssid,
          loading: !wifiStore.wifi
        },
        {
          title: 'IP address',
          value: wifiStore.wifi?.ip_config?.address,
          loading: !wifiStore.wifi,
          class: 'overflow-visible whitespace-normal break-all'
        },
        {
          title: 'Channel',
          value: wifiStore.wifi?.channel ? String(wifiStore.wifi.channel) : undefined,
          loading: !wifiStore.wifi
        }
      ],
      [
        {
          title: 'Security',
          value: wifiStore.wifi?.security,
          loading: !wifiStore.wifi
        },
        {
          title: 'BSSID',
          value: wifiStore.wifi?.bssid,
          loading: !wifiStore.wifi,
          class: 'overflow-visible whitespace-normal break-all'
        },
        {
          title: 'Signal strength',
          value: wifiStore.wifi?.rssi ? `${wifiStore.wifi.rssi} dBm` : 'Unknown',
          loading: !wifiStore.wifi
        }
      ]
    ];
  } else {
    return [
      [
        {
          title: 'Wi-Fi status',
          value: wifiStore.wifi?.state,
          loading: !wifiStore.wifi,
          class: 'capitalize'
        }
      ]
    ];
  }
});

async function init () {
  await deviceStore.fetchDeviceStatus();
  await deviceStore.fetchApiVersion();
}

onMounted(async () => {
  await init();
  window.addEventListener('device-reconnected', init);
});
onBeforeUnmount(() => window.removeEventListener('device-reconnected', init));
</script>
