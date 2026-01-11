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

      <ModalGeneric
        v-model:open="showUpdateModal"
        data-id="modal-update-firmware"
        title="Update firmware"
        :dismissible="stage === 'idle' || stage === 'error' || stage === 'success'"
        :show-close-button="stage === 'idle' || stage === 'error' || stage === 'success'"
        wide
        :no-actions="stage === 'idle' && !firmwareFileModel || stage === 'unpacking' || stage === 'updating' || stage === 'success'"
        :primary-action-props="
          stage === 'idle' ? {
            label: 'Start update',
            onClick: startFirmwareUpdateFromFile
          } : stage === 'uploading' ? {
            label: 'Cancel',
            color: 'neutral',
            onClick: reload
          } : stage === 'error' ? {
            class: 'hidden'
          } : {}
        "
        :secondary-action-props="
          stage === 'idle' ? {
            label: 'Cancel',
            onClick: () => { showUpdateModal = false; firmwareFileModel = null; }
          } : stage === 'uploading' || stage === 'error' ? {
            class: 'hidden'
          } : {}
        "
        @close:prevent="stage === 'success' ? reload() : null"
      >
        <template #body>
          <template v-if="stage === 'idle'">
            <UFileUpload
              v-if="!firmwareFileModel"
              v-model="firmwareFileModel"
              data-id="modal-update-firmware-file-upload"
              accept=".tar"
              class="w-full h-[400px] rounded-xl"
              label="Upload Firmware file (.tar)"
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

          <template v-else-if="stage !== 'error'">
            <div
              data-id="modal-update-in-progress"
              class="flex flex-col gap-6"
            >
              <div class="flex flex-col gap-2">
                <div
                  v-if="stage === 'uploading'"
                  data-id="modal-update-in-progress-stage-uploading-message"
                >
                  Update in progress. Do not disconnect the cable.
                </div>
                <div
                  v-else-if="stage === 'updating'"
                  data-id="modal-update-in-progress-stage-updating-message"
                >
                  Update in progress.
                </div>
                <div class="text-muted">Current version: {{ fwVersionPolifilled }}</div>
              </div>

              <div class="flex items-center gap-2">
                <CircularProgress
                  v-if="stage === 'uploading'"
                  v-model="deviceStore.firmwareUpdate.progress"
                  data-id="modal-update-in-progress-uploading-circular"
                  size="32px"
                  :thickness="0.25"
                  color="#6A7282"
                  track-color="#99A1AF26"
                />
                <CircularProgress
                  v-if="stage === 'unpacking'"
                  v-model="indeterminateProgressModel"
                  data-id="modal-update-in-progress-unpacking-circular"
                  size="32px"
                  :thickness="0.25"
                  color="#6A7282"
                  track-color="#99A1AF26"
                  class="animate-spin"
                />
                <div
                  v-if="stage !== 'uploading' && stage !== 'unpacking'"
                  data-id="modal-update-in-progress-upload-success-icon"
                  class="flex p-2 rounded-full bg-green-500/15"
                >
                  <UIcon
                    name="i-ri-check-line"
                    class="size-4 text-green-500"
                  />
                </div>
                <div class="text-sm">
                  <div class="font-medium">Uploading {{ firmwareFileModel?.name || 'firmware package' }}</div>
                  <div
                    v-if="stage === 'uploading'"
                    data-id="modal-update-in-progress-uploading-percentage"
                    class="text-neutral-600 dark:text-neutral-300"
                  >
                    {{ deviceStore.firmwareUpdate.progress }}%
                  </div>
                  <div
                    v-else-if="stage === 'unpacking'"
                    data-id="modal-update-in-progress-unpacking-message"
                    class="text-neutral-600 dark:text-neutral-300"
                  >
                    Unpacking firmware package
                  </div>
                </div>
              </div>

              <div
                v-if="stage === 'updating' || stage === 'success'"
                class="relative flex items-center gap-2"
              >
                <USeparator
                  orientation="vertical"
                  class="absolute h-3 left-4 -top-4"
                  :ui="{
                    border: 'border-neutral-300 dark:border-neutral-600'
                  }"
                />
                <CircularProgress
                  v-if="stage === 'updating'"
                  v-model="indeterminateProgressModel"
                  data-id="modal-update-in-progress-updating-circular"
                  size="32px"
                  :thickness="0.25"
                  color="#6A7282"
                  track-color="#99A1AF26"
                  class="animate-spin"
                />
                <div
                  v-else-if="stage === 'success'"
                  data-id="modal-update-in-progress-full-update-success-icon"
                  class="flex p-2 rounded-full bg-green-500/15"
                >
                  <UIcon
                    name="i-ri-check-line"
                    class="size-4 text-green-500"
                  />
                </div>
                <div class="text-sm">
                  <div class="font-medium">Installing firmware</div>
                  <div
                    v-if="stage === 'updating'"
                    class="text-neutral-600 dark:text-neutral-300"
                  >
                    Check status on the BUSY Bar screen
                  </div>
                </div>
              </div>
            </div>
          </template>

          <template v-else>
            <div
              data-id="modal-update-firmware-error-message"
              class="flex flex-col gap-6"
            >
              <div>Failed to install the update.</div>
              <div class="flex items-center gap-2">
                <div class="flex p-2 rounded-full bg-red-500/15">
                  <UIcon
                    name="i-ri-error-warning-line"
                    class="size-4 text-red-500"
                  />
                </div>
                <div class="text-sm">
                  <div class="font-medium">Upload failed</div>
                  <div class="text-neutral-600 dark:text-neutral-300">An error occurred during the upload</div>
                </div>
              </div>
            </div>
          </template>
        </template>
      </ModalGeneric>

      <!-- <UButton
        label="Update"
        icon="i-ri-download-cloud-line"
        :ui="{
          base: 'px-2.5 py-2 rounded-full'
        }"
        class="justify-center sm:justify-start"
      /> -->
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
const deviceScreenStreamStore = useDeviceScreenStreamStore();

