<template>
  <SectionCard
    :key="title + (connected ? '-connected' : '') + (showNetworksList ? '-networks-list' : '')"
    data-id="network-section-wifi"
    :title="title"
    :icon="showNetworksList ? undefined : (connected || wifiStore.wifi?.state === 'connecting') ? 'i-bi-wifi-4' : 'i-bi-wifi-off'"
  >
    <template
      v-if="!connected && showNetworksList"
      #leading-actions
    >
      <UButton
        data-id="network-section-wifi-back-button"
        icon="i-ri-arrow-left-line"
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
        v-if="connected"
        data-id="network-section-wifi-status-connected"
        class="flex items-center gap-2 text-green-500"
      >
        <div class="relative top-[-1px] size-2.5 rounded-full bg-green-500" />
        Connected
      </div>
      <div
        v-else-if="wifiStore.wifi?.state === 'connecting'"
        data-id="network-section-wifi-status-connecting"
      >
        Connecting...
      </div>
      <div
        v-else
        data-id="network-section-wifi-status-disconnected"
      >
        Disconnected
      </div>
    </template>

    <template #actions>
      <UButton
        v-if="!connected && !showNetworksList"
        data-id="network-section-wifi-select-button"
        :label="wifiStore.wifi?.state === 'connecting' ? 'Connecting...' : 'Select network'"
        :ui="{
          base: 'px-2.5 py-2 rounded-full'
        }"
        class="justify-center sm:justify-start"
        :loading="loading.state || loading.list || wifiStore.wifi?.state === 'connecting'"
        @click="listWifiNetworks"
      />
      <UButton
        v-if="connected && !showNetworksList"
        data-id="network-section-wifi-forget-button"
        label="Forget network"
        variant="soft"
        color="error"
        :ui="{
          base: 'px-2.5 py-2 rounded-full'
        }"
        class="justify-center sm:justify-start"
        :loading="loading.forget"
        @click="forgetNetwork"
      />
      <UTooltip
        v-if="!connected && showNetworksList"
        text="Add network"
        :delay-duration="0"
      >
        <UButton
          data-id="network-section-wifi-add-button"
          icon="i-ri-add-line"
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
          data-id="network-section-wifi-add-button"
          icon="i-bi-refresh"
          variant="ghost"
          color="neutral"
          square
          :ui="{
            base: 'p-3 rounded-full'
          }"
          class="justify-center sm:justify-start"
          :class="loading.list ? 'animate-spin' : ''"
          @click="() => {
            listWifiNetworks();
          }"
        />
      </UTooltip>
    </template>

    <div
      v-if="connected"
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
          class="p-0"
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
    <div
      v-if="connected"
      class="grid sm:grid-cols-2 gap-y-3 gap-x-1"
    >
      <div
        v-for="[property, value] in Object.entries({
          'SSID': wifiStore.wifi?.ssid,
          'IP Method': wifiStore.wifi?.ip_config?.ip_method === 'dhcp' ? 'DHCP' : 'Static',
          'Security': wifiStore.wifi?.security || 'Open',
          'IP Type': wifiStore.wifi?.ip_config?.ip_type === 'ipv4' ? 'IPv4' : 'IPv6'
        })"
        :key="property"
        class="flex"
      >
        <div class="w-[120px] text-muted">{{ property }}</div>
        <div class="max-w-[140px] md:max-w-[180px] text-ellipsis overflow-hidden">{{ value }}</div>
      </div>
    </div>

    <template
      v-if="!connected && showNetworksList"
      #raw-body
    >
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
    wide
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
    <template #body>
      <template v-if="!connectToExistingNetwork">
        <UFormField label="Network name">
          <UInput
            v-model="connectModel.ssid"
            name="ssid"
            size="xl"
            variant="soft"
            :ui="{ base: 'ring-1 ring-glass' }"
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
            :ui="{ base: 'ring-1 ring-glass' }"
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
          :ui="{ base: 'ring-1 ring-glass' }"
          :type="showPassword ? 'text' : 'password'"
          :placeholder="connectToExistingNetwork ? 'Password' : ''"
          @keyup.enter="isConnectInvalid || loading.connect ? null : connectToNetwork()"
        >
          <template #trailing>
            <UButton
              :icon="showPassword ? 'i-ri-eye-close-line' : 'i-ri-eye-line'"
              variant="ghost"
              color="neutral"
              square
              class="rounded-full"
              :ui="{
                leadingIcon: 'size-6 text-muted'
              }"
              @click="showPassword = !showPassword"
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
          trailing-icon="i-ri-arrow-down-s-fill"
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
                v-model="connectModel.ipConfig.ipMethod"
                name="ip-method"
                :items="['dhcp', 'static']"
                size="xl"
                variant="soft"
                :ui="{ base: 'ring-1 ring-glass' }"
                class="w-full"
              />
            </UFormField>
            <template v-if="connectModel.ipConfig.ipMethod === 'static'">
              <UFormField label="Address">
                <UInput
                  v-model="connectModel.ipConfig.address"
                  v-maska="'###.###.###.###'"
                  name="ip-address"
                  placeholder="___.___.___.___"
                  size="xl"
                  variant="soft"
                  :ui="{ base: 'ring-1 ring-glass' }"
                  @keyup.enter="isConnectInvalid || loading.connect ? null : connectToNetwork()"
                />
              </UFormField>
              <UFormField label="Subnet Mask">
                <UInput
                  v-model="connectModel.ipConfig.mask"
                  v-maska="'###.###.###.###'"
                  name="subnet-mask"
                  placeholder="___.___.___.___"
                  size="xl"
                  variant="soft"
                  :ui="{ base: 'ring-1 ring-glass' }"
                  @keyup.enter="isConnectInvalid || loading.connect ? null : connectToNetwork()"
                />
              </UFormField>
              <UFormField label="Gateway">
                <UInput
                  v-model="connectModel.ipConfig.gateway"
                  v-maska="'###.###.###.###'"
                  name="gateway"
                  placeholder="___.___.___.___"
                  size="xl"
                  variant="soft"
                  :ui="{ base: 'ring-1 ring-glass' }"
                  @keyup.enter="isConnectInvalid || loading.connect ? null : connectToNetwork()"
                />
              </UFormField>
            </template>
          </div>
        </template>
      </UCollapsible>
    </template>
  </ModalGeneric>
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
  networks.value = await wifiStore.listWifiNetworks();
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
  ipConfig: {
    ipMethod: 'dhcp' as 'dhcp' | 'static',
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
    ipConfig: {
      ipMethod: 'dhcp',
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
  if (!wifiStore.wifi || !wifiStore.wifi.ssid) {
    return;
  }
  loading.value.forget = true;
  await wifiStore.disconnectFromWifiNetwork();
  await refreshWifiState();
  loading.value.forget = false;
}

const connected = computed(() => wifiStore.wifi?.state === 'connected');

const title = computed(() => {
  if (wifiStore.wifi?.state === 'connected' || wifiStore.wifi?.state === 'connecting') {
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
  if (rssi < 60) {
    return 'i-bi-wifi-4';
  }
  if (rssi < 70) {
    return 'i-bi-wifi-3';
  }
  if (rssi < 80) {
    return 'i-bi-wifi-2';
  }
  return 'i-bi-wifi-2';
}

async function init () {
  await refreshWifiState();
}

onMounted(async () => {
  await init();
  window.addEventListener('device-reconnected', init);
});
onBeforeUnmount(() => window.removeEventListener('device-reconnected', init));
</script>
