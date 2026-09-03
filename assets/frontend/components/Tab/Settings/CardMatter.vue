<template>
  <SectionCard
    data-id="settings-section-matter"
    icon="i-bi-smart-home"
    title="Smart home"
    :subtitle="matterStore.matterCommissioning.fabricCount > 0 ? `${matterStore.matterCommissioning.fabricCount} connections` : ''"
  >
    <template #actions>
      <UButton
        v-if="matterStore.matterCommissioning.fabricCount > 0"
        label="Forget all pairings"
        variant="outline"
        color="neutral"
        @click="() => { showMatterDeleteModal = true; }"
      />
      <UButton
        label="Pair device"
        icon="i-bi-plus"
        color="neutral"
        class="justify-center sm:justify-start"
        @click="matterStore.requestMatterLink()"
      />

      <ModalGeneric
        v-model:open="matterStore.matterLink.showModal"
        data-id="modal-matter-link"
        title="Pair device with Matter"
        :icon="colorMode.value === 'dark' ? 'i-bi-matter-bubble-dark' : 'i-bi-matter-bubble'"
        :dismissable="false"
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
            <div>Scan the QR code</div>
            <canvas
              ref="matterQrCanvasRef"
              data-id="matter-link-qr-code"
              width="184"
              height="184"
              aria-label="Matter QR code"
              class="size-[184px] bg-white rounded-xl mb-6"
            />

            <div>Or enter the code in your smart home app</div>
            <CopyButton
              :text="matterStore.matterLink.manualCode"
              variant="subtle"
              color="neutral"
              size="xl"
              class="w-full rounded-xl p-4 pr-5 text-2xl ring-black/4 dark:ring-white/4"
            />

            <div class="w-full flex justify-end mt-6 text-muted text-sm">
              Expires in&nbsp;<CountDown :ms="matterStore.matterLink.expiresInMs" />
            </div>
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
          content: 'max-w-[360px]',
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
            <div>Restarting BUSY Bar...</div>
          </div>
        </template>
      </UModal>
    </template>
  </SectionCard>
</template>

<script setup lang="ts">
import { drawQrCodeOnCanvas } from '../../../util/qrCode';

const matterStore = useMatterStore();
const colorMode = useColorMode();
const matterQrCanvasRef = ref<HTMLCanvasElement | null>(null);

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

const showMatterDeleteModal = ref(false);
const showRebootingModal = ref(false);

async function deleteMatterPairings () {
  await matterStore.deleteAllPairings();
  showMatterDeleteModal.value = false;
  showRebootingModal.value = true;
}

async function init () {
  showRebootingModal.value = false;
  await matterStore.fetchMatterCommissioning();
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
onBeforeUnmount(() => window.removeEventListener('device-reconnected', init));
</script>
