export function getColorAlpha (value: string): number {
  const normalizedValue = value.trim();

  if (normalizedValue.startsWith('#')) {
    const hexValue = normalizedValue.slice(1);

    if (hexValue.length === 4 || hexValue.length === 8) {
      const alphaHex = hexValue.length === 4
        ? `${hexValue[3]}${hexValue[3]}`
        : hexValue.slice(6, 8);
      const alpha = Number.parseInt(alphaHex, 16) / 255;

      return Number.isNaN(alpha) ? 1 : Math.min(Math.max(alpha, 0), 1);
    }

    return 1;
  }

  const rgbaMatch = normalizedValue.match(/^rgba?\(\s*[\d.]+\s*,\s*[\d.]+\s*,\s*[\d.]+(?:\s*,\s*([\d.]+))?\s*\)$/i);

  if (!rgbaMatch) {
    return 1;
  }

  const alpha = Number.parseFloat(rgbaMatch[1] ?? '1');

  return Number.isNaN(alpha) ? 1 : Math.min(Math.max(alpha, 0), 1);
}

export function isColorFullyTransparent (value: string): boolean {
  return getColorAlpha(value) <= 0;
}
