<template>
  <SectionCard
    data-id="settings-section-debug"
    icon="i-bi-bug"
    title="Debug log"
  >
    <template #subtitle>
      <p class="text-sm text-muted">Download a debug log file for sharing with
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
    const path = await deviceStore.requestDebugLogDump();
    if (!path) {
      return;
    }

    const blob = await deviceStore.fetchDebugLog(path);
    if (!blob) {
      return;
    }

    // YYYY-MM-DDTHH:mm:ss -> YYYY-MM-DD_HH-mm-ss
    const timestamp = new Date().toISOString().slice(0, 19).replace('T', '_').replaceAll(':', '-');
    downloadFile(blob, `busybar-debug-log-${timestamp}.txt`);
  } finally {
    loading.value = false;
  }
}
</script>
