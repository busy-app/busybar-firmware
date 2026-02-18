<template>
  <div
    data-id="page-login"
    class="w-full min-h-[calc(100vh-2rem)] flex flex-col items-center justify-center gap-6"
  >
    <template v-if="!initialLoading">
      <template v-if="deviceStore.httpAPIAccess?.mode === 'disabled'">
        <div class="text-xl font-medium">HTTP API disabled over Wi-Fi</div>

        <img
          src="~/assets/images/locked-bar.png"
          class="w-[300px] my-4"
        >
      </template>
      <template v-else>
        <div class="text-xl font-medium">Virtual LAN is locked</div>

        <img
          src="~/assets/images/locked-bar.png"
          class="w-[328px] mb-4 mt-2"
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
                :icon="pms.passwordModel.showCurrent ? 'i-bi-eye' : 'i-bi-eye-shut'"
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

        <div class="w-full flex flex-col items-center gap-2">
          <UButton
            data-id="page-login-unlock-button"
            label="Unlock"
            color="neutral"
            size="lg"
            block
            :loading="loading"
            @click="attemptUnlock()"
          />

          <UButton
            data-id="page-login-forgot-password-button"
            label="Forgot password?"
            color="neutral"
            variant="ghost"
            size="lg"
            block
            @click="forgotPasswordModal = true"
          />
        </div>
      </template>
    </template>

    <UModal
      v-model:open="forgotPasswordModal"
      data-id="modal-forgot-password"
      title="Forgot your password?"
      description="A forgotten password cannot be recovered but only reset using a wired connection. Connect your BUSY Bar with a USB cable to reset the password. "
      :ui="{
        description: 'hidden',
        header: 'hidden',
        body: 'p-0 sm:p-0 overflow-visible',
        close: 'hidden'
      }"
    >
      <template #body>
        <div
          class="flex flex-col gap-6 p-6 bg-no-repeat"
          :style="`background-image: url(${connectCableImage}); background-size: 150%; background-position: center`"
        >
          <div class="text-xl font-medium">Forgot your password?</div>

          <div class="h-50" />
          <div class="text-center">A forgotten password cannot be recovered but only reset using a wired connection. Connect your BUSY Bar with a USB cable to reset the password.</div>

          <div class="flex justify-end">
            <UButton
              color="neutral"
              label="Got it"
              size="lg"
              class="min-w-20 justify-center"
              @click="forgotPasswordModal = false"
            />
          </div>
        </div>
      </template>
    </UModal>
  </div>
</template>

<script setup lang="ts">
import { vMaska } from 'maska/vue';
import connectCableImage from '@/assets/images/connect-cable.png';

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

const forgotPasswordModal = ref(false);

async function attemptUnlock () {
  if (loading.value) {
    return;
  }
  loading.value = true;

  try {
    apiStore.apiKey = pms.passwordModel.current;
    deviceStore.busyBar.setApiKey(apiStore.apiKey);
    await deviceStore.fetchDeviceName(true);
    await navigateTo('/', { external: true});
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
  if (apiStore.apiKey) {
    deviceStore.busyBar.setApiKey(apiStore.apiKey);
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
