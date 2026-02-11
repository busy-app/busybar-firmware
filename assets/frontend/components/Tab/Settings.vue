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
            size="sm"
            :variant="mute.isMuted ? 'solid' : 'subtle'"
            color="neutral"
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
            size="sm"
            :variant="isBrightnessAuto ? 'solid' : 'subtle'"
            color="neutral"
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
        v-model="timezoneStore.timezone"
        :items="tzListStore.timezoneOptions.map(tz => ({ label: `UTC${tz.offset}, ${tz.name}`, value: tz.name }))"
        variant="soft"
        size="xl"
        class="h-12 min-w-56"
        :ui="{
          base: 'text-base rounded-xl',
          label: 'text-base',
          item: 'text-base'
        }"
        @update:model-value="timezoneStore.setTimezone(timezoneStore.timezone!)"
      />
    </template>
  </SectionCard>

  <SectionCard
    data-id="settings-section-matter"
    icon="i-bi-smart-home"
    title="Smart home"
    :subtitle="matterStore.matterCommissioning.fabricCount > 0 ? `${matterStore.matterCommissioning.fabricCount} connections` : ''"
  >
    <template #actions>
      <UButton
        v-if="matterStore.matterCommissioning.fabricCount > 0"
        label="Forget all pairings"
        variant="outline"
        color="neutral"
        @click="showMatterDeleteModal = true"
      />
      <UButton
        label="Pair device"
        icon="i-bi-plus"
        @click="matterStore.requestMatterLink()"
      />

      <ModalGeneric
        v-model:open="matterStore.matterLink.showModal"
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
              v-html="matterStore.matterLink.qrCode"
            />

            <div>Or enter the code in your smart home app</div>
            <CopyButton
              :text="matterStore.matterLink.manualCode"
              variant="subtle"
              color="neutral"
              size="xl"
              class="w-full rounded-xl p-4 pr-5 text-2xl ring-black/4 dark:ring-white/4"
            />

            <div class="w-full flex justify-end mt-6 text-muted text-sm">
              Expires in&nbsp;<CountDown :ms="matterStore.matterLink.expiresInMs" />
            </div>
          </div>
        </template>
      </ModalGeneric>

      <UModal
        v-model:open="showMatterDeleteModal"
        data-id="modal-matter-delete"
        title="Forget all pairings?"
        description="Your BUSY Bar will be removed from your smart home automations. The device will restart after removal."
        :ui="{
          content: 'max-w-[480px] divide-none bg-neutral-100/90 dark:bg-neutral-800/75 backdrop-blur-[5px] ring-1 ring-glass',
          description: 'hidden',
          header: 'hidden',
          body: 'p-0 sm:p-0 overflow-visible',
          close: 'hidden',
          overlay: 'bg-neutral-900/20 dark:bg-neutral-900/80'
        }"
      >
        <template #body>
          <div
            class="flex flex-col gap-6 p-6 bg-no-repeat"
            :style="`background-image: url(${matterDeleteImage}); background-size: 100%; background-position: center calc(50% - 25px)`"
          >
            <div class="text-left text-xl font-medium">Forget all pairings?</div>

            <div class="h-36" />
            <div class="text-center">Your BUSY Bar will be removed from your smart home automations. The device will restart after removal.</div>

            <div class="flex justify-end gap-4">
              <UButton
                color="neutral"
                variant="ghost"
                label="Cancel"
                class="min-w-20 justify-center"
                @click="showMatterDeleteModal = false"
              />
              <UButton
                label="Forget all pairings"
                variant="soft"
                color="error"
                class="min-w-20 justify-center"
                @click="deleteMatterPairings()"
              />
            </div>
          </div>
        </template>
      </UModal>

      <UModal
        v-model:open="showRebootingModal"
        data-id="modal-rebooting"
        title="Restarting BUSY Bar..."
        description="Restarting BUSY Bar..."
        :ui="{
          content: 'max-w-[360px] divide-none bg-neutral-100/90 dark:bg-neutral-800/75 backdrop-blur-[5px] ring-1 ring-glass',
          description: 'hidden',
          header: 'hidden',
          body: 'p-5 sm:p-5',
          close: 'hidden',
          overlay: 'bg-neutral-900/20 dark:bg-neutral-900/80'
        }"
      >
        <template #body>
          <div class="flex items-center gap-4 py-4">
            <CircularProgress
              v-model="indeterminateProgressModel"
              size="32px"
              :thickness="0.25"
              class="animate-spin"
            />
            <div>Restarting BUSY Bar...</div>
          </div>
        </template>
      </UModal>
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
import matterDeleteImage from '@/assets/images/matter-delete.png';

