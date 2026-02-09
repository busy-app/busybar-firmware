<template>
  <UCard
    data-id="settings-section-sound-brightness"
    :ui="{
      root: 'ring-1 ring-glass rounded-3xl bg-elevated/50 divide-none',
      header: 'p-4 sm:p-6',
      body: 'p-4 sm:p-6',
      footer: 'p-4 sm:p-6'
    }"
  >
    <div class="grid sm:grid-cols-2 divide-y sm:divide-x sm:divide-y-0 divide-neutral-300/30 dark:divide-neutral-700/30">
      <div class="flex flex-col gap-8 pb-6 sm:pb-0 sm:pr-6">
        <div class="flex justify-between items-center">
          <UIcon
            data-id="mute-icon"
            :name="mute.isMuted ? 'i-ri-volume-mute-line' : 'i-ri-volume-up-line'"
            class="size-7"
          />

          <UButton
            data-id="mute-button"
            label="Mute"
            icon="i-ri-volume-mute-line"
            variant="subtle"
            size="sm"
            :color="mute.isMuted ? 'primary' : 'neutral'"
            class="rounded-full"
            @click="mute.isMuted ? unmute() : setVolumeToMute()"
          />
        </div>

        <div class="flex flex-col gap-2.5">
          <div class="flex justify-between items-center">
            <div class="text-lg font-medium">Sound</div>
            <div
              data-id="volume-percentage"
              class="text-muted"
            >
              {{ nextVolumeNumber || volumeNumber }}%
            </div>
          </div>

          <USlider
            v-model="nextVolumeNumber"
            data-id="volume-slider"
            :step="5"
            :default-value="volumeNumber"
            :ui="{
              root: '',
              track: 'h-[14px]',
              range: `${mute.isMuted ? 'bg-neutral' : 'bg-primary'} rounded-r-none`,
              thumb: `${mute.isMuted ? 'bg-neutral' : 'bg-primary'} ring-4 ring-white size-[6px] focus-visible:outline-none`
            }"
            @change="onChangeAudioSlider"
          />
        </div>
      </div>

      <div class="flex flex-col gap-8 pt-6 sm:pt-0 sm:pl-6">
        <div class="flex justify-between items-center">
          <UIcon
            data-id="brightness-auto-icon"
            :name="isBrightnessAuto ? 'i-busy-brightness-auto' : 'i-ri-sun-line'"
            class="size-7"
          />

          <UButton
            data-id="brightness-auto-button"
            label="Auto"
            icon="i-ri-input-method-line"
            variant="subtle"
            size="sm"
            :color="isBrightnessAuto ? 'primary' : 'neutral'"
            class="rounded-full"
            @click="isBrightnessAuto ? disableAutoBrightness() : setBrightnessToAuto()"
          />
        </div>

        <div class="flex flex-col gap-2.5">
          <div class="flex justify-between items-center">
            <div class="text-lg font-medium">Brightness</div>
            <div
              v-if="!isBrightnessAuto"
              data-id="brightness-percentage"
              class="text-muted"
            >
              {{ nextBrightnessNumber || brightnessNumber }}%
            </div>
            <div
              v-else
              data-id="brightness-auto"
              class="text-muted"
            >
              Automatic
            </div>
          </div>

          <USlider
            v-model="nextBrightnessNumber"
            data-id="brightness-slider"
            :step="5"
            :default-value="brightnessNumber"
            :ui="{
              root: '',
              track: 'h-[14px]',
              range: `${isBrightnessAuto ? 'bg-neutral' : 'bg-primary'} rounded-r-none`,
              thumb: `${isBrightnessAuto ? 'bg-neutral' : 'bg-primary'} ring-4 ring-white size-[6px] focus-visible:outline-none`
            }"
            @change="onChangeBrightnessSlider"
          />
        </div>
      </div>
    </div>
  </UCard>

  <SectionCard
    data-id="settings-section-timezone"
    icon="i-bi-timezone"
    title="Timezone"
  >
    <template #actions>
      <USelect
        v-if="tzListStore.timezoneOptions?.length"
        v-model="deviceStore.timezone"
        :items="tzListStore.timezoneOptions.map(tz => ({ label: `UTC${tz.offset}, ${tz.name}`, value: tz.name }))"
        variant="soft"
        size="xl"
        class="h-12 min-w-56"
        :ui="{
          base: 'text-base rounded-xl',
          label: 'text-base',
          item: 'text-base'
        }"
        @update:model-value="deviceStore.setTimezone(deviceStore.timezone!)"
      />
    </template>
  </SectionCard>

  <SectionCard
    data-id="settings-section-matter"
    icon="i-bi-smart-home"
    title="Smart home"
    :subtitle="deviceStore.matterCommissioning.fabricCount > 0 ? `${deviceStore.matterCommissioning.fabricCount} connections` : ''"
  >
    <template #actions>
      <UButton
        label="Pair device"
        icon="i-bi-plus"
        @click="deviceStore.requestMatterLink()"
      />

      <ModalGeneric
        v-model:open="deviceStore.matterLink.showModal"
        data-id="modal-matter-link"
        title="Pair device with Matter"
        :icon="colorMode.value === 'dark' ? 'i-bi-matter-bubble-dark' : 'i-bi-matter-bubble'"
        :dismissable="false"
        no-actions
        @after:leave="onMatterLinkModalClose"
      >
        <template #icon>
          <UIcon
            :name="colorMode.value === 'dark' ? 'i-bi-matter-bubble-dark' : 'i-bi-matter-bubble'"
            class="size-10 mr-1"
          />
        </template>
        <template #body>
          <div class="flex flex-col items-center gap-4">
            <div>Scan the QR code</div>
            <div
              data-id="matter-link-qr-code"
              class="w-[184px] bg-white rounded-xl mb-6"
              v-html="deviceStore.matterLink.qrCode"
            />

            <div>Or enter the code in your smart home app</div>
            <CopyButton
              :text="deviceStore.matterLink.manualCode"
              variant="subtle"
              color="neutral"
              size="xl"
              class="w-full rounded-xl p-4 pr-5 text-2xl ring-black/4 dark:ring-white/4"
            />

            <div class="w-full flex justify-end mt-6 text-muted text-sm">
              Expires in&nbsp;<CountDown :ms="deviceStore.matterLink.expiresInMs" />
            </div>
          </div>
        </template>
      </ModalGeneric>
    </template>
  </SectionCard>

  <SectionCard
    data-id="settings-section-primary"
    icon="i-bi-info-fill"
    title="About device"
  >
    <div class="flex flex-col gap-4">
      <div class="flex items-center gap-2.5">
        <UIcon
          name="i-ri-information-fill"
          class="size-6"
        />
        <div class="font-medium">Device</div>
      </div>
      <div class="grid sm:grid-cols-2 gap-y-3 gap-x-1">
        <div
          v-for="[property, value] in Object.entries({
            'Firmware': fwVersionPolifilled,
            'Build date': system?.build_date,
            'Uptime': system?.uptime,
            'API version': deviceStore.apiVersion?.api_semver
          })"
          :key="property"
          class="flex"
        >
          <div class="w-[120px] text-muted">{{ property }}</div>
          <div class="max-w-[140px] md:max-w-[180px] text-ellipsis overflow-hidden">{{ value }}</div>
        </div>
      </div>
    </div>

    <div class="flex flex-col gap-4">
      <div class="flex items-center gap-2.5">
        <UIcon
          name="i-ri-cpu-fill"
          class="size-6"
        />
        <div class="font-medium">Hardware</div>
      </div>
      <div class="grid sm:grid-cols-2 gap-y-3 gap-x-1">
        <div
          v-for="[property, value] in Object.entries({
            'Main display resolution': '72×16 (LED)',
            'Central MCU': 'STM32U5M',
            'Main display refresh rate': '60 Hz',
            'Wireless MCU': 'Silicon Labs SiWG917',
            'RAM size': '2.5 MB'
          })"
          :key="property"
          class="flex"
        >
          <div class="w-[120px] text-muted">{{ property }}</div>
          <div class="max-w-[140px] md:max-w-[180px] text-ellipsis overflow-hidden">{{ value }}</div>
        </div>
      </div>
    </div>
  </SectionCard>
