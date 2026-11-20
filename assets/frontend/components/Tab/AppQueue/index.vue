<template>
  <div class="flex flex-col gap-4">
    <TabAppQueueCard />
    <TabAppQueueCatalogCard />
  </div>
</template>

<script setup lang="ts">
import { appQueueErrorLabel } from '@/util/appQueue';

const appQueueStore = useAppQueueStore();
const appsStore = useAppsStore();
const toast = useToast();

watch(() => appQueueStore.lastFinished, finished => {
  if (!finished) {
    return;
  }

  const { entry } = finished;
  const title = appQueueStore.appTitle(entry.app_id);

  if (entry.outcome.kind === 'success') {
    toast.add({
      title: entry.operation.type === 'install' ? 'App installed' : 'App removed',
      description: title,
      icon: 'i-bi-checkmark-circle-fill',
      color: 'success'
    });
  } else if (entry.outcome.kind === 'error' && entry.outcome.error === 'cancelled') {
    toast.add({
      title: 'Operation cancelled',
      description: title,
      icon: 'i-bi-cross',
      color: 'neutral'
    });
  } else if (entry.outcome.kind === 'error') {
    toast.add({
      title: entry.operation.type === 'install' ? 'Install failed' : 'Removal failed',
      description: `${title}: ${appQueueErrorLabel(entry.outcome.error)}`,
      icon: 'i-bi-error-fill',
      color: 'error'
    });
  }
});

onMounted(async () => {
  appQueueStore.startSync();

  await Promise.allSettled([
    appQueueStore.refreshQueue(),
    appQueueStore.fetchCatalog(),
    // The installed list is used for the button states and app names; the Apps tab keeps using the same store.
    appsStore.apps.length > 0 ? Promise.resolve() : appsStore.fetchApps()
  ]);
});

onBeforeUnmount(() => {
  appQueueStore.stopSync();
});
</script>
