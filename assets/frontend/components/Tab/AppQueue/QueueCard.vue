<template>
  <SectionCard
    data-id="app-queue-section-queue"
    title="App operation queue"
    icon="i-bi-layer-down"
    :ui="{ title: 'font-medium' }"
  >
    <template #actions>
      <div class="flex items-center gap-2">
        <UButton
          data-id="app-queue-section-queue-refresh-button"
          icon="i-bi-refresh"
          color="neutral"
          variant="outline"
          :loading="appQueueStore.queueLoading"
          aria-label="Refresh queue"
          @click="appQueueStore.refreshQueue()"
        />

        <UButton
          data-id="app-queue-section-queue-cancel-all-button"
          label="Cancel all"
          icon="i-bi-cross"
          color="error"
          variant="outline"
          :disabled="!appQueueStore.hasActiveOperations || cancellingAll"
          :loading="cancellingAll"
          @click="handleCancelAll"
        />
      </div>
    </template>

    <template #raw-body>
      <div class="flex flex-col gap-2">
        <div
          v-if="appQueueStore.queueError"
          data-id="app-queue-section-queue-error"
          class="flex items-center gap-2 rounded-xl bg-accented/25 p-4 text-sm text-muted dark:bg-elevated/75"
        >
          <UIcon
            name="i-bi-alert"
            class="size-5 shrink-0 text-red-600"
          />
          <span class="min-w-0 flex-1">{{ appQueueStore.queueError }}</span>
          <UButton
            label="Retry"
            color="neutral"
            variant="ghost"
            @click="appQueueStore.refreshQueue()"
          />
        </div>

        <div
          v-if="appQueueStore.activeItems.length === 0 && appQueueStore.finishedItems.length === 0 && !appQueueStore.queueLoading"
          data-id="app-queue-section-queue-empty"
          class="flex flex-col items-center justify-center gap-2 rounded-xl border border-accented py-8 text-center"
        >
          <UIcon
            name="i-bi-layer-down"
            class="size-8 text-muted"
          />
          <div class="text-muted">
            Queue is empty
          </div>
          <div class="max-w-96 text-sm text-balance text-muted">
            Install or remove an app from the catalog below to see the operation here.
          </div>
        </div>

        <TabAppQueueItem
          v-for="item in activeItems"
          :key="`active-${item.item_id}`"
          :item="item"
          :cancelling="cancellingIds.includes(item.item_id)"
          @cancel="handleCancel(item)"
        />

        <template v-if="finishedItems.length > 0">
          <div class="mt-2 px-2 text-xs text-muted">
            Recently finished
          </div>

          <TabAppQueueFinishedItem
            v-for="item in finishedItems"
            :key="`finished-${item.item_id}`"
            :item="item"
          />
        </template>
      </div>
    </template>
  </SectionCard>
</template>

<script setup lang="ts">
import type { ActiveQueueItem, FinishedQueueItem } from '@/stores/appQueueStore';

const appQueueStore = useAppQueueStore();
const toast = useToast();

const cancellingAll = ref(false);
const cancellingIds = ref<number[]>([]);

// The newest finished operation goes on top, the oldest at the bottom.
const finishedItems = computed<FinishedQueueItem[]>(() => [...appQueueStore.finishedItems].reverse());
const activeItems = computed<ActiveQueueItem[]>(() => [...appQueueStore.activeItems].sort((a, b) => a.item_id - b.item_id));

async function handleCancel (item: ActiveQueueItem) {
  if (cancellingIds.value.includes(item.item_id)) {
    return;
  }

  cancellingIds.value = [...cancellingIds.value, item.item_id];
  try {
    await appQueueStore.cancelItem(item.item_id);
  } catch (error) {
    toast.add({
      title: 'Failed to cancel the operation',
      description: error instanceof Error ? error.message : undefined,
      icon: 'i-bi-error-fill',
      color: 'error'
    });
  } finally {
    cancellingIds.value = cancellingIds.value.filter(id => id !== item.item_id);
  }
}

async function handleCancelAll () {
  cancellingAll.value = true;
  try {
    await appQueueStore.cancelAll();
  } catch (error) {
    toast.add({
      title: 'Failed to cancel the queue',
      description: error instanceof Error ? error.message : undefined,
      icon: 'i-bi-error-fill',
      color: 'error'
    });
  } finally {
    cancellingAll.value = false;
  }
}
</script>
