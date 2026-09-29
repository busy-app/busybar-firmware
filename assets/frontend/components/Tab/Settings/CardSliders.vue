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

const sending = ref({
  audio: false,
  brightness: false
});

// One request in flight per control; only the latest requested value is sent next.
function createSerialSender<T> (key: keyof typeof sending.value, send: (value: T) => Promise<unknown>) {
  let queued: { value: T } | null = null;

  async function drain () {
    sending.value[key] = true;
    let lastSent: { value: T } | null = null;

    try {
      while (queued) {
        const { value } = queued;
        queued = null;

        if (lastSent && lastSent.value === value) {
          continue;
        }

        lastSent = { value };
        await send(value);
        await new Promise(resolve => setTimeout(resolve, Number(configStore.get('sliderDebounceDelay'))));
      }
    } finally {
      sending.value[key] = false;
    }
  }

  return (value: T) => {
    queued = { value };

    if (!sending.value[key]) {
      drain();
    }
  };
}

const sendAudioVolume = createSerialSender<number>('audio', volume => audioStore.setAudioVolume(volume));
const sendDisplayBrightness = createSerialSender<number | 'auto'>('brightness', value => brightnessStore.setDisplayBrightness({ value }));

async function refreshAudioVolume () {
  if (sending.value.audio) {
    return;
  }

  await audioStore.fetchAudioVolume();
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

// Only external changes matter here; our own in-flight writes must not override local mute state.
watch(volumeNumber, (newValue, oldValue) => {
  if (sending.value.audio) {
    return;
  }

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

async function onChangeAudioSlider () {
  await nextTick();
  setAudioVolume();
}

function setAudioVolume () {
  if (nextVolumeNumber.value === undefined) {
    return;
  }

  mute.value.isMuted = false;
  sendAudioVolume(nextVolumeNumber.value);
}

function setVolumeToMute () {
  mute.value.volumeBeforeMute = nextVolumeNumber.value ?? volumeNumber.value;
  nextVolumeNumber.value = mute.value.volumeBeforeMute;
  mute.value.isMuted = true;
  sendAudioVolume(0);
}

async function refreshDisplayBrightness () {
  if (sending.value.brightness) {
    return;
  }

  await brightnessStore.fetchDisplayBrightness();
}

const nextBrightnessNumber = ref<number | undefined>(undefined);
const brightnessNumber = computed(() => isNaN(Number(brightnessStore.displayBrightness?.value)) ? 50 : Number(brightnessStore.displayBrightness?.value));
const isBrightnessAuto = computed(() => brightnessStore.displayBrightness?.value === 'auto');

watch(() => brightnessStore.displayBrightness?.value, newValue => {
  if (sending.value.brightness) {
    return;
  }

  if (newValue !== 'auto') {
    nextBrightnessNumber.value = Number(newValue ?? 50);
  }
}, { immediate: true });

function disableAutoBrightness () {
  nextBrightnessNumber.value = 50;
  setDisplayBrightness();
}

async function onChangeBrightnessSlider () {
  await nextTick();
  setDisplayBrightness();
}

function setDisplayBrightness () {
  if (nextBrightnessNumber.value === undefined) {
    return;
  }

  sendDisplayBrightness(nextBrightnessNumber.value);
}

function setBrightnessToAuto () {
  nextBrightnessNumber.value = 50;
  sendDisplayBrightness('auto');
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
