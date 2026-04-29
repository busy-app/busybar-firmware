<template>
  <TabDrawToolStatusGallery
    v-if="!isEditorOpen"
    @new-status="openEditor"
  />

  <TabDrawToolEditor
    v-else
    @back="handleEditorBack"
  />
</template>

<script setup lang="ts">
const dts = useDrawToolStore();
const editorStore = useDrawToolEditorStore();

const isEditorOpen = ref(false);
const statusDirectoryRefreshTimer = ref<number | null>(null);

const STATUS_DIRECTORY_REFRESH_INTERVAL_MS = 30000;

function openEditor () {
  editorStore.resetEditor();
  isEditorOpen.value = true;
}

async function handleEditorBack () {
  isEditorOpen.value = false;
  await dts.refreshStatusDirectory();
}

onMounted(() => {
  void dts.refreshStatusDirectory();

  statusDirectoryRefreshTimer.value = window.setInterval(() => {
    void dts.refreshStatusDirectory({ silent: true });
  }, STATUS_DIRECTORY_REFRESH_INTERVAL_MS);
});

onBeforeUnmount(() => {
  if (statusDirectoryRefreshTimer.value !== null) {
    window.clearInterval(statusDirectoryRefreshTimer.value);
  }
});
</script>
