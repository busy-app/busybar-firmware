<template>
  <ModalGeneric
    v-model:open="telemetryStore.showConsentModal"
    data-id="modal-telemetry-consent"
    title="Help improve BUSY Bar"
    header-class="pb-2 sm:pb-2"
    :dismissible="false"
  >
    <template #body>
      <section class="flex flex-col gap-2">
        <p class="text-muted">
          BUSY Bar can share usage and device statistics with us.
          <br>
          It stays off unless you turn it on.
        </p>

        <div class="flex flex-col gap-3">
          <div class="rounded-xl bg-accented/25 p-3 dark:bg-elevated">
            <div class="flex max-h-54 flex-col gap-6 overflow-y-scroll pr-2 text-muted [&::-webkit-scrollbar]:w-1 [&::-webkit-scrollbar-thumb]:rounded-full [&::-webkit-scrollbar-thumb]:bg-accented">
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
                v-if="appConfig.links.telemetryDataList"
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
        </div>
      </section>
    </template>

    <template #actions>
      <footer class="mt-2 flex justify-end gap-2">
        <UButton
          data-id="modal-telemetry-consent-decline"
          label="Don't share"
          variant="outline"
          color="neutral"
          size="lg"
          class="min-w-20 justify-center px-4"
          :disabled="telemetryStore.saving"
          @click="telemetryStore.answerConsent(false)"
        />
        <UButton
          data-id="modal-telemetry-consent-accept"
          label="Share data"
          variant="outline"
          color="neutral"
          size="lg"
          class="min-w-20 justify-center px-4"
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
