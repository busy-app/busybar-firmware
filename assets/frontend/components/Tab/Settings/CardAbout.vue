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
            'Serial number': system?.serial_number || 'Unknown',
            'Front display resolution': '72×16 (LED)',
            'Mac address [Bluetooth]': 'Unknown',
            'Main display refresh rate': '60 Hz',
            'Mac address [Wi-Fi]': 'Unknown',
            'Back display resolution': '160×80 (OLED)',
            'Mac address [USB]': 'Unknown',
            'Central MCU': 'STM32U5M',
            'Hardware version': 'Unknown',
            'RAM size': '2.5 MB',
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
            'Commit hash': system?.commit_hash,
            'Uptime': system?.uptime ? system.uptime.slice(0, system.uptime.lastIndexOf(' ')) : 'Unknown'
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
            'Wi-Fi status': wifiStore.wifi?.state || 'Unknown',
            'Security': wifiStore.wifi?.security || 'Unknown',
            'Name': wifiStore.wifi?.ssid,
            'BSSID': wifiStore.wifi?.bssid || 'Unknown',
            'IP address': wifiStore.wifi?.ip_config?.address || 'Unknown',
            'Signal strength': wifiStore.wifi?.rssi ? `${wifiStore.wifi.rssi} dBm` : 'Unknown',
            'Channel': wifiStore.wifi?.channel || 'Unknown'
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
