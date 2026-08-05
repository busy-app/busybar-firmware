<template>
  <SectionCard
    :key="title + (connected ? '-connected' : '') + (showNetworksList ? '-networks-list' : '')"
    data-id="network-section-wifi"
    :title="title"
    :icon="sectionIcon"
  >
    <template
      v-if="!connected && showNetworksList"
      #leading-actions
    >
      <UButton
        data-id="network-section-wifi-back-button"
        icon="i-bi-arrow-back"
        variant="ghost"
        color="neutral"
        square
        :ui="{
          base: 'p-3 rounded-full'
        }"
        @click="handleReturnFromNetworksList"
      />
    </template>

    <template
      v-if="!showNetworksList"
      #subtitle
    >
      <div
        v-if="connected && !reconnecting"
        data-id="network-section-wifi-status-connected"
        class="flex items-center gap-2 text-primary"
      >
        <div class="relative top-[-1px] size-2.5 rounded-full bg-primary" />
        Connected
      </div>
      <div
        v-else-if="connecting"
        data-id="network-section-wifi-status-connecting"
      >
        Connecting...
      </div>
      <div
        v-else-if="reconnecting"
        data-id="network-section-wifi-status-reconnecting"
        :class="showOffline ? 'flex items-center gap-2 text-red-500' : ''"
      >
        <template v-if="!showOffline">
          Reconnecting...
        </template>
        <template v-else>
          <div class="relative top-[-1px] size-2.5 rounded-full bg-red-500" />
          Offline
        </template>
      </div>
      <div
        v-else
        data-id="network-section-wifi-status-disconnected"
      >
        Not connected
      </div>
    </template>

    <template #actions>
      <UButton
        v-if="!connected && !reconnecting && !showNetworksList"
        data-id="network-section-wifi-select-button"
        :label="connecting ? 'Connecting...' : 'Select network'"
        :ui="{
          base: 'rounded-full'
        }"
        class="justify-center sm:justify-start"
        :loading="loading.state || loading.list || connecting"
        @click="listWifiNetworks"
      />
      <UButton
        v-if="(connected || reconnecting) && !showNetworksList"
        data-id="network-section-wifi-forget-button"
        label="Forget network"
        variant="outline"
        color="neutral"
        :ui="{
          base: 'rounded-full hover:bg-error/20 hover:text-error hover:ring-0'
        }"
        class="justify-center sm:justify-start"
        :loading="loading.forget"
        @click="() => { forgetNetworkModal = true; }"
      />
      <UTooltip
        v-if="!connected && showNetworksList"
        text="Add network"
        :delay-duration="0"
      >
        <UButton
          data-id="network-section-wifi-add-button"
          icon="i-bi-plus"
          variant="ghost"
          color="neutral"
          square
          :ui="{
            base: 'p-3 rounded-full'
          }"
          class="justify-center sm:justify-start"
          @click="() => {
            initConnectModel();
            connectToExistingNetwork = false;
            showConnectModal = true;
          }"
        />
      </UTooltip>
      <UTooltip
        v-if="!connected && showNetworksList"
        text="Refresh networks"
        :delay-duration="0"
      >
        <UButton
          data-id="network-section-wifi-refresh-button"
          icon="i-bi-refresh"
          variant="ghost"
          color="neutral"
          square
          :ui="{
            base: 'p-3 rounded-full'
          }"
          class="justify-center sm:justify-start"
          :class="loading.list ? 'animate-spin' : ''"
          :disabled="loading.state || loading.list || connecting"
          @click="() => {
            listWifiNetworks();
          }"
        />
      </UTooltip>
    </template>

    <div
      v-if="connected && !reconnecting"
      class="flex justify-between items-center"
    >
      <div>IP Address</div>
      <template v-if="wifiStore.wifi?.ip_config?.address">
        <CopyButton
          v-if="copyAvailable"
          data-id="network-section-wifi-ip-address-copy-button"
          :text="wifiStore.wifi?.ip_config?.address"
          variant="link"
          color="neutral"
          class="p-0 text-base gap-2"
        />
        <div
          v-else
          data-id="network-section-wifi-ip-address-no-copy"
          class="text-sm text-muted"
        >
          {{ wifiStore.wifi?.ip_config?.address }}
        </div>
      </template>
      <div
        v-else
        data-id="network-section-wifi-ip-address-unavailable"
        class="text-sm text-muted"
      >
        Unable to get IP address
      </div>
    </div>
    <ContentList
      v-if="connected"
      :items="networkContent"
    />

    <template
      v-if="!connected && showNetworksList"
      #raw-body
    >
      <div class="mb-3 relative -top-2">If your network isn’t listed, try a manual scan using the Refresh button.</div>

      <div
        data-id="network-section-wifi-networks"
        class="flex flex-col gap-1.5"
      >
        <div
          v-for="network in networks"
          :key="network.ssid"
          class="flex gap-2 items-center px-2.5 py-2.5 hover:bg-elevated rounded-xl"
          :class="loading.connect ? 'cursor-wait' : 'cursor-pointer'"
          @click="() => {
            if (loading.connect) {
              return;
            }
            initConnectModel();
            if (network.ssid) {
              connectModel.ssid = network.ssid;
            }
            if (network.security === 'Open') {
              return connectToNetwork();
            }
            if (network.security) {
              connectModel.security = network.security;
            }
            connectToExistingNetwork = true;
            showConnectModal = true;
          }"
        >
          <UIcon
            :name="wifiIconByRssi(network.rssi)"
            class="size-6 text-muted"
          />
          <div class="grow">{{ network.ssid }}</div>
          <UIcon
            v-if="loading.connect && connectModel.ssid === network.ssid"
            name="i-busy-loader"
            class="size-6 text-muted animate-spin"
          />
          <template v-else>
            <UTooltip
              v-if="network.security !== 'Open'"
              :text="network.security"
              :delay-duration="0"
            >
              <UIcon
                name="i-bi-lock-simple"
                class="size-6 text-muted"
              />
            </UTooltip>
          </template>
        </div>
        <div
          v-if="networks.length === 0"
          class="flex gap-2 items-center px-2.5 py-2.5 rounded-xl"
        >
          <UIcon
            name="i-bi-wifi-no-connection"
            class="size-6 text-muted"
          />
          <div class="grow">Wi-Fi networks not found</div>
        </div>
      </div>
    </template>
  </SectionCard>

  <ModalGeneric
    v-model:open="showConnectModal"
    data-id="modal-connect-wifi"
    :title="connectToExistingNetwork ? `Connect to ${connectModel.ssid}` : 'Add network'"
    :description="connectToExistingNetwork ? 'Enter the network security password.' : 'Enter the name and security type of the network you want to connect to.'"
    :primary-action-props="{
      label: 'Connect',
      loading: loading.connect,
      disabled: isConnectInvalid,
      onClick: connectToNetwork
    }"
    :secondary-action-props="{
      label: 'Cancel',
      variant: 'ghost',
      disabled: loading.connect,
      onClick: () => { showConnectModal = false; }
    }"
  >
    <template #icon>
      <UIcon
        :name="connectToExistingNetwork ? 'i-bi-wifi-4' : 'i-bi-wifi-add-network'"
        class="size-8 text-muted"
      />
    </template>
    <template #body>
      <template v-if="!connectToExistingNetwork">
        <UFormField label="Network name">
          <UInput
            v-model="connectModel.ssid"
            name="ssid"
            size="xl"
            placeholder="Enter SSID"
            variant="subtle"
            :ui="{ base: 'ring-1 ring-glass bg-accented/50' }"
            @keyup.enter="isConnectInvalid || loading.connect ? null : connectToNetwork()"
          />
        </UFormField>
        <UFormField label="Security">
          <USelect
            v-model="connectModel.security"
            name="security"
            :items="[
              'Open',
              'WPA',
              'WPA2',
              'WEP',
              'WPA Enterprise',
              'WPA2 Enterprise',
              'WPA WPA2 Mixed',
              'WPA3',
              'WPA3 Transition',
              'WPA3 Enterprise',
              'WPA3 Transition Enterprise'
            ]"
            size="xl"
            variant="soft"
            :ui="{ base: 'ring-1 ring-glass bg-accented/50' }"
            class="w-full"
          />
        </UFormField>
      </template>
      <UFormField
        v-if="connectModel.security !== 'Open'"
        :label="connectToExistingNetwork ? '' : 'Password'"
      >
        <UInput
          v-model="connectModel.password"
          name="password"
          size="xl"
          variant="soft"
          :ui="{ base: 'ring-1 ring-glass bg-accented/50' }"
          :type="showPassword ? 'text' : 'password'"
          :placeholder="connectToExistingNetwork ? 'Password' : ''"
          @keyup.enter="isConnectInvalid || loading.connect ? null : connectToNetwork()"
        >
          <template #trailing>
            <UButton
              :icon="showPassword ? 'i-bi-eye' : 'i-bi-eye-shut'"
              variant="ghost"
              color="neutral"
              square
              class="rounded-full"
              :ui="{
                leadingIcon: 'size-6 text-muted'
              }"
              @click="() => { showPassword = !showPassword; }"
            />
          </template>
        </UInput>
      </UFormField>

      <UCollapsible
        data-id="modal-connect-wifi-advanced-options"
        class="flex flex-col gap-6 w-full"
      >
        <UButton
          data-id="modal-connect-wifi-advanced-options-toggle"
          class="group"
          label="Advanced options"
          color="neutral"
          variant="link"
          trailing-icon="i-bi-chevron-down"
          :ui="{
            base: 'px-0',
            trailingIcon: 'group-data-[state=open]:rotate-180 transition-transform duration-200'
          }"
          block
        />

        <template #content>
          <div class="flex flex-col gap-6">
            <UFormField label="IP Settings">
              <USelect
                v-model="connectModel.ip_config.ip_method"
                name="ip-method"
                :items="['dhcp', 'static']"
                size="xl"
                variant="soft"
                :ui="{ base: 'ring-1 ring-glass bg-accented/50' }"
                class="w-full"
              />
            </UFormField>
            <template v-if="connectModel.ip_config.ip_method === 'static'">
              <UFormField label="Address">
                <UInput
                  v-model="connectModel.ip_config.address"
                  v-maska="'###.###.###.###'"
                  name="ip-address"
                  placeholder="___ ___ ___ ___"
                  size="xl"
                  variant="soft"
                  :ui="{ base: 'ring-1 ring-glass bg-accented/50' }"
                  @keyup.enter="isConnectInvalid || loading.connect ? null : connectToNetwork()"
                />
              </UFormField>
              <UFormField label="Subnet Mask">
                <UInput
                  v-model="connectModel.ip_config.mask"
                  v-maska="'###.###.###.###'"
                  name="subnet-mask"
                  placeholder="___ ___ ___ ___"
                  size="xl"
                  variant="soft"
                  :ui="{ base: 'ring-1 ring-glass bg-accented/50' }"
                  @keyup.enter="isConnectInvalid || loading.connect ? null : connectToNetwork()"
                />
              </UFormField>
              <UFormField label="Gateway">
                <UInput
                  v-model="connectModel.ip_config.gateway"
                  v-maska="'###.###.###.###'"
                  name="gateway"
                  placeholder="___ ___ ___ ___"
                  size="xl"
                  variant="soft"
                  :ui="{ base: 'ring-1 ring-glass bg-accented/50' }"
                  @keyup.enter="isConnectInvalid || loading.connect ? null : connectToNetwork()"
                />
              </UFormField>
            </template>
          </div>
        </template>
      </UCollapsible>
    </template>
  </ModalGeneric>

  <ModalGeneric
    v-model:open="forgetNetworkModal"
    data-id="modal-forget-wifi"
    title="Forget current Wi-Fi network?"
    description="The device will be disconnected, and Virtual LAN will no longer be accessible via wireless connection."
    :primary-action-props="{
      label: 'Forget network',
      variant: 'soft',
      color: 'error',
      loading: loading.forget,
      onClick: forgetNetwork
    }"
    :secondary-action-props="{
      label: 'Cancel',
      variant: 'ghost',
      disabled: loading.forget,
      onClick: () => { forgetNetworkModal = false; }
    }"
  />
