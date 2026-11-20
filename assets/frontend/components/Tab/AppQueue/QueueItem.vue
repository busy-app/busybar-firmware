<template>
  <div
    :data-id="`app-queue-item-${item.item_id}`"
    class="flex items-center gap-3 rounded-xl bg-accented/25 p-3 dark:bg-elevated/75"
  >
    <img
      :src="icon"
      alt=""
      width="32"
      height="32"
      class="size-8 shrink-0 [image-rendering:pixelated]"
      @error="iconBroken = true"
    >

    <div class="min-w-0 flex-1">
      <div class="flex min-w-0 items-center gap-2">
        <div class="truncate font-medium">
          {{ title }}
        </div>

        <UBadge
          :label="operationLabel"
          :color="item.operation.type === 'install' ? 'primary' : 'error'"
          variant="subtle"
          size="sm"
        />
      </div>

      <div
        v-if="hasTitle"
        class="truncate text-xs text-muted"
      >
        {{ item.app_id }}
      </div>

      <div class="mt-2 flex items-center gap-3">
        <UProgress
          :model-value="progress"
          :color="item.status === 'pending' ? 'neutral' : 'primary'"
          size="sm"
          class="min-w-0 flex-1"
        />

        <div class="shrink-0 text-right text-xs text-muted">
          {{ statusLabel }}
        </div>
      </div>
    </div>

    <UButton
      v-if="item.status === 'pending'"
      :data-id="`app-queue-item-${item.item_id}-cancel-button`"
      icon="i-bi-cross"
      color="neutral"
      variant="ghost"
      :loading="cancelling"
      aria-label="Cancel operation"
      @click="emit('cancel')"
    />
    <div
      v-else
      class="w-8 text-center text-xs text-muted"
    >
      <UIcon
        name="i-busy-loader"
        class="size-5 animate-spin"
      />
    </div>
  </div>
</template>

<script setup lang="ts">
import unknownAppIcon from '@/assets/images/unknown-app-icon.png';
import type { ActiveQueueItem } from '@/stores/appQueueStore';
import { appQueueProgress, appQueueStepLabel } from '@/util/appQueue';

const props = defineProps<{
  item: ActiveQueueItem;
  cancelling?: boolean;
}>();

const emit = defineEmits<{
  (e: 'cancel'): void;
}>();

const appQueueStore = useAppQueueStore();

const iconBroken = ref(false);

watch(() => props.item.app_id, () => {
  iconBroken.value = false;
});

const title = computed(() => appQueueStore.appTitle(props.item.app_id));
const hasTitle = computed(() => appQueueStore.hasTitle(props.item.app_id));
const icon = computed(() => (iconBroken.value ? unknownAppIcon : appQueueStore.appIcon(props.item.app_id) ?? unknownAppIcon));
const operationLabel = computed(() => (props.item.operation.type === 'install' ? 'Install' : 'Remove'));

const progress = computed(() => appQueueProgress(props.item.detail?.step, props.item.detail?.progress));

const statusLabel = computed(() => {
  if (props.item.status === 'pending') {
    return 'Pending';
  }

  const step = appQueueStepLabel(props.item.detail?.step) || 'In progress';
  return progress.value === null || progress.value === undefined ? step : `${step} ${progress.value}%`;
});
</script>
