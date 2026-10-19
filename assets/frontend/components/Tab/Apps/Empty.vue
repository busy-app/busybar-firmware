<template>
  <SectionCard
    data-id="apps-section-empty"
    title="Your apps"
    :ui="{ title: 'font-medium' }"
  >
    <template #raw-body>
      <div class="flex flex-col items-center justify-center gap-6 py-6">
        <img
          :src="appsIllustration"
          alt=""
          width="440"
          height="172"
          class="max-w-full rounded-xl bg-[#171717] dark:bg-transparent"
        >

        <div class="flex max-w-[336px] flex-col gap-2 text-center">
          <div
            data-id="apps-section-empty-title"
            class="text-xl font-medium text-highlighted"
          >
            No apps yet
          </div>

          <div
            data-id="apps-section-empty-description"
            class="text-balance text-muted"
          >
            Add custom apps to BUSY Bar and make it work your way
          </div>
        </div>

        <UButton
          data-id="apps-section-empty-add-button"
          label="Add app"
          icon="i-ri-add-line"
          color="neutral"
          class="w-fit"
          :ui="{
            base: 'px-4 py-2.5 gap-1.5'
          }"
          @click="emit('add')"
        />
      </div>
    </template>
  </SectionCard>
</template>

<script setup lang="ts">
import appsIllustration from '@/assets/icons/apps/illustration.svg';

const emit = defineEmits<{
  (e: 'add'): void;
}>();
</script>
