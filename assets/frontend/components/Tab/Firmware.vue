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
        icon="i-ri-upload-2-line"
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
          :loading="deviceStore.autoUpdate.isChecking"
          @click="deviceStore.autoUpdate.isManualCheck = true; deviceStore.requestAutoUpdateCheck()"
        />
      </UTooltip>

      <!-- File upload modal (only for idle stage) -->
      <ModalGeneric
        v-model:open="showUpdateModal"
        data-id="modal-update-firmware"
        title="Update firmware"
        dismissible
        show-close-button
        wide
        :no-actions="!firmwareFileModel"
        :primary-action-props="{
          label: 'Start update',
          onClick: startFirmwareUpdateFromFile
        }"
        :secondary-action-props="{
          label: 'Cancel',
          onClick: () => { showUpdateModal = false; firmwareFileModel = null; }
        }"
      >
        <template #body>
          <UFileUpload
            v-if="!firmwareFileModel"
            v-model="firmwareFileModel"
            data-id="modal-update-firmware-file-upload"
            accept=".tar,.tgz"
            class="w-full h-[400px] rounded-xl"
            label="Upload Firmware file (.tar/.tgz)"
            description="Drag and drop to upload"
            :ui="{
              icon: 'size-6',
              label: 'text-lg',
              description: 'text-sm'
            }"
          >
            <template #actions>
              <UButton
                label="Select file"
                color="neutral"
                variant="soft"
                :ui="{ base: 'bg-neutral-200/50 dark:bg-neutral-700/50' }"
                class="mt-2"
              />
            </template>
          </UFileUpload>
          <div
            v-else
            data-id="modal-update-firmware-file-uploaded"
            class="flex flex-col gap-6"
          >
            <div class="flex flex-col gap-2">
              <div>This file will be uploaded to your BUSY Bar and its firmware will be updated.</div>
              <div class="text-muted">Current version: {{ fwVersionPolifilled }}</div>
            </div>

            <div class="flex justify-between items-center p-3 ring-1 ring-muted rounded-xl">
              <div class="flex items-center gap-4">
                <UIcon
                  name="i-ri-file-zip-line"
                  class="size-6"
                />
                <div data-id="modal-update-firmware-file-uploaded-name">{{ firmwareFileModel?.name || 'File name unknown' }}</div>
              </div>
              <UButton
                data-id="modal-update-firmware-file-uploaded-remove-button"
                icon="i-ri-delete-bin-7-line"
                variant="soft"
                color="neutral"
                square
                size="lg"
                class="p-2.5"
                @click="firmwareFileModel = null"
              />
            </div>
          </div>
        </template>
      </ModalGeneric>
    </template>

    <div class="grid sm:grid-cols-2 gap-y-3 gap-x-1">
      <div
        v-for="[property, value] in Object.entries({
          'Version': system?.version,
          'Build date': system?.build_date,
          'Branch': system?.branch,
          'Commit hash': system?.commit_hash
        })"
        :key="property"
        class="flex"
      >
        <div class="w-[120px] text-muted">{{ property }}</div>
        <div class="max-w-[140px] md:max-w-[180px] text-ellipsis overflow-hidden">{{ value }}</div>
      </div>
    </div>
  </SectionCard>
</template>

<script setup lang="ts">
const deviceStore = useDeviceStore();
const wifiStore = useWifiStore();
const deviceScreenStreamStore = useDeviceScreenStreamStore();

const loading = ref({
  systemStatus: false
});

const system = computed(() => deviceStore.deviceStatus?.system);
const fwVersionPolifilled = computed(() => system.value?.version === 'unknown' ? `${system.value.branch} ${system.value.commit_hash}` : system.value?.version);

const showUpdateModal = ref(false);

const firmwareFileModel = ref<File | null>(null);

async function initFirmwareUpdateFromFile () {
  await deviceStore.fetchDeviceStatus();
  const charge = deviceStore.deviceStatus?.power?.battery_charge;

  if (charge !== undefined && charge < 40) {
    deviceStore.autoUpdate.modals.batteryLow = true;
    return;
  }

  deviceStore.fileUpdate.stage = UpdateStage.IDLE;
  deviceStore.fileUpdate.progress = 0;
  deviceStore.fileUpdate.error = '';
  deviceStore.fileUpdate.firmwareFile = null;
  firmwareFileModel.value = null;
  showUpdateModal.value = true;
}

async function startFirmwareUpdateFromFile () {
  deviceStore.fileUpdate.firmwareFile = firmwareFileModel.value;
  try {
    await deviceScreenStreamStore.stopScreenStream();

    // temp
    showUpdateModal.value = false;
    deviceStore.autoUpdate.modals.updating = true;

    await deviceStore.uploadFirmware();
    if (deviceStore.fileUpdate.stage !== UpdateStage.ERROR) {
      deviceStore.fileUpdate.stage = UpdateStage.UPDATING;
    }
  } catch (error) {
    console.error('Firmware update failed:', error);
    deviceStore.fileUpdate.stage = UpdateStage.ERROR;
    deviceStore.fileUpdate.error = error instanceof Error ? error.message : 'Unknown error';
  }
}

const updatePollingInterval = ref<NodeJS.Timeout | null>(null);
watch(() => deviceStore.fileUpdate.stage, newStage => {
  if (newStage !== UpdateStage.UPDATING) {
    if (newStage === UpdateStage.SUCCESS) {
      if (updatePollingInterval.value) {
        clearInterval(updatePollingInterval.value);
      }
      deviceStore.fetchDeviceStatus()
        .then(status => {
          deviceStore.deviceStatus = status;
        });
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
        deviceStore.fileUpdate.stage = UpdateStage.SUCCESS;
        deviceStore.fileUpdate.progress = 0;
      })
      .catch(() => {
        // ignore the error
      });
    loading.value.systemStatus = false;
  }, 3000);
});

async function init () {
  await deviceStore.getApiVersion();
  await deviceStore.getDeviceStatus();
}

onMounted(async () => {
  await init();
  window.addEventListener('device-reconnected', init);
});
onBeforeUnmount(() => window.removeEventListener('device-reconnected', init));
</script>
