<template>
  <nav
    data-id="layout-default-header"
    class="relative h-12 flex justify-between items-center"
  >
    <div class="flex gap-6">
      <UIcon
        data-id="layout-default-header-logo"
        name="i-busy-bar-logo"
        class="w-[70px] h-[28px]"
        @click="onLogoClick"
      />
      <div
        v-if="power"
        data-id="layout-default-header-power"
        class="flex items-center gap-1.5"
      >
        <div class="relative flex">
          <BatteryIndicator
            :charge="power?.battery_charge"
            :state="power?.state"
            class="size-7"
          />
        </div>
        <div>{{ power?.battery_charge }}%</div>
      </div>
      <div class="hidden md:flex items-center gap-2">
        <div
          data-id="layout-default-header-connection-state"
          class="flex items-center gap-1"
        >
          <template v-if="deviceStore.isConnected">
            <UIcon
              :name="deviceStore.connectionType === 'usb' ? 'i-bi-usb' : 'i-bi-wifi-4'"
              class="w-[18px] h-[22px]"
            />
            Connected
          </template>
          <template v-else>
            <UIcon
              name="i-bi-alert"
              class="w-[18px] h-[22px] text-red-500"
            />
            Disconnected
          </template>
        </div>
      </div>
    </div>

    <div class="absolute left-1/2 -translate-x-1/2">
      <UDropdownMenu
        data-id="layout-default-header-device-menu"
        :items="[
          {
            label: 'Rename',
            icon: 'i-ri-pencil-line',
            onSelect: () => {
              nameModel = '';
              showRenameModal = true;
            }
          }
          /* {
            label: 'Restart',
            icon: 'i-ri-restart-line',
            onSelect: () => { showRestartModal = true; }
          } */
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
          :label="deviceStore.deviceName"
          size="lg"
          trailing-icon="i-bi-caret-down"
          color="neutral"
          variant="ghost"
          class="text-xl rounded-md"
          :ui="{
            trailingIcon: 'size-6 text-neutral-500'
          }"
        />
      </UDropdownMenu>

      <ModalGeneric
        v-model:open="showRenameModal"
        data-id="model-rename"
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
            name="new-name"
            size="xl"
            variant="soft"
            :ui="{ base: 'ring-1 ring-glass' }"
            :disabled="loading.rename"
            @keyup.enter="updateDeviceName"
          />
        </template>
      </ModalGeneric>

      <ModalGeneric
        v-model:open="showRestartModal"
        data-id="model-restart"
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
        data-id="layout-default-header-user-menu"
        :items="userDropdownItems"
        :content="{
          align: 'end',
          side: 'bottom',
          sideOffset: 8
        }"
        :ui="{
          itemLabelExternalIcon: 'hidden'
        }"
      >
        <UButton
          data-id="layout-default-header-user-menu-trigger"
          icon="i-busy-user-fill"
          size="lg"
          square
          color="neutral"
          variant="ghost"
          class="rounded-full"
        />

        <template #signin-trailing>
          <UIcon
            name="i-bi-open-in-new"
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
const tabStore = useTabStore();

const colorMode = useColorMode();

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
        href: 'https://cloud.busy.app',
        target: '_blank'
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
  await deviceStore.setDeviceName(nameModel.value.trim());
  showRenameModal.value = false;
  loading.value.rename = false;
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
  deviceStore.busyBar.setApiKey('');
  await navigateTo('/login');
}

const power = computed(() => deviceStore.deviceStatus?.power);

const logoClickCounter = ref(0);
const clickTimeout = ref<number | null>(null);
function onLogoClick () {
  logoClickCounter.value += 1;
  if (clickTimeout.value) {
    clearTimeout(clickTimeout.value);
  }
  clickTimeout.value = window.setTimeout(() => {
    logoClickCounter.value = 0;
    clickTimeout.value = null;
  }, 2000);
  if (logoClickCounter.value >= 10) {
    tabStore.showHiddenTabs = !tabStore.showHiddenTabs;
    logoClickCounter.value = 0;
  }
}

onMounted(async () => {
  await deviceStore.detectConnectionType();
  if (deviceStore.connectionType === 'usb') {
    passwordSetItems.splice(0, 1);
  }

  nameModel.value = await deviceStore.getDeviceName();
});
</script>
