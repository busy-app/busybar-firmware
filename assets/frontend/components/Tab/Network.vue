<template>
  <SectionCard
    :key="title"
    data-id="network-section-wifi"
    :title="title"
    :icon="networks.length ? undefined : connected ? 'i-ri-signal-wifi-fill' : 'i-ri-signal-wifi-line'"
  >
    <template
      v-if="!connected && networks.length"
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
        @click="networks = []"
      />
    </template>

    <template
      v-if="!networks.length"
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
        v-else
        data-id="network-section-wifi-status-disconnected"
      >
        Disconnected
      </div>
    </template>

    <template #actions>
      <UButton
        v-if="!connected && !networks.length"
        data-id="network-section-wifi-select-button"
        label="Select network"
        :ui="{
          base: 'px-2.5 py-2 rounded-full'
        }"
        class="justify-center sm:justify-start"
        :loading="loading.state || loading.list"
        @click="listWifiNetworks"
      />
      <UButton
        v-if="connected && !networks.length"
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
        v-if="!connected && networks.length"
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
          class="px-0"
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
      v-if="!connected && networks.length"
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
            connectModel.ssid = network.ssid;
            if (network.security === 'Open') {
              return connectToNetwork();
            }
            connectModel.security = network.security;
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
                name="i-ri-lock-line"
                class="size-6 text-muted"
              />
            </UTooltip>
          </template>
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
      <template v-if="!connectToExistingNetwork">
        <UFormField label="Network name">
          <UInput
            v-model="connectModel.ssid"
            name="ssid"
            size="xl"
            variant="soft"
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
          :type="showPassword ? 'text' : 'password'"
          :placeholder="connectToExistingNetwork ? 'Password' : ''"
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
            <UFormField label="IP Type">
              <USelect
                v-model="connectModel.ip_config.ip_type"
                name="ip-type"
                :items="['ipv4', 'ipv6']"
                size="xl"
                variant="soft"
                class="w-full"
              />
            </UFormField>
            <UFormField label="IP Settings">
              <USelect
                v-model="connectModel.ip_config.ip_method"
                name="ip-method"
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
                  name="ip-address"
                  placeholder="___.___.___.___"
                  size="xl"
                  variant="soft"
                />
              </UFormField>
              <UFormField label="Subnet Mask">
                <UInput
                  v-model="connectModel.ip_config.mask"
                  v-maska="'###.###.###.###'"
                  name="subnet-mask"
                  placeholder="___.___.___.___"
                  size="xl"
                  variant="soft"
                />
              </UFormField>
              <UFormField label="Gateway">
                <UInput
                  v-model="connectModel.ip_config.gateway"
                  v-maska="'###.###.###.###'"
                  name="gateway"
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
    v-if="deviceStore.httpAPIAccess"
    data-id="network-section-http-api"
    title="HTTP API"
    icon="i-ri-exchange-2-line"
  >
    <div class="flex justify-between items-center">
      <div>Over USB</div>
      <UButton
        variant="link"
        class="px-0"
        href="http://10.0.4.20/docs"
        target="_blank"
      >
        <span class="underline">http://10.0.4.20/docs</span>
        <UIcon
          name="i-ri-external-link-line"
          class="size-4"
        />
      </UButton>
    </div>
    <div class="flex flex-col gap-1">
      <div class="flex justify-between items-center gap-y-4 gap-x-6">
        <div>Over Wi-Fi</div>
        <UButton
          v-if="connected"
          variant="link"
          class="px-0"
          :href="`http://${wifiStore.wifi?.ip_config?.address}/docs`"
          target="_blank"
        >
          <span class="underline">http://{{ wifiStore.wifi?.ip_config?.address }}/docs</span>
          <UIcon
            name="i-ri-external-link-line"
            class="size-4"
          />
        </UButton>
      </div>
      <div
        v-if="!connected"
        class="text-sm text-muted"
      >
        Connect to a Wi-Fi network to enable access to HTTP API over Wi-Fi
      </div>
      <template v-else>
        <div class="flex justify-between items-center gap-y-4 gap-x-6 mt-3">
          <div class="flex flex-col gap-1">
            <div>HTTP API access</div>
            <div class="text-sm text-muted">Send requests and control BUSY Bar when connected via Wi-Fi</div>
          </div>

          <USwitch
            v-model="httpApiSwitchModel"
            data-id="network-section-http-api-switch"
            :loading="loading.access"
            @update:model-value="handleHttpApiToggle"
          />
        </div>

        <div
          v-if="deviceStore.httpAPIAccess.mode !== 'disabled'"
          class="flex justify-between items-center gap-y-4 gap-x-6 mt-3"
        >
          <div class="flex flex-col gap-1">
            <div class="flex items-center gap-2">
              <div>Security</div>
              <UBadge
                v-if="deviceStore.httpAPIAccess.mode === 'key'"
                data-id="network-section-http-api-status-secure"
                icon="i-ri-lock-fill"
                label="Secure"
                color="success"
                size="sm"
                class="rounded-full"
              />
              <UBadge
                v-else-if="deviceStore.httpAPIAccess.mode === 'enabled'"
                data-id="network-section-http-api-status-insecure"
                icon="i-ri-alert-fill"
                label="Insecure"
                color="warning"
                size="sm"
                class="rounded-full"
              />
            </div>
            <div
              v-if="deviceStore.httpAPIAccess.mode === 'key'"
              class="text-sm text-muted"
            >
              A password will be asked each time this page is opened with a BUSY Bar connected via Wi-Fi
            </div>
            <div
              v-else-if="deviceStore.httpAPIAccess.mode === 'enabled'"
              class="text-sm text-muted"
            >
              Anyone on the same Wi-Fi network will be able to access the device via this page. We recommend setting a password that will be asked each time this page is opened with a BUSY Bar connected via Wi-Fi.
            </div>
          </div>

          <UDropdownMenu
            v-if="deviceStore.httpAPIAccess.mode === 'key'"
            data-id="network-section-http-api-security-dropdown"
            :items="[
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
              icon="i-ri-more-fill"
              variant="ghost"
              color="neutral"
              square
            />
          </UDropdownMenu>

          <UButton
            v-else
            data-id="network-section-http-api-set-password-button"
            label="Set password"
            variant="soft"
            color="primary"
            @click="() => {
              pms.passwordModel.current = '';
              pms.passwordModel.currentWrong = false;
              pms.passwordModel.new = '';
              pms.showSetPasswordModal = true;
            }"
          />
        </div>
      </template>

      <ModalGeneric
        v-model:open="showEnableHttpApiModal"
        data-id="modal-http-api"
        title="Enable access to HTTP API?"
        description="Anyone on the same Wi-Fi network will be able to access the device via this page. We recommend setting a password that will be asked each time you open this page with a BUSY Bar connected via Wi-Fi."
        show-close-button
        wide
        :primary-action-props="{
          label: 'Set password and enable',
          loading: loading.access,
          onClick: async () => {
            showEnableHttpApiModal = false;
            pms.passwordModel.current = '';
            pms.passwordModel.currentWrong = false;
            pms.passwordModel.new = '';
            pms.showSetPasswordModal = true;
          }
        }"
        :secondary-action-props="{
          label: 'Continue without password',
          loading: loading.access,
          onClick: async () => {
            loading.access = true;
            await deviceStore.setHttpAPIAccess('enabled');
            loading.access = false;
            showEnableHttpApiModal = false;
          }
        }"
      />
    </div>
  </SectionCard>
