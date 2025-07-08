<template>
  <!-- sidebar width: 388px bar image + 2*24px padding -->
  <nav class="w-[436px] h-[calc(100vh-64px)] fixed flex flex-col px-6 pt-6">
    <UCard
      class="flex flex-col flex-1 overflow-auto rounded-none"
      :ui="{
        root: 'bg-transparent shadow-none ring-0 divide-none',
        header: 'p-0 sm:p-0',
        body: 'flex-1 px-0 pt-8 pb-0 sm:px-0 sm:pt-16 sm:pb-0',
        footer: 'p-0 sm:p-0'
      }"
    >
      <template #header>
        <div class="flex items-start justify-between">
          <div class="flex flex-col">
            <span class="text-title text-neutral-800 dark:text-neutral-200 gap-0.5">BUSY Bar</span>
            <div>
              <UBadge
                icon="i-tabler-usb"
                size="sm"
                color="success"
                variant="soft"
                class=""
              >
                Connected
              </UBadge>
            </div>
          </div>

          <div class="mt-1.5">
            <ToggleTheme size="lg" />
          </div>
        </div>
      </template>

      <div class="flex flex-col items-center gap-y-12">
        <div class="flex flex-col items-center">
          <ScreenStream class="mb-1" />

          <span class="text-neutral-500">
            Firmware Version
            <span class="text-neutral-800 dark:text-neutral-100 ml-2">
              {{ deviceStore.version.version === 'unknown' ? deviceStore.version.commit_hash : deviceStore.version.version }}
            </span>
          </span>

          <UModal
            v-model="firmwareUpdateModal"
            title="Firmware Update"
            :dismissible="deviceStore.update.stage === 'idle' || deviceStore.update.stage === 'error'"
          >
            <UButton
              variant="soft"
              color="neutral"
              size="lg"
              label="Update from file"
              class="mt-4 rounded-full"
              icon="i-tabler-upload"
              @click="initFirmwareUpdateFromFile"
            />
            <template #header>
              <div class="w-full flex items-center justify-between">
                <span class="text-title-secondary">Firmware update</span>
                {{ updateStepText }}
              </div>
            </template>

            <template #body>
              <div class="flex flex-col items-center justify-center gap-6">
                <template v-if="deviceStore.update.stage === 'idle'">
                  <p class="text-label">
                    Select a firmware file (.tar archive).
                  </p>

                  <UInput
                    v-model="firmwareFileModel"
                    type="file"
                    icon="i-tabler-file-upload"
                    accept=".tar"
                    size="xl"
                    variant="subtle"
                    @change="onFileUpdate"
                  />

                  <UButton
                    variant="solid"
                    color="primary"
                    size="xl"
                    label="Start Update"
                    class="w-full flex justify-center"
                    :disabled="!firmwareFileModel"
                    @click="startFirmwareUpdateFromFile"
                  />
                </template>
                <template v-else-if="deviceStore.update.stage === 'uploading'">
                  <UIcon
                    name="i-tabler-file-upload"
                    class="h-10 w-10 text-neutral-500 dark:text-neutral-400"
                  />
                  <p class="text-label">
                    Uploading firmware: {{ deviceStore.update.progress }}%
                  </p>
                  <UProgress v-model="deviceStore.update.progress" />
                </template>
                <template v-else-if="deviceStore.update.stage === 'unpacking'">
                  <UIcon
                    name="i-tabler-refresh"
                    class="h-10 w-10 text-neutral-500 dark:text-neutral-400 animate-spin"
                  />
                  <p class="text-label">
                    Unpacking firmware file...
                  </p>
                </template>
                <template v-else-if="deviceStore.update.stage === 'updating'">
                  <UIcon
                    name="i-tabler-eye-exclamation"
                    class="h-10 w-10 text-neutral-500 dark:text-neutral-400"
                  />
                  <p class="text-label">
                    Updating firmware. Pay attention to the front screen.
                  </p>
                </template>
                <template v-else-if="deviceStore.update.stage === 'error'">
                  <p class="text-label text-red-500 dark:text-red-400">
                    Update failed: {{ deviceStore.update.error }}
                  </p>
                </template>
              </div>
            </template>
          </UModal>
        </div>

        <div class="grid grid-cols-2 gap-4 w-full">
          <UModal title="Wi-Fi Settings">
            <SettingsNetworkButton
              :enabled="wifiStore.wifi.state !== 'disabled'"
              icon="i-tabler-wifi"
              :title="wifiStore.wifi.state !== 'disabled' && wifiStore.wifi.ssid ? wifiStore.wifi.ssid : 'Wi-Fi'"
              :subtitle="wifiStore.wifi.state !== 'disabled' ? wifiStore.wifi.ssid ? 'Connected' : 'On' : 'Off'"
              :loading="loading.wifi.state"
              @click="handleWifiClick"
            />
            <template #header>
              <div class="w-full flex items-center justify-between">
                <span class="text-title-secondary">Wi-Fi Settings</span>
                <USwitch
                  v-model="wifiEnabledModal"
                  :loading="loading.wifi.state"
                  @change="toggleWifiState"
                />
              </div>
            </template>

            <template #body>
              <template v-if="wifiStore.wifi.state !== 'disabled'">
                <!-- Connected network -->
                <template v-if="wifiStore.wifi.ssid">
                  <div class="flex items-center justify-between w-full mb-4">
                    <span class="text-label text-neutral-800 dark:text-neutral-200">Connected</span>
                  </div>
                  <div class="flex items-center justify-between p-3 rounded-xl bg-neutral-200 dark:bg-neutral-700 mb-6">
                    <div class="w-full flex items-center gap-3">
                      <UIcon
                        name="i-tabler-wifi"
                        class="h-6 w-6 text-positive-500"
                      />

                      <div class="w-full flex flex-col gap-1">
                        <span class="text-label text-neutral-800 dark:text-neutral-200 ml-0.5">{{ wifiStore.wifi.ssid }}</span>
                      </div>
                    </div>

                    <UButton
                      variant="solid"
                      color="error"
                      size="md"
                      label="Disconnect"
                      :loading="loading.wifi.ssid"
                      @click="disconnectFromWifi"
                    />
                  </div>
                </template>

                <!-- Networks list loader -->
                <template v-if="loading.wifi.networks">
                  <div class="flex flex-col items-center justify-center h-48">
                    <UIcon
                      name="i-tabler-refresh"
                      class="h-10 w-10 text-neutral-500 dark:text-neutral-400 mb-2 animate-spin"
                    />
                    <p class="text-label text-neutral-500 dark:text-neutral-400">
                      Loading available networks...
                    </p>
                  </div>
                </template>

                <template v-else>
                  <!-- Networks list header -->
                  <div
                    v-if="!wifiStore.wifi.ssid"
                    class="flex items-center justify-between w-full mb-4"
                  >
                    <span class="text-label text-neutral-800 dark:text-neutral-200">Select network</span>

                    <UButton
                      variant="soft"
                      color="primary"
                      size="sm"
                      label="Refresh"
                      icon="i-tabler-refresh"
                      @click="getWifiNetworks"
                    />
                  </div>

                  <template v-if="availableWifiNetworks.length > 0">
                    <!-- Networks list -->
                    <div class="grid grid-cols-1 gap-3">
                      <div
                        v-for="network in availableWifiNetworks.filter(n => n.ssid !== wifiStore.wifi.ssid)"
                        :key="network.ssid"
                        class="flex items-center justify-between p-3 rounded-xl transition-colors"
                        :class="networkToConnect && networkToConnect.ssid === network.ssid ? 'bg-neutral-200 dark:bg-neutral-700 cursor-auto' : 'cursor-pointer bg-neutral-100 dark:bg-neutral-800 hover:bg-neutral-200 dark:hover:bg-neutral-700'"
                        @click="networkToConnect = network"
                      >
                        <div class="w-full flex items-center gap-3">
                          <UTooltip
                            :delay-duration="0"
                            :text="`${network.rssi} dBm`"
                          >
                            <UIcon
                              :name="wifiIconByRssi(network.rssi)"
                              class="h-6 w-6 text-neutral-600 dark:text-neutral-400"
                            />
                          </UTooltip>

                          <div class="w-full flex flex-col gap-1">
                            <span class="text-label text-neutral-800 dark:text-neutral-200 ml-0.5">{{ network.ssid }}</span>
                            <UInput
                              v-if="networkToConnect && networkToConnect.ssid === network.ssid"
                              v-model="wifiPasswordModal"
                              type="password"
                              placeholder="Enter password"
                              variant="subtle"
                              size="sm"
                              class="mt-1 max-w-80"
                              :class="failedToConnect ? 'ring-red-500' : ''"
                            />
                          </div>
                        </div>

                        <UButton
                          v-if="networkToConnect && networkToConnect.ssid === network.ssid"
                          variant="solid"
                          color="primary"
                          size="md"
                          icon="i-tabler-arrow-right"
                          :loading="loading.wifi.ssid"
                          @click.stop="connectToWifi(network)"
                        />
                        <UTooltip
                          v-else
                          :delay-duration="0"
                          :text="network.security"
                        >
                          <UIcon
                            :name="network.security === 'Open' ? 'i-tabler-lock-open' : 'i-tabler-lock'"
                            class="h-6 w-6 text-neutral-500 dark:text-neutral-400"
                          />
                        </UTooltip>
                      </div>
                    </div>
                  </template>

                  <!-- Networks list empty -->
                  <template v-else>
                    <!-- Empty response -->
                    <template v-if="!wifiStore.wifi.ssid">
                      <div class="flex flex-col items-center justify-center h-48">
                        <UIcon
                          name="i-tabler-plane-departure"
                          class="h-10 w-10 text-neutral-500 dark:text-neutral-400 mb-2"
                        />
                        <p class="text-label text-neutral-500 dark:text-neutral-400">
                          No available networks found.
                        </p>
                      </div>
                    </template>

                    <!-- Connected, couldn't scan -->
                    <template v-else>
                      <div class="flex flex-col items-center justify-center h-48">
                        <UIcon
                          name="i-tabler-wifi-off"
                          class="h-10 w-10 text-neutral-500 dark:text-neutral-400 mb-2"
                        />
                        <p class="text-label text-neutral-500 dark:text-neutral-400">
                          Disconnect from current network to view available networks.
                        </p>
                      </div>
                    </template>
                  </template>
                </template>
              </template>

              <!-- Wi-Fi disabled -->
              <template v-else>
                <div class="flex flex-col items-center justify-center h-48">
                  <UIcon
                    name="i-tabler-wifi-off"
                    class="h-10 w-10 text-neutral-500 dark:text-neutral-400 mb-2"
                  />
                  <p class="text-label text-neutral-500 dark:text-neutral-400">
                    Enable Wi-Fi to view available networks.
                  </p>
                </div>
              </template>
            </template>
          </UModal>

          <SettingsNetworkButton
            :enabled="bluetooth.enabled"
            icon="i-tabler-bluetooth"
            :title="bluetooth.connected ? bluetooth.deviceName : 'Bluetooth'"
            :subtitle="bluetoothStatusText"
            @click="bluetooth.enabled = !bluetooth.enabled"
          />
        </div>
      </div>
    </UCard>
  </nav>
