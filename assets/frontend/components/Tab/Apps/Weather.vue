<template>
  <SectionCard
    data-id="apps-section-weather"
    title="Weather"
    :ui="{ title: 'font-medium' }"
  >
    <template #leading-actions>
      <UButton
        data-id="apps-section-weather-back-button"
        icon="i-bi-arrow-back"
        variant="ghost"
        color="neutral"
        square
        :ui="{
          base: 'p-2.5 rounded-full'
        }"
        @click="emit('back')"
      />
    </template>

    <template #raw-body>
      <TabAppsGeolocation />
    </template>
  </SectionCard>
</template>

<script setup lang="ts">
const emit = defineEmits<{
  back: [];
}>();
</script>
