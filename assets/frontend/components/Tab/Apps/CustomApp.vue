<template>
  <SectionCard
    :data-id="`apps-section-${app.manifest.id}`"
    :title="app.manifest.name"
    :ui="{ title: 'font-medium', titleWrapper: 'gap-2' }"
  >
    <template #leading-actions>
      <SectionBackButton
        :data-id="`apps-section-${app.manifest.id}-back-button`"
        @click="emit('back')"
      />
    </template>
  </SectionCard>
</template>

<script setup lang="ts">
import type { AppManifest } from '@/util/readAppPackage';

defineProps<{
  app: {
    manifest: Pick<AppManifest, 'id' | 'name'>;
  };
}>();

const emit = defineEmits<{
  (e: 'back'): void;
}>();
</script>
