<template>
  <div class="flex flex-col gap-3">
    <div class="flex items-center gap-2">
      <div class="min-w-0 flex-1 truncate text-muted">
        Auto-detect location
      </div>
      <USwitch
        :model-value="!manual"
        @update:model-value="setAutoDetect"
      />
    </div>

    <div class="flex min-w-0 items-center gap-3">
      <UIcon
        name="i-bi-location"
        class="size-6 shrink-0"
      />
      <div class="truncate font-medium text-toned">
        {{ manual && value.mode === 'auto' ? 'Choose a city' : value.name }}
      </div>
    </div>

    <UInputMenu
      v-if="manual"
      v-model="selectedCity"
      v-model:search-term="searchTerm"
      :items="cityItems"
      ignore-filter
      :loading="searching"
      placeholder="Search city"
      icon="i-bi-search"
      color="neutral"
      class="w-full"
      :ui="{ trailing: 'hidden' }"
      @update:model-value="selectCity"
    />
  </div>
</template>

<script setup lang="ts">
import type { AppSettingsGeolocationValue } from '@/stores/appsStore';

type CityItem = CitySuggestion & { label: string };

const SEARCH_DEBOUNCE_MS = 300;
const NAME_MAX_LENGTH = 128;
const AUTO_LOCATION_NAME = 'Auto';

const props = defineProps<{
  defaultValue: AppSettingsGeolocationValue;
}>();

const value = defineModel<AppSettingsGeolocationValue>({ required: true });

const manual = ref(value.value.mode === 'fixed');
const searchTerm = ref('');
const selectedCity = ref<CityItem>();
const suggestions = ref<CitySuggestion[]>([]);
const searching = ref(false);

const cityItems = computed<CityItem[]>(() => suggestions.value.map(city => ({ ...city, label: cityLabel(city) })));

let searchTimeout: ReturnType<typeof setTimeout> | undefined;
let searchController: AbortController | undefined;

function setAutoDetect (enabled: boolean) {
  manual.value = !enabled;

  if (enabled) {
    value.value = {
      mode: 'auto',
      name: props.defaultValue.mode === 'auto' ? props.defaultValue.name : AUTO_LOCATION_NAME
    };
  }
}

function selectCity (city: CityItem | undefined) {
  if (!city) {
    return;
  }

  value.value = {
    mode: 'fixed',
    name: city.label.slice(0, NAME_MAX_LENGTH),
    lat: city.lat,
    lon: city.lng
  };

  selectedCity.value = undefined;
  searchTerm.value = '';
  suggestions.value = [];
}

async function findCities (query: string) {
  searchController?.abort();

  if (query.trim().length < MIN_QUERY_LENGTH) {
    suggestions.value = [];
    return;
  }

  searchController = new AbortController();
  searching.value = true;

  try {
    suggestions.value = await searchCities(query.trim(), searchController.signal);
  } catch {
    suggestions.value = [];
  } finally {
    searching.value = false;
  }
}

watch(searchTerm, query => {
  clearTimeout(searchTimeout);
  searchTimeout = setTimeout(() => findCities(query), SEARCH_DEBOUNCE_MS);
});

onBeforeUnmount(() => {
  clearTimeout(searchTimeout);
  searchController?.abort();
});
</script>
