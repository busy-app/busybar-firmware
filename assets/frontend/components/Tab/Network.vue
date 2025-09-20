<template>
  <SectionCard
    :key="title"
    :title="title"
    :icon="networks.length ? undefined : connected ? 'i-ri-signal-wifi-fill' : 'i-ri-signal-wifi-line'"
  >
    <template
      v-if="!connected && networks.length"
      #leading-actions
    >
      <UButton
        icon="i-ri-arrow-left-line"
        variant="ghost"
        color="neutral"
        square
        :ui="{
          base: 'p-3 rounded-full'
        }"
        @click="networks = []"
      />
    </template>

    <template
      v-if="!networks.length"
      #subtitle
    >
      <div
        v-if="connected"
        class="flex items-center gap-2 text-green-500"
      >
        <div class="relative top-[-1px] size-2.5 rounded-full bg-green-500" />
        Connected
      </div>
      <div v-else>Disconnected</div>
    </template>

    <template #actions>
      <UButton
        v-if="!connected && !networks.length"
        label="Select network"
        :ui="{
          base: 'px-2.5 py-2 rounded-full'
        }"
        :loading="loading.state || loading.list"
        @click="listWifiNetworks"
      />
      <UButton
        v-if="connected && !networks.length"
        label="Forget network"
        variant="soft"
        color="error"
        :ui="{
          base: 'px-2.5 py-2 rounded-full'
        }"
        :loading="loading.forget"
        @click="forgetNetwork"
      />
      <UTooltip
        v-if="!connected && networks.length"
        text="Add network"
        :delay-duration="0"
      >
        <UButton
          icon="i-ri-add-line"
          variant="ghost"
          color="neutral"
          square
          :ui="{
            base: 'p-3 rounded-full'
          }"
          @click="showConnectModal = true"
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
          :text="wifiStore.wifi?.ip_config?.address"
          variant="link"
          color="neutral"
          class="px-0"
        />
        <div
          v-else
          class="text-sm text-muted"
        >
          {{ wifiStore.wifi?.ip_config?.address }}
        </div>
      </template>
      <div
        v-else
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
      v-if="!connected && networks.length"
      #raw-body
    >
      <div class="flex flex-col gap-1.5">
        <div
          v-for="network in networks"
          :key="network.ssid"
          class="flex gap-2 items-center px-2.5 py-2.5 cursor-pointer hover:bg-elevated rounded-xl"
          @click="() => {
            connectModel.ssid = network.ssid;
            connectModel.security = network.security;
            showConnectModal = true;
          }"
        >
          <UIcon
            :name="wifiIconByRssi(network.rssi)"
            class="size-6 text-muted"
          />
          <div class="grow">{{ network.ssid }}</div>
          <UTooltip
            v-if="network.security !== 'Open'"
            :text="network.security"
            :delay-duration="0"
          >
            <UIcon
              name="i-ri-lock-line"
              class="size-6 text-muted"
            />
          </UTooltip>
        </div>
      </div>
    </template>
  </SectionCard>

  <ModalGeneric
    v-model:open="showConnectModal"
    title="Add Wi-Fi Network"
    description="Enter the name and security type of the network you want to connect to."
    wide
    :primary-action-props="{
      label: 'Connect',
      loading: loading.connect,
      disabled: connectModel.ssid === '' || (connectModel.security !== 'Open' && connectModel.password === ''),
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
      <UFormField label="Network name">
        <UInput
          v-model="connectModel.ssid"
          size="xl"
          variant="soft"
        />
      </UFormField>
      <UFormField label="Security">
        <USelect
          v-model="connectModel.security"
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
          class="w-full"
        />
      </UFormField>
      <UFormField
        v-if="connectModel.security !== 'Open'"
        label="Password"
      >
        <UInput
          v-model="connectModel.password"
          size="xl"
          variant="soft"
          :type="showPassword ? 'text' : 'password'"
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

      <UCollapsible class="flex flex-col gap-6 w-full">
        <UButton
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
            <UFormField label="IP Type">
              <USelect
                v-model="connectModel.ip_config.ip_type"
                :items="['ipv4', 'ipv6']"
                size="xl"
                variant="soft"
                class="w-full"
              />
            </UFormField>
            <UFormField label="IP Settings">
              <USelect
                v-model="connectModel.ip_config.ip_method"
                :items="['dhcp', 'static']"
                size="xl"
                variant="soft"
                class="w-full"
              />
            </UFormField>
            <template v-if="connectModel.ip_config.ip_method === 'static'">
              <UFormField label="Address">
                <UInput
                  v-model="connectModel.ip_config.address"
                  v-maska="'###.###.###.###'"
                  placeholder="___.___.___.___"
                  size="xl"
                  variant="soft"
                />
              </UFormField>
              <UFormField label="Subnet Mask">
                <UInput
                  v-model="connectModel.ip_config.mask"
                  v-maska="'###.###.###.###'"
                  placeholder="___.___.___.___"
                  size="xl"
                  variant="soft"
                />
              </UFormField>
              <UFormField label="Gateway">
                <UInput
                  v-model="connectModel.ip_config.gateway"
                  v-maska="'###.###.###.###'"
                  placeholder="___.___.___.___"
                  size="xl"
                  variant="soft"
                />
              </UFormField>
            </template>
          </div>
        </template>
      </UCollapsible>
    </template>
  </ModalGeneric>

  <SectionCard
    title="HTTP API"
    icon="i-ri-exchange-2-line"
  >
    <div class="flex justify-between items-center">
      <div>Over USB</div>
      <UButton
        variant="link"
        class="px-0"
      >
        <span class="underline">http://10.04.20/docs</span>
        <UIcon
          name="i-ri-external-link-line"
          class="size-4"
        />
      </UButton>
    </div>
    <div class="flex flex-col gap-1">
      <div>Over Wi-Fi</div>
      <div class="text-sm text-muted">Connect to a Wi-Fi network to enable access to HTTP API over Wi-Fi</div>
    </div>
  </SectionCard>