</template>

<script setup lang="ts">
const deviceStore = useDeviceStore();
const deviceScreenStreamStore = useDeviceScreenStreamStore();
const wifiStore = useWifiStore();

const loading = ref({
  device: {
    version: false
  },
  wifi: {
    state: false,
    networks: false,
    ssid: false
  }
});

const availableWifiNetworks = ref<WifiNetwork[]>([]);

async function getWifiNetworks () {
  if (wifiStore.wifi.ssid) {
    return;
  }
  loading.value.wifi.networks = true;
  availableWifiNetworks.value = await wifiStore.listWifiNetworks();
  loading.value.wifi.networks = false;
}

async function handleWifiClick () {
  if (wifiStore.wifi.state !== 'disabled') {
    await getWifiNetworks();
  }
}

const wifiEnabledModal = ref(false);

async function toggleWifiState () {
  if (wifiStore.wifi.state !== 'disabled') {
    loading.value.wifi.state = true;
    await wifiStore.disableWifi();
  } else {
    loading.value.wifi.state = true;
    await wifiStore.enableWifi();
    availableWifiNetworks.value = [];
    networkToConnect.value = null;
    failedToConnect.value = false;
    await getWifiNetworks();
  }
  wifiEnabledModal.value = wifiStore.wifi.state !== 'disabled';
  loading.value.wifi.state = false;
}

