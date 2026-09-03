<template>
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
          @click="() => { pms.passwordModel.showCurrent = !pms.passwordModel.showCurrent; }"
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
      @click="() => { forgotPasswordModal = true; }"
    />
  </div>

  <ModalIllustrated
    v-model:open="forgotPasswordModal"
    data-id="modal-forgot-password"
    title="Forgot your password?"
    description="A forgotten password cannot be recovered but only reset using a wired connection. Connect your BUSY Bar with a USB cable to reset the password."
    :images="{
      light: insertCableImage,
      dark: insertCableImageDark
    }"
    :primary-action-props="{
      label: 'Got it',
      onClick: () => { forgotPasswordModal = false }
    }"
  >
    <div class="flex items-center gap-1.5">
      Press
      <div class="inline-flex p-2 rounded-full bg-elevated ring ring-inset ring-accented">
        <UIcon
          name="i-bi-user-fill"
          class="size-4"
        />
      </div>
      <UIcon
        name="i-bi-arrow-right"
        class="size-5"
      />
      <div class="inline-flex px-2 py-1 rounded-lg bg-elevated ring ring-inset ring-accented font-meduim">Manage password</div>
      <UIcon
        name="i-bi-arrow-right"
        class="size-5"
      />
      <div class="inline-flex px-2 py-1 rounded-lg bg-elevated ring ring-inset ring-accented font-meduim">Change</div>
    </div>
  </ModalIllustrated>
</template>

<script lang="ts" setup>
import { vMaska } from 'maska/vue';
import insertCableImage from '@/assets/images/insert-cable-image.png';
import insertCableImageDark from '@/assets/images/insert-cable-image-dark.png';

const pms = usePasswordModalStore();
const deviceStore = useDeviceStore();
const apiStore = useApiStore();

const loading = ref(false);

const forgotPasswordModal = ref(false);

async function attemptUnlock () {
  if (loading.value) {
    return;
  }
  loading.value = true;

  try {
    apiStore.apiKey = pms.passwordModel.current;
    deviceStore.busyBar.setHTTPAccessPassword(apiStore.apiKey);
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
</script>
