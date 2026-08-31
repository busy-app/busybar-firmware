const PHOTON_URL = 'https://photon.komoot.io';
const REQUEST_TIMEOUT_MS = 8000;

export const MIN_QUERY_LENGTH = 2;
export const MAX_QUERY_LENGTH = 100;

export interface CitySuggestion {
  id: string;
  name: string;
  state?: string;
  country?: string;
  lat: number;
  lng: number;
  timezone?: string;
}

interface LocationSearchResult {
  name: string;
  country: string | null;
  country_code: string | null;
  admin1: string | null;
  latitude: number;
  longitude: number;
  timezone: string;
}

interface LocationsResponse {
  results: LocationSearchResult[];
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
    name?: string;
    city?: string;
    state?: string;
    country?: string;
  };
}

interface PhotonResponse {
  features: PhotonFeature[];
}

function getApiUrl () {
  return useRuntimeConfig().public.apiUrl;
}

function toSuggestion (result: LocationSearchResult) {
  return {
    id: `${result.latitude},${result.longitude}`,
    name: result.name,
    state: result.admin1 ?? undefined,
    country: result.country ?? undefined,
    lat: result.latitude,
    lng: result.longitude,
    timezone: result.timezone
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
  const q = query.slice(0, MAX_QUERY_LENGTH);

  const response = await $fetch<LocationsResponse>('/weather/v1/locations', {
    baseURL: getApiUrl(),
    query: { query: q },
    timeout: REQUEST_TIMEOUT_MS,
    signal
  });

  return response.results.map(toSuggestion);
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
