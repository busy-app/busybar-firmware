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
      <div class="flex flex-col gap-6 p-6 bg-no-repeat">
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
              v-if="firmwareStore.autoUpdate.progress < 100"
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

        <template v-if="stage === UpdateStage.ERROR">
          <div class="flex items-center gap-2.5">
            <UIcon
              name="i-bi-error-fill"
              class="size-6 text-red-600"
            />
            <div>
              {{ firmwareStore.autoUpdate.error.stage === UpdateStage.LOADING ? 'Unable to download update.' : 'Update failed.' }}
              Try these steps until the issue is resolved:
            </div>
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
1. Check internet connection on your BUSY Bar\n
    [How to connect BUSY Bar to Wi-Fi](https://go.busy.app/bar-wifi-connect)
2. Restart your BUSY Bar by holding Start and Back buttons for 3 seconds\n
    [How to restart BUSY Bar](https://go.busy.app/bar-restart)
3. Charge your BUSY Bar up to 40%\n
    [How to charge BUSY Bar](https://go.busy.app/bar-battery-charge)
4. Update your BUSY Bar from another device via BUSY App\n
    [How to update BUSY Bar firmware](https://go.busy.app/bar-firmware-update)
5. Try again later — the update server may be unavailable right now
`;
const updateErrorMarkdown = `
1. Restart your BUSY Bar by holding Start and Back buttons for 3 seconds\n
    [How to restart BUSY Bar](https://go.busy.app/bar-restart)
3. Charge your BUSY Bar up to 40%\n
    [How to charge BUSY Bar](https://go.busy.app/bar-battery-charge)
4. Update your BUSY Bar from another device via BUSY App\n
    [How to update BUSY Bar firmware](https://go.busy.app/bar-firmware-update)
5. If you’re using a custom file, make sure it’s the correct package (.tgz)
`;

const indeterminateProgressModel = ref(null);
</script>
