<template>
  <TabAppsAppScreen
    :app-id="app.id"
    :title="app.name"
    updatable
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
        v-if="saved"
        :data-id="`apps-section-${app.id}-saved`"
        class="flex min-w-0 items-center gap-1.5 text-toned"
      >
        <UIcon
          name="i-bi-checkmark-circle-fill"
          class="size-5 shrink-0 text-success"
        />
        <span>Saved</span>
        <span class="truncate text-muted">· Restart the app to apply</span>
      </div>

      <UButton
        :data-id="`apps-section-${app.id}-delete-button`"
        class="ml-auto shrink-0"
        icon="i-bi-trash"
        label="Delete app"
        color="neutral"
        variant="ghost"
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
const deleting = ref(false);
const showDeleteModal = ref(false);
const schema = ref<AppSettingsSchema>();
const settings = ref<AppSettingsDocument>();

let savedSettings = '';
let saveTimeout: ReturnType<typeof setTimeout> | undefined;
let savedTimeout: ReturnType<typeof setTimeout> | undefined;

async function loadSettings () {
  try {
    const loadedSettings = await appsStore.getSettings(props.app.id);
    const loadedSchema = await appsStore.readSettingsSchema(props.app.id);

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
    saved.value = JSON.stringify(settings.value) === serialized;

    clearTimeout(savedTimeout);
    savedTimeout = setTimeout(() => {
      saved.value = false;
    }, SAVED_INDICATOR_DURATION);
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
    showDeleteModal.value = false;
    emit('deleted', props.app);
  } catch (error) {
    await handleHTTPError(error, 'Couldn\'t delete the app');
  } finally {
    deleting.value = false;
  }
}

watch(settings, () => {
  saved.value = false;
  clearTimeout(saveTimeout);
  saveTimeout = setTimeout(saveSettings, SAVE_DELAY);
}, { deep: true });

onMounted(loadSettings);

onBeforeUnmount(() => {
  clearTimeout(savedTimeout);

  if (saveTimeout) {
    clearTimeout(saveTimeout);
    saveSettings();
  }
});
</script>
