<template>
  <TabAppsAppScreen
    app-id="weather"
    title="Weather"
    @back="emit('back')"
  >
    <TabAppsGeolocation />
  </TabAppsAppScreen>
</template>

<script setup lang="ts">
const emit = defineEmits<{
  back: [];
}>();
</script>
