<template>
  <SectionCard
    data-id="apps-section-your-apps"
    title="Your apps"
    :ui="{ title: 'font-medium' }"
  >
    <template #actions>
      <UButton
        data-id="apps-section-your-apps-add-button"
        label="Add app"
        icon="i-ri-add-line"
        color="neutral"
        variant="outline"
        class="w-fit"
        :ui="{
          base: 'px-4 py-2.5 gap-1.5'
        }"
        @click="emit('add')"
      />
    </template>

    <template #raw-body>
      <div class="grid grid-cols-[repeat(auto-fit,minmax(min(100%,19.5rem),1fr))] gap-4">
        <slot />
      </div>
    </template>
  </SectionCard>
</template>

<script setup lang="ts">
const emit = defineEmits<{
  (e: 'add'): void;
}>();
</script>
