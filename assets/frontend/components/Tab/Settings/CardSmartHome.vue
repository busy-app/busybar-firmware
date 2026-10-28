<template>
  <SectionCard
    data-id="settings-section-smart-home"
    icon="i-bi-smart-home"
    title="Smart home"
  >
    <template #raw-body>
      <div class="flex flex-col gap-1 rounded-group">
        <div
          data-id="settings-section-matter"
          class="bg-accented/25 dark:bg-elevated/75 p-4 flex flex-col sm:flex-row sm:items-center gap-4 sm:gap-2"
        >
          <div class="grow min-w-0 flex flex-col gap-1">
            <div class="flex items-center gap-2">
              <UIcon
                name="i-bi-logo-matter"
                class="size-6 shrink-0"
              />
              <span class="truncate">Matter</span>
            </div>
            <p
              v-if="matterStore.matterCommissioning.fabricCount > 0"
              class="text-sm text-muted"
            >
              {{ matterStore.matterCommissioning.fabricCount }} connections
            </p>
          </div>
          <div class="flex flex-col sm:flex-row gap-2">
            <UButton
              v-if="matterStore.matterCommissioning.fabricCount > 0"
              label="Forget all pairings"
              variant="outline"
              color="neutral"
              class="rounded-full justify-center"
              @click="() => { showMatterDeleteModal = true; }"
            />
            <UButton
              label="Pair device"
              icon="i-bi-plus"
              class="rounded-full justify-center"
              @click="matterStore.requestMatterLink()"
            />
          </div>
        </div>

        <div
          v-if="accessTokensStore.tokens.length > 0"
          class="bg-accented/25 dark:bg-elevated/75 overflow-hidden"
        >
          <UButton
            data-id="settings-section-home-assistant-open"
            variant="ghost"
            color="neutral"
            trailing-icon="i-bi-chevron-right"
            class="w-full p-4 gap-2 rounded-none text-base font-normal text-left"
            :ui="{ trailingIcon: 'size-6 text-muted' }"
            @click="emit('openHomeAssistant')"
          >
            <span class="grow min-w-0 flex flex-col gap-1">
              <span class="flex items-center gap-2">
                <UIcon
                  name="i-bi-logo-home-assistant"
                  class="size-6 shrink-0"
                />
                <span class="truncate">Home Assistant</span>
              </span>
              <span class="text-sm text-muted">
                {{ accessTokensStore.tokens.length }} {{ accessTokensStore.tokens.length === 1 ? 'token' : 'tokens' }}
              </span>
            </span>
          </UButton>
        </div>
        <div
          v-else
          data-id="settings-section-home-assistant"
          class="bg-accented/25 dark:bg-elevated/75 p-4 flex flex-wrap items-center gap-2"
        >
          <div class="min-h-9 grow flex items-center gap-2">
            <UIcon
              name="i-bi-logo-home-assistant"
              class="size-6 shrink-0"
            />
            <span class="whitespace-nowrap">Home Assistant</span>
          </div>
          <UButton
            variant="link"
            class="p-0 gap-1 shrink-0 whitespace-nowrap"
            :href="appConfig.links.homeAssistantDocs"
            target="_blank"
          >
            <span class="underline">How to connect</span>
            <UIcon
              name="i-bi-open-in-new"
              class="size-4"
            />
          </UButton>
        </div>
      </div>
    </template>
  </SectionCard>

  <ModalGeneric
    v-model:open="matterStore.matterLink.showModal"
    data-id="modal-matter-link"
    title="Pair device with Matter"
    :icon="colorMode.value === 'dark' ? 'i-bi-matter-bubble-dark' : 'i-bi-matter-bubble'"
    show-close-button
    no-actions
    @after:leave="onMatterLinkModalClose"
  >
    <template #icon>
      <UIcon
        :name="colorMode.value === 'dark' ? 'i-bi-matter-bubble-dark' : 'i-bi-matter-bubble'"
        class="size-10 mr-1"
      />
    </template>
    <template #body>
      <div class="flex flex-col items-center gap-4">
        <p>Scan the QR code</p>
        <canvas
          ref="matterQrCanvasRef"
          data-id="matter-link-qr-code"
          width="184"
          height="184"
          aria-label="Matter QR code"
          class="size-46 bg-white rounded-xl mb-6"
        />

        <p>Or enter the code in your smart home app</p>
        <CopyButton
          :text="matterStore.matterLink.manualCode"
          variant="subtle"
          color="neutral"
          size="xl"
          class="w-full rounded-xl p-4 pr-5 text-2xl ring-black/4 dark:ring-white/4"
        />

        <p class="w-full flex justify-end mt-6 text-muted text-sm">
          Expires in&nbsp;<CountDown :ms="matterStore.matterLink.expiresInMs" />
        </p>
      </div>
    </template>
  </ModalGeneric>

  <ModalGeneric
    v-model:open="showMatterDeleteModal"
    data-id="modal-matter-delete"
    title="Forget all pairings?"
    description="Your BUSY Bar will be removed from your smart home automations. The device will restart after removal."
    :primary-action-props="{
      label: 'Forget all pairings',
      variant: 'soft',
      color: 'error',
      onClick: deleteMatterPairings
    }"
    :secondary-action-props="{
      label: 'Cancel',
      variant: 'ghost',
      color: 'neutral',
      onClick: () => { showMatterDeleteModal = false }
    }"
  >
    <template #icon>
      <UIcon
        name="i-bi-trash"
        class="size-8"
      />
    </template>
  </ModalGeneric>

  <UModal
    v-model:open="showRebootingModal"
    data-id="modal-rebooting"
    title="Restarting BUSY Bar..."
    description="Restarting BUSY Bar..."
    :ui="{
      content: 'max-w-90',
      description: 'hidden',
      header: 'hidden',
      body: 'p-5 sm:p-5',
      close: 'hidden'
    }"
  >
    <template #body>
      <div class="flex items-center gap-4 py-4">
        <UIcon
          name="i-busy-loader"
          class="size-6 text-muted animate-spin"
        />
        <span>Restarting BUSY Bar...</span>
      </div>
    </template>
  </UModal>
