<template>
  <div>
    <TabAppsCustomApp
      v-if="currentApp"
      :key="`${currentApp.id}-${currentApp.version}`"
      :app="currentApp"
      @back="openApp = undefined"
      @update="openUploader(currentApp.id)"
      @deleted="onAppDeleted"
    />
    <TabAppsCard
      v-else
      @add="openUploader()"
    >
      <TabAppsAppCard
        v-for="app in appsStore.apps"
        :key="app.id"
        :data-id="`apps-section-app-${app.id}`"
        :title="app.name"
        :icon="appsStore.icons[app.id]"
        @click="openApp = app.id"
      />
    </TabAppsCard>

    <TabAppsUploaderModal
      v-model:open="showAddAppModal"
      :update-app-id="updateAppId"
      @installed="onAppInstalled"
    />
  </div>
</template>

<script setup lang="ts">
import type { AppInfo } from '@busy-app/busy-lib';

const toast = useToast();
const appsStore = useAppsStore();

const openApp = ref<string>();
const showAddAppModal = ref(false);
const updateAppId = ref<string>();

const currentApp = computed(() => appsStore.apps.find(app => app.id === openApp.value));

function openUploader (appId?: string) {
  updateAppId.value = appId;
  showAddAppModal.value = true;
}

function onAppInstalled (app: AppInfo, updated: boolean) {
  toast.add({
    title: updated ? 'App updated' : 'App added',
    description: updated
      ? `${app.name} has been updated to version ${app.version}.`
      : `${app.name} has been added to your BUSY Bar.`,
    icon: 'i-bi-checkmark-circle-fill',
    color: 'success'
  });
}

function onAppDeleted (app: AppInfo) {
  openApp.value = undefined;

  toast.add({
    title: 'App deleted',
    description: `${app.name} has been removed from your BUSY Bar.`,
    icon: 'i-bi-checkmark-circle-fill',
    color: 'success'
  });
}

onMounted(appsStore.fetchApps);
</script>
