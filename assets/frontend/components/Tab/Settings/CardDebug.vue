<template>
  <div
    data-id="settings-section-debug"
    class="w-full flex flex-col md:flex-row items-start md:items-center justify-between p-6 ring-1 ring-glass rounded-3xl bg-elevated/50 gap-6 md:gap-3"
  >
    <div class="flex items-start gap-2.5">
      <UIcon
        name="i-bi-bug"
        class="size-6"
      />
      <div class="flex flex-col gap-1">
        <p class="font-medium">Debug log</p>
        <p class="text-sm text-muted">Download a debug log file for sharing with
          <a
            class="underline"
            href="https://go.busy.app/support"
            target="_blank"
          >
            support
          </a>
        </p>
      </div>
    </div>

    <div class="w-full md:w-fit flex flex-col md:flex-row items-stretch md:items-end gap-2">
      <UButton
        data-id="settings-section-debug-download-button"
        class="justify-center"
        label="Download debug log"
        icon="i-bi-download"
        variant="soft"
        color="neutral"
        :loading="loading"
        @click="handleDownload"
      />
    </div>
  </div>
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