</template>

<script setup lang="ts">
import { drawQrCodeOnCanvas } from '../../../util/qrCode';

const emit = defineEmits<{
  openHomeAssistant: [];
}>();

const REBOOT_TIMEOUT_MS = 60_000;
let rebootTimeout: ReturnType<typeof setTimeout> | null = null;

const matterStore = useMatterStore();
const accessTokensStore = useAccessTokensStore();
const colorMode = useColorMode();
const appConfig = useAppConfig();

const matterQrCanvasRef = ref<HTMLCanvasElement | null>(null);
const showMatterDeleteModal = ref(false);
const showRebootingModal = ref(false);

function renderMatterQrCode () {
  if (!matterQrCanvasRef.value) {
    return;
  }

  drawQrCodeOnCanvas(matterQrCanvasRef.value, matterStore.matterLink.qrCodeMatrix);
}

function onMatterLinkModalClose () {
  if (matterStore.matterLink.timeout) {
    clearTimeout(matterStore.matterLink.timeout);
  }

  matterStore.matterLink.qrCodeMatrix = null;
  matterStore.matterLink.manualCode = '';
  matterStore.matterLink.availableUntil = null;
  matterStore.matterLink.timeout = null;
  matterStore.matterLink.expiresInMs = 0;

  return matterStore.fetchMatterCommissioning();
}

function clearRebootTimeout () {
  if (rebootTimeout) {
    clearTimeout(rebootTimeout);
    rebootTimeout = null;
  }
}

async function deleteMatterPairings () {
  const erased = await matterStore.deleteAllPairings();
  showMatterDeleteModal.value = false;

  if (!erased) {
    return;
  }

  showRebootingModal.value = true;
  rebootTimeout = setTimeout(init, REBOOT_TIMEOUT_MS);
}

async function init () {
  clearRebootTimeout();
  showRebootingModal.value = false;

  await Promise.all([
    matterStore.fetchMatterCommissioning(),
    accessTokensStore.fetchTokens()
  ]);
}

watch(() => matterStore.matterCommissioning.fabricCount, (newValue, oldValue) => {
  if (matterStore.matterLink.showModal && oldValue < newValue) {
    matterStore.matterLink.showModal = false;
    onMatterLinkModalClose();
  }
});

watch(
  () => [matterStore.matterLink.showModal, matterStore.matterLink.qrCodeMatrix],
  async ([showModal, qrCodeMatrix]) => {
    if (!showModal || !qrCodeMatrix) {
      renderMatterQrCode();
      return;
    }

    await nextTick();
    renderMatterQrCode();
  },
  { deep: false }
);

onMounted(async () => {
  await init();
  renderMatterQrCode();
  window.addEventListener('device-reconnected', init);
});

onBeforeUnmount(() => {
  window.removeEventListener('device-reconnected', init);
  clearRebootTimeout();
});
</script>
