<template>
  <TabAppsAppScreen
    :app-id="app.id"
    :title="app.name"
    :settings-saving="settingsSaving"
    @back="emit('back')"
    @update="emit('update')"
  >
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

    <div class="flex items-center gap-4 pt-4">
      <div
        v-if="saved || needsRestart"
        :data-id="`apps-section-${app.id}-saved`"
        class="flex min-w-0 items-center gap-2"
      >
        <template v-if="saved">
          <UIcon
            name="i-bi-checkmark-circle-fill"
            class="size-5 shrink-0 text-success"
          />
          <span class="text-toned">Saved</span>
        </template>

        <UButton
          v-if="needsRestart"
          :data-id="`apps-section-${app.id}-restart-button`"
          label="Restart the app to apply"
          icon="i-bi-refresh"
          color="neutral"
          variant="outline"
          class="shrink-0"
          :loading="restarting"
          :disabled="settingsSaving"
          @click="restartApp"
        />
      </div>

      <UButton
        :data-id="`apps-section-${app.id}-delete-button`"
        class="ml-auto shrink-0"
        icon="i-bi-trash"
        label="Delete app"
        color="neutral"
        variant="outline"
        @click="() => { showDeleteModal = true; }"
      />
    </div>

    <ModalGeneric
      v-model:open="showDeleteModal"
      data-id="modal-delete-app"
      title="Delete this app?"
      description="This app will be deleted from your BUSY Bar and the local web interface."
      :primary-action-props="{
        label: 'Delete app',
        variant: 'soft',
        color: 'error',
        loading: deleting,
        onClick: deleteApp
      }"
      :secondary-action-props="{
        label: 'Cancel',
        variant: 'ghost',
        disabled: deleting,
        onClick: () => { showDeleteModal = false; }
      }"
    />
  </TabAppsAppScreen>
</template>

<script setup lang="ts">
import type { AppInfo, AppSettingsDocument } from '@busy-app/busy-lib';
import type { AppSettingsSchema } from '@/stores/appsStore';

const props = defineProps<{
  app: AppInfo;
}>();

const emit = defineEmits<{
  (e: 'back' | 'update'): void;
  (e: 'deleted', app: AppInfo): void;
}>();

const SAVE_DELAY = 1500;
const SAVED_INDICATOR_DURATION = 5000;

const appsStore = useAppsStore();

const loading = ref(true);
const saved = ref(false);
const settingsSaving = ref(false);
const restarting = ref(false);
const deleting = ref(false);
const showDeleteModal = ref(false);
const schema = ref<AppSettingsSchema>();
const settings = ref<AppSettingsDocument>();

// An app reads its settings once at startup, so saved changes only reach it on a restart.
const needsRestart = ref(false);

let savedSettings = '';
let isSaving = false;
let saveTimeout: ReturnType<typeof setTimeout> | undefined;
let savedTimeout: ReturnType<typeof setTimeout> | undefined;

async function loadSettings () {
  try {
    const loadedSettings = await appsStore.getSettings(props.app.id);
    const loadedSchema = await appsStore.readSettingsSchema(props.app.id);

    schema.value = loadedSchema;
    savedSettings = JSON.stringify(loadedSettings);
    settings.value = loadedSettings;
  } catch (error) {
    if (httpErrorStatus(error) !== 404) {
      await handleHTTPError(error, 'Couldn\'t load app settings');
    }
  } finally {
    loading.value = false;
  }
}

async function saveSettings () {
  saveTimeout = undefined;

  if (isSaving || !settings.value) {
    return;
  }

  isSaving = true;
  let didSave = false;

  try {
    while (settings.value) {
      const serialized = JSON.stringify(settings.value);

      if (serialized === savedSettings || (schema.value && hasInvalidAppSettings(schema.value.fields, settings.value.values))) {
        break;
      }

      await appsStore.setSettings(props.app.id, JSON.parse(serialized));
      savedSettings = serialized;
      didSave = true;
    }
  } catch (error) {
    await handleHTTPError(error, 'Couldn\'t save app settings');
  } finally {
    isSaving = false;
    settingsSaving.value = saveTimeout !== undefined;
  }

  if (didSave) {
    saved.value = true;
    needsRestart.value = true;
    clearTimeout(savedTimeout);
    savedTimeout = setTimeout(() => {
      saved.value = false;
    }, SAVED_INDICATOR_DURATION);
  }
}

async function restartApp () {
  restarting.value = true;
  clearTimeout(savedTimeout);
  saved.value = false;

  try {
    await appsStore.restartApp(props.app.id);
    needsRestart.value = false;
  } catch (error) {
    await handleHTTPError(error, 'Couldn\'t restart the app');
  } finally {
    restarting.value = false;
  }
}

async function deleteApp () {
  deleting.value = true;
  clearTimeout(saveTimeout);
  saveTimeout = undefined;

  try {
    await appsStore.removeApp(props.app.id);
    showDeleteModal.value = false;
    emit('deleted', props.app);
  } catch (error) {
    await handleHTTPError(error, 'Couldn\'t delete the app');
  } finally {
    deleting.value = false;
  }
}

watch(settings, () => {
  if (!settings.value || JSON.stringify(settings.value) === savedSettings) {
    return;
  }

  saved.value = false;
  settingsSaving.value = true;
  clearTimeout(saveTimeout);
  saveTimeout = setTimeout(saveSettings, SAVE_DELAY);
}, { deep: true });

watch(() => appsStore.runningAppId, () => {
  needsRestart.value = false;
});

onMounted(loadSettings);

onBeforeUnmount(() => {
  clearTimeout(savedTimeout);

  if (saveTimeout) {
    clearTimeout(saveTimeout);
    saveSettings();
  }
});
</script>