</template>

<script setup lang="ts">
import { vMaska } from 'maska/vue';
import type { WifiConnectParams, WifiNetwork } from '@busy-app/busy-lib';

const wifiStore = useWifiStore();

const loading = ref({
  state: false,
  enable: false,
  list: false,
  connect: false,
  forget: false,
  access: false
});

async function refreshWifiState () {
  loading.value.state = true;
  wifiStore.wifi = await wifiStore.fetchWifiState();
  loading.value.state = false;
}

const networks = ref<WifiNetwork[]>([]);
const showNetworksList = ref(false);
async function listWifiNetworks () {
  loading.value.list = true;
  if (!(networks.value.length > 0 && showConnectModal.value)) {
    networks.value = (await wifiStore.listWifiNetworks()) ?? [];
  }
  loading.value.list = false;
  showNetworksList.value = true;

  if (!scanTimeout.value) {
    scanTimeout.value = setTimeout(async () => {
      if (wifiStore.wifi?.state === 'connected' || loading.value.connect) {
        return;
      }
      scanTimeout.value = null;
      await listWifiNetworks();
    }, 10000);
  }
}

const scanTimeout = ref<NodeJS.Timeout | null>(null);

function handleReturnFromNetworksList () {
  showNetworksList.value = false;
  networks.value = [];
  if (scanTimeout.value) {
    clearTimeout(scanTimeout.value);
    scanTimeout.value = null;
  }
}

