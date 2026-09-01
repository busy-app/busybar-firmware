const PHOTON_URL = 'https://photon.komoot.io';
const REQUEST_TIMEOUT_MS = 8000;
const SEARCH_LIMIT = 8;
const MAX_QUERY_LENGTH = 100;

export const MIN_QUERY_LENGTH = 2;

export interface CitySuggestion {
  id: string;
  name: string;
  state?: string;
  country?: string;
  lat: number;
  lng: number;
  timezone?: string;
}

interface ResolvedLocation {
  latitude: number;
  longitude: number;
  name: string | null;
  timezone: string;
  utc_offset_seconds: number;
  source: 'request' | 'geoip';
}

interface ForecastResponse {
  location: ResolvedLocation;
}

interface PhotonFeature {
  properties: {
    osm_id?: number;
    osm_type?: string;
    name?: string;
    city?: string;
    state?: string;
    country?: string;
  };
  geometry: { coordinates: [number, number] };
}

interface PhotonResponse {
  features: PhotonFeature[];
}

function getApiUrl () {
  return useRuntimeConfig().public.apiUrl;
}

function toSuggestion (feature: PhotonFeature) {
  const { properties, geometry } = feature;

  if (!properties.name) {
    return null;
  }

  const [lng, lat] = geometry.coordinates;

  return {
    id: `${properties.osm_type ?? 'X'}${properties.osm_id ?? properties.name}`,
    name: properties.name,
    state: properties.state,
    country: properties.country,
    lat,
    lng
  };
}

export function formatCoordinates (lat: number, lng: number) {
  return `${lat.toFixed(4)}, ${lng.toFixed(4)}`;
}

export function cityLabel (city: CitySuggestion) {
  const state = city.state === city.name ? undefined : city.state;

  return [city.name, state, city.country].filter(Boolean).join(', ');
}

export async function searchCities (query: string, signal?: AbortSignal) {
  const response = await $fetch<PhotonResponse>('/api', {
    baseURL: PHOTON_URL,
    query: {
      q: query.slice(0, MAX_QUERY_LENGTH),
      limit: SEARCH_LIMIT,
      lang: 'en',
      layer: 'city'
    },
    timeout: REQUEST_TIMEOUT_MS,
    retry: false,
    signal
  });

  const seen = new Set<string>();

  return response.features.reduce<CitySuggestion[]>((cities, feature) => {
    const city = toSuggestion(feature);

    if (!city) {
      return cities;
    }

    const key = [city.name, city.state, city.country].join('|');

    if (seen.has(key)) {
      return cities;
    }

    seen.add(key);
    cities.push(city);
    return cities;
  }, []);
}

export async function resolveByCoords (lat: number, lng: number, signal?: AbortSignal) {
  const response = await $fetch<PhotonResponse>('/reverse', {
    baseURL: PHOTON_URL,
    query: { lat, lon: lng, limit: 1, lang: 'en' },
    timeout: REQUEST_TIMEOUT_MS,
    signal
  });

  const properties = response.features[0]?.properties;

  const name = properties?.city ?? properties?.name;

  if (!name) {
    return null;
  }

  return {
    id: `${lat},${lng}`,
    name,
    state: properties.state,
    country: properties.country,
    lat,
    lng
  };
}

export async function resolveByGeoIp (signal?: AbortSignal) {
  const response = await $fetch<ForecastResponse>('/weather/v1/forecast', {
    baseURL: getApiUrl(),
    timeout: REQUEST_TIMEOUT_MS,
    retry: false,
    signal
  });

  const { location } = response;

  if (location.source !== 'geoip') {
    return null;
  }

  return {
    id: `${location.latitude},${location.longitude}`,
    name: location.name ?? formatCoordinates(location.latitude, location.longitude),
    lat: location.latitude,
    lng: location.longitude,
    timezone: location.timezone
  };
}