const wifiPasswordModal = ref('');
const networkToConnect = ref<WifiNetwork | null>(null);
const failedToConnect = ref(false);

async function connectToWifi (network: WifiNetwork) {
  failedToConnect.value = false;
  loading.value.wifi.ssid = true;
  try {
    await wifiStore.connectToWifiNetwork({
      ssid: network.ssid,
      password: wifiPasswordModal.value,
      security: network.security,
      ip_config: {
        ip_method: 'dhcp',
        ip_type: 'ipv4'
      }
    });
    await wifiStore.updateWifiState();
    availableWifiNetworks.value = [];
    loading.value.wifi.ssid = false;
    networkToConnect.value = null;
    wifiPasswordModal.value = '';
    failedToConnect.value = false;
  } catch (error) {
    loading.value.wifi.ssid = false;
    console.error('Failed to connect to Wi-Fi:', error);
    failedToConnect.value = true;
  } finally {
    wifiPasswordModal.value = '';
  }
}

async function disconnectFromWifi () {
  loading.value.wifi.ssid = true;
  try {
    await wifiStore.disconnectFromWifiNetwork();
  } catch (error) {
    console.error('Failed to disconnect from Wi-Fi:', error);
  } finally {
    await wifiStore.updateWifiState();
    loading.value.wifi.ssid = false;
    await getWifiNetworks();
  }
}

