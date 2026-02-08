<template>
  <UModal
    v-model:open="deviceStore.autoUpdate.modals.updating"
    data-id="modal-auto-update-updating"
    title="Update firmware"
    :description="step"
    :ui="{
      content: 'max-w-[640px] divide-none bg-neutral-100/90 dark:bg-neutral-800/75 backdrop-blur-[5px] ring-1 ring-glass',
      description: 'hidden',
      header: 'hidden',
      body: 'p-0 sm:p-0 overflow-visible',
      close: 'hidden',
      overlay: 'bg-neutral-900/20 dark:bg-neutral-900/80'
    }"
  >
    <template #body>
      <div
        class="flex flex-col gap-6 p-6 bg-no-repeat"
        :style="step === UpdateStage.SUCCESS ? `background-image: url(${updateSuccessImage}); background-size: 400px; background-position: center 40px` : ''"
      >
        <div class="flex items-center justify-between text-xl font-medium">
          <div>Update firmware</div>

          <UButton
            v-if="step !== UpdateStage.UPLOADING && step !== UpdateStage.UPDATING"
            color="neutral"
            variant="ghost"
            icon="i-bi-cross"
            @click="deviceStore.autoUpdate.modals.updating = false"
          />
        </div>

        <template v-if="step === UpdateStage.UPLOADING">
          <div class="flex items-center justify-between mt-2.5">
            <div class="flex items-center gap-2.5">
              <UIcon
                name="i-bi-download"
                class="size-6"
              />
              <div>Downloading firmware</div>
            </div>

            <div class="text-muted">{{ deviceStore.autoUpdate.progress }}%</div>
          </div>

          <UProgress
            v-model="deviceStore.autoUpdate.progress"
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
              variant="ghost"
              label="Cancel"
              @click="deviceStore.abortAutoUpdateDownload"
            />
          </div>
        </template>

        <template v-if="step === UpdateStage.UPDATING">
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

        <template v-if="step === UpdateStage.SUCCESS">
          <div class="h-36" />
          <div class="text-center pb-6">
            <div class="text-lg font-medium">Update completed</div>
            <div>Your BUSY Bar is now running the latest firmware ({{ deviceStore.deviceStatus?.system?.version }}).</div>
          </div>
        </template>

        <template v-if="step === UpdateStage.ERROR">
          <div class="flex items-center gap-2.5">
            <UIcon
              name="i-bi-error-fill"
              class="size-6 text-red-600"
            />
            <div>An error occurred during the {{ deviceStore.autoUpdate.error.step === UpdateStage.UPLOADING ? 'download' : 'update' }}</div>
          </div>

          <div class="p-4 bg-accented/25 rounded-xl text-sm">
            <MDC
              :value="deviceStore.autoUpdate.error.step === UpdateStage.UPLOADING ? downloadErrorMarkdown : updateErrorMarkdown"
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
import type { UpdateStage } from '@/stores/deviceStore';

const deviceStore = useDeviceStore();
const step = computed(() => deviceStore.autoUpdate.step);

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
- Make sure you are using the correct update package (.tar)
- Restart your BUSY Bar (hold Start and Back for 3 seconds, then release), and try again
- Charge device completely, and try again

If none of the above steps helped, please contact our [Customer Support team](https://support.busy.app).
`;

const indeterminateProgressModel = ref(null);
</script>
