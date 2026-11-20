<template>
  <SectionCard
    data-id="app-queue-section-catalog"
    title="Catalog apps"
    icon="i-bi-apps"
    :ui="{ title: 'font-medium' }"
  >
    <template #actions>
      <div class="flex items-center gap-2">
        <UBadge
          v-if="appQueueStore.catalogTotal > 0"
          :label="formatItemCount(appQueueStore.catalogTotal)"
          color="neutral"
          variant="subtle"
          size="sm"
        />

        <UButton
          data-id="app-queue-section-catalog-refresh-button"
          icon="i-bi-refresh"
          color="neutral"
          variant="outline"
          :loading="appQueueStore.catalogLoading"
          aria-label="Refresh catalog"
          @click="appQueueStore.fetchCatalog()"
        />
      </div>
    </template>

    <template #raw-body>
      <div class="flex flex-col gap-2">
        <div
          v-if="appQueueStore.catalogError"
          data-id="app-queue-section-catalog-error"
          class="flex flex-col gap-2 rounded-xl bg-accented/25 p-4 dark:bg-elevated/75"
        >
          <div class="flex items-center gap-2 text-sm text-muted">
            <UIcon
              name="i-bi-alert"
              class="size-5 shrink-0 text-red-600"
            />
            <span class="min-w-0 flex-1">{{ appQueueStore.catalogError }}</span>
          </div>
          <div class="text-xs text-muted">
            The catalog API is not reachable from this origin. Use the local dev server (pnpm dev) to test the catalog.
          </div>
          <UButton
            label="Retry"
            color="neutral"
            variant="ghost"
            class="w-fit"
            @click="appQueueStore.fetchCatalog()"
          />
        </div>

        <div
          v-else-if="appQueueStore.catalogLoading && appQueueStore.catalogApps.length === 0"
          data-id="app-queue-section-catalog-loading"
          class="flex items-center justify-center gap-2 py-8 text-muted"
        >
          <UIcon
            name="i-busy-loader"
            class="size-5 animate-spin"
          />
          <span>Loading catalog…</span>
        </div>

        <div
          v-else-if="appQueueStore.catalogApps.length === 0"
          data-id="app-queue-section-catalog-empty"
          class="flex flex-col items-center justify-center gap-2 rounded-xl border border-accented py-8 text-center text-muted"
        >
          <UIcon
            name="i-bi-apps"
            class="size-8"
          />
          <div>No apps in the catalog</div>
        </div>

        <TabAppQueueCatalogApp
          v-for="app in appQueueStore.catalogApps"
          :key="app.id"
          :app="app"
        />
      </div>
    </template>
  </SectionCard>
</template>

<script setup lang="ts">
import { formatItemCount } from '@/util/appQueue';

const appQueueStore = useAppQueueStore();
</script>
