<template>
  <UModal
    v-model:open="firmwareStore.autoUpdate.modals.updating"
    data-id="modal-auto-update-updating"
    title="Update firmware"
    :dismissible="stage !== UpdateStage.LOADING && stage !== UpdateStage.UPDATING"
    :description="String(stage)"
    :ui="{
      content: 'max-w-[640px]',
      description: 'hidden',
      header: 'hidden',
      body: 'p-0 sm:p-0 overflow-visible',
      close: 'hidden'
    }"
  >
    <template #body>
      <div
        class="flex flex-col gap-6 p-6 bg-no-repeat"
        :style="stage === UpdateStage.SUCCESS ? `background-image: url(${updateSuccessImage}); background-size: 400px; background-position: center 40px` : ''"
      >
        <div class="flex items-center justify-between text-xl font-medium">
          <div>Update firmware</div>

          <UButton
            v-if="stage !== UpdateStage.LOADING && stage !== UpdateStage.UPDATING"
            color="neutral"
            variant="ghost"
            icon="i-bi-cross"
            @click="handleModalClose"
          />
        </div>

        <template v-if="stage === UpdateStage.LOADING">
          <div class="flex items-center justify-between mt-2.5">
            <div class="flex items-center gap-2.5">
              <UIcon
                name="i-bi-download"
                class="size-6"
              />
              <div>{{ updateType === 'file' ? 'Uploading firmware' : 'Downloading firmware' }}</div>
            </div>

            <div class="text-muted">{{ firmwareStore.autoUpdate.progress }}%</div>
          </div>

          <UProgress
            v-model="firmwareStore.autoUpdate.progress"
            size="lg"
            color="success"
            class="mb-2.5"
            :ui="{
              indicator: 'duration-500'
            }"
          />

          <div class="w-full flex justify-end">
            <UButton
              color="neutral"
              size="lg"
              variant="ghost"
              label="Cancel"
              @click="handleAbortDownload"
            />
          </div>
        </template>

        <template v-if="stage === UpdateStage.UPDATING">
          <div class="flex items-center justify-between mt-2.5">
            <div class="flex items-center gap-2.5">
              <UIcon
                name="i-bi-firmware"
                class="size-6"
              />
              <div>Installing firmware</div>
            </div>

            <div class="text-muted">View status on BUSY Bar screen</div>
          </div>

          <UProgress
            v-model="indeterminateProgressModel"
            size="lg"
            color="success"
            class="mb-2.5"
          />
        </template>

        <template v-if="stage === UpdateStage.SUCCESS">
          <div class="h-36" />
          <div class="text-center pb-6">
            <div class="text-lg font-medium">Update completed</div>
            <div>Your BUSY Bar is now running the updated firmware ({{ deviceStore.deviceStatus?.system?.version }}).</div>
          </div>
        </template>

        <template v-if="stage === UpdateStage.ERROR">
          <div class="flex items-center gap-2.5">
            <UIcon
              name="i-bi-error-fill"
              class="size-6 text-red-600"
            />
            <div>An error occurred during the {{ firmwareStore.autoUpdate.error.stage === UpdateStage.LOADING ? 'download' : 'update' }}</div>
          </div>

          <div class="p-4 bg-accented/25 rounded-xl text-sm">
            <MDC
              :value="firmwareStore.autoUpdate.error.stage === UpdateStage.LOADING ? downloadErrorMarkdown : updateErrorMarkdown"
              tag="article"
            />
          </div>
        </template>
      </div>
    </template>
  </UModal>
</template>

<script lang="ts" setup>
import updateSuccessImage from '@/assets/images/update-success.png';

const deviceStore = useDeviceStore();
const firmwareStore = useFirmwareStore();
const updateType = computed(() => firmwareStore.fileUpdate.stage !== UpdateStage.IDLE ? 'file' : 'auto');
const stage = computed(() => {
  if (updateType.value === 'file') {
    return firmwareStore.fileUpdate.stage;
  }
  return firmwareStore.autoUpdate.stage;
});

function handleAbortDownload () {
  if (firmwareStore.fileUpdate.stage === UpdateStage.LOADING) {
    window.location.reload();
  }
  firmwareStore.abortAutoUpdateDownload();
}

function handleModalClose () {
  firmwareStore.fileUpdate.stage = UpdateStage.IDLE;
  firmwareStore.autoUpdate.stage = UpdateStage.IDLE;
  firmwareStore.autoUpdate.modals.updating = false;
}

const downloadErrorMarkdown = `
- Ensure you have a stable internet connection
- Make sure you are using the latest browser version
- Wait a few minutes, refresh the page, and try again
- Go to your browser's settings and find the option to clear browsing cache, close the browser and reopen it, and try again
- Try to update using a different browser
- Try connecting to another Wi-Fi network

If none of the above steps helped, please contact our [Customer Support team](https://support.busy.app).
`;
const updateErrorMarkdown = `
- Make sure you are using the correct update package (.tar or .tgz)
- Restart your BUSY Bar (hold Start and Back for 3 seconds, then release), and try again
- Charge device completely, and try again

If none of the above steps helped, please contact our [Customer Support team](https://support.busy.app).
`;

const indeterminateProgressModel = ref(null);
</script>