const loading = ref({
  systemStatus: false
});

const system = computed(() => deviceStore.deviceStatus?.system);
const fwVersionPolifilled = computed(() => system.value?.version === 'unknown' ? `${system.value.branch} ${system.value.commit_hash}` : system.value?.version);

const showUpdateModal = ref(false);
const stage = computed(() => deviceStore.firmwareUpdate.stage);

const firmwareFileModel = ref<File | null>(null);

const indeterminateProgressModel = ref(75);

function reload () {
  location.reload();
}

function initFirmwareUpdateFromFile () {
  deviceStore.firmwareUpdate.stage = 'idle' as UpdateStage;
  deviceStore.firmwareUpdate.progress = 0;
  deviceStore.firmwareUpdate.error = '';
  deviceStore.firmwareUpdate.firmwareFile = null;
  firmwareFileModel.value = null;
  showUpdateModal.value = true;
}

async function startFirmwareUpdateFromFile () {
  deviceStore.firmwareUpdate.firmwareFile = firmwareFileModel.value;
  try {
    await deviceScreenStreamStore.stopScreenStream();
    await deviceStore.uploadFirmware();
    if (deviceStore.firmwareUpdate.stage !== 'error') {
      deviceStore.firmwareUpdate.stage = 'updating';
    }
  } catch (error) {
    console.error('Firmware update failed:', error);
    deviceStore.firmwareUpdate.stage = 'error';
    deviceStore.firmwareUpdate.error = error instanceof Error ? error.message : 'Unknown error';
  }
}

const updatePollingInterval = ref<NodeJS.Timeout | null>(null);
watch(() => deviceStore.firmwareUpdate.stage, newStage => {
  if (newStage !== 'updating') {
    if (newStage === 'success') {
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
    deviceStore.fetchSystemStatus(true) // throw the error to avoid exiting the polling
      .then(() => {
        clearInterval(updatePollingInterval.value!);
        deviceStore.firmwareUpdate.stage = 'success';
        deviceStore.firmwareUpdate.progress = 0;
      })
      .catch(() => {
        // ignore the error
      });
    loading.value.systemStatus = false;
  }, 3000);
});

onMounted(async () => {
  await deviceStore.getApiVersion();
  await deviceStore.getDeviceStatus();
});
</script>
