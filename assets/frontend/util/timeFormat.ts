const RELATIVE_UNITS: [Intl.RelativeTimeFormatUnit, number][] = [
  ['year', 365 * 24 * 60 * 60],
  ['month', 30 * 24 * 60 * 60],
  ['day', 24 * 60 * 60],
  ['hour', 60 * 60],
  ['minute', 60],
  ['second', 1]
];

const relativeTimeFormat = new Intl.RelativeTimeFormat('en', { numeric: 'auto' });

export const formatTimeAgo = (timestampMs: number, nowMs = Date.now()) => {
  const seconds = Math.round((timestampMs - nowMs) / 1000);

  for (const [unit, unitSeconds] of RELATIVE_UNITS) {
    if (Math.abs(seconds) >= unitSeconds || unit === 'second') {
      return relativeTimeFormat.format(Math.round(seconds / unitSeconds), unit);
    }
  }

  return '';
};

export const formatDateDMY = (timestampMs: number) => {
  const date = new Date(timestampMs);

  return `${pad2(date.getDate())}.${pad2(date.getMonth() + 1)}.${date.getFullYear()}`;
};
