<template>
  <div
    v-if="useStateStreamStore().showResourceLimitErrorBanner"
    class="w-full flex items-center justify-between p-4 pr-6 ring-1 ring-glass rounded-3xl
      bg-linear-to-r from-warning-50 to-warning-50
      dark:from-30% dark:from-warning-950/80 dark:to-warning-800/80"
  >
    <div class="flex items-start gap-2.5">
      <UIcon
        name="i-bi-alert-fill"
        class="size-7.5 text-warning"
      />
      <div class="flex flex-col gap-1">
        <span class="text-warning font-medium">Too many connections to the device</span>
        <span class="text-sm text-highlighted/90">Close other apps or tabs connected to it, and try again</span>
      </div>
    </div>

    <div>
      <UButton
        data-id="update-banner-update-button"
        color="neutral"
        label="Try again"
        class="justify-center h-9"
        @click="dispatchRestartEvent"
      />
    </div>
  </div>
</template>

<script setup lang="ts">
function dispatchRestartEvent () {
  window.dispatchEvent(new CustomEvent('protobuf-websocket-restart'));
}
</script>
