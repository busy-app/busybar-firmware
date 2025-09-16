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

      <ModalGeneric
        v-model:open="showSetPasswordModal"
        title="Set password"
        description="This password will be asked each time you open this page with a BUSY Bar connected via Wi-Fi. Remember your password, as a forgotten one cannot be recovered, but only reset via a wired connection."
        wide
        :primary-action-props="{
          label: 'Set password',
          loading: loading.password,
          disabled: newPasswordValidation !== '' || passwordModel.new === '',
          onClick: setPassword
        }"
        :secondary-action-props="{
          label: 'Cancel',
          variant: 'ghost',
          disabled: loading.password,
          onClick: () => { showSetPasswordModal = false; }
        }"
      >
        <template #body>
          <UFormField
            label="Password"
            :error="newPasswordValidation"
          >
            <UInput
              v-model="passwordModel.new"
              v-maska="'##########'"
              size="xl"
              variant="soft"
              :type="passwordModel.showNew ? 'text' : 'password'"
              placeholder="From 4 to 10 digits"
            >
              <template #trailing>
                <UButton
                  :icon="passwordModel.showNew ? 'i-ri-eye-close-line' : 'i-ri-eye-line'"
                  variant="ghost"
                  color="neutral"
                  square
                  class="rounded-full"
                  :ui="{
                    leadingIcon: 'size-6 text-muted'
                  }"
                  @click="passwordModel.showNew = !passwordModel.showNew"
                />
              </template>
            </UInput>
          </UFormField>
        </template>
      </ModalGeneric>

      <ModalGeneric
        v-model:open="showUpdatePasswordModal"
        title="Change password"
        description="Enter current and new passwords. Remember your password, as a forgotten one cannot be recovered, but only reset via a wired connection."
        wide
        :primary-action-props="{
          label: 'Update password',
          loading: loading.password,
          disabled: newPasswordValidation !== '' || currentPasswordValidation !== '' || passwordModel.current === '' || passwordModel.new === '',
          onClick: setPassword
        }"
        :secondary-action-props="{
          label: 'Cancel',
          variant: 'ghost',
          disabled: loading.password,
          onClick: () => { showUpdatePasswordModal = false; }
        }"
      >
        <template #body>
          <UFormField
            label="Current password"
            :error="currentPasswordValidation"
          >
            <UInput
              v-model="passwordModel.current"
              v-maska="'##########'"
              size="xl"
              variant="soft"
              :type="passwordModel.showCurrent ? 'text' : 'password'"
              placeholder="Enter password"
            >
              <template #trailing>
                <UButton
                  :icon="passwordModel.showCurrent ? 'i-ri-eye-close-line' : 'i-ri-eye-line'"
                  variant="ghost"
                  color="neutral"
                  square
                  class="rounded-full"
                  :ui="{
                    leadingIcon: 'size-6 text-muted'
                  }"
                  @click="passwordModel.showCurrent = !passwordModel.showCurrent"
                />
              </template>
            </UInput>
          </UFormField>

          <UFormField
            label="New password"
            :error="newPasswordValidation"
          >
            <UInput
              v-model="passwordModel.new"
              v-maska="'##########'"
              size="xl"
              variant="soft"
              :type="passwordModel.showNew ? 'text' : 'password'"
              placeholder="From 4 to 10 digits"
            >
              <template #trailing>
                <UButton
                  :icon="passwordModel.showNew ? 'i-ri-eye-close-line' : 'i-ri-eye-line'"
                  variant="ghost"
                  color="neutral"
                  square
                  class="rounded-full"
                  :ui="{
                    leadingIcon: 'size-6 text-muted'
                  }"
                  @click="passwordModel.showNew = !passwordModel.showNew"
                />
              </template>
            </UInput>
          </UFormField>
        </template>
      </ModalGeneric>

      <ModalGeneric
        v-model:open="showRemovePasswordModal"
        title="Remove password"
        description="If the password is not set, anyone on the same Wi-Fi network will be able to access the device via this page."
        wide
        :primary-action-props="{
          label: 'Remove password',
          loading: loading.password,
          disabled: currentPasswordValidation !== '' || passwordModel.current === '',
          onClick: removePassword
        }"
        :secondary-action-props="{
          label: 'Cancel',
          variant: 'ghost',
          disabled: loading.password,
          onClick: () => { showRemovePasswordModal = false; }
        }"
      >
        <template #body>
          <UFormField
            label="Current password"
            :error="currentPasswordValidation"
          >
            <UInput
              v-model="passwordModel.current"
              v-maska="'##########'"
              size="xl"
              variant="soft"
              :type="passwordModel.showCurrent ? 'text' : 'password'"
              placeholder="Enter password"
            >
              <template #trailing>
                <UButton
                  :icon="passwordModel.showCurrent ? 'i-ri-eye-close-line' : 'i-ri-eye-line'"
                  variant="ghost"
                  color="neutral"
                  square
                  class="rounded-full"
                  :ui="{
                    leadingIcon: 'size-6 text-muted'
                  }"
                  @click="passwordModel.showCurrent = !passwordModel.showCurrent"
                />
              </template>
            </UInput>
          </UFormField>
        </template>
      </ModalGeneric>
    </div>
  </nav>
</template>

<script setup lang="ts">
import { vMaska } from 'maska/vue';

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
        icon: 'i-ri-pencil-line',
        onSelect: () => {
          passwordModel.value.current = '';
          passwordModel.value.currentWrong = false;
          passwordModel.value.new = '';
          showUpdatePasswordModal.value = true;
        }
      },
      {
        label: 'Remove',
        icon: 'i-ri-lock-unlock-line',
        onSelect: () => {
          passwordModel.value.current = '';
          passwordModel.value.currentWrong = false;
          showRemovePasswordModal.value = true;
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
      passwordModel.value.new = '';
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
  password: false
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
const showUpdatePasswordModal = ref(false);
const showRemovePasswordModal = ref(false);
const passwordModel = ref({
  current: '',
  showCurrent: false,
  currentWrong: false,
  new: '',
  showNew: false
});
const newPasswordValidation = computed(() => {
  return passwordModel.value.new !== ''
    && (
      /[^0-9]/.test(passwordModel.value.new)
        ? 'Invalid password (only digits allowed)'
        : passwordModel.value.new.length > 10
          ? 'Password too long'
          : passwordModel.value.new.length < 4 && passwordModel.value.new !== ''
            ? 'Password too short'
            : ''
    );
});
const currentPasswordValidation = computed(() => {
  return passwordModel.value.currentWrong ? 'Incorrect password. Try again.' : '';
});

async function setPassword () {
  loading.value.password = true;
  await deviceStore.setHttpAPIAccess('key', passwordModel.value.new);
  deviceStore.httpAPIAccess = await deviceStore.fetchHttpAPIAccess();
  httpApiAccess.value = deviceStore.httpAPIAccess;
  loading.value.password = false;
  showSetPasswordModal.value = false;
  showUpdatePasswordModal.value = false;
}

async function removePassword () {
  loading.value.password = true;
  await deviceStore.setHttpAPIAccess('enabled');
  deviceStore.httpAPIAccess = await deviceStore.fetchHttpAPIAccess();
  httpApiAccess.value = deviceStore.httpAPIAccess;
  loading.value.password = false;
  showRemovePasswordModal.value = false;
}
</script>
