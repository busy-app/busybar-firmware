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

    <div class="hidden absolute left-1/2 -translate-x-1/2">
      <UDropdownMenu
        :items="[
          {
            label: 'Rename',
            icon: 'i-ri-pencil-line',
            onSelect: () => {
              nameModel = '';
              showRenameModal = true;
            }
          },
          {
            label: 'Restart',
            icon: 'i-ri-restart-line',
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
        :primary-action-props="{
          label: 'Rename',
          loading: loading.rename,
          disabled: nameModel.trim() === '',
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
        :items="userDropdownItems"
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
const deviceStore = useDeviceStore();
const pms = usePasswordModalStore();
const apiStore = useApiStore();

const colorMode = useColorMode();

const host = location.hostname;

const httpApiAccess = ref(await deviceStore.getHttpAPIAccess());

const passwordSetItems = [
  {
    label: 'Lock down',
    icon: 'i-ri-lock-fill',
    onSelect: () => lockDown
  },
  {
    label: 'Password',
    icon: 'i-ri-lock-password-line',
    children: [
      {
        label: 'Change',
        icon: 'i-ri-pencil-line',
        onSelect: () => {
          pms.passwordModel.current = '';
          pms.passwordModel.currentWrong = false;
          pms.passwordModel.new = '';
          pms.showUpdatePasswordModal = true;
        }
      },
      {
        label: 'Remove',
        icon: 'i-ri-lock-unlock-line',
        onSelect: () => {
          pms.passwordModel.current = '';
          pms.passwordModel.currentWrong = false;
          pms.showRemovePasswordModal = true;
        }
      }
    ]
  }
];

const passwordUnsetItems = [
  {
    label: 'Set password',
    icon: 'i-ri-lock-password-line',
    onSelect: () => {
      pms.passwordModel.current = '';
      pms.passwordModel.currentWrong = false;
      pms.passwordModel.new = '';
      pms.showSetPasswordModal = true;
    }
  }
];

const userDropdownItems = computed(() => {
  const baseItems = [
    [
      {
        label: 'Sign in to BUSY Account',
        icon: 'i-ri-account-circle-fill',
        slot: 'signin' as const,
        type: 'link',
        href: 'https://cloud.busy.app'
      }
    ],
    [
      {
        label: `${colorMode.value === 'dark' ? 'Light' : 'Dark'} theme`,
        icon: colorMode.value === 'dark' ? 'i-ri-sun-line' : 'i-ri-moon-line',
        onSelect: () => colorMode.preference = colorMode.value === 'dark' ? 'light' : 'dark'
      }
    ]
  ];

  if (httpApiAccess.value === undefined) {
    return [
      [
        {
          label: 'Loading...',
          icon: 'i-busy-loader',
          disabled: true,
          ui: {
            itemLeadingIcon: 'animate-spin'
          }
        }
      ],
      ...baseItems
    ];
  } else if (httpApiAccess.value.mode === 'key') {
    return [passwordSetItems, ...baseItems];
  } else {
    return [passwordUnsetItems, ...baseItems];
  }
});

const showRenameModal = ref(false);
const nameModel = ref('');

const showRestartModal = ref(false);

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

async function lockDown () {
  apiStore.apiKey = null;
  await navigateTo('/login');
}
</script>