const deviceStore = useDeviceStore();
const audioStore = useAudioStore();
const brightnessStore = useBrightnessStore();
const timezoneStore = useTimezoneStore();
const tzListStore = useTzListStore();
const matterStore = useMatterStore();
const colorMode = useColorMode();

const system = computed(() => deviceStore.deviceStatus?.system);
const fwVersionPolifilled = computed(() => system.value?.version === 'unknown' ? `${system.value.branch} ${system.value.commit_hash}` : system.value?.version);

const loading = ref({
  audio: false,
  brightness: false
});

async function refreshAudioVolume () {
  loading.value.audio = true;
  await audioStore.fetchAudioVolume();
  loading.value.audio = false;
}

const nextVolumeNumber = ref<number | undefined>(undefined);
const volumeNumber = computed(() => {
  if (mute.value.isMuted) {
    return mute.value.volumeBeforeMute;
  } else {
    return audioStore.audio?.volume === undefined ? 50 : audioStore.audio?.volume;
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

  await audioStore.setAudioVolume(v);
  audioStore.audio = { volume: v };

  setTimeout(() => {
    loading.value.audio = false;
  }, 250);
}

async function setVolumeToMute () {
  loading.value.audio = true;
  mute.value.volumeBeforeMute = volumeNumber.value;
  await audioStore.setAudioVolume(0);
  mute.value.isMuted = true;
  loading.value.audio = false;
}

async function refreshDisplayBrightness () {
  loading.value.brightness = true;
  await brightnessStore.fetchDisplayBrightness();
  loading.value.brightness = false;
}

const nextBrightnessNumber = ref<number | undefined>(undefined);
const brightnessNumber = computed(() => isNaN(Number(brightnessStore.displayBrightness?.front)) ? 50 : Number(brightnessStore.displayBrightness?.front));
const isBrightnessAuto = computed(() => brightnessStore.displayBrightness?.front === 'auto');

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

  await brightnessStore.setDisplayBrightness({
    front: b,
    back: b
  });
  brightnessStore.displayBrightness = {
    front: b,
    back: b
  };

  setTimeout(() => {
    loading.value.brightness = false;
  }, 250);
}

async function setBrightnessToAuto () {
  loading.value.brightness = true;
  await brightnessStore.setDisplayBrightness({
    front: 'auto',
    back: 'auto'
  });
  brightnessStore.displayBrightness = {
    front: 'auto',
    back: 'auto'
  };
  nextBrightnessNumber.value = 50;
  loading.value.brightness = false;
}

function onMatterLinkModalClose () {
  if (matterStore.matterLink.timeout) {
    clearTimeout(matterStore.matterLink.timeout);
  }
  matterStore.matterLink.qrCode = '';
  matterStore.matterLink.manualCode = '';
  matterStore.matterLink.availableUntil = null;
  matterStore.matterLink.timeout = null;
  matterStore.matterLink.expiresInMs = 0;

  return matterStore.fetchMatterCommissioning();
}

const showMatterDeleteModal = ref(false);
const showRebootingModal = ref(false);
const indeterminateProgressModel = ref(60);

async function deleteMatterPairings () {
  await matterStore.deleteAllPairings();
  showMatterDeleteModal.value = false;
  showRebootingModal.value = true;
}

async function init () {
  showRebootingModal.value = false;

  await deviceStore.fetchDeviceStatus();
  await deviceStore.fetchApiVersion();
  await refreshAudioVolume();
  await refreshDisplayBrightness();
  if (!tzListStore.timezoneOptions || tzListStore.timezoneOptions.length === 0) {
    await tzListStore.fetchTimezoneOptions();
  }
  await timezoneStore.fetchTimezone();
  await matterStore.fetchMatterCommissioning();
}

onMounted(async () => {
  await init();
  window.addEventListener('device-reconnected', init);
});
onBeforeUnmount(() => window.removeEventListener('device-reconnected', init));
</script>
