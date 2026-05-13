<template>
  <nav
    data-id="layout-default-header"
    class="relative h-12 flex justify-between items-center"
  >
    <div class="flex gap-3">
      <UIcon
        data-id="layout-default-header-logo"
        name="i-busy-logo-white"
        class="w-[70px] h-[28px] mr-5"
        @click="onLogoClick"
      />
      <div
        v-if="power"
        data-id="layout-default-header-power"
        class="hidden md:flex items-center gap-1.5"
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
        class="group hidden xl:flex items-center text-base gap-2 px-1 py-0.5 rounded-md"
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

      <!-- Show connection as static string between lg and xl screens -->
      <div class="hidden lg:flex xl:hidden items-center text-base gap-2 px-1">
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
      </div>
    </div>

    <div class="absolute left-1/2 -translate-x-1/2">
      <UDropdownMenu
        data-id="layout-default-header-device-menu"
        :items="[
          {
            label: 'Rename',
            icon: 'i-bi-edit',
            onSelect: () => {
              nameModel = deviceStore.deviceName || '';
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
          content: 'w-36 bg-elevated ring-accented/50',
          group: 'border-accented/50',
          item: 'data-[state=open]:before:bg-accented/50 data-highlighted:before:bg-accented/50',
          itemLabelExternalIcon: 'hidden'
        }"
      >
        <UButton
          :label="deviceStore.deviceName"
          size="lg"
          trailing-icon="i-bi-caret-down"
          color="neutral"
          variant="ghost"
          class="max-w-[calc(100vw-2rem-5.625rem-2.25rem)] sm:max-w-auto relative -right-4 xl:-right-2 text-xl rounded-md truncate"
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
            :maxlength="maxNameLength"
            name="new-name"
            size="xl"
            variant="soft"
            :ui="{ base: 'ring-1 ring-glass bg-accented/50' }"
            :disabled="loading.rename"
            @keyup.enter="updateDeviceName"
          >
          <template #trailing>
            <div
              id="character-count"
              class="text-xs text-muted tabular-nums"
              aria-live="polite"
              role="status"
            >
              {{ nameModel?.length }}/{{ maxNameLength }}
            </div>
          </template>
        </UInput>
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
        size="lg"
        :ui="{
          content: 'bg-elevated ring-accented/50',
          group: 'border-accented/50',
          item: 'data-[state=open]:before:bg-accented/50 data-highlighted:before:bg-accented/50',
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

        <template #signin>
          <div class="flex flex-col gap-2 rounded-md">
            <div class="flex justify-between items-center gap-2">
              <UIcon
                name="i-busy-logo"
                class="w-12 h-5"
              />
              <UIcon
                name="i-bi-open-in-new"
                class="size-5"
              />
            </div>
            <div class="max-w-[230px] min-w-[200px] text-xs text-muted">
              {{ accountLinked ? 'Manage your BUSY Account' : 'Link your device to BUSY Account to sync your data across all platforms' }}
            </div>
          </div>
        </template>
      </UDropdownMenu>
    </div>
  </nav>
</template>

<script setup lang="ts">
const deviceStore = useDeviceStore();
const pms = usePasswordModalStore();
const tabStore = useTabStore();

const colorMode = useColorMode();
const maxNameLength = 18;

const passwordSetItems = computed(() => [
  {
    label: 'Manage password',
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
  },
  {
    label: `${colorMode.value === 'dark' ? 'Switch to light' : 'Switch to dark'} theme`,
    icon: colorMode.value === 'dark' ? 'i-bi-brightness' : 'i-bi-moon',
    onSelect: () => colorMode.preference = colorMode.value === 'dark' ? 'light' : 'dark'
  }
]);

const passwordUnsetItems = computed(() => [
  {
    label: 'Set password',
    icon: 'i-bi-password',
    onSelect: () => {
      pms.passwordModel.current = '';
      pms.passwordModel.currentWrong = false;
      pms.passwordModel.new = '';
      pms.showSetPasswordModal = true;
    }
  },
  {
    label: `${colorMode.value === 'dark' ? 'Switch to light' : 'Switch to dark'} theme`,
    icon: colorMode.value === 'dark' ? 'i-bi-brightness' : 'i-bi-moon',
    onSelect: () => colorMode.preference = colorMode.value === 'dark' ? 'light' : 'dark'
  }
]);

const userDropdownItems = computed(() => {
  const baseItems = [
    [
      {
        slot: 'signin' as const,
        type: 'link',
        href: 'https://cloud.busy.app',
        target: '_blank'
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
    return [[...passwordSetItems.value], ...baseItems];
  } else {
    return [[...passwordUnsetItems.value], ...baseItems];
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

const accountLinked = computed(() => deviceStore.accountInfo?.linked);

async function init () {
  await deviceStore.fetchHttpAPIAccess();

  await deviceStore.detectConnectionType();

  nameModel.value = await deviceStore.fetchDeviceName();

  await deviceStore.fetchAccountInfo();
}

const urlHost = computed(() => window.location.host);

onMounted(async () => {
  await init();
  window.addEventListener('device-reconnected', init);
});
onBeforeUnmount(() => {
  window.removeEventListener('device-reconnected', init);
});
</script>
