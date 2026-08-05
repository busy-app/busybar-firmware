<template>
  <SectionCard
    data-id="settings-section-debug"
    icon="i-bi-bug"
    title="Debug log"
  >
    <template #subtitle>
      <p class="text-sm text-muted">Download the dump.log file for sharing with
        <a
          class="underline"
          href="https://go.busy.app/support"
          target="_blank"
        >
          support
        </a>
      </p>
    </template>

    <template #actions>
      <UButton
        data-id="settings-section-debug-download-button"
        label="Download debug log"
        icon="i-bi-download"
        variant="soft"
        color="neutral"
        class="justify-center sm:justify-start"
        :loading="loading"
        @click="handleDownload"
      />
    </template>
  </SectionCard>
</template>

<script setup lang="ts">
const deviceStore = useDeviceStore();

const loading = ref(false);

async function handleDownload () {
  loading.value = true;

  try {
    if (!await deviceStore.requestDebugLogDump()) {
      return;
    }

    const blob = await deviceStore.fetchDebugLog();
    if (!blob) {
      return;
    }

    downloadFile(blob, 'dump.log');
  } finally {
    loading.value = false;
  }
}
</script>
