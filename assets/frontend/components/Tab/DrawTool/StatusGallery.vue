<template>
  <SectionCard
    data-id="draw-tool-gallery-section"
    title="Your statuses"
  >
    <template #actions>
      <UButton
        data-id="draw-tool-gallery-refresh"
        label="Refresh"
        color="neutral"
        variant="soft"
        icon="i-bi-arrow-clockwise"
        :loading="isRefreshingStatusDirectory"
        @click="void dts.refreshStatusDirectory()"
      />
      <UButton
        data-id="draw-tool-gallery-new-status"
        label="Create"
        color="neutral"
        variant="soft"
        icon="i-bi-plus"
        @click="emit('newStatus')"
      />
    </template>

    <template #raw-body>
      <div>
        <div
          v-if="!statusGalleryFiles.length"
          class="flex min-h-48 items-center justify-center rounded-2xl border border-dashed border-default px-6 text-center text-sm text-muted"
        >
          Your statuses will appear here after you save them to the device.
        </div>

        <div
          v-else
          class="grid grid-cols-2 gap-4"
        >
          <article
            v-for="status in statusGalleryFiles"
            :key="status.name"
            class="overflow-hidden rounded-2xl border border-default bg-elevated/60"
          >
            <div class="aspect-[72/16] bg-default p-3">
              <img
                :src="status.previewUrl"
                :alt="status.name"
                class="h-full w-full object-contain [image-rendering:pixelated]"
              >
            </div>

            <div class="flex items-start justify-between gap-3 p-3">
              <div class="min-w-0">
                <div class="truncate text-sm font-medium text-default">
                  {{ status.name }}
                </div>
                <div class="text-xs text-muted">
                  {{ status.size >= 0 ? bytesToSize(status.size) : 'Unknown size' }}
                </div>
              </div>
            </div>
          </article>
        </div>
      </div>
    </template>
  </SectionCard>
</template>

<script setup lang="ts">
import { storeToRefs } from 'pinia';

const emit = defineEmits<{
  newStatus: [];
}>();

const dts = useDrawToolStore();
const { statusGalleryFiles, isRefreshingStatusDirectory } = storeToRefs(dts);
</script>
