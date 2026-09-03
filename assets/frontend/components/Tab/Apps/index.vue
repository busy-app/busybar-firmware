<template>
  <component
    :is="currentApp.component"
    v-if="currentApp"
    @back="openApp = undefined"
  />
  <TabAppsCard
    v-else
    @add="showAddAppModal = true"
  >
    <TabAppsAppCard
      v-for="app in APPS"
      :key="app.id"
      :data-id="`apps-section-app-${app.id}`"
      :title="app.title"
      :icon="app.icon"
      @click="openApp = app.id"
    />
  </TabAppsCard>

  <TabAppsUploaderModal
    v-model:open="showAddAppModal"
    @select="onAppFileSelect"
  />
</template>

<script setup lang="ts">
import { TabAppsWeather } from '#components';
import weatherIcon from '@/assets/icons/apps/weather.svg?url';

type AppId = typeof APPS[number]['id'];

const APPS = [
  { id: 'weather', title: 'Weather', icon: weatherIcon, component: TabAppsWeather }
] as const;

const toast = useToast();

const openApp = ref<AppId>();
const showAddAppModal = ref(false);

const currentApp = computed(() => APPS.find(app => app.id === openApp.value));

function onAppFileSelect (file: File) {
  toast.add({
    title: 'File added',
    description: `${file.name} is ready, but installing apps is not available yet.`,
    icon: 'i-bi-info'
  });
}
</script>