</template>

<script setup lang="ts">
import { vMaska } from 'maska/vue';

const wifiStore = useWifiStore();

const loading = ref({
  state: false,
  enable: false,
  list: false,
  connect: false,
  forget: false
});

async function refreshWifiState () {
  loading.value.state = true;
  wifiStore.wifi = await wifiStore.fetchWifiState();
  loading.value.state = false;
}

const networks = ref<WifiNetwork[]>([]);

async function listWifiNetworks () {
  loading.value.list = true;
  networks.value = await wifiStore.listWifiNetworks();
  loading.value.list = false;
}

const showConnectModal = ref(false);
const showPassword = ref(false);
const connectModel = ref({
  ssid: '',
  security: 'Open' as WifiSecurity,
  password: '',
  ip_config: {
    ip_method: 'dhcp' as 'dhcp' | 'static',
    ip_type: 'ipv4' as 'ipv4' | 'ipv6',
    address: '',
    mask: '',
    gateway: ''
  } as WifiConnectIPConfig
});

async function connectToNetwork () {
  if (!connectModel.value.ssid) {
    return;
  }
  loading.value.connect = true;
  await wifiStore.connectToWifiNetwork(connectModel.value);
  await new Promise(resolve => setTimeout(resolve, 1000));
  await refreshWifiState();
  networks.value = [];
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
  if (wifiStore.wifi?.state === 'connected') {
    return wifiStore.wifi?.ssid || 'Wi-Fi';
  } else if (networks.value.length > 0) {
    return 'Select network';
  }
  return 'Wi-Fi';
});

function wifiIconByRssi (rssi: number): string {
  if (rssi < 60) {
    return 'i-ri-signal-wifi-fill';
  }
  if (rssi < 70) {
    return 'i-ri-signal-wifi-3-fill';
  }
  if (rssi < 80) {
    return 'i-ri-signal-wifi-2-fill';
  }
  return 'i-ri-signal-wifi-1-fill';
}

onMounted(async () => {
  await refreshWifiState();
});
</script>
