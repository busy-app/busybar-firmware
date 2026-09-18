import { device } from "@shared/device";
import {
  formatDay,
  formatHM,
  formatMonth,
  formatWeekday,
} from "@shared/datetime";
import { fetchCurrent, fetchHourly, type CurrentWeather } from "./api.ts";
import { CITIES, wmoToAnim, wmoToIcon } from "./cities.ts";
import { GRAPH_H, renderGraphPng } from "./graph.ts";
import manifest from "./appmeta/manifest.json";

// Weather (FRONT, 72×16): condition icon on the left, temperature on the right. The bottom line alternates between "10:50, 8 Jul" and "10:50, Sat".
// Resources ship with the package; the graph is drawn and uploaded at runtime.

// The manifest id is what the firmware addresses elements and assets by.
const APP = manifest.id;

// Fixed for now.
const CITY = CITIES[0];

const REFRESH_MS = 10 * 60 * 1000; // refresh weather every 10 minutes
const TOGGLE_MS = 2000; // swap the bottom line every 2 seconds
const SCREEN_MS = 6000; // swap the screen (weather ↔ graph) every 6 seconds

const GRAPH_FILE = "graph.png";

const ICON = { x: 0, y: 0 }; // 7×7 icon on the left
const TEXT_X = 20; // text to the right of the icon

let weather: CurrentWeather | null = null;
let hourly: number[] = []; // hourly temperatures for the graph
let showDate = true; // true → line A (day + month), false → line B (weekday)
let screen: "weather" | "graph" = "weather"; // current screen

/** The bottom line, per the current alternation state. */
function subLine(now: Date): string {
  const hm = formatHM(now);
  return showDate
    ? `${hm}, ${formatDay(now)} ${formatMonth(now)}`
    : `${hm}, ${formatWeekday(now)}`;
}

async function refresh(): Promise<void> {
  try {
    weather = await fetchCurrent(CITY);
  } catch (err) {
    console.error("weather: weather request failed", err);
  }
  try {
    hourly = await fetchHourly(CITY);
    // Overwritten on every refresh.
    const png = await renderGraphPng(hourly);
    await device.AssetsUpload({
      application_name: APP,
      file: GRAPH_FILE,
      data: png,
    });
  } catch (err) {
    console.error("weather: forecast/graph failed", err);
  }
}

/**
 * Draws the weather animation and temperature. The animation loops on its own; ticks must not redraw it.
 */
async function drawWeather(): Promise<void> {
  if (!weather) return;

  const anim = `${wmoToAnim(weather.code)}.anim`;
  const rounded = Math.round(weather.temp);
  const temp = `${rounded >= 0 ? "+" : "-"}${rounded}°C`; // "+23°C"

  await device.DisplayDraw({
    application_name: APP,
    priority: 50,
    elements: [
      {
        type: "animation",
        id: "wicon",
        display: "front",
        path: anim,
        x: ICON.x,
        y: ICON.y,
        loop: true,
        await_previous_end: false,
        opacity: 100,
      },
      {
        type: "text",
        id: "temp",
        display: "front",
        x: TEXT_X,
        y: 0,
        text: temp,
        font: "bold",
        color: "#FFFFFFFF",
      },
    ],
  });
}

/** Redraws only the bottom line, by reusing its id. */
async function drawSub(): Promise<void> {
  if (!weather) return;

  await device.DisplayDraw({
    application_name: APP,
    priority: 50,
    elements: [
      {
        type: "text",
        id: "sub",
        display: "front",
        x: TEXT_X,
        y: 9,
        text: subLine(new Date()),
        font: "small",
        color: "#AAAAAAFF",
      },
    ],
  });
}

/** Graph screen: date and time on top, the hourly temperature PNG below. */
async function drawGraph(): Promise<void> {
  if (!weather) return;

  const now = new Date();
  const line = `${formatDay(now)} ${formatMonth(now)} ${formatHM(now)}`; // "23 May 22:00"
  const icon = `${wmoToIcon(weather.code)}.png`;
  const rounded = Math.round(weather.temp);
  const temp = `${rounded >= 0 ? "+" : "-"}${rounded}`;

  await device.DisplayDraw({
    application_name: APP,
    priority: 50,
    elements: [
      {
        type: "text",
        id: "sub",
        display: "front",
        x: 0,
        y: 0,
        text: line,
        font: "small",
        color: "#AAAAAAFF",
      },
      {
        type: "image",
        id: "wicon",
        display: "front",
        x: 51,
        y: 0,
        path: icon,
        opacity: 100,
      },
      {
        type: "text",
        id: "temp",
        display: "front",
        x: 60,
        y: -1,
        text: temp,
        font: "small",
        color: "#FFFFFFFF",
      },
      {
        type: "image",
        id: "graph",
        display: "front",
        path: GRAPH_FILE,
        x: 0,
        y: 16 - GRAPH_H,
        opacity: 100,
      },
    ],
  });
}

/** Draws the current screen from scratch. */
async function drawScreen(): Promise<void> {
  await device.DisplayClear({ application_name: APP });
  if (screen === "weather") {
    await drawWeather();
    await drawSub();
  } else {
    await drawGraph();
  }
}

export default function run(): void {
  void (async () => {
    await refresh();
    await drawScreen();
  })().catch((err) => console.error("weather: startup failed", err));

  // Weather screen only.
  setInterval(() => {
    if (screen !== "weather") return;
    showDate = !showDate;
    void drawSub();
  }, TOGGLE_MS);

  // Advance the clock on whichever screen is showing.
  setInterval(() => {
    if (screen === "weather") void drawSub();
    else void drawGraph();
  }, 1000);

  setInterval(() => {
    screen = screen === "weather" ? "graph" : "weather";
    void drawScreen();
  }, SCREEN_MS);

  setInterval(() => {
    void (async () => {
      await refresh();
      await drawScreen();
    })();
  }, REFRESH_MS);
}
