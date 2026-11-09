<template>
  <ModalGeneric
    v-model:open="telemetryStore.showConsentModal"
    data-id="modal-telemetry-consent"
    title="Help improve BUSY Bar"
    :dismissible="false"
  >
    <template #body>
      <section class="flex flex-col gap-4">
        <p class="text-muted">
          BUSY Bar can share usage and device statistics with us. It stays off unless you turn it on.
        </p>

        <div class="rounded-xl bg-accented/25 dark:bg-elevated/75 py-3 pl-3 pr-2">
          <div class="max-h-53 overflow-y-scroll pr-3 flex flex-col gap-6 text-muted [&::-webkit-scrollbar]:w-1 [&::-webkit-scrollbar-thumb]:rounded-full [&::-webkit-scrollbar-thumb]:bg-accented">
            <section class="flex flex-col gap-6">
              <p>If you turn it on, we only collect:</p>
              <ul class="list-disc pl-6 text-default">
                <li
                  v-for="item in COLLECTED_DATA"
                  :key="item"
                >
                  {{ item }}
                </li>
              </ul>
            </section>
            <p>We never collect sound, images, screen contents, Wi&#8209;Fi names or passwords, or your name or email.</p>
            <p>
              Flipper FZCO uses this data to improve BUSY Bar, fix problems, and count devices anonymously, such as per city.
              It’s not used for ads or marketing. Data is linked to this device’s serial number.
            </p>
            <ULink
              data-id="modal-telemetry-consent-data-list"
              class="w-fit text-primary underline"
              :href="appConfig.links.telemetryDataList"
              target="_blank"
            >
              Full list of data
            </ULink>
          </div>
        </div>

        <p class="text-muted">Change anytime in Settings → Share device stats</p>
      </section>
    </template>

    <template #actions>
      <footer class="flex justify-end gap-2 mt-2">
        <UButton
          data-id="modal-telemetry-consent-decline"
          label="Don't share"
          variant="outline"
          color="neutral"
          size="lg"
          class="min-w-20 px-4 justify-center"
          :disabled="telemetryStore.saving"
          @click="telemetryStore.answerConsent(false)"
        />
        <UButton
          data-id="modal-telemetry-consent-accept"
          label="Share data"
          variant="outline"
          color="neutral"
          size="lg"
          class="min-w-20 px-4 justify-center"
          :disabled="telemetryStore.saving"
          @click="telemetryStore.answerConsent(true)"
        />
      </footer>
    </template>
  </ModalGeneric>
</template>

<script setup lang="ts">
const COLLECTED_DATA = [
  'Timers, apps, and settings you use',
  'Dial and button use, as totals',
  'Battery, network, and firmware status',
  'Matter and BUSY account status'
];

const telemetryStore = useTelemetryStore();
const appConfig = useAppConfig();
</script>
