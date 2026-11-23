<template>
  <section
    data-id="settings-section-telemetry"
    class="flex w-full items-start justify-between gap-3 rounded-3xl bg-elevated/50 p-6 ring-1 ring-glass"
  >
    <UIcon
      name="i-bi-share"
      class="size-6 shrink-0"
    />
    <div class="flex min-w-0 grow flex-col gap-1">
      <h2 class="font-medium">Share device stats</h2>
      <p class="text-sm text-muted">Usage data helps us improve BUSY</p>
    </div>
    <USwitch
      data-id="settings-section-telemetry-switch"
      aria-label="Share device stats"
      class="self-center"
      :class="{ 'opacity-60': !telemetryStore.enabled }"
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
