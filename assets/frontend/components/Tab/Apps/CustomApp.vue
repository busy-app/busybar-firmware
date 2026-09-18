<template>
  <SectionCard
    :data-id="`apps-section-${app.id}`"
    :title="app.name"
    :ui="{ title: 'font-medium', titleWrapper: 'gap-2' }"
  >
    <template #leading-actions>
      <SectionBackButton
        :data-id="`apps-section-${app.id}-back-button`"
        @click="emit('back')"
      />
    </template>

    <template #raw-body>
      <UIcon
        v-if="loading"
        name="i-busy-loader"
        class="size-6 animate-spin text-muted"
      />

      <div
        v-else-if="schema && settings"
        :data-id="`apps-section-${app.id}-settings`"
        class="flex flex-col gap-1 rounded-group"
      >
        <TabAppsSettingsFields
          v-model:values="settings.values"
          :fields="schema.fields"
        />
      </div>

      <div
        v-else
        :data-id="`apps-section-${app.id}-no-settings`"
        class="text-muted"
      >
        This app has no settings.
      </div>

      <div class="flex justify-end pt-4">
        <UButton
          :data-id="`apps-section-${app.id}-delete-button`"
          icon="i-bi-trash"
          label="Delete app"
          color="neutral"
          variant="ghost"
          :loading="deleting"
          @click="deleteApp"
        />
      </div>
    </template>
  </SectionCard>
</template>

<script setup lang="ts">
import type { AppInfo, AppSettingsDocument } from '@busy-app/busy-lib';
import type { AppSettingsSchema } from '@/stores/appsStore';

const props = defineProps<{
  app: AppInfo;
}>();

const emit = defineEmits<{
  (e: 'back'): void;
  (e: 'deleted', app: AppInfo): void;
}>();

const SAVE_DELAY = 500;

const appsStore = useAppsStore();

const loading = ref(true);
const deleting = ref(false);
const schema = ref<AppSettingsSchema>();
const settings = ref<AppSettingsDocument>();

let savedSettings = '';
let saveTimeout: ReturnType<typeof setTimeout> | undefined;

async function loadSettings () {
  try {
    const [loadedSchema, loadedSettings] = await Promise.all([
      appsStore.readSettingsSchema(props.app.id),
      appsStore.getSettings(props.app.id)
    ]);

    schema.value = loadedSchema;
    settings.value = loadedSettings;
    savedSettings = JSON.stringify(loadedSettings);
  } catch (error) {
    if ((error as { status?: number })?.status !== 404) {
      await handleHTTPError(error, 'Couldn\'t load app settings');
    }
  } finally {
    loading.value = false;
  }
}

async function saveSettings () {
  saveTimeout = undefined;

  if (!settings.value) {
    return;
  }

  const serialized = JSON.stringify(settings.value);
  if (serialized === savedSettings) {
    return;
  }

  try {
    await appsStore.setSettings(props.app.id, settings.value);
    savedSettings = serialized;
  } catch (error) {
    await handleHTTPError(error, 'Couldn\'t save app settings');
  }
}

async function deleteApp () {
  deleting.value = true;
  clearTimeout(saveTimeout);
  saveTimeout = undefined;

  try {
    await appsStore.removeApp(props.app.id);
    emit('deleted', props.app);
  } catch (error) {
    await handleHTTPError(error, 'Couldn\'t delete the app');
  } finally {
    deleting.value = false;
  }
}

watch(settings, () => {
  clearTimeout(saveTimeout);
  saveTimeout = setTimeout(saveSettings, SAVE_DELAY);
}, { deep: true });

onMounted(loadSettings);

onBeforeUnmount(() => {
  if (saveTimeout) {
    clearTimeout(saveTimeout);
    saveSettings();
  }
});
</script>
