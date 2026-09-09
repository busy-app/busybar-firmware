<template>
  <div class="flex flex-col gap-1 rounded-group">
    <div
      class="bg-accented/25 dark:bg-[var(--ui-surface-card)] p-4 flex flex-col gap-4"
      :class="geolocationStore.autoDetect ? 'rounded-[12px]' : 'rounded-[12px_12px_4px_4px]'"
    >
      <div class="flex items-center gap-2 w-full">
        <div class="flex-1 min-w-0 truncate">
          Auto-detect location
        </div>
        <USwitch
          v-model="autoDetect"
          data-id="apps-section-geolocation-auto-detect-switch"
          :loading="geolocationStore.loading.resolve"
        />
      </div>

      <USeparator class="-mx-4 w-auto" />

      <div class="flex items-center gap-2 w-full">
        <div class="flex flex-1 flex-col gap-2 items-start min-w-0">
          <div class="text-muted w-full truncate">
            Location
          </div>
          <div class="flex items-center gap-3 w-full min-w-0">
            <UIcon
              name="i-bi-location"
              class="size-6 shrink-0"
            />
            <div
              data-id="apps-section-geolocation-label"
              class="font-medium text-toned truncate"
            >
              {{ geolocationStore.label }}
            </div>
          </div>
        </div>

        <UButton
          data-id="apps-section-geolocation-share-button"
          label="Share my location"
          variant="outline"
          color="neutral"
          class="min-w-20 shrink-0"
          :loading="geolocationStore.loading.share"
          :disabled="geolocationStore.autoDetect"
          @click="shareLocation"
        />
      </div>
    </div>

    <UInputMenu
      v-if="!geolocationStore.autoDetect"
      ref="cityMenu"
      v-model="selectedCity"
      v-model:search-term="searchTerm"
      data-id="apps-section-geolocation-city-search"
      :items="cityItems"
      ignore-filter
      :loading="geolocationStore.loading.search"
      :disabled="geolocationStore.autoDetect"
      placeholder="Search city"
      icon="i-bi-search"
      variant="none"
      color="neutral"
      size="xl"
      class="w-full"
      :ui="{
        root: 'rounded-[4px_4px_12px_12px]!',
        base: 'rounded-[inherit] py-3 text-base bg-accented/25 dark:bg-[var(--ui-surface-card)] disabled:opacity-60',
        leadingIcon: 'size-6',
        trailing: 'hidden',
        content: 'shadow-[0_10px_15px_-3px_rgba(0,0,0,0.1),0_4px_6px_-2px_rgba(0,0,0,0.05)]'
      }"
      @update:model-value="onCitySelect"
    />
  </div>
</template>

<script setup lang="ts">
type CityItem = CitySuggestion & { label: string };

const SEARCH_DEBOUNCE_MS = 300;

let searchTimeout: number | null = null;

const geolocationStore = useGeolocationStore();
const toast = useToast();

const cityMenu = useTemplateRef('cityMenu');
const searchTerm = ref('');
const selectedCity = ref<CityItem | undefined>(undefined);

const cityItems = computed<CityItem[]>(() => geolocationStore.suggestions.map(city => ({ ...city, label: cityLabel(city) })));

const autoDetect = computed({
  get: () => geolocationStore.autoDetect,
  set: value => {
    resetSearch();
    geolocationStore.setAutoDetect(value);
  }
});

async function onCitySelect (city: CityItem | undefined) {
  if (!city) {
    return;
  }

  await geolocationStore.setFixedFromCity(city);

  await nextTick();
  resetSearch();
  blurSearch();
}

function resetSearch () {
  selectedCity.value = undefined;
  searchTerm.value = '';
  geolocationStore.suggestions = [];
}

function blurSearch () {
  cityMenu.value?.inputRef?.$el?.blur();
}

async function shareLocation () {
  if (!navigator.geolocation) {
    toast.add({
      title: 'Location unavailable',
      description: 'This browser does not support sharing your location.',
      icon: 'i-bi-alert',
      color: 'error'
    });
    return;
  }

  geolocationStore.loading.share = true;

  try {
    const position = await new Promise<GeolocationPosition>((resolve, reject) => {
      navigator.geolocation.getCurrentPosition(resolve, reject, {
        enableHighAccuracy: false,
        timeout: 10000
      });
    });

    const { latitude, longitude } = position.coords;

    resetSearch();
    await geolocationStore.setFixedFromCoords(latitude, longitude);
  } catch {
    toast.add({
      title: 'Couldn\'t share location',
      description: 'Allow location access and try again.',
      icon: 'i-bi-alert',
      color: 'error'
    });
  } finally {
    geolocationStore.loading.share = false;
  }
}

watch(searchTerm, query => {
  if (searchTimeout) {
    clearTimeout(searchTimeout);
  }

  searchTimeout = window.setTimeout(() => {
    searchTimeout = null;
    geolocationStore.findCities(query);
  }, SEARCH_DEBOUNCE_MS);
});

onMounted(() => {
  if (geolocationStore.autoDetect && !geolocationStore.location.name) {
    geolocationStore.refreshAutoLocation();
  }
});

onBeforeUnmount(() => {
  if (searchTimeout) {
    clearTimeout(searchTimeout);
  }
});
</script>
