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
        v-if="running"
        :data-id="`apps-section-${appId}-restart-button`"
        label="Restart"
        icon="i-bi-refresh"
        color="neutral"
        variant="outline"
        class="justify-center"
        :disabled="actionDisabled('restart')"
        :ui="BUTTON_UI"
        @click="runAction('restart')"
      />

      <UButton
        :data-id="`apps-section-${appId}-${running ? 'stop' : 'start'}-button`"
        :label="running ? 'Stop' : 'Start on BUSY Bar'"
        :icon="running ? 'i-bi-control-stop' : 'i-bi-control-play'"
        color="neutral"
        variant="solid"
        class="justify-center"
        :loading="pendingAction === 'start' || pendingAction === 'stop'"
        :disabled="checkingRunning || actionDisabled(running ? 'stop' : 'start')"
        :ui="BUTTON_UI"
        @click="runAction(running ? 'stop' : 'start')"
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

const RUNNING_POLL_MS = 2000;

const BUTTON_UI = { base: 'px-4 py-2.5 gap-1.5' };

let runningPoll: ReturnType<typeof setInterval> | undefined;

const props = defineProps<{
  appId: string;
  title: string;
  settingsSaving?: boolean;
}>();

const emit = defineEmits<{
  (e: 'back' | 'update'): void;
}>();

const appsStore = useAppsStore();

const pendingAction = ref<AppAction>();
const checkingRunning = ref(true);

const running = computed(() => pendingAction.value === 'restart' || appsStore.runningAppId === props.appId);

async function syncRunning () {
  if (pendingAction.value) {
    return;
  }

  await appsStore.refreshRunningApp(props.appId);
}

function actionDisabled (action: AppAction) {
  if (pendingAction.value !== undefined) {
    return pendingAction.value !== action;
  }

  return action !== 'stop' && !!props.settingsSaving;
}

async function runAction (action: AppAction) {
  if (pendingAction.value || (action !== 'stop' && props.settingsSaving)) {
    return;
  }

  pendingAction.value = action;

  try {
    switch (action) {
      case 'start':
        await appsStore.launchApp(props.appId);
        break;
      case 'stop':
        await appsStore.quitApp();
        break;
      case 'restart':
        await appsStore.restartApp(props.appId);
        break;
    }
  } catch (error) {
    await handleHTTPError(error, ACTION_ERROR[action]);
  } finally {
    pendingAction.value = undefined;
  }
}

onMounted(() => {
  syncRunning().finally(() => {
    checkingRunning.value = false;
  });

  runningPoll = setInterval(syncRunning, RUNNING_POLL_MS);
});

onBeforeUnmount(() => {
  clearInterval(runningPoll);
});
</script>
