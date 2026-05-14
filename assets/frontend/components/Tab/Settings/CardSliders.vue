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
    <div class="grid sm:grid-cols-2 divide-y sm:divide-x sm:divide-y-0 divide-accented/30">
      <div class="flex flex-col gap-8 pb-6 sm:pb-0 sm:pr-6">
        <div class="flex justify-between items-center">
          <UIcon
            data-id="mute-icon"
            :name="mute.isMuted ? 'i-bi-sound-off' : 'i-bi-sound'"
            class="size-7"
          />

          <UButton
            data-id="mute-button"
            label="Mute"
            icon="i-bi-sound-off"
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
              {{ displayedVolumeNumber }}%
            </div>
          </div>

          <USlider
            v-model="nextVolumeNumber"
            data-id="volume-slider"
            :step="5"
            :default-value="volumeNumber"
            :ui="{
              root: '',
              track: 'h-[14px] bg-accented/50 dark:bg-accented',
              range: `${mute.isMuted ? 'bg-neutral' : 'bg-primary-500'} rounded-r-none`,
              thumb: `${mute.isMuted ? 'bg-neutral' : 'bg-primary-500'} ring-4 ring-white size-[6px] focus-visible:outline-none`
            }"
            @change="onChangeAudioSlider"
          />
        </div>
      </div>

      <div class="flex flex-col gap-8 pt-6 sm:pt-0 sm:pl-6">
        <div class="flex justify-between items-center">
          <UIcon
            data-id="brightness-auto-icon"
            :name="isBrightnessAuto ? 'i-bi-brightness-auto-control' : 'i-bi-brightness'"
            class="size-7"
          />

          <UButton
            data-id="brightness-auto-button"
            label="Auto"
            icon="i-bi-brightness-auto-control"
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
              {{ nextBrightnessNumber ?? brightnessNumber }}%
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
              track: 'h-[14px] bg-accented/50 dark:bg-accented',
              range: `${isBrightnessAuto ? 'bg-neutral' : 'bg-primary-500'} rounded-r-none`,
              thumb: `${isBrightnessAuto ? 'bg-neutral' : 'bg-primary-500'} ring-4 ring-white size-[6px] focus-visible:outline-none`
            }"
            @change="onChangeBrightnessSlider"
          />
        </div>
      </div>
    </div>
  </UCard>
</template>

<script setup lang="ts">
const audioStore = useAudioStore();
const brightnessStore = useBrightnessStore();
const deviceStore = useDeviceStore();
const configStore = useConfigStore();

const loading = ref({
  audio: false,
  brightness: false
});

async function refreshAudioVolume () {
  loading.value.audio = true;
  await audioStore.fetchAudioVolume();
  loading.value.audio = false;
}

const mute = ref({
  isMuted: false,
  volumeBeforeMute: 50
});

const nextVolumeNumber = ref<number | undefined>(undefined);
const volumeNumber = computed(() => audioStore.audio?.volume ?? 50);
const displayedVolumeNumber = computed(() => {
  if (mute.value.isMuted) {
    return mute.value.volumeBeforeMute;
  }

  return nextVolumeNumber.value ?? volumeNumber.value;
});

watch(volumeNumber, (newValue, oldValue) => {
  if (mute.value.isMuted) {
    if (newValue === 0) {
      return;
    }

    mute.value.isMuted = false;
  }

  if (newValue > 0) {
    mute.value.volumeBeforeMute = newValue;
  }

  if (newValue !== oldValue || nextVolumeNumber.value === undefined) {
    nextVolumeNumber.value = newValue;
  }
}, { immediate: true });

async function refreshSlidersData () {
  if (!deviceStore.refreshInterval) {
    return;
  }

  await Promise.all([
    refreshAudioVolume(),
    refreshDisplayBrightness()
  ]);
}

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
  }, Number(configStore.get('sliderDebounceDelay')));
}

async function setVolumeToMute () {
  loading.value.audio = true;
  mute.value.volumeBeforeMute = volumeNumber.value;
  nextVolumeNumber.value = mute.value.volumeBeforeMute;
  mute.value.isMuted = true;
  await audioStore.setAudioVolume(0);
  loading.value.audio = false;
}

async function refreshDisplayBrightness () {
  loading.value.brightness = true;
  await brightnessStore.fetchDisplayBrightness();
  loading.value.brightness = false;
}

const nextBrightnessNumber = ref<number | undefined>(undefined);
const brightnessNumber = computed(() => isNaN(Number(brightnessStore.displayBrightness?.value)) ? 50 : Number(brightnessStore.displayBrightness?.value));
const isBrightnessAuto = computed(() => brightnessStore.displayBrightness?.value === 'auto');

watch(() => brightnessStore.displayBrightness?.value, newValue => {
  if (newValue !== 'auto') {
    nextBrightnessNumber.value = Number(newValue ?? 50);
  }
}, { immediate: true });

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
    value: b
  });
  brightnessStore.displayBrightness = {
    value: b
  };

  setTimeout(() => {
    loading.value.brightness = false;
  }, Number(configStore.get('sliderDebounceDelay')));
}

async function setBrightnessToAuto () {
  loading.value.brightness = true;
  await brightnessStore.setDisplayBrightness({
    value: 'auto'
  });
  brightnessStore.displayBrightness = {
    value: 'auto'
  };
  nextBrightnessNumber.value = 50;
  loading.value.brightness = false;
}

const refreshInterval = ref<NodeJS.Timeout | null>(null);

async function init () {
  await refreshAudioVolume();
  await refreshDisplayBrightness();

  if (refreshInterval.value) {
    clearInterval(refreshInterval.value);
  }
  refreshInterval.value = setInterval(() => {
    refreshSlidersData();
  }, Number(configStore.get('httpPollingInterval')) * 6);
}

onMounted(async () => {
  await init();
  window.addEventListener('device-reconnected', init);
});
onBeforeUnmount(() => {
  window.removeEventListener('device-reconnected', init);
  if (refreshInterval.value) {
    clearInterval(refreshInterval.value);
  }
});
</script>
