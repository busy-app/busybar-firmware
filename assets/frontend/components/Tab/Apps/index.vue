<template>
  <component
    :is="currentApp.component"
    v-if="currentApp"
    @back="openApp = undefined"
  />
  <TabAppsYourApps v-else>
    <TabAppsAppCard
      v-for="app in APPS"
      :key="app.id"
      :data-id="`apps-section-app-${app.id}`"
      :title="app.title"
      :icon="app.icon"
      @click="openApp = app.id"
    />
  </TabAppsYourApps>
</template>

<script setup lang="ts">
import { TabAppsWeather } from '#components';
import weatherIcon from '@/assets/icons/apps/weather.svg?url';

const APPS = [
  { id: 'weather', title: 'Weather', icon: weatherIcon, component: TabAppsWeather }
] as const;

type AppId = typeof APPS[number]['id'];

const openApp = ref<AppId>();

const currentApp = computed(() => APPS.find(app => app.id === openApp.value));
</script>
