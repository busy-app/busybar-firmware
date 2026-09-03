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
    @uploaded="onAppUploaded"
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

function onAppUploaded (file: File) {
  toast.add({
    title: 'App added',
    description: `${file.name} has been uploaded to your BUSY Bar.`,
    icon: 'i-bi-checkmark-circle-fill',
    color: 'success'
  });
}
</script>
