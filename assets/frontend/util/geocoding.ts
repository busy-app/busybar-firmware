import type { TimezoneInfo } from '@busy-app/busy-lib';

const LOCATIONS_PATH = '/weather/v1/locations';
const REQUEST_TIMEOUT_MS = 8000;
const MAX_QUERY_LENGTH = 100;
const DISPLAY_LANGUAGE = 'en';

export const MIN_QUERY_LENGTH = 2;

const QUERY_LANGUAGES: [script: RegExp, language: string][] = [
  [/[ҐґЄєІіЇї]/, 'uk'],
  [/\p{Script=Cyrillic}/u, 'ru'],
  [/\p{Script=Hiragana}|\p{Script=Katakana}/u, 'ja'],
  [/\p{Script=Han}/u, 'zh'],
  [/\p{Script=Hangul}/u, 'ko'],
  [/\p{Script=Arabic}/u, 'ar'],
  [/\p{Script=Hebrew}/u, 'he'],
  [/\p{Script=Devanagari}/u, 'hi'],
  [/\p{Script=Bengali}/u, 'bn'],
  [/\p{Script=Greek}/u, 'el'],
  [/\p{Script=Thai}/u, 'th'],
  [/\p{Script=Georgian}/u, 'ka'],
  [/\p{Script=Armenian}/u, 'hy']
];

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

export function cityLabel (city: CitySuggestion) {
  const state = city.state === city.name ? undefined : city.state;

  return [city.name, state, city.country].filter(Boolean).join(', ');
}

function queryLanguage (query: string) {
  return QUERY_LANGUAGES.find(([script]) => script.test(query))?.[1] ?? DISPLAY_LANGUAGE;
}

export async function searchCities (query: string, signal?: AbortSignal) {
  const response = await $fetch<LocationsResponse>(LOCATIONS_PATH, {
    baseURL: useRuntimeConfig().public.appsApiUrl,
    query: {
      query: query.slice(0, MAX_QUERY_LENGTH),
      language: queryLanguage(query)
    },
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
