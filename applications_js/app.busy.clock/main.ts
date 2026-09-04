import { device } from "@shared/device";
import { column, render, row, stack, type Node } from "@shared/layout";
import manifest from "./appmeta/manifest.json";
import { loadValues } from "@shared/settings";
import {
  colonDimmed,
  DEFAULTS,
  formatDate,
  formatDayOfMonth,
  formatMonth,
  formatTimeParts,
  formatWeekday,
  normalizeSettings,
  splitTime,
  type ClockSettings,
} from "./time.ts";

const APP = manifest.id;
const ICON_FILE = "calendar.png";

/** Calendar icon size. */
const ICON = { width: 13, height: 14 };

const SUFFIX_GAP = 1;
/** Gap between the icon and the text. */
const ICON_GAP = 2;
/** Gap between the time and date lines. */
const LINE_GAP = 2;

const WHITE = "#FFFFFFFF";
const SUBTLE_WHITE = "#FFFFFF80";

let settings: ClockSettings = DEFAULTS;

/** Reads the settings. Their absence is not an error. */
async function loadSettings(): Promise<void> {
  settings = normalizeSettings(await loadValues());
}

/**
 * The bottom line, per settings:
 *   "29.07.2026, Tue" | "Sep, Tue" | "29.07.2026" | "Sep"
 */
function subtitle(now: Date): string {
  const parts: string[] = [];

  if (settings.show_date) parts.push(formatDate(now, settings));
  else parts.push(formatMonth(now));

  if (settings.show_weekday) parts.push(formatWeekday(now));

  return parts.join(", ");
}

/**
 * The time, followed by AM/PM in 12-hour format. Split into segments, each with its own color, and keyed by segment index.
 */
function timeRow(now: Date): Node {
  const { time, suffix } = formatTimeParts(now, settings);
  const dim = colonDimmed(now, settings);

  const segments: Node[] = splitTime(time).map((segment, i) => ({
    type: "text",
    id: segment.colon ? `colon${i}` : `time${i}`,
    text: segment.text,
    font: "bold",
    color: segment.colon && dim ? SUBTLE_WHITE : WHITE,
  }));

  if (!suffix) return row({}, segments);

  // Shares the baseline with the digits by default.
  return row({ gap: SUFFIX_GAP }, [
    row({}, segments),
    { type: "text", id: "suffix", text: suffix, font: "small", color: WHITE },
  ]);
}

/**
 * Calendar icon with the day inside, centered horizontally and dropped into the white field below the icon's header.
 */
const DAY_OFFSET = { dy: 7 };

function calendarIcon(now: Date): Node {
  return stack({ justify: "center" }, [
    { type: "image", id: "icon", path: ICON_FILE, ...ICON },
    {
      type: "text",
      id: "day",
      text: formatDayOfMonth(now),
      font: "small",
      color: "#000000FF",
      ...DAY_OFFSET,
    },
  ]);
}

/**
 * The whole frame, centered as one block. The icon shows only when the full date is off.
 */
function frame(now: Date): Node {
  const sub = settings.show_date || settings.show_weekday ? subtitle(now) : "";
  const showIcon = sub.length > 0 && !settings.show_date;

  // Lines left-aligned to each other, the column centered as a whole.
  const textColumn = column({ gap: LINE_GAP, align: "start" }, [
    timeRow(now),
    ...(sub
      ? [{ type: "text" as const, id: "sub", text: sub, font: "small" as const, color: SUBTLE_WHITE }]
      : []),
  ]);

  if (!showIcon) {
    return column({ justify: "center", align: "center" }, [textColumn]);
  }

  return row({ justify: "center", align: "center", gap: ICON_GAP }, [
    calendarIcon(now),
    textColumn,
  ]);
}

async function draw(): Promise<void> {
  const elements = render(frame(new Date()));
  await device.DisplayDraw({ application_name: APP, priority: 50, elements });
}

export default function run(): void {
  // Settings first, then draw.
  void loadSettings()
    .then(() => draw())
    .catch((err) =>
      console.error(
        `clock-new: failed to initialize the app — ${err instanceof Error ? err.message : String(err)}`,
      ),
    );

  // Tick errors are reported, not thrown.
  setInterval(() => {
    void draw().catch((err) =>
      console.error(`clock-new: ${err instanceof Error ? err.message : String(err)}`),
    );
  }, 1000);
}