</template>

<script setup lang="ts">
import { vMaska } from 'maska/vue';

const wifiStore = useWifiStore();
const deviceStore = useDeviceStore();
const pms = usePasswordModalStore();

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

async function listWifiNetworks () {
  loading.value.list = true;
  networks.value = await wifiStore.listWifiNetworks();
  loading.value.list = false;
}

const showConnectModal = ref(false);
const showPassword = ref(false);
const connectToExistingNetwork = ref(false);
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
const initConnectModel = () => {
  connectModel.value = {
    ssid: '',
    security: 'Open',
    password: '',
    ip_config: {
      ip_method: 'dhcp',
      ip_type: 'ipv4',
      address: '',
      mask: '',
      gateway: ''
    }
  };
};

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

const httpApiSwitchModel = ref(false);
const showEnableHttpApiModal = ref(false);

watch(() => deviceStore.httpAPIAccess, access => {
  if (!access) {
    return;
  }
  if (access.mode === 'key' || access.mode === 'enabled') {
    httpApiSwitchModel.value = true;
  } else {
    httpApiSwitchModel.value = false;
  }
}, { immediate: true, deep: true });

async function handleHttpApiToggle (value: boolean) {
  if (!value) {
    loading.value.access = true;
    await deviceStore.setHttpAPIAccess('disabled');
    loading.value.access = false;
  } else {
    if (deviceStore.httpAPIAccess?.mode === 'key') {
      httpApiSwitchModel.value = true;
    } else {
      showEnableHttpApiModal.value = true;
      httpApiSwitchModel.value = false;
    }
  }
}

onMounted(async () => {
  await refreshWifiState();
});
</script>
