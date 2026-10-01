<template>
  <div class="flex flex-col gap-1 rounded-group">
    <div
      class="flex flex-col gap-4 bg-accented/25 p-4 dark:bg-[var(--ui-surface-card)]"
      :class="manual ? 'rounded-[12px_12px_4px_4px]' : 'rounded-[12px]'"
    >
      <div class="flex w-full items-center gap-2">
        <div class="min-w-0 flex-1 truncate">
          Auto-detect location
        </div>
        <USwitch
          :model-value="!manual"
          @update:model-value="setAutoDetect"
        />
      </div>

      <USeparator class="-mx-4 w-auto" />

      <div class="flex w-full items-center gap-2">
        <div class="flex min-w-0 flex-1 flex-col items-start gap-2">
          <div class="w-full truncate text-muted">
            {{ label }}
          </div>
          <div class="flex w-full min-w-0 items-center gap-3">
            <UIcon
              name="i-bi-location"
              class="size-6 shrink-0"
            />
            <div class="truncate font-medium text-toned">
              {{ value.name }}
            </div>
          </div>
        </div>

        <UButton
          v-if="canShareLocation"
          label="Share my location"
          variant="outline"
          color="neutral"
          class="min-w-20 shrink-0"
          :loading="sharing"
          :disabled="!manual"
          @click="shareLocation"
        />
      </div>
    </div>

    <UInputMenu
      v-if="manual"
      ref="cityMenu"
      v-model="selectedCity"
      v-model:search-term="searchTerm"
      :items="cityItems"
      ignore-filter
      placeholder="Search city"
      icon="i-bi-search"
      variant="none"
      color="neutral"
      size="xl"
      class="w-full"
      :ui="{
        root: 'rounded-[4px_4px_12px_12px]!',
        base: 'rounded-[inherit] py-3 text-base bg-accented/25 dark:bg-[var(--ui-surface-card)]',
        leadingIcon: 'size-6',
        trailing: 'hidden',
        content: 'shadow-[0_10px_15px_-3px_rgba(0,0,0,0.1),0_4px_6px_-2px_rgba(0,0,0,0.05)]'
      }"
      @update:model-value="selectCity"
    >
      <template #empty="{ searchTerm: term }">
        <div class="flex items-center gap-2">
          <UIcon
            v-if="searching"
            name="i-busy-loader"
            class="size-4 shrink-0 animate-spin"
          />
          <span>{{ emptyLabel(term) }}</span>
        </div>
      </template>
    </UInputMenu>
  </div>
</template>

<script setup lang="ts">
import type { AppSettingsGeolocationValue } from '@/stores/appsStore';

type CityItem = CitySuggestion & { label: string };

const SEARCH_DEBOUNCE_MS = 300;
const NAME_MAX_LENGTH = 128;
const AUTO_LOCATION_NAME = 'Auto';

const canShareLocation = window.isSecureContext && !!navigator.geolocation;

let searchTimeout: ReturnType<typeof setTimeout> | undefined;
let searchController: AbortController | undefined;

const props = defineProps<{
  label: string;
  defaultValue: AppSettingsGeolocationValue;
}>();

const value = defineModel<AppSettingsGeolocationValue>({ required: true });

const toast = useToast();

const cityMenu = useTemplateRef('cityMenu');
const manual = ref(value.value.mode === 'fixed');
const searchTerm = ref('');
const selectedCity = ref<CityItem>();
const suggestions = ref<CitySuggestion[]>([]);
const searching = ref(false);
const sharing = ref(false);

const cityItems = computed<CityItem[]>(() => suggestions.value.map(city => ({ ...city, label: cityLabel(city) })));

function setAutoDetect (enabled: boolean) {
  manual.value = !enabled;

  if (enabled) {
    value.value = {
      mode: 'auto',
      name: props.defaultValue.mode === 'auto' ? props.defaultValue.name : AUTO_LOCATION_NAME
    };
  }
}

function setFixedLocation (name: string, lat: number, lon: number) {
  value.value = {
    mode: 'fixed',
    name: truncateUtf8(name, NAME_MAX_LENGTH),
    lat,
    lon
  };
}

async function selectCity (city: CityItem | undefined) {
  if (!city) {
    return;
  }

  setFixedLocation(city.label, city.lat, city.lng);

  await nextTick();

  selectedCity.value = undefined;
  searchTerm.value = '';
  suggestions.value = [];
  cityMenu.value?.inputRef?.$el?.blur();
}

async function shareLocation () {
  sharing.value = true;

  try {
    const position = await new Promise<GeolocationPosition>((resolve, reject) => {
      navigator.geolocation.getCurrentPosition(resolve, reject, {
        enableHighAccuracy: false,
        timeout: 10000
      });
    });

    const { latitude, longitude } = position.coords;
    const city = await resolveByCoords(latitude, longitude);

    setFixedLocation(city ? cityLabel(city) : formatCoordinates(latitude, longitude), latitude, longitude);
  } catch {
    toast.add({
      title: 'Couldn\'t share location',
      description: 'Allow location access and try again.',
      icon: 'i-bi-alert',
      color: 'error'
    });
  } finally {
    sharing.value = false;
  }
}

function emptyLabel (term: string | undefined) {
  if (searching.value) {
    return 'Searching…';
  }

  return (term ?? '').trim().length < MIN_QUERY_LENGTH
    ? `Type at least ${MIN_QUERY_LENGTH} characters`
    : 'No cities found';
}

async function findCities (query: string) {
  searchController?.abort();

  const trimmed = query.trim();

  if (trimmed.length < MIN_QUERY_LENGTH) {
    suggestions.value = [];
    searching.value = false;
    return;
  }

  const controller = new AbortController();
  searchController = controller;

  try {
    suggestions.value = await searchCities(trimmed, controller.signal);
  } catch {
    if (!controller.signal.aborted) {
      suggestions.value = [];
    }
  } finally {
    if (!controller.signal.aborted) {
      searching.value = false;
    }
  }
}

watch(searchTerm, query => {
  clearTimeout(searchTimeout);
  searching.value = query.trim().length >= MIN_QUERY_LENGTH;
  searchTimeout = setTimeout(() => findCities(query), SEARCH_DEBOUNCE_MS);
});

onBeforeUnmount(() => {
  clearTimeout(searchTimeout);
  searchController?.abort();
});
</script>
