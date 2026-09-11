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
        :data-id="`apps-section-app-${app.manifest.id}`"
        :title="app.manifest.name"
        :icon="app.icon"
        @click="openApp = appKey(app)"
      />
    </TabAppsCard>

    <TabAppsUploaderModal
      v-model:open="showAddAppModal"
      @uploaded="onAppUploaded"
    />
  </div>
</template>

<script setup lang="ts">
import type { Component } from 'vue';
import { TabAppsWeather, TabAppsCustomApp } from '#components';
import weatherIcon from '@/assets/icons/apps/weather.svg?url';
import type { AppManifest, AppPackage } from '@/util/readAppPackage';

interface App {
  manifest: Pick<AppManifest, 'id' | 'name'> & Partial<AppManifest>;
  icon?: string;
  native?: boolean;
}

const NATIVE_VIEWS: Record<string, Component> = { weather: markRaw(TabAppsWeather) };

const toast = useToast();

const openApp = ref<string>();
const showAddAppModal = ref(false);
const apps = ref<App[]>([
  { manifest: { id: 'weather', name: 'Weather' }, icon: weatherIcon, native: true }
]);

const currentApp = computed(() => apps.value.find(app => appKey(app) === openApp.value));

const currentAppView = computed(() => {
  if (!currentApp.value) {
    return undefined;
  }

  const nativeView = currentApp.value.native ? NATIVE_VIEWS[currentApp.value.manifest.id] : undefined;

  return nativeView
    ? { component: nativeView, props: {} }
    : { component: markRaw(TabAppsCustomApp), props: { app: currentApp.value } };
});

function appKey (app: App) {
  return `${app.native ? 'native' : 'custom'}-${app.manifest.id}`;
}

function onAppUploaded (appPackage: AppPackage) {
  const installedIndex = apps.value.findIndex(app => !app.native && app.manifest.id === appPackage.manifest.id);

  if (installedIndex === -1) {
    apps.value.push(appPackage);
  } else {
    apps.value[installedIndex] = appPackage;
  }

  toast.add({
    title: 'App added',
    description: `${appPackage.manifest.name} has been added to your BUSY Bar.`,
    icon: 'i-bi-checkmark-circle-fill',
    color: 'success'
  });
}
</script>