</template>

<script setup lang="ts">
const deviceStore = useDeviceStore();
const tzListStore = useTzListStore();
const colorMode = useColorMode();

const system = computed(() => deviceStore.deviceStatus?.system);
const fwVersionPolifilled = computed(() => system.value?.version === 'unknown' ? `${system.value.branch} ${system.value.commit_hash}` : system.value?.version);

const loading = ref({
  audio: false,
  brightness: false
});

async function refreshAudioVolume () {
  loading.value.audio = true;
  deviceStore.audio = await deviceStore.getAudioVolume();
  loading.value.audio = false;
}

const nextVolumeNumber = ref<number | undefined>(undefined);
const volumeNumber = computed(() => {
  if (mute.value.isMuted) {
    return mute.value.volumeBeforeMute;
  } else {
    return deviceStore.audio?.volume === undefined ? 50 : deviceStore.audio?.volume;
  }
});
const mute = ref({
  isMuted: false,
  volumeBeforeMute: 50
});

function unmute () {
  nextVolumeNumber.value = mute.value.volumeBeforeMute;
  setAudioVolume();
}

function onChangeAudioSlider () {
  setAudioVolume();
}

async function setAudioVolume () {
  if (loading.value.audio || nextVolumeNumber.value === undefined) {
    return;
  }
  mute.value.isMuted = false;

  loading.value.audio = true;
  const v = nextVolumeNumber.value;

  await deviceStore.setAudioVolume(v);
  deviceStore.audio = { volume: v };

  setTimeout(() => {
    loading.value.audio = false;
  }, 250);
}

