import type { TimezoneInfo } from '@busy-app/busy-lib';

const LOCATIONS_PATH = '/weather/v1/locations';
const REQUEST_TIMEOUT_MS = 8000;
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

interface ApiLocation {
  name: string;
  country?: string;
  country_code?: string;
  admin1?: string;
  latitude: number;
  longitude: number;
  timezone?: string;
}

interface LocationsResponse {
  results: ApiLocation[];
}

function toSuggestion (location: ApiLocation): CitySuggestion {
  return {
    id: `${location.latitude},${location.longitude}`,
    name: location.name,
    state: location.admin1,
    country: location.country,
    lat: location.latitude,
    lng: location.longitude,
    timezone: location.timezone
  };
}

// Only the commented-out reverse lookup needs it.
// export function formatCoordinates (lat: number, lng: number) {
//   return `${lat.toFixed(4)}, ${lng.toFixed(4)}`;
// }

export function cityLabel (city: CitySuggestion) {
  const state = city.state === city.name ? undefined : city.state;

  return [city.name, state, city.country].filter(Boolean).join(', ');
}

export async function searchCities (query: string, signal?: AbortSignal) {
  const response = await $fetch<LocationsResponse>(LOCATIONS_PATH, {
    baseURL: useRuntimeConfig().public.apiUrl,
    query: { query: query.slice(0, MAX_QUERY_LENGTH) },
    timeout: REQUEST_TIMEOUT_MS,
    retry: false,
    signal
  });

  return (response.results ?? []).map(toSuggestion);
}

function zoneCity (timezone?: string) {
  return timezone?.split('/').pop()?.replaceAll('_', ' ');
}

function zoneOffset (timezone?: string) {
  if (!timezone) {
    return undefined;
  }

  try {
    const parts = new Intl.DateTimeFormat('en-US', { timeZone: timezone, timeZoneName: 'longOffset' }).formatToParts();
    return parts.find(part => part.type === 'timeZoneName')?.value.replace('GMT', '');
  } catch {
    return undefined;
  }
}

export async function resolveByTimezone (zone: TimezoneInfo, signal?: AbortSignal) {
  if (zone.name.length < MIN_QUERY_LENGTH) {
    return undefined;
  }

  const cities = await searchCities(zone.name, signal);

  return cities.find(city => zoneCity(city.timezone) === zone.name)
    ?? cities.find(city => zoneOffset(city.timezone) === zone.offset);
}

// Naming a place by coordinates, which "Share my location" needs, has nowhere to go: the apps API
// has no reverse lookup, and its /weather/v1/forecast wants the device key. Kept, with the button
// hidden in TabAppsSettingsGeolocation, until one of the two is open to the web UI.
// const PHOTON_URL = 'https://photon.komoot.io';
//
// interface PhotonResponse {
//   features: {
//     properties: {
//       name?: string;
//       city?: string;
//       state?: string;
//       country?: string;
//     };
//   }[];
// }
//
// export async function resolveByCoords (lat: number, lng: number, signal?: AbortSignal) {
//   const response = await $fetch<PhotonResponse>('/reverse', {
//     baseURL: PHOTON_URL,
//     query: { lat, lon: lng, limit: 1, lang: 'en' },
//     timeout: REQUEST_TIMEOUT_MS,
//     signal
//   });
//
//   const properties = response.features[0]?.properties;
//   const name = properties?.city ?? properties?.name;
//
//   if (!name) {
//     return undefined;
//   }
//
//   return {
//     id: `${lat},${lng}`,
//     name,
//     state: properties.state,
//     country: properties.country,
//     lat,
//     lng
//   } satisfies CitySuggestion;
// }
