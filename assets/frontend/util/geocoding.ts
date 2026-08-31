export interface CitySuggestion {
  id: string;
  name: string;
  state?: string;
  country?: string;
  lat: number;
  lng: number;
}

interface PhotonProperties {
  osm_id?: number;
  osm_type?: string;
  name?: string;
  city?: string;
  state?: string;
  country?: string;
}

interface PhotonFeature {
  properties: PhotonProperties;
  geometry: { coordinates: [number, number] };
}

interface PhotonResponse {
  features: PhotonFeature[];
}

const PHOTON_URL = 'https://photon.komoot.io';
const REQUEST_TIMEOUT_MS = 8000;
const SEARCH_LIMIT = 8;
const LANG = 'en';

export function cityLabel (city: CitySuggestion) {
  const state = city.state === city.name ? undefined : city.state;

  return [city.name, state, city.country].filter(Boolean).join(', ');
}

function suggestionKey (city: CitySuggestion) {
  return [city.name, city.state, city.country].join('|');
}

function toSuggestion (feature: PhotonFeature, name: string) {
  const { properties, geometry } = feature;
  const [lng, lat] = geometry.coordinates;

  return {
    id: `${properties.osm_type ?? 'X'}${properties.osm_id ?? name}`,
    name,
    state: properties.state,
    country: properties.country,
    lat,
    lng
  };
}

export async function searchCities (query: string, signal?: AbortSignal) {
  const response = await $fetch<PhotonResponse>('/api', {
    baseURL: PHOTON_URL,
    query: {
      q: query,
      limit: SEARCH_LIMIT,
      lang: LANG,
      layer: 'city'
    },
    timeout: REQUEST_TIMEOUT_MS,
    signal
  });

  const seen = new Set<string>();

  return response.features.reduce<CitySuggestion[]>((cities, feature) => {
    const name = feature.properties.name;

    if (!name) {
      return cities;
    }

    const city = toSuggestion(feature, name);
    const key = suggestionKey(city);

    if (seen.has(key)) {
      return cities;
    }

    seen.add(key);
    cities.push(city);
    return cities;
  }, []);
}

export async function reverseGeocode (lat: number, lng: number, signal?: AbortSignal) {
  const response = await $fetch<PhotonResponse>('/reverse', {
    baseURL: PHOTON_URL,
    query: { lat, lon: lng, limit: 1, lang: LANG },
    timeout: REQUEST_TIMEOUT_MS,
    signal
  });

  const feature = response.features[0];

  if (!feature) {
    return null;
  }

  const name = feature.properties.city ?? feature.properties.name;

  if (!name) {
    return null;
  }

  return { ...toSuggestion(feature, name), lat, lng };
}
