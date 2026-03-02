<template>
  <SectionCard
    data-id="firmware-section-primary"
    title="Firmware"
    :subtitle="fwVersionPolifilled"
    icon="i-bi-firmware-fill"
  >
    <template #actions>
      <UButton
        data-id="firmware-section-primary-update-from-file-button"
        label="Update from file"
        icon="i-bi-upload"
        variant="link"
        :ui="{
          base: 'px-2.5 py-2 rounded-full'
        }"
        class="justify-center sm:justify-start"
        @click="initFirmwareUpdateFromFile"
      />
      <UTooltip
        :text="wifiStore.wifi?.state !== 'connected' ? 'Connect to Wi-Fi to check for updates' : ''"
        :delay-duration="0"
      >
        <UButton
          data-id="firmware-section-primary-check-for-updates-button"
          label="Check for updates"
          :disabled="wifiStore.wifi?.state !== 'connected'"
          :loading="firmwareStore.autoUpdate.isChecking"
          @click="firmwareStore.autoUpdate.isManualCheck = true; firmwareStore.requestAutoUpdateCheck()"
        />
      </UTooltip>
    </template>

    <div class="flex justify-between items-center">
      <div class="flex flex-col gap-1">
        <span>Auto-update</span>
        <span class="text-sm text-muted">Updates will be downloaded and installed automatically during the night</span>
      </div>

      <div>
        <USwitch
          v-model="firmwareStore.autoUpdateSelfCheck.is_enabled"
          @change="firmwareStore.setAutoUpdateSelfCheck(firmwareStore.autoUpdateSelfCheck)"
        />
      </div>
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
  </SectionCard>
</template>

<script setup lang="ts">
const deviceStore = useDeviceStore();
const firmwareStore = useFirmwareStore();
const wifiStore = useWifiStore();

const system = computed(() => deviceStore.deviceStatus?.system);
const fwVersionPolifilled = computed(() => system.value?.version === 'unknown' ? `${system.value.branch} ${system.value.commit_hash}` : system.value?.version);

async function initFirmwareUpdateFromFile () {
  await deviceStore.fetchDeviceStatus();
  const charge = deviceStore.deviceStatus?.power?.battery_charge;

  if (charge !== undefined && charge < 40) {
    firmwareStore.autoUpdate.modals.batteryLow = true;
    return;
  }

  firmwareStore.fileUpdate.stage = UpdateStage.IDLE;
  firmwareStore.fileUpdate.progress = 0;
  firmwareStore.fileUpdate.error = '';
  firmwareStore.fileUpdate.firmwareFile = null;
  firmwareStore.fileUpdate.showFileUploadModal = true;
}

const loading = ref({
  systemStatus: false
});

const updatePollingInterval = ref<NodeJS.Timeout | null>(null);
watch(() => firmwareStore.fileUpdate.stage, newStage => {
  if (newStage !== UpdateStage.UPDATING) {
    if (newStage === UpdateStage.SUCCESS) {
      if (updatePollingInterval.value) {
        clearInterval(updatePollingInterval.value);
      }
      return;
    }
    return;
  }
  if (updatePollingInterval.value) {
    clearInterval(updatePollingInterval.value);
  }
  updatePollingInterval.value = setInterval(() => {
    if (loading.value.systemStatus) {
      return;
    }
    loading.value.systemStatus = true;
    deviceStore.fetchDeviceName(true) // throw the error to avoid exiting the polling
      .then(() => {
        clearInterval(updatePollingInterval.value!);
        firmwareStore.fileUpdate.stage = UpdateStage.SUCCESS;
        firmwareStore.fileUpdate.progress = 0;
      })
      .catch(() => {
        // ignore the error
      });
    loading.value.systemStatus = false;
  }, 3000);
});

async function init () {
  await deviceStore.fetchApiVersion();
  await deviceStore.fetchDeviceStatus();
  await firmwareStore.fetchAutoUpdateSelfCheck();
}

onMounted(async () => {
  await init();
  window.addEventListener('device-reconnected', init);
});
onBeforeUnmount(() => window.removeEventListener('device-reconnected', init));
</script>
