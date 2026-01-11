<template>
  <SectionCard data-id="settings-section-primary">
    <div class="grid sm:grid-cols-2 divide-y sm:divide-x sm:divide-y-0 divide-neutral-300/30 dark:divide-neutral-700/30 p-2">
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
              {{ brightnessNumber }}%
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

onMounted(async () => {
  await deviceStore.getDeviceStatus();
  await deviceStore.getApiVersion();
  await refreshAudioVolume();
  await refreshDisplayBrightness();
});
</script>
