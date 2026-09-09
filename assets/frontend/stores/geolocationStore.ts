import { defineStore } from 'pinia';

export type GeolocationValue
  = | { mode: 'auto'; name?: string }
    | { mode: 'fixed'; lat: number; lng: number; name: string };

export function isValidLocation (value: GeolocationValue) {
  if (value.mode === 'auto') {
    return true;
  }

  return value.name !== undefined
    && value.lat >= -90 && value.lat <= 90
    && value.lng >= -180 && value.lng <= 180;
}

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

  async function resolveByIp () {
    loading.value.resolve = true;

    try {
      const city = await resolveByGeoIp();

      if (city) {
        lastResolvedCity.value = city;
      }

      return city;
    } catch (error) {
      console.warn('Geoip lookup failed', error);
      return null;
    } finally {
      loading.value.resolve = false;
    }
  }

  async function setFixedFromCoords (lat: number, lng: number) {
    let city: CitySuggestion = {
      id: `${lat},${lng}`,
      name: formatCoordinates(lat, lng),
      lat,
      lng
    };

    try {
      city = await resolveByCoords(lat, lng) ?? city;
    } catch (error) {
      console.warn('Reverse geocoding failed, labelling by coordinates', error);
    }

    lastResolvedCity.value = city;
    return setFixedFromCity(city);
  }

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
    setFixedFromCoords,
    setLocation,
    setFixedFromCity,
    refreshAutoLocation
  };
}, {
  persist: {
    key: 'geolocationStore',
    storage: piniaPluginPersistedstate.localStorage(),
    pick: ['location', 'autoDetect']
  }
});
