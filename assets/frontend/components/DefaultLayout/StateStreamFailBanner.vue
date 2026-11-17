<template>
  <div
    v-if="stateStreamStore.showStateStreamFailBanner"
    class="flex w-full items-center justify-between rounded-3xl bg-linear-to-r from-warning-50 to-warning-50 p-4
      pr-6 ring-1 ring-glass
      dark:from-warning-950/80 dark:from-30% dark:to-warning-800/80"
  >
    <div class="flex items-start gap-2.5">
      <UIcon
        name="i-bi-alert-fill"
        class="size-7.5 text-warning"
      />
      <div class="flex flex-col gap-1">
        <span class="font-medium text-warning">Can’t access your BUSY Bar screen</span>
        <span class="text-sm text-highlighted/90">Try again or reconnect your device</span>
      </div>
    </div>

    <div>
      <UButton
        data-id="update-banner-update-button"
        color="neutral"
        label="Try again"
        class="h-9 justify-center"
        @click="dispatchRestartEvent"
      />
    </div>
  </div>
</template>

<script setup lang="ts">
const stateStreamStore = useStateStreamStore();

function dispatchRestartEvent () {
  if (stateStreamStore.streamNotRestartable) {
    location.reload();
    return;
  }
  window.dispatchEvent(new CustomEvent('protobuf-websocket-restart'));
}
</script>