const showConnectModal = ref(false);
const showPassword = ref(false);
const connectToExistingNetwork = ref(false);
const connectModel = ref<WifiConnectParams>({
  ssid: '',
  security: 'Open',
  password: '',
  ip_config: {
    ip_method: 'dhcp' as 'dhcp' | 'static',
    address: '',
    mask: '',
    gateway: ''
  }
});
const initConnectModel = () => {
  connectModel.value = {
    ssid: '',
    security: 'Open',
    password: '',
    ip_config: {
      ip_method: 'dhcp',
      address: '',
      mask: '',
      gateway: ''
    }
  };
};

const isConnectInvalid = computed(() => connectModel.value.ssid === '' || (connectModel.value.security !== 'Open' && connectModel.value.password === ''));

async function connectToNetwork () {
  if (!connectModel.value.ssid) {
    return;
  }
  loading.value.connect = true;
  await wifiStore.connectToWifiNetwork(connectModel.value);
  await new Promise(resolve => setTimeout(resolve, 1000));
  await refreshWifiState();
  handleReturnFromNetworksList();
  loading.value.connect = false;
  showConnectModal.value = false;
}

async function forgetNetwork () {
  const wasConnectedViaWifi = useDeviceStore().connectionType === 'wifi';
  loading.value.forget = true;
  await wifiStore.disconnectFromWifiNetwork();
  forgetNetworkModal.value = false;
  if (wasConnectedViaWifi) {
    window.location.reload();
  }
  await refreshWifiState();
  loading.value.forget = false;
}

