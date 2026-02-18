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
      <div class="grid sm:grid-cols-2 gap-y-3 gap-x-1">
        <div
          v-for="[property, value] in Object.entries({
            'Name': deviceStore.deviceName || 'Unknown',
            'Serial number': 'Unknown',
            'Mac address [Bluetooth]': 'Unknown',
            'Uptime': system?.uptime ? system.uptime.slice(0, system.uptime.lastIndexOf(' ')) : 'Unknown',
            'Mac address [Wi-Fi]': 'Unknown',
            'Hardware target': 'Unknown',
            'Mac address [USB]': 'Unknown',
            'Production date': 'Unknown'
          })"
          :key="property"
          class="flex gap-2"
        >
          <div class="w-[120px] text-muted">{{ property }}</div>
          <div class="max-w-[140px] md:max-w-[180px] text-ellipsis overflow-hidden">{{ value }}</div>
        </div>
      </div>
    </div>

    <div class="flex flex-col gap-4">
      <div class="flex items-center gap-2.5">
        <UIcon
          name="i-bi-hardware"
          class="size-6"
        />
        <div class="font-medium">Hardware</div>
      </div>
      <div class="grid sm:grid-cols-2 gap-y-3 gap-x-1">
        <div
          v-for="[property, value] in Object.entries({
            'Main display resolution': '72×16 (LED)',
            'Central MCU': 'STM32U5M',
            'Main display refresh rate': '60 Hz',
            'Wireless MCU': 'Silicon Labs SiWG917',
            'Back display resolution': '160×80 (OLED)',
            'RAM size': '2.5 MB'
          })"
          :key="property"
          class="flex gap-2"
        >
          <div class="w-[120px] text-muted">{{ property }}</div>
          <div class="max-w-[140px] md:max-w-[180px] text-ellipsis overflow-hidden">{{ value }}</div>
        </div>
      </div>
    </div>

    <div class="flex flex-col gap-4">
      <div class="flex items-center gap-2.5">
        <UIcon
          name="i-bi-firmware"
          class="size-6"
        />
        <div class="font-medium">Firmware</div>
      </div>
      <div class="grid sm:grid-cols-2 gap-y-3 gap-x-1">
        <div
          v-for="[property, value] in Object.entries({
            'Version': system?.version,
            'Build date': system?.build_date,
            'Branch': system?.branch,
            'API version': deviceStore.apiVersion?.api_semver || 'Unknown',
            'Commit hash': system?.commit_hash
          })"
          :key="property"
          class="flex gap-2"
        >
          <div class="w-[120px] text-muted">{{ property }}</div>
          <div class="max-w-[140px] md:max-w-[180px] text-ellipsis overflow-hidden">{{ value }}</div>
        </div>
      </div>
    </div>

    <div class="flex flex-col gap-4">
      <div class="flex items-center gap-2.5">
        <UIcon
          name="i-bi-battery-5"
          class="size-6"
        />
        <div class="font-medium">Battery</div>
      </div>
      <div class="grid sm:grid-cols-2 gap-y-3 gap-x-1">
        <div
          v-for="[property, value] in Object.entries({
            'Status': batteryStatus,
            'Charge cycles': 'Unknown',
            'Health': 'Unknown',
            'Temperature': 'Unknown',
            'Voltage': power?.battery_voltage ? `${(Number(power.battery_voltage) / 1000).toFixed(1)} V` : 'Unknown',
            'Charge counter': 'Unknown',
            'Capacity (system)': 'Unknown',
            'Remaining': 'Unknown'
          })"
          :key="property"
          class="flex gap-2"
        >
          <div class="w-[120px] text-muted">{{ property }}</div>
          <div class="max-w-[140px] md:max-w-[180px] text-ellipsis overflow-hidden">{{ value }}</div>
        </div>
      </div>
    </div>

    <div class="flex flex-col gap-4">
      <div class="flex items-center gap-2.5">
        <UIcon
          name="i-bi-network"
          class="size-6"
        />
        <div class="font-medium">Network</div>
      </div>
      <div class="grid sm:grid-cols-2 gap-y-3 gap-x-1">
        <div
          v-for="[property, value] in Object.entries({
            'Name': wifiStore.wifi?.ssid,
            'Signal strength': wifiStore.wifi?.rssi ? `${wifiStore.wifi.rssi} dBm` : 'Unknown',
            'Channel': wifiStore.wifi?.channel || 'Unknown',
            'BSSID': wifiStore.wifi?.bssid || 'Unknown',
            'Security': wifiStore.wifi?.security || 'Open'
          })"
          :key="property"
          class="flex gap-2"
        >
          <div class="w-[120px] text-muted">{{ property }}</div>
          <div class="max-w-[140px] md:max-w-[180px] text-ellipsis overflow-hidden">{{ value }}</div>
        </div>
      </div>
    </div>
  </SectionCard>
</template>

<script setup lang="ts">
const deviceStore = useDeviceStore();
const wifiStore = useWifiStore();

const system = computed(() => deviceStore.deviceStatus?.system);
const power = computed(() => deviceStore.deviceStatus?.power);

const batteryStatus = computed(() => {
  if (!power.value) {
    return 'Unknown';
  }
  if (power.value.state === 'charging') {
    return `Charging (${power.value.battery_charge}%)`;
  }
  if (power.value.state === 'discharging') {
    return `Discharging (${power.value.battery_charge}%)`;
  }
  return `${power.value.battery_charge}%`;
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
