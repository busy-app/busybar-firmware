<template>
  <TabAppsWeather
    v-if="openApp === 'weather'"
    @back="openApp = undefined"
  />
  <TabAppsYourApps v-else>
    <TabAppsAppCard
      v-for="app in apps"
      :key="app.id"
      :data-id="`apps-section-app-${app.id}`"
      :title="app.title"
      :icon="app.icon"
      @click="onAppClick(app.id)"
    />
  </TabAppsYourApps>
</template>

<script setup lang="ts">
import weatherIcon from '@/assets/icons/apps/weather.svg?url';

const apps = [
  { id: 'weather', title: 'Weather', icon: weatherIcon }
] as const;

type AppId = typeof apps[number]['id'];

const openApp = ref<Extract<AppId, 'weather'>>();

function onAppClick (id: AppId) {
  if (id === 'weather') {
    openApp.value = 'weather';
  }
}
</script>
