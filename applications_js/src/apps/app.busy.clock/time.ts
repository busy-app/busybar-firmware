// Time and date formatting, per appmeta/settings.json.


/** Values of the `time`/`date` settings fields. */
export interface ClockSettings {
  time_format: "24h" | "12h";
  show_seconds: boolean;
  blinking_colon: boolean;
  show_date: boolean;
  date_format: "DD.MM.YYYY" | "DD/MM/YYYY" | "MM/DD/YYYY" | "YYYY-MM-DD" | "DD-MM-YYYY" | "YYYY.MM.DD";
  year_digits: "2" | "4";
  show_weekday: boolean;
}

/** Defaults, matching appmeta/settings.json. */
export const DEFAULTS: ClockSettings = {
  time_format: "24h",
  show_seconds: false,
  blinking_colon: true,
  show_date: true,
  date_format: "DD.MM.YYYY",
  year_digits: "4",
  show_weekday: true,
};

const MONTHS = [
  "Jan", "Feb", "Mar", "Apr", "May", "Jun",
  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec",
];
const DAYS = ["Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"];

// 12-hour format suffixes.
const SUFFIXES = ["AM", "PM"];

const pad = (n: number) => String(n).padStart(2, "0");

/**
 * The time, with the AM/PM suffix returned separately. In 12-hour format the hour has no leading zero.
 */
export function formatTimeParts(
  d: Date,
  s: ClockSettings = DEFAULTS,
): { time: string; suffix: string } {
  const seconds = s.show_seconds ? `:${pad(d.getSeconds())}` : "";
  if (s.time_format === "12h") {
    const h24 = d.getHours();
    const h12 = h24 % 12 === 0 ? 12 : h24 % 12;
    return {
      time: `${h12}:${pad(d.getMinutes())}${seconds}`,
      suffix: h24 < 12 ? SUFFIXES[0] : SUFFIXES[1],
    };
  }
  return { time: `${pad(d.getHours())}:${pad(d.getMinutes())}${seconds}`, suffix: "" };
}

/** A piece of the time string: digits or a colon. */
export interface TimeSegment {
  text: string;
  /** Colons take their own opacity. */
  colon: boolean;
}

/** Splits the time into digit groups and colons: "12:05" → ["12", ":", "05"]. */
export function splitTime(time: string): TimeSegment[] {
  const segments: TimeSegment[] = [];
  for (const part of time.split(":")) {
    if (segments.length > 0) segments.push({ text: ":", colon: true });
    segments.push({ text: part, colon: false });
  }
  return segments;
}

/** Whether the colons are in their dimmed blink phase. */
export function colonDimmed(d: Date, s: ClockSettings = DEFAULTS): boolean {
  return s.blinking_colon && d.getSeconds() % 2 === 1;
}

/** The time as one string, e.g. "14:05" or "2:05 PM". */
export function formatTime(d: Date, s: ClockSettings = DEFAULTS): string {
  const { time, suffix } = formatTimeParts(d, s);
  return suffix ? `${time} ${suffix}` : time;
}

/** The date in the selected pattern; YYYY marks position, not length. */
export function formatDate(d: Date, s: ClockSettings = DEFAULTS): string {
  const DD = pad(d.getDate());
  const MM = pad(d.getMonth() + 1);
  const year = d.getFullYear();
  const YYYY = s.year_digits === "2" ? pad(year % 100) : String(year);

  switch (s.date_format) {
    case "DD/MM/YYYY":
      return `${DD}/${MM}/${YYYY}`;
    case "MM/DD/YYYY":
      return `${MM}/${DD}/${YYYY}`;
    case "YYYY-MM-DD":
      return `${YYYY}-${MM}-${DD}`;
    case "DD-MM-YYYY":
      return `${DD}-${MM}-${YYYY}`;
    case "YYYY.MM.DD":
      return `${YYYY}.${MM}.${DD}`;
    default:
      return `${DD}.${MM}.${YYYY}`;
  }
}

/** Three-letter month, e.g. "Jul". */
export function formatMonth(d: Date): string {
  return MONTHS[d.getMonth()];
}

/** Three-letter weekday, e.g. "Fri". */
export function formatWeekday(d: Date): string {
  return DAYS[d.getDay()];
}

/** Day of the month, without a leading zero. */
export function formatDayOfMonth(d: Date): string {
  return String(d.getDate());
}

/** Normalizes stored values; unknown and missing fields fall back to defaults. */
export function normalizeSettings(raw: Record<string, unknown> | null | undefined): ClockSettings {
  const values = raw ?? {};
  const pick = <K extends keyof ClockSettings>(key: K, allowed?: readonly ClockSettings[K][]): ClockSettings[K] => {
    const v = values[key];
    if (typeof v !== typeof DEFAULTS[key]) return DEFAULTS[key];
    if (allowed && !allowed.includes(v as ClockSettings[K])) return DEFAULTS[key];
    return v as ClockSettings[K];
  };

  return {
    time_format: pick("time_format", ["24h", "12h"]),
    show_seconds: pick("show_seconds"),
    blinking_colon: pick("blinking_colon"),
    show_date: pick("show_date"),
    date_format: pick("date_format", [
      "DD.MM.YYYY",
      "DD/MM/YYYY",
      "MM/DD/YYYY",
      "YYYY-MM-DD",
      "DD-MM-YYYY",
      "YYYY.MM.DD",
    ]),
    year_digits: pick("year_digits", ["2", "4"]),
    show_weekday: pick("show_weekday"),
  };
}
