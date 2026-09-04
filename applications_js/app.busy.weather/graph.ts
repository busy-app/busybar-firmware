import { tempColor } from "./tempColor.ts";

export const GRAPH_W = 72;
export const GRAPH_H = 9;

/**
 * Draws the hourly temperature graph as a PNG. The fill is colored by temperature with a vertical brightness gradient, topped by a white outline; the current moment is marked at x=0.
 */
export async function renderGraphPng(temps: number[]): Promise<ArrayBuffer> {
  const canvas = new OffscreenCanvas(GRAPH_W, GRAPH_H);
  const ctx = canvas.getContext("2d")!;
  ctx.clearRect(0, 0, GRAPH_W, GRAPH_H);

  if (temps.length < 2) {
    return canvas.convertToBlob({ type: "image/png" }).then((b) => b.arrayBuffer());
  }

  let min = temps[0];
  let max = temps[0];
  for (const t of temps) {
    if (t < min) min = t;
    if (t > max) max = t;
  }
  let range = max - min;
  if (range < 1) range = 1;

  // Temperature at column x (linear interpolation between hourly points).
  const tempAt = (x: number): number => {
    const pos = (x / (GRAPH_W - 1)) * (temps.length - 1);
    const i = Math.floor(pos);
    const f = pos - i;
    if (i >= temps.length - 1) return temps[temps.length - 1];
    return temps[i] * (1 - f) + temps[i + 1] * f;
  };

  // Pixel height of the curve's top at column x (0 is the top).
  const topY = (x: number): number => {
    const norm = (tempAt(x) - min) / range; // 0..1, warmer is higher
    const usable = GRAPH_H - 2; // top margin for the line and marker
    return Math.round(1 + (1 - norm) * usable);
  };

  // From the curve's top down to the bottom.
  for (let x = 0; x < GRAPH_W; x++) {
    const [r, g, b] = tempColor(tempAt(x));
    const top = topY(x);
    for (let y = top; y < GRAPH_H; y++) {
      // 0 at the line, 1 at the bottom; brightness drops to 55%.
      const depth = GRAPH_H - 1 > top ? (y - top) / (GRAPH_H - 1 - top) : 0;
      const k = 1 - depth * 0.45;
      ctx.fillStyle = `rgb(${Math.round(r * k)},${Math.round(g * k)},${Math.round(b * k)})`;
      ctx.fillRect(x, y, 1, 1);
    }
  }

  // White outline along the top of the curve.
  ctx.fillStyle = "#ffffff";
  for (let x = 0; x < GRAPH_W; x++) {
    ctx.fillRect(x, topY(x), 1, 1);
  }

  // Current moment.
  const my = topY(0);
  ctx.fillStyle = "#ffffff";
  // Vertical bar from the curve down to the bottom.
  ctx.fillRect(0, my, 1, GRAPH_H - my);
  // Dot on the curve.
  ctx.beginPath();
  ctx.arc(0.5, my, 1.4, 0, Math.PI * 2);
  ctx.fill();

  const blob = await canvas.convertToBlob({ type: "image/png" });
  return blob.arrayBuffer();
}
