<template>
  <div
    data-id="layout-locked"
    class="min-h-screen p-4 sm:px-6"
  >
    <UContainer>
      <div
        v-if="!initialLoading"
        class="mx-auto w-full max-w-82"
      >
        <slot />
      </div>
      <template v-else>
        <div class="absolute top-0 left-0 flex h-screen w-screen items-center justify-center">
          <UIcon
            name="i-busy-loader"
            class="size-12 animate-spin text-muted"
          />
        </div>
      </template>
    </UContainer>
  </div>
</template>

<script lang="ts" setup>
const deviceStore = useDeviceStore();
const apiStore = useApiStore();

const initialLoading = ref(true);

async function init () {
  if (apiStore.apiKey) {
    deviceStore.busyBar.setHTTPAccessPassword(apiStore.apiKey);
  }

  try {
    await deviceStore.fetchDeviceName(true);
    await navigateTo('/');
  } catch {
    // if access.mode is 'disabled', don't ask for password
    await deviceStore.fetchHttpAPIAccess();
  }
  setTimeout(() => {
    initialLoading.value = false;
  }, 500);
}

onMounted(async () => {
  await init();
  window.addEventListener('device-reconnected', init);
});
onBeforeUnmount(() => window.removeEventListener('device-reconnected', init));
</script>