const connected = computed(() => wifiStore.wifi?.state === 'connected');
const connecting = computed(() => wifiStore.wifi?.state === 'connecting');
const reconnecting = computed(() => wifiStore.wifi?.state === 'reconnecting' || (wifiStore.wifi?.channel === 0 && wifiStore.wifi?.rssi === 0));

const reconnectTimeout = ref<NodeJS.Timeout | null>(null);
const RECONNECT_TIMEOUT_DURATION = 10000;
const showOffline = ref(false);
function setReconnectTimeout () {
  if (!reconnectTimeout.value) {
    reconnectTimeout.value = setTimeout(() => {
      if (reconnecting.value) {
        showOffline.value = true;
      }
      reconnectTimeout.value = null;
    }, RECONNECT_TIMEOUT_DURATION);
  }
}
watch(reconnecting, newValue => {
  if (newValue) {
    setReconnectTimeout();
  } else {
    if (reconnectTimeout.value) {
      clearTimeout(reconnectTimeout.value);
      reconnectTimeout.value = null;
    }
    showOffline.value = false;
  }
});

const title = computed(() => {
  if (connected.value || connecting.value || reconnecting.value) {
    return wifiStore.wifi?.ssid || 'Wi-Fi';
  } else if (networks.value.length > 0) {
    return 'Select network';
  }
  return 'Wi-Fi';
});

function wifiIconByRssi (rssi: WifiNetwork['rssi']): string {
  if (!rssi) {
    return 'i-bi-wifi-1';
  }
  if (rssi > -60) {
    return 'i-bi-wifi-4';
  }
  if (rssi > -70) {
    return 'i-bi-wifi-3';
  }
  if (rssi > -80) {
    return 'i-bi-wifi-2';
  }
  return 'i-bi-wifi-2';
}

const sectionIcon = computed(() => {
  if (showOffline.value) {
    return 'i-bi-wifi-off';
  }
  if (showNetworksList.value) {
    return undefined;
  }
  if (connecting.value || reconnecting.value) {
    return 'i-bi-wifi-1';
  }
  if (connected.value) {
    return wifiIconByRssi(wifiStore.wifi?.rssi);
  }
  return 'i-bi-wifi-1';
});

const forgetNetworkModal = ref(false);

const networkContent = computed(() => [
  [
    {
      title: 'Name',
      value: wifiStore.wifi?.ssid,
      loading: !wifiStore.wifi
    },
    {
      title: 'IP address',
      value: wifiStore.wifi?.ip_config?.address,
      loading: !wifiStore.wifi,
      class: 'overflow-visible whitespace-normal break-all'
    },
    {
      title: 'Channel',
      value: wifiStore.wifi?.channel ? String(wifiStore.wifi.channel) : undefined,
      loading: !wifiStore.wifi
    }
  ],
  [
    {
      title: 'Security',
      value: wifiStore.wifi?.security,
      loading: !wifiStore.wifi
    },
    {
      title: 'BSSID',
      value: wifiStore.wifi?.bssid,
      loading: !wifiStore.wifi,
      class: 'overflow-visible whitespace-normal break-all'
    },
    {
      title: 'Signal strength',
      value: wifiStore.wifi?.rssi ? `${wifiStore.wifi.rssi} dBm` : 'Unknown',
      loading: !wifiStore.wifi
    }
  ]
]);

async function init () {
  await refreshWifiState();
}

onMounted(async () => {
  await init();
  window.addEventListener('device-reconnected', init);

  if (reconnecting.value) {
    setReconnectTimeout();
  }
});
onBeforeUnmount(() => window.removeEventListener('device-reconnected', init));
</script>
