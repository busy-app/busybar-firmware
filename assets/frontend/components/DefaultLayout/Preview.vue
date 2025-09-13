<template>
  <div class="w-full flex flex-col items-center">
    <ScreenStream class="py-6" />

    <div class="text-neutral-500">
      Firmware Version
      <span class="text-neutral-800 dark:text-neutral-100 ml-2">
        {{ deviceStore.version.version === 'unknown' ? deviceStore.version.commit_hash : deviceStore.version.version }}
      </span>
    </div>
  </div>
</template>

<script setup lang="ts">
const deviceStore = useDeviceStore();
</script>
