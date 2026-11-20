<template>
  <div
    :data-id="`app-queue-finished-item-${item.item_id}`"
    class="flex items-center gap-3 rounded-xl border border-accented/60 p-3 opacity-80"
  >
    <img
      :src="icon"
      alt=""
      width="32"
      height="32"
      class="size-8 shrink-0 grayscale [image-rendering:pixelated]"
      @error="iconBroken = true"
    >

    <div class="min-w-0 flex-1">
      <div class="flex min-w-0 items-center gap-2">
        <div class="truncate font-medium">
          {{ title }}
        </div>

        <UBadge
          :label="operationLabel"
          color="neutral"
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
    </div>

    <UBadge
      :label="outcomeLabel"
      :color="outcomeColor"
      variant="subtle"
      size="sm"
      :icon="outcomeIcon"
    />
  </div>
</template>

<script setup lang="ts">
import unknownAppIcon from '@/assets/images/unknown-app-icon.png';
import type { FinishedQueueItem } from '@/stores/appQueueStore';
import { appQueueErrorLabel } from '@/util/appQueue';

const props = defineProps<{
  item: FinishedQueueItem;
}>();

const appQueueStore = useAppQueueStore();

const iconBroken = ref(false);

const title = computed(() => appQueueStore.appTitle(props.item.app_id));
const hasTitle = computed(() => appQueueStore.hasTitle(props.item.app_id));
const icon = computed(() => (iconBroken.value ? unknownAppIcon : appQueueStore.appIcon(props.item.app_id) ?? unknownAppIcon));
const operationLabel = computed(() => (props.item.operation.type === 'install' ? 'Install' : 'Remove'));

const outcome = computed(() => props.item.outcome);

const outcomeLabel = computed(() => {
  if (outcome.value.kind === 'error' && outcome.value.error === 'cancelled') {
    return 'Cancelled';
  }

  if (outcome.value.kind === 'error') {
    return appQueueErrorLabel(outcome.value.error);
  }

  return 'Finished';
});

const outcomeColor = computed(() => {
  if (outcome.value.kind === 'success') {
    return 'success' as const;
  }

  if (outcome.value.kind === 'error' && outcome.value.error !== 'cancelled') {
    return 'error' as const;
  }

  return 'neutral' as const;
});

const outcomeIcon = computed(() => {
  if (outcome.value.kind === 'success') {
    return 'i-bi-checkmark-circle-fill';
  }

  if (outcome.value.kind === 'error' && outcome.value.error !== 'cancelled') {
    return 'i-bi-error-fill';
  }

  return undefined;
});
</script>
