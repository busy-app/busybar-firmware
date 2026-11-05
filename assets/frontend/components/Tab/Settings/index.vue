<template>
  <TabSettingsHomeAssistant
    v-if="showHomeAssistant"
    @back="showHomeAssistant = false"
  />
  <template v-else>
    <TabSettingsCardSliders />
    <TabSettingsCardTimezone />
    <TabSettingsCardSmartHome @open-home-assistant="showHomeAssistant = true" />
    <TabSettingsCardAbout />
    <TabSettingsCardDebug />
  </template>
</template>

<script setup lang="ts">
const showHomeAssistant = ref(false);
</script>
