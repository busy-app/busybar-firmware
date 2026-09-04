import type { City } from "./cities.ts";

export type CurrentWeather = {
  /** Temperature, °C. */
  temp: number;
  /** WMO weather code. */
  code: number;
};

type OpenMeteoResponse = {
  current?: {
    temperature_2m?: number;
    weather_code?: number;
  };
  hourly?: {
    time?: string[];
    temperature_2m?: number[];
  };
};

/** Number of forecast hours in the graph window. */
export const GRAPH_HOURS = 24;

/** Fetches current weather for a city via open-meteo (no API key). */
export async function fetchCurrent(city: City): Promise<CurrentWeather> {
  const url =
    "https://api.open-meteo.com/v1/forecast" +
    `?latitude=${city.lat}` +
    `&longitude=${city.lon}` +
    "&current=temperature_2m,weather_code" +
    `&timezone=${encodeURIComponent(city.tzName)}`;

  const res = await fetch(url);
  if (!res.ok) {
    throw new Error(`open-meteo: HTTP ${res.status}`);
  }

  const data = (await res.json()) as OpenMeteoResponse;
  const temp = data.current?.temperature_2m;
  const code = data.current?.weather_code;
  if (typeof temp !== "number" || typeof code !== "number") {
    throw new Error("open-meteo: no current in response");
  }

  return { temp, code };
}

/** Hourly temperatures in °C, GRAPH_HOURS ahead of the current hour. */
export async function fetchHourly(city: City): Promise<number[]> {
  const url =
    "https://api.open-meteo.com/v1/forecast" +
    `?latitude=${city.lat}` +
    `&longitude=${city.lon}` +
    "&hourly=temperature_2m" +
    "&forecast_days=2" +
    `&timezone=${encodeURIComponent(city.tzName)}`;

  const res = await fetch(url);
  if (!res.ok) {
    throw new Error(`open-meteo: HTTP ${res.status}`);
  }

  const data = (await res.json()) as OpenMeteoResponse;
  const times = data.hourly?.time;
  const temps = data.hourly?.temperature_2m;
  if (!times || !temps || times.length === 0) {
    throw new Error("open-meteo: no hourly in response");
  }

  // The first time not earlier than now.
  const nowMs = Date.now();
  let start = times.findIndex((t) => new Date(t).getTime() >= nowMs);
  if (start < 0) start = 0;

  return temps.slice(start, start + GRAPH_HOURS);
}
