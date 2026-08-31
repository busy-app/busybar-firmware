import { defineStore } from 'pinia';

export type GeolocationValue
  = | { mode: 'auto'; name?: string }
    | { mode: 'fixed'; lat: number; lng: number; name: string };

const MIN_QUERY_LENGTH = 2;

export function isValidLocation (value: GeolocationValue): boolean {
  if (value.mode === 'auto') {
    return true;
  }

  return value.name !== undefined
    && value.lat >= -90 && value.lat <= 90
    && value.lng >= -180 && value.lng <= 180;
}

// TODO: geoip lives behind api.dev.busy.app, which requires a client certificate — the browser
// cannot present one, so this call has to go through the bar. Blocked on a firmware endpoint.
const MOCK_GEOIP_CITY: CitySuggestion = {
  id: 'N60571493',
  name: 'Belgrade',
  state: 'Central Serbia',
  country: 'Serbia',
  lat: 44.8178131,
  lng: 20.4568974
};

export const useGeolocationStore = defineStore('geolocation', () => {
  const location = ref<GeolocationValue>({ mode: 'auto' });
  const autoDetect = ref(true);
  const loading = ref({
    search: false,
    share: false,
    resolve: false
  });
  const suggestions = ref<CitySuggestion[]>([]);
  const lastResolvedCity = ref<CitySuggestion | null>(null);

  const label = computed(() => location.value.name ?? 'Auto');

  /** Only the newest keystroke's results may land — earlier ones are aborted. */
  let searchController: AbortController | null = null;

  async function findCities (query: string) {
    const trimmed = query.trim();

    searchController?.abort();

    if (trimmed.length < MIN_QUERY_LENGTH) {
      suggestions.value = [];
      loading.value.search = false;
      return [];
    }

    searchController = new AbortController();
    const controller = searchController;
    loading.value.search = true;

    try {
      suggestions.value = await searchCities(trimmed, controller.signal);
      return suggestions.value;
    } catch (error) {
      if (!controller.signal.aborted) {
        console.warn('City search failed', error);
        suggestions.value = [];
      }
      return [];
    } finally {
      if (searchController === controller) {
        searchController = null;
        loading.value.search = false;
      }
    }
  }

  /** MOCK: geoip lookup — the backend resolves by the IP of the incoming request. */
  async function resolveByIp () {
    loading.value.resolve = true;

    try {
      await new Promise(resolve => setTimeout(resolve, 400));
      lastResolvedCity.value = MOCK_GEOIP_CITY;
      return MOCK_GEOIP_CITY;
    } finally {
      loading.value.resolve = false;
    }
  }

  async function findCityByCoords (lat: number, lng: number) {
    const city = await reverseGeocode(lat, lng);

    if (city) {
      lastResolvedCity.value = city;
    }

    return city;
  }

  /** TODO: persist to the device once the geolocation endpoint lands in busy-lib. */
  async function setLocation (value: GeolocationValue) {
    if (!isValidLocation(value)) {
      return false;
    }

    location.value = value;
    return true;
  }

  function setFixedFromCity (city: CitySuggestion) {
    lastResolvedCity.value = city;

    return setLocation({
      mode: 'fixed',
      lat: city.lat,
      lng: city.lng,
      name: cityLabel(city)
    });
  }

  async function refreshAutoLocation () {
    if (!autoDetect.value) {
      return;
    }

    const city = await resolveByIp();
    await setLocation({ mode: 'auto', ...(city ? { name: cityLabel(city) } : {}) });
  }

  async function setAutoDetect (enabled: boolean) {
    autoDetect.value = enabled;

    if (enabled) {
      await setLocation({ mode: 'auto' });
      await refreshAutoLocation();
      return;
    }

    if (location.value.mode === 'auto' && lastResolvedCity.value) {
      await setFixedFromCity(lastResolvedCity.value);
    }
  }

  return {
    location,
    autoDetect,
    setAutoDetect,
    loading,
    suggestions,
    lastResolvedCity,
    label,
    findCities,
    resolveByIp,
    findCityByCoords,
    setLocation,
    setFixedFromCity,
    refreshAutoLocation
  };
}, {
  // TODO: drop once the device stores the location itself.
  persist: {
    key: 'geolocationStore',
    storage: piniaPluginPersistedstate.localStorage(),
    pick: ['location', 'autoDetect']
  }
});
