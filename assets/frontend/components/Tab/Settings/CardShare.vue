<template>
  <UCard
    data-id="settings-section-share"
    :ui="{
      root: 'ring-1 ring-glass rounded-3xl bg-elevated/50 divide-none',
      body: 'p-6'
    }"
  >
    <div class="w-full flex items-start gap-3">
      <div class="pt-0.5 shrink-0">
        <UIcon
          name="i-bi-share"
          class="size-6"
        />
      </div>

      <div class="flex flex-1 min-w-0 flex-col gap-1">
        <p class="font-medium truncate">Share device data</p>
        <p class="text-sm text-muted">Includes firmware, battery, app usage, and account info</p>
      </div>

      <div class="flex items-center self-stretch shrink-0">
        <USwitch
          :model-value="telemetryStore.enabled ?? true"
          :disabled="telemetryStore.enabled === undefined || saving"
          data-id="settings-section-share-switch"
          @update:model-value="onToggle"
        />
      </div>
    </div>
  </UCard>
</template>

<script setup lang="ts">
const telemetryStore = useTelemetryStore();

const saving = ref(false);

async function onToggle (value: boolean) {
  saving.value = true;
  await telemetryStore.setTelemetry(value);
  saving.value = false;
}

async function init () {
  await telemetryStore.fetchTelemetry();
}

onMounted(async () => {
  await init();
  window.addEventListener('device-reconnected', init);
});

onBeforeUnmount(() => window.removeEventListener('device-reconnected', init));
</script>
