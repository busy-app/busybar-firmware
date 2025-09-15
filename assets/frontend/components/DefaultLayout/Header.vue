<template>
  <nav class="relative h-12 flex justify-between items-center">
    <div class="flex gap-4">
      <UIcon
        name="i-busy-bar-logo"
        class="w-[70px] h-[28px]"
      />
      <div class="hidden md:flex items-center gap-2">
        <div class="flex items-center gap-1">
          <UIcon
            name="i-ri-usb-line"
            class="w-[18px] h-[22px]"
          />
          Connected
        </div>
        <div class="text-muted">{{ host }}</div>
      </div>
    </div>

    <div class="absolute left-1/2 -translate-x-1/2">
      <UDropdownMenu
        :items="[
          {
            label: 'Rename',
            icon: 'i-ri-pencil-line',
            onSelect: () => { nameModel = ''; showRenameModal = true; }
          },
          {
            label: 'Restart',
            icon: 'i-ri-reset-left-line',
            onSelect: () => { showRestartModal = true; }
          }
        ]"
        :content="{
          align: 'start',
          side: 'bottom',
          sideOffset: 8
        }"
        :ui="{
          content: 'w-36'
        }"
      >
        <UButton
          label="BUSY Bar"
          size="lg"
          trailing-icon="i-ri-arrow-down-s-fill"
          color="neutral"
          variant="ghost"
          class="text-xl"
        />
      </UDropdownMenu>

      <ModalGeneric
        v-model:open="showRenameModal"
        title="Rename device"
        description="Enter a new name for your BUSY Bar."
        :primary-action-props="{
          label: 'Rename',
          loading: loading.rename,
          onClick: updateDeviceName
        }"
        :secondary-action-props="{
          label: 'Cancel',
          variant: 'ghost',
          disabled: loading.rename,
          onClick: () => { showRenameModal = false; }
        }"
      >
        <template #body>
          <UInput
            v-model="nameModel"
            size="xl"
            variant="soft"
          />
        </template>
      </ModalGeneric>

      <ModalGeneric
        v-model:open="showRestartModal"
        title="Restart BUSY Bar?"
        description="Web control will be back after the reboot."
        :primary-action-props="{
          label: 'Restart',
          loading: loading.restart,
          onClick: restartDevice
        }"
        :secondary-action-props="{
          label: 'Cancel',
          variant: 'ghost',
          disabled: loading.restart,
          onClick: () => { showRestartModal = false; }
        }"
      />
    </div>

    <div class="flex gap-4 items-center">
      <UDropdownMenu
        :items="[
          [
            {
              label: 'Lock down',
              icon: 'i-ri-lock-fill'
            },
            {
              label: 'Password',
              icon: 'i-ri-lock-password-line',
              children: [
                {
                  label: 'Change password',
                  icon: 'i-ri-pencil-line'
                },
                {
                  label: 'Forgot password?',
                  icon: 'i-ri-question-line'
                }
              ]
            }
          ],
          [
            {
              label: 'Sign in to BUSY Account',
              icon: 'i-ri-account-circle-fill',
              slot: 'signin' as const
            }
          ],
          [
            {
              label: `${colorMode.value === 'dark' ? 'Light' : 'Dark'} theme`,
              icon: colorMode.value === 'dark' ? 'i-ri-sun-line' : 'i-ri-moon-line',
              onSelect: () => colorMode.preference = colorMode.value === 'dark' ? 'light' : 'dark'
            }
          ]
        ]"
        :content="{
          align: 'end',
          side: 'bottom',
          sideOffset: 8
        }"
        :ui="{
        }"
      >
        <UButton
          icon="i-busy-user-fill"
          size="lg"
          square
          color="neutral"
          variant="soft"
          class="rounded-full"
        />
        <template #signin-trailing>
          <UIcon
            name="i-ri-external-link-line"
            class="shrink-0 size-5 ml-4"
          />
        </template>
      </UDropdownMenu>
    </div>
  </nav>
</template>

<script setup lang="ts">
const showRenameModal = ref(false);
const nameModel = ref('');

const showRestartModal = ref(false);

const colorMode = useColorMode();

const host = location.hostname;

const loading = ref({
  rename: false,
  restart: false
});

async function updateDeviceName () {
  loading.value.rename = true;
  try {
    await new Promise(resolve => setTimeout(resolve, 2000));
  } catch {
    //
  } finally {
    loading.value.rename = false;
    showRenameModal.value = false;
  }
}

async function restartDevice () {
  loading.value.restart = true;
  try {
    await new Promise(resolve => setTimeout(resolve, 2000));
  } catch {
    //
  } finally {
    loading.value.restart = false;
    showRestartModal.value = false;
  }
}
</script>