async function setVolumeToMute () {
  loading.value.audio = true;
  mute.value.volumeBeforeMute = volumeNumber.value;
  await deviceStore.setAudioVolume(0);
  mute.value.isMuted = true;
  loading.value.audio = false;
}

async function refreshDisplayBrightness () {
  loading.value.brightness = true;
  deviceStore.displayBrightness = await deviceStore.getDisplayBrightness();
  loading.value.brightness = false;
}

const nextBrightnessNumber = ref<number | undefined>(undefined);
const brightnessNumber = computed(() => isNaN(Number(deviceStore.displayBrightness?.front)) ? 50 : Number(deviceStore.displayBrightness?.front));
const isBrightnessAuto = computed(() => deviceStore.displayBrightness?.front === 'auto');

function disableAutoBrightness () {
  nextBrightnessNumber.value = 50;
  setDisplayBrightness();
}

function onChangeBrightnessSlider () {
  setDisplayBrightness();
}

async function setDisplayBrightness () {
  if (loading.value.brightness || nextBrightnessNumber.value === undefined) {
    return;
  }

  loading.value.brightness = true;
  const b = nextBrightnessNumber.value;

  await deviceStore.setDisplayBrightness({
    front: b,
    back: b
  });
  deviceStore.displayBrightness = {
    front: b,
    back: b
  };

  setTimeout(() => {
    loading.value.brightness = false;
  }, 250);
}

async function setBrightnessToAuto () {
  loading.value.brightness = true;
  await deviceStore.setDisplayBrightness({
    front: 'auto',
    back: 'auto'
  });
  deviceStore.displayBrightness = {
    front: 'auto',
    back: 'auto'
  };
  nextBrightnessNumber.value = 50;
  loading.value.brightness = false;
}

function onMatterLinkModalClose () {
  if (deviceStore.matterLink.timeout) {
    clearTimeout(deviceStore.matterLink.timeout);
  }
  deviceStore.matterLink.qrCode = '';
  deviceStore.matterLink.manualCode = '';
  deviceStore.matterLink.availableUntil = null;
  deviceStore.matterLink.timeout = null;
  deviceStore.matterLink.expiresInMs = 0;
}

async function init () {
  await deviceStore.getDeviceStatus();
  await deviceStore.getApiVersion();
  await refreshAudioVolume();
  await refreshDisplayBrightness();
  if (!tzListStore.timezoneOptions || tzListStore.timezoneOptions.length === 0) {
    await tzListStore.fetchTimezoneOptions();
  }
  await deviceStore.fetchTimezone();
}

onMounted(async () => {
  await init();
  window.addEventListener('device-reconnected', init);
});
onBeforeUnmount(() => window.removeEventListener('device-reconnected', init));
</script>
