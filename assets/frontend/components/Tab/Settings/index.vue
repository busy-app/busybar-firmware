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
    <TabSettingsCardTelemetry v-if="telemetryStore.isSupported" />
    <TabSettingsCardDebug />
  </template>
</template>

<script setup lang="ts">
const telemetryStore = useTelemetryStore();

const showHomeAssistant = ref(false);
</script>