function wifiIconByRssi (rssi: number): string {
  if (rssi < 60) {
    return 'i-tabler-wifi';
  }
  if (rssi < 70) {
    return 'i-tabler-wifi-2';
  }
  if (rssi < 80) {
    return 'i-tabler-wifi-1';
  }
  return 'i-tabler-wifi-0';
}

const bluetooth = ref({
  deviceName: 'Bluetooth Device Name',
  enabled: false,
  connected: false
});
const bluetoothStatusText = computed(() => {
  if (bluetooth.value.connected) {
    return 'Connected';
  } else if (bluetooth.value.enabled) {
    return 'On';
  } else {
    return 'Off';
  }
});

const firmwareFileModel = ref('');

async function onFileUpdate (event: Event) {
  deviceStore.update.firmwareFile = null;
  const file = (event.target as HTMLInputElement)?.files?.[0];
  if (!file) {
    return;
  }
  deviceStore.update.firmwareFile = file;
}

const firmwareUpdateModal = ref(false);

function initFirmwareUpdateFromFile () {
  deviceStore.update.stage = 'idle' as UpdateStage;
  deviceStore.update.progress = 0;
  deviceStore.update.error = '';
  deviceStore.update.firmwareFile = null;
  firmwareFileModel.value = '';
}

async function startFirmwareUpdateFromFile () {
  try {
    await deviceScreenStreamStore.stopScreenStream();
    await deviceStore.uploadFirmware();
    if (deviceStore.update.stage !== 'error') {
      deviceStore.update.stage = 'updating';
    }
  } catch (error) {
    console.error('Firmware update failed:', error);
    deviceStore.update.stage = 'error';
    deviceStore.update.error = error instanceof Error ? error.message : 'Unknown error';
  }
}

const updatePollingInterval = ref<NodeJS.Timeout | null>(null);
watch(() => deviceStore.update.stage, newStage => {
  if (newStage !== 'updating') {
    return;
  }
  if (updatePollingInterval.value) {
    clearInterval(updatePollingInterval.value);
  }
  updatePollingInterval.value = setInterval(() => {
    if (loading.value.device.version) {
      return;
    }
    loading.value.device.version = true;
    deviceStore.updateDeviceVersion(true) // throw the error to avoid exiting the polling
      .then(() => {
        clearInterval(updatePollingInterval.value!);
        firmwareUpdateModal.value = false;
        deviceStore.update.stage = 'idle';
        deviceStore.update.progress = 0;
        window.location.reload();
      })
      .catch(() => {
        // ignore the error
      });
    loading.value.device.version = false;
  }, 1000);
});

const updateStepText = computed(() => {
  switch (deviceStore.update.stage) {
    case 'idle':
      return 'Step 1/4';
    case 'uploading':
      return 'Step 2/4';
    case 'unpacking':
      return 'Step 3/4';
    case 'updating':
      return 'Step 4/4';
    default:
      return '';
  }
});

onMounted(async () => {
  loading.value.device.version = true;
  await deviceStore.updateDeviceVersion();
  loading.value.device.version = false;

  loading.value.wifi.state = true;
  await wifiStore.updateWifiState();
  wifiEnabledModal.value = wifiStore.wifi.state !== 'disabled';
  loading.value.wifi.state = false;
});
</script>
