<template>
  <div
    data-id="layout-locked"
    class="min-h-screen px-4 sm:px-6 py-4"
  >
    <UContainer>
      <div v-if="!initialLoading" class="w-full max-w-[328px] mx-auto">
        <slot />
      </div>
      <template v-else>
        <div class="w-screen h-screen absolute top-0 left-0 flex items-center justify-center">
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
