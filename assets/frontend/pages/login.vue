<template>
  <div
    data-id="page-login"
    class="w-full min-h-[calc(100vh-2rem)] flex flex-col items-center justify-center gap-6"
  >
    <template v-if="!initialLoading">
      <template v-if="deviceStore.httpAPIAccess?.mode === 'disabled'">
        <div class="text-xl font-medium">HTTP API disabled over Wi-Fi</div>

        <img
          src="~/assets/images/locked_bar.png"
          class="w-[300px] my-4"
        >
      </template>
      <template v-else>
        <div class="text-xl font-medium">Virtual LAN is locked</div>

        <img
          src="~/assets/images/locked_bar.png"
          class="w-[300px] my-4"
        >

        <div>Enter your password to unlock</div>
        <UFormField
          class="w-full"
          :error="pms.currentPasswordValidation"
        >
          <UInput
            v-model="pms.passwordModel.current"
            v-maska="'##########'"
            name="current-password"
            size="xl"
            variant="soft"
            :type="pms.passwordModel.showCurrent ? 'text' : 'password'"
            placeholder="Password"
            @update:model-value="pms.passwordModel.currentWrong = false"
            @keyup.enter="loading ? null : attemptUnlock()"
          >
            <template #trailing>
              <UButton
                :icon="pms.passwordModel.showCurrent ? 'i-ri-eye-close-line' : 'i-ri-eye-line'"
                variant="ghost"
                color="neutral"
                square
                class="rounded-full"
                :ui="{
                  leadingIcon: 'size-6 text-muted'
                }"
                @click="pms.passwordModel.showCurrent = !pms.passwordModel.showCurrent"
              />
            </template>
          </UInput>
        </UFormField>

        <UButton
          data-id="page-login-unlock-button"
          label="Unlock"
          color="primary"
          size="lg"
          class="h-10"
          block
          :loading="loading"
          @click="attemptUnlock()"
        />
      </template>
    </template>
  </div>
</template>

<script setup lang="ts">
import { vMaska } from 'maska/vue';

useHead({
  title: '🔒 BUSY Bar Virtual LAN',
  meta: [
    {
      name: 'description',
      content: '[Locked] Control your BUSY Bar in the browser'
    }
  ]
});

definePageMeta({
  layout: 'locked'
});

const pms = usePasswordModalStore();
const deviceStore = useDeviceStore();
const apiStore = useApiStore();

const initialLoading = ref(true);
const loading = ref(false);

async function attemptUnlock () {
  if (loading.value) {
    return;
  }
  loading.value = true;

  try {
    apiStore.apiKey = pms.passwordModel.current;
    deviceStore.busyBar.setApiKey(apiStore.apiKey);
    await deviceStore.fetchDeviceName(true);
    await navigateTo('/');
  } catch (error: unknown) {
    if ((error as { status?: number })?.status === 403) {
      apiStore.apiKey = null;
      pms.passwordModel.current = '';
      pms.passwordModel.currentWrong = true;
    } else {
      await handleHTTPError(error, 'Failed to unlock Virtual LAN');
    }
  }
  loading.value = false;
}

async function init () {
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
