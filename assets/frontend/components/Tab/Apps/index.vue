<template>
  <div>
    <component
      :is="currentAppView.component"
      v-if="currentAppView"
      v-bind="currentAppView.props"
      @back="openApp = undefined"
    />
    <TabAppsCard
      v-else
      @add="showAddAppModal = true"
    >
      <TabAppsAppCard
        v-for="app in apps"
        :key="appKey(app)"
        :data-id="`apps-section-app-${app.id}`"
        :title="app.name"
        :icon="app.icon"
        @click="openApp = appKey(app)"
      />
    </TabAppsCard>

    <TabAppsUploaderModal
      v-model:open="showAddAppModal"
      @installed="onAppInstalled"
    />
  </div>
</template>

<script setup lang="ts">
import type { Component } from 'vue';
import type { AppInfo } from '@busy-app/busy-lib';
import { TabAppsWeather, TabAppsCustomApp } from '#components';
import weatherIcon from '@/assets/icons/apps/weather.svg?url';

interface AppListItem {
  id: string;
  name: string;
  icon?: string;
  native?: boolean;
  info?: AppInfo;
}

const NATIVE_APPS: AppListItem[] = [
  { id: 'weather', name: 'Weather', icon: weatherIcon, native: true }
];

const NATIVE_VIEWS: Record<string, Component> = { weather: markRaw(TabAppsWeather) };

const toast = useToast();
const appsStore = useAppsStore();

const openApp = ref<string>();
const showAddAppModal = ref(false);

const apps = computed<AppListItem[]>(() => [
  ...NATIVE_APPS,
  ...appsStore.apps.map(app => ({ id: app.id, name: app.name, icon: appsStore.icons[app.id], info: app }))
]);

const currentApp = computed(() => apps.value.find(app => appKey(app) === openApp.value));

const currentAppView = computed(() => {
  if (!currentApp.value) {
    return undefined;
  }

  if (currentApp.value.native) {
    return { component: NATIVE_VIEWS[currentApp.value.id]!, props: {} };
  }

  return { component: markRaw(TabAppsCustomApp), props: { app: currentApp.value.info } };
});

function appKey (app: AppListItem) {
  return `${app.native ? 'native' : 'custom'}-${app.id}`;
}

function onAppInstalled (app: AppInfo) {
  toast.add({
    title: 'App added',
    description: `${app.name} has been added to your BUSY Bar.`,
    icon: 'i-bi-checkmark-circle-fill',
    color: 'success'
  });
}

onMounted(appsStore.fetchApps);
</script>
