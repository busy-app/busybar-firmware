<template>
  <div
    data-id="layout-default"
    class="w-screen min-h-screen px-4 sm:px-6 py-4"
  >
    <UContainer>
      <template v-if="shouldLoadDefaultPage">
        <DefaultLayoutHeader />
        <DefaultLayoutPreview class="pb-10" />

        <PasswordSetModal />
        <PasswordUpdateModal />
        <PasswordRemoveModal />

        <div class="w-full relative flex flex-col items-center xl:items-start gap-4 xl:grid xl:grid-cols-[160px_auto_160px] xl:gap-0">
          <DefaultLayoutTabs />
          <div class="w-full max-w-[688px] mx-auto">
            <slot />
          </div>
        </div>
      </template>
      <template v-else>
        <div class="w-screen h-screen absolute top-0 left-0 flex items-center justify-center">
          <UIcon
            name="i-busy-loader"
            class="size-12 animation-spin"
          />
        </div>
      </template>
    </UContainer>
  </div>
</template>

<script setup lang="ts">
const deviceStore = useDeviceStore();

if (!useRuntimeConfig().public.disablePolling) {
  deviceStore.setRefreshInterval();
} else {
  console.log('Polling disabled');
}

const shouldLoadDefaultPage = ref(false);
onMounted(async () => {
  // early access check
  try {
    await deviceStore.fetchDeviceName(true);
  } catch (error) {
    if ((error as { status: number }).status === 403) {
      return await navigateTo('/login');
    }
  }
  shouldLoadDefaultPage.value = true;
});

onBeforeUnmount(() => {
  deviceStore.clearRefreshInterval();
});
</script>
