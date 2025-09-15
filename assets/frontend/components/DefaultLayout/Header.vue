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
            name="i-tabler-usb"
            class="w-[18px] h-[22px]"
          />
          Connected
        </div>
        <div class="text-muted">10.0.4.20</div>
      </div>
    </div>

    <div class="absolute left-1/2 -translate-x-1/2">
      <UDropdownMenu
        :items="[
          {
            label: 'Rename',
            icon: 'i-tabler-wifi',
            onSelect: () => { nameModel = ''; showRenameModal = true; }
          },
          {
            label: 'Restart',
            icon: 'i-tabler-wifi',
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
          trailing-icon="i-tabler-chevron-down"
          color="neutral"
          variant="ghost"
          class="text-xl"
        />
      </UDropdownMenu>

      <UModal
        v-model:open="showRenameModal"
        title="Rename device"
        :ui="{
          content: 'max-w-[360px] divide-none',
          header: 'pb-0'
        }"
      >
        <template #header>
          <div class="text-xl">Rename device</div>
        </template>

        <template #body>
          <UInput
            v-model="nameModel"
            size="xl"
            variant="soft"
          />

          <div class="flex justify-end gap-2 mt-8">
            <UButton
              label="Cancel"
              variant="ghost"
              :disabled="loading.rename"
              @click="showRenameModal = false"
            />
            <UButton
              label="Rename"
              :loading="loading.rename"
              class="px-3 py-2.5 rounded-full"
              @click="updateDeviceName"
            />
          </div>
        </template>
      </UModal>

      <UModal
        v-model:open="showRestartModal"
        title="Restart BUSY Bar?"
        :ui="{
          content: 'max-w-[360px] divide-none',
          header: 'pb-0'
        }"
      >
        <template #header>
          <div class="text-xl">Rename device</div>
        </template>

        <template #body>
          Web control will be back after the reboot.

          <div class="flex justify-end gap-2 mt-8">
            <UButton
              label="Cancel"
              variant="ghost"
              :disabled="loading.restart"
              @click="showRestartModal = false"
            />
            <UButton
              label="Restart"
              :loading="loading.restart"
              class="px-3 py-2.5 rounded-full"
              @click="restartDevice"
            />
          </div>
        </template>
      </UModal>
    </div>

    <div class="flex gap-4 items-center">
      <UDropdownMenu
        :items="[
          [
            {
              label: 'Lock down',
              icon: 'i-tabler-wifi'
            },
            {
              label: 'Password',
              icon: 'i-tabler-login-2',
              children: [
                {
                  label: 'Change password',
                  icon: 'i-tabler-key'
                },
                {
                  label: 'Forgot password',
                  icon: 'i-tabler-help-circle'
                }
              ]
            }
          ],
          [
            {
              label: 'Sign in to BUSY Account',
              icon: 'i-tabler-wifi',
              slot: 'signin' as const
            }
          ],
          [
            {
              label: `${colorMode.value === 'dark' ? 'Light' : 'Dark'} theme`,
              icon: colorMode.value === 'dark' ? 'i-tabler-sun-filled' : 'i-tabler-moon-filled',
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
          icon="i-tabler-login-2"
          size="lg"
          square
          color="neutral"
          variant="soft"
          class="rounded-full"
        />
        <template #signin-trailing>
          <UIcon
            name="i-tabler-external-link"
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
