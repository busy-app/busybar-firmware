// Temperature (°C) → RGB across 6 stops. Range -30..+45°C: cold violet → light blue → gray → yellow → orange (ordinary warmth, ~20-30°C) → red (extreme heat, 40°C+).

const STOPS = [0.0, 0.35, 0.5, 0.65, 0.8, 1.0];
const COLORS: [number, number, number][] = [
  [0x20, 0x00, 0x8a], // -30: violet
  [0x67, 0xa8, 0xf5], // ~-4: light blue
  [0xcf, 0xcf, 0xcf], // ~7:  gray
  [0xff, 0xc8, 0x00], // ~19: yellow
  [0xd2, 0x8a, 0x1e], // ~30: orange/ochre
  [0xff, 0x30, 0x00], // +45: red
];

/** Temperature (°C) → [r, g, b] 0..255. */
export function tempColor(celsius: number): [number, number, number] {
  let t = (celsius + 30) / 75;
  if (t < 0) t = 0;
  if (t > 1) t = 1;

  let seg = 4;
  for (let i = 0; i < 5; i++) {
    if (t < STOPS[i + 1]) {
      seg = i;
      break;
    }
  }

  const f = (t - STOPS[seg]) / (STOPS[seg + 1] - STOPS[seg]);
  const lerp = (a: number, b: number) => Math.floor(a + f * (b - a));
  return [
    lerp(COLORS[seg][0], COLORS[seg + 1][0]),
    lerp(COLORS[seg][1], COLORS[seg + 1][1]),
    lerp(COLORS[seg][2], COLORS[seg + 1][2]),
  ];
}
