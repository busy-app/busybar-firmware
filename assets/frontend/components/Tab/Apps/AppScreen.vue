<template>
  <SectionCard
    :data-id="`apps-section-${appId}`"
    :title="title"
    :ui="{ title: 'font-medium', titleWrapper: 'gap-2' }"
  >
    <template #leading-actions>
      <SectionBackButton
        :data-id="`apps-section-${appId}-back-button`"
        @click="emit('back')"
      />
    </template>

    <template #actions>
      <UButton
        :data-id="`apps-section-${appId}-update-button`"
        label="Update app"
        icon="i-bi-upload"
        color="neutral"
        variant="outline"
        class="justify-center"
        :ui="BUTTON_UI"
        @click="emit('update')"
      />

      <UButton
        :data-id="`apps-section-${appId}-${running ? 'stop' : 'start'}-button`"
        :label="running ? 'Stop' : 'Start'"
        :icon="running ? 'i-bi-control-stop' : 'i-bi-control-play'"
        color="neutral"
        variant="solid"
        class="justify-center"
        :loading="pending"
        :disabled="!running && settingsSaving"
        :ui="BUTTON_UI"
        @click="toggle"
      />
    </template>

    <template #raw-body>
      <slot />
    </template>
  </SectionCard>
</template>

<script setup lang="ts">
const BUTTON_UI = { base: 'px-4 py-2.5 gap-1.5' };

const props = defineProps<{
  appId: string;
  title: string;
  settingsSaving?: boolean;
}>();

const emit = defineEmits<{
  (e: 'back' | 'update'): void;
}>();

const appsStore = useAppsStore();

const pending = ref(false);

const running = computed(() => appsStore.runningAppId === props.appId);

async function toggle () {
  if (pending.value || (!running.value && props.settingsSaving)) {
    return;
  }

  const stopping = running.value;
  pending.value = true;

  try {
    if (stopping) {
      await appsStore.quitApp();
    } else {
      await appsStore.launchApp(props.appId);
    }
  } catch (error) {
    await handleHTTPError(error, stopping ? 'Couldn\'t stop the app' : 'Couldn\'t start the app');
  } finally {
    pending.value = false;
  }
}
</script>
