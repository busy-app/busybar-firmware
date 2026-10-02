<template>
  <TabAppsAppScreen
    :app-id="app.id"
    :title="app.name"
    :settings-saving="settingsSaving"
    :needs-restart="needsRestart"
    @back="emit('back')"
  >
    <template #title-status="{ restarting }">
      <UBadge
        v-if="settingsSaving"
        :data-id="`apps-section-${app.id}-saving-badge`"
        icon="i-bi-loader"
        label="Saving changes"
        color="neutral"
        variant="soft"
        :ui="{ leadingIcon: 'animate-spin' }"
      />

      <UBadge
        v-else-if="needsRestart || restarting"
        :data-id="`apps-section-${app.id}-restart-badge`"
        icon="i-bi-info"
        label="Restart the app to apply changes"
        color="warning"
        variant="soft"
      />

      <UBadge
        v-else-if="saved"
        :data-id="`apps-section-${app.id}-saved-badge`"
        icon="i-bi-checkmark"
        label="Saved"
        color="success"
        variant="soft"
      />
    </template>

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
        :app-id="app.id"
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

    <div class="flex items-center justify-between pt-6">
      <UButton
        :data-id="`apps-section-${app.id}-update-button`"
        icon="i-bi-upload"
        label="Update app"
        color="neutral"
        variant="link"
        :ui="TEXT_BUTTON_UI"
        @click="emit('update')"
      />

      <UButton
        :data-id="`apps-section-${app.id}-delete-button`"
        icon="i-bi-trash"
        label="Delete app"
        color="neutral"
        variant="link"
        :ui="TEXT_BUTTON_UI"
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

const TEXT_BUTTON_UI = {
  base: 'text-white hover:text-white active:text-white',
  leadingIcon: 'text-white'
};

const SAVE_DELAY = 1500;

let savedSettings = '';
let isSaving = false;
let saveTimeout: ReturnType<typeof setTimeout> | undefined;

const appsStore = useAppsStore();

const loading = ref(true);
const settingsSaving = ref(false);
const deleting = ref(false);
const showDeleteModal = ref(false);
const schema = ref<AppSettingsSchema>();
const settings = ref<AppSettingsDocument>();

const saved = computed(() => appsStore.settingsSavedByApp[props.app.id] === true);
const running = computed(() => appsStore.runningAppId === props.app.id);
const needsRestart = computed(() => running.value && saved.value);

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

  try {
    while (settings.value) {
      const serialized = JSON.stringify(settings.value);

      if (serialized === savedSettings || (schema.value && hasInvalidAppSettings(schema.value.fields, settings.value.values))) {
        break;
      }

      await appsStore.setSettings(props.app.id, JSON.parse(serialized));
      savedSettings = serialized;
    }
  } catch (error) {
    await handleHTTPError(error, 'Couldn\'t save app settings');
  } finally {
    isSaving = false;
    settingsSaving.value = saveTimeout !== undefined;
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

  settingsSaving.value = true;
  clearTimeout(saveTimeout);
  saveTimeout = setTimeout(saveSettings, SAVE_DELAY);
}, { deep: true });

onMounted(() => {
  if (!running.value) {
    appsStore.settingsSavedByApp[props.app.id] = false;
  }

  loadSettings();
});

onBeforeUnmount(() => {
  if (saveTimeout) {
    clearTimeout(saveTimeout);
    saveSettings();
  }
});
</script>
