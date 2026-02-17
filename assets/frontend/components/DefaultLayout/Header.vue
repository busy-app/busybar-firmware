<template>
  <nav
    data-id="layout-default-header"
    class="relative h-12 flex justify-between items-center"
  >
    <div class="flex gap-3">
      <UIcon
        data-id="layout-default-header-logo"
        name="i-busy-bar-logo"
        class="w-[70px] h-[28px] mr-5"
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
      <CopyButton
        :text="urlHost"
        size="md"
        variant="ghost"
        color="neutral"
        class="group hidden md:flex items-center text-base gap-2 px-1 py-0.5 rounded-md"
        icon-class="opacity-0 transition-opacity group-hover:opacity-100"
      >
        <div
          data-id="layout-default-header-connection-state"
          class="flex items-center gap-2"
        >
          <template v-if="deviceStore.isConnected">
            <UIcon
              :name="deviceStore.connectionType === 'usb' ? 'i-bi-usb-alt' : 'i-bi-wifi-4'"
              class="size-5"
            />
            Connected
          </template>
          <template v-else>
            <UIcon
              name="i-bi-alert"
              class="size-5 text-warning"
            />
            Disconnected
          </template>

          <div class="ml-1 opacity-0 transition-opacity group-hover:opacity-100">{{ urlHost }}</div>
        </div>
      </CopyButton>
    </div>

    <div class="absolute left-1/2 -translate-x-1/2">
      <UDropdownMenu
        data-id="layout-default-header-device-menu"
        :items="[
          {
            label: 'Rename',
            icon: 'i-bi-edit',
            onSelect: () => {
              nameModel = '';
              showRenameModal = true;
            }
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
        <template #icon>
          <UIcon
            name="i-bi-edit"
            class="size-8 text-muted"
          />
        </template>
        <template #body>
          <UInput
            v-model="nameModel"
            name="new-name"
            size="xl"
            variant="soft"
            :ui="{ base: 'ring-1 ring-glass bg-accented/50' }"
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
          icon="i-bi-user-fill"
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

const passwordSetItems = [
  {
    label: 'Lock down',
    icon: 'i-bi-lock',
    onSelect: () => {
      lockDown();
    }
  },
  {
    label: 'Virtual LAN password',
    icon: 'i-bi-password',
    children: [
      {
        label: 'Change',
        icon: 'i-bi-edit',
        onSelect: () => {
          pms.passwordModel.current = '';
          pms.passwordModel.currentWrong = false;
          pms.passwordModel.new = '';
          pms.showUpdatePasswordModal = true;
        }
      },
      {
        label: 'Remove',
        icon: 'i-bi-unlock',
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
    icon: 'i-bi-password',
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
        label: 'Log in to BUSY Account',
        icon: 'i-bi-user',
        slot: 'signin' as const,
        type: 'link',
        href: 'https://cloud.busy.app',
        target: '_blank'
      }
    ],
    [
      {
        label: `${colorMode.value === 'dark' ? 'Switch to light' : 'Switch to dark'} theme`,
        icon: colorMode.value === 'dark' ? 'i-bi-brightness' : 'i-bi-moon',
        onSelect: () => colorMode.preference = colorMode.value === 'dark' ? 'light' : 'dark'
      }
    ]
  ];

  if (deviceStore.httpAPIAccess === undefined) {
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
  } else if (deviceStore.httpAPIAccess.mode === 'key') {
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
  await navigateTo('/login', { external: true });
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

async function init () {
  await deviceStore.fetchHttpAPIAccess();

  await deviceStore.detectConnectionType();
  if (deviceStore.connectionType === 'usb') {
    passwordSetItems.splice(0, 1);
  }

  nameModel.value = await deviceStore.fetchDeviceName();
}

const urlHost = computed(() => window.location.host);

onMounted(async () => {
  await init();
  window.addEventListener('device-reconnected', init);
});
onBeforeUnmount(() => window.removeEventListener('device-reconnected', init));
</script>
