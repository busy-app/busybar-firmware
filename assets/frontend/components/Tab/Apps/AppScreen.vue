<template>
  <SectionCard
    :data-id="`apps-section-${appId}`"
    :title="title"
    :ui="{ title: 'font-medium', titleWrapper: 'gap-2' }"
  >
    <template #title>
      <div class="flex flex-wrap items-center gap-x-4 gap-y-2">
        <span>{{ title }}</span>
        <slot name="title-status" :restarting="pending === 'restart'" />
      </div>
    </template>

    <template #leading-actions>
      <SectionBackButton
        :data-id="`apps-section-${appId}-back-button`"
        @click="emit('back')"
      />
    </template>

    <template #actions>
      <UButton
        v-if="needsRestart || pending === 'restart'"
        :data-id="`apps-section-${appId}-restart-button`"
        :label="pending === 'restart' ? 'Restarting...' : 'Restart'"
        icon="i-bi-restart"
        color="neutral"
        variant="outline"
        class="justify-center"
        :loading="pending === 'restart'"
        :disabled="settingsSaving || pending !== undefined"
        :ui="BUTTON_UI"
        @click="run('restart')"
      />

      <UButton
        :data-id="`apps-section-${appId}-${running ? 'stop' : 'start'}-button`"
        :label="running ? 'Stop' : 'Start'"
        :icon="running ? 'i-bi-control-stop' : 'i-bi-control-play'"
        color="neutral"
        variant="solid"
        class="justify-center"
        :loading="pending === 'stop' || pending === 'start'"
        :disabled="pending === 'restart' || (!running && settingsSaving)"
        :ui="BUTTON_UI"
        @click="run(running ? 'stop' : 'start')"
      />
    </template>

    <template #raw-body>
      <slot />
    </template>
  </SectionCard>
</template>

<script setup lang="ts">
type AppAction = 'start' | 'stop' | 'restart';

const ACTION_ERROR: Record<AppAction, string> = {
  start: 'Couldn\'t start the app',
  stop: 'Couldn\'t stop the app',
  restart: 'Couldn\'t restart the app'
};

const BUTTON_UI = { base: 'px-4 py-2.5 gap-1.5' };

const props = defineProps<{
  appId: string;
  title: string;
  settingsSaving?: boolean;
  needsRestart?: boolean;
}>();

const emit = defineEmits<{
  (e: 'back'): void;
}>();

const appsStore = useAppsStore();

const pending = ref<AppAction>();

const running = computed(() => appsStore.runningAppId === props.appId);

async function run (action: AppAction) {
  if (pending.value || (action !== 'stop' && props.settingsSaving)) {
    return;
  }

  pending.value = action;

  try {
    if (action === 'restart') {
      await appsStore.restartApp(props.appId);
    } else if (action === 'stop') {
      await appsStore.quitApp();
    } else {
      await appsStore.launchApp(props.appId);
    }
  } catch (error) {
    await handleHTTPError(error, ACTION_ERROR[action]);
  } finally {
    pending.value = undefined;
  }
}
</script>
