<template>
  <div
    :data-id="`app-queue-catalog-app-${app.alias}`"
    class="flex flex-col gap-3 rounded-xl bg-accented/25 p-3 sm:flex-row sm:items-center dark:bg-elevated/75"
  >
    <div class="flex min-w-0 flex-1 items-start gap-3">
      <img
        :src="icon"
        alt=""
        width="40"
        height="40"
        class="mt-0.5 size-10 shrink-0 [image-rendering:pixelated]"
        @error="iconBroken = true"
      >

      <div class="flex min-w-0 flex-1 flex-col gap-1">
        <div class="flex min-w-0 flex-wrap items-center gap-2">
          <div class="truncate font-medium">
            {{ title }}
          </div>

          <UBadge
            v-if="app.is_official"
            label="Official"
            color="primary"
            variant="subtle"
            size="sm"
          />

          <span
            v-if="version"
            class="text-xs text-muted"
          >
            v{{ version }}{{ installed ? ' installed' : '' }}
          </span>
        </div>

        <div
          v-if="description"
          class="text-sm text-muted"
        >
          {{ description }}
        </div>

        <div
          v-if="activeItem"
          data-id="app-queue-catalog-app-progress"
          class="mt-1 flex items-center gap-2"
        >
          <UProgress
            :model-value="progress"
            :color="activeItem.status === 'pending' ? 'neutral' : 'primary'"
            size="sm"
            class="min-w-0 flex-1"
            :ui="{ indicator: 'duration-300' }"
          />
          <div class="shrink-0 text-xs text-muted">
            {{ statusLabel }}
          </div>
        </div>

        <div
          v-else-if="finishedLabel"
          data-id="app-queue-catalog-app-result"
          class="mt-1 flex items-center gap-1.5 text-xs"
          :class="finishedClass"
        >
          <UIcon
            v-if="finishedIcon"
            :name="finishedIcon"
            class="size-4"
          />
          <span>{{ finishedLabel }}</span>
        </div>
      </div>
    </div>

    <UButton
      :data-id="`app-queue-catalog-app-${app.alias}-action-button`"
      :label="actionLabel"
      :icon="installed ? 'i-bi-trash' : 'i-bi-download'"
      :color="installed ? 'error' : 'primary'"
      :variant="installed ? 'outline' : 'solid'"
      :loading="pending"
      :disabled="Boolean(activeItem)"
      class="w-full shrink-0 justify-center sm:w-28"
      @click="handleAction"
    />
  </div>
</template>

<script setup lang="ts">
import unknownAppIcon from '@/assets/images/unknown-app-icon.png';
import type { CatalogApplication } from '@/util/catalog';
import { catalogAppId, catalogTitle } from '@/util/catalog';
import { appQueueErrorLabel, appQueueProgress, appQueueStepLabel } from '@/util/appQueue';

const props = defineProps<{
  app: CatalogApplication;
}>();

const appQueueStore = useAppQueueStore();
const appsStore = useAppsStore();
const toast = useToast();

const pending = ref(false);
const iconBroken = ref(false);

const appId = computed(() => catalogAppId(props.app));
const title = computed(() => catalogTitle(props.app) ?? appId.value);
const description = computed(() => props.app.current_version?.short_description ?? undefined);

const installedApp = computed(() => appsStore.apps.find(app => app.id === appId.value));
const installed = computed(() => Boolean(installedApp.value));
const version = computed(() => installedApp.value?.version ?? props.app.current_version?.version);
const icon = computed(() => (iconBroken.value ? unknownAppIcon : props.app.current_version?.icon_uri ?? unknownAppIcon));

const activeItem = computed(() => appQueueStore.activeByApp.get(appId.value));
const finishedItem = computed(() => appQueueStore.finishedByApp.get(appId.value));

const progress = computed(() => appQueueProgress(activeItem.value?.detail?.step, activeItem.value?.detail?.progress));
const statusLabel = computed(() => {
  if (!activeItem.value || activeItem.value.status === 'pending') {
    return 'Queued';
  }

  const step = appQueueStepLabel(activeItem.value.detail?.step) || 'In progress';
  return progress.value === null || progress.value === undefined ? step : `${step} ${progress.value}%`;
});

const finishedLabel = computed(() => {
  const outcome = finishedItem.value?.outcome;
  if (!outcome) {
    return undefined;
  }

  if (outcome.kind === 'success') {
    return 'Operation finished';
  }

  if (outcome.kind === 'error' && outcome.error === 'cancelled') {
    return 'Operation cancelled';
  }

  if (outcome.kind === 'unknown') {
    return 'Operation finished';
  }

  return appQueueErrorLabel(outcome.error);
});

const finishedIcon = computed(() => {
  const outcome = finishedItem.value?.outcome;
  if (outcome?.kind === 'success') {
    return 'i-bi-checkmark-circle-fill';
  }

  if (outcome?.kind === 'error' && outcome.error !== 'cancelled') {
    return 'i-bi-error-fill';
  }

  return undefined;
});

const finishedClass = computed(() => {
  const outcome = finishedItem.value?.outcome;
  if (outcome?.kind === 'success') {
    return 'text-green-600';
  }

  if (outcome?.kind === 'error' && outcome.error !== 'cancelled') {
    return 'text-red-600';
  }

  return 'text-muted';
});

const actionLabel = computed(() => (installed.value ? 'Remove' : 'Install'));

watch(() => props.app.alias, () => {
  iconBroken.value = false;
});

async function handleAction () {
  if (pending.value || activeItem.value) {
    return;
  }

  pending.value = true;
  try {
    const operation = installed.value
      ? { type: 'remove' as const }
      : appQueueStore.appOperation(props.app);

    if (!operation) {
      toast.add({
        title: 'Cannot install the app',
        description: 'The catalog has no package for this version.',
        icon: 'i-bi-error-fill',
        color: 'error'
      });
      return;
    }

    const results = await appQueueStore.addOperations([{ app_id: appId.value, operation }]);

    const failed = results.filter(result => result.error !== null && result.error !== undefined);
    if (failed.length > 0) {
      toast.add({
        title: installed.value ? 'Removal was not queued' : 'Installation was not queued',
        description: `${title.value}: ${appQueueErrorLabel(failed[0]!.error)}`,
        icon: 'i-bi-error-fill',
        color: 'error'
      });
    }
  } catch (error) {
    toast.add({
      title: 'Failed to change the app queue',
      description: error instanceof Error ? error.message : undefined,
      icon: 'i-bi-error-fill',
      color: 'error'
    });
  } finally {
    pending.value = false;
  }
}
</script>
