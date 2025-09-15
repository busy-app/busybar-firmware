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

      <ModalGeneric
        v-model:open="showSetPasswordModal"
        title="Set password"
        :primary-action-props="{
          label: 'Set password',
          loading: loading.setPassword,
          disabled: setPasswordModel.password === '' || setPasswordModel.password !== setPasswordModel.confirmPassword,
          onClick: setPassword
        }"
        :secondary-action-props="{
          label: 'Cancel',
          variant: 'ghost',
          disabled: loading.setPassword,
          onClick: () => { showSetPasswordModal = false; }
        }"
      >
        <template #body>
          <UFormField
            label="New password, (4-10 digits)"
            :error="setPasswordModel.password !== ''
              && (
                /[^0-9]/.test(setPasswordModel.password)
                  ? 'Invalid password (only digits allowed)'
                  : setPasswordModel.password.length > 10
                    ? 'Password too long'
                    : setPasswordModel.password.length < 4 && setPasswordModel.password !== ''
                      ? 'Password too short'
                      : ''
              )"
          >
            <UInput
              v-model="setPasswordModel.password"
              size="xl"
              variant="soft"
            />
          </UFormField>
          <UFormField
            label="Confirm new password"
            :error="setPasswordModel.password !== setPasswordModel.confirmPassword && setPasswordModel.confirmPassword !== '' ? 'Passwords do not match' : ''"
          >
            <UInput
              v-model="setPasswordModel.confirmPassword"
              size="xl"
              variant="soft"
            />
          </UFormField>
        </template>
      </ModalGeneric>
    </div>
  </nav>
</template>

<script setup lang="ts">
const deviceStore = useDeviceStore();

const colorMode = useColorMode();

const host = location.hostname;

const httpApiAccess = ref(await deviceStore.getHttpAPIAccess());

const passwordSetItems = [
  {
    label: 'Lock down',
    icon: 'i-ri-lock-fill'
  },
  {
    label: 'Password',
    icon: 'i-ri-lock-password-line',
    children: [
      {
        label: 'Change',
        icon: 'i-ri-pencil-line'
      },
      {
        label: 'Remove',
        icon: 'i-ri-lock-unlock-line'
      }
    ]
  }
];

const passwordUnsetItems = [
  {
    label: 'Set password',
    icon: 'i-ri-lock-password-line',
    onSelect: () => {
      setPasswordModel.value.password = '';
      setPasswordModel.value.confirmPassword = '';
      showSetPasswordModal.value = true;
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
  restart: false,
  setPassword: false
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

const showSetPasswordModal = ref(false);
const setPasswordModel = ref({
  password: '',
  confirmPassword: ''
});

async function setPassword () {
  loading.value.setPassword = true;
  await deviceStore.setHttpAPIAccess('key', setPasswordModel.value.password);
  deviceStore.httpAPIAccess = await deviceStore.fetchHttpAPIAccess();
  httpApiAccess.value = deviceStore.httpAPIAccess;
  loading.value.setPassword = false;
  showSetPasswordModal.value = false;
}
</script>
