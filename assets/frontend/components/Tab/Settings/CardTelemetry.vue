<template>
  <section
    data-id="settings-section-telemetry"
    class="w-full flex items-start justify-between p-6 ring-1 ring-glass rounded-3xl bg-elevated/50 gap-3"
  >
    <UIcon
      name="i-bi-share"
      class="size-6 shrink-0"
    />
    <div class="grow min-w-0 flex flex-col gap-1">
      <h2 class="font-medium">Share device stats</h2>
      <p class="text-sm text-muted">Usage data helps us improve BUSY</p>
    </div>
    <USwitch
      data-id="settings-section-telemetry-switch"
      aria-label="Share device stats"
      class="self-center"
      :model-value="telemetryStore.enabled"
      :loading="telemetryStore.saving"
      :disabled="telemetryStore.saving"
      @update:model-value="handleToggle"
    />
  </section>
</template>

<script setup lang="ts">
const telemetryStore = useTelemetryStore();

async function handleToggle (value: boolean) {
  if (value) {
    telemetryStore.requestEnable();
    return;
  }

  await telemetryStore.disable();
}
</script>
