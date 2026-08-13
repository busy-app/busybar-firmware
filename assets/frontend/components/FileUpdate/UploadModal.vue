<template>
  <ModalGeneric
    v-model:open="firmwareStore.fileUpdate.showFileUploadModal"
    data-id="modal-update-firmware"
    title="Update firmware"
    dismissible
    show-close-button
    wide
    :no-actions="!firmwareStore.fileUpdate.firmwareFile"
    :primary-action-props="{
      label: 'Start update',
      onClick: firmwareStore.startFirmwareUpdateFromFile
    }"
    :secondary-action-props="{
      label: 'Cancel',
      onClick: () => { firmwareStore.fileUpdate.showFileUploadModal = false; firmwareStore.fileUpdate.firmwareFile = null; }
    }"
  >
    <template #body>
      <UFileUpload
        v-if="!firmwareStore.fileUpdate.firmwareFile"
        v-model="firmwareStore.fileUpdate.firmwareFile"
        data-id="modal-update-firmware-file-upload"
        accept=".tgz"
        class="w-full h-[400px] rounded-xl"
        label="Upload Firmware file (.tgz)"
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
              name="i-bi-archive"
              class="size-6"
            />
            <div data-id="modal-update-firmware-file-uploaded-name">{{ firmwareStore.fileUpdate.firmwareFile?.name || 'File name unknown' }}</div>
          </div>
          <UButton
            data-id="modal-update-firmware-file-uploaded-remove-button"
            icon="i-bi-trash"
            variant="soft"
            color="neutral"
            square
            size="lg"
            class="p-2.5"
            @click="() => { firmwareStore.fileUpdate.firmwareFile = null; }"
          />
        </div>
      </div>
    </template>
  </ModalGeneric>
</template>

<script setup lang="ts">
const deviceStore = useDeviceStore();
const firmwareStore = useFirmwareStore();

const fwVersionPolifilled = computed(() => {
  const firmware = deviceStore.deviceStatus?.firmware;
  return firmware?.version === 'unknown' ? `${firmware?.branch} ${firmware?.commit_hash}` : firmware?.version;
});
</script>
