<template>
  <SectionCard
    data-id="apps-section-weather"
    title="Weather"
    :ui="{ title: 'font-medium', titleWrapper: 'gap-2' }"
  >
    <template #leading-actions>
      <SectionBackButton
        data-id="apps-section-weather-back-button"
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
