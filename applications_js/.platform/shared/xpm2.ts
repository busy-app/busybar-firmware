// XPM2 generator for the xpmbitmap element in DisplayDraw.
//
// Unlike XPM1, XPM2 is a plain text format with no C wrapper: signature, header line, palette lines, pixel lines, all newline-separated. That is exactly what goes into the element's data field:
//
//   generateXpm2({
//     palette: { '.': 'None', '#': '#FF0000', 'O': '#00FF00' },
//     grid: [
//       '.#.#.',
//       '#####',
//       '.OOO.',
//     ],
//   })
//   ! XPM2
//   5 3 3 1
//   . c None
//   # c #FF0000
//   O c #00FF00
//   .#.#.
//   #####
//   .OOO.
//
// Built by string concatenation: no compression, no checksums.

/** `c` color, `g` grayscale, `g4` 4-level grayscale, `m` monochrome, `s` symbolic name. A symbol may carry several; the device picks one to match its display. */
export type Visual = 'c' | 'g' | 'g4' | 'm' | 's'

/** Values can be hex (#RGB, #RRGGBB, #RRRGGGBBB, #RRRRGGGGBBBB), or named (none, black, white, red, green, blue, yellow, cyan, magenta, gray, grey). */
export type ColorValue = string

/** Color of a palette symbol. A string is shorthand for visual `c`; an object gives several, e.g. `{ c: '#FF0000', m: 'white' }`. */
export type PaletteEntry = ColorValue | Partial<Record<Visual, ColorValue>>

export interface Xpm2Spec {
  /** Symbol → color. Same symbols as in grid. */
  palette: Record<string, PaletteEntry>
  /** Pixel rows, top to bottom. One character per pixel. */
  grid: string[]
}

/** Element spec limits: ncolors <= 32, cpp <= 4. */
const MAX_COLORS = 32

/** Visual order within a palette line. */
const VISUALS: Visual[] = ['m', 'g4', 'g', 'c', 's']

/** Strips alpha from #RRGGBBAA, which XPM2 doesn't support. */
function normalizeColor(color: ColorValue): string {
  if (color.charAt(0) !== '#') return color
  return color.length === 9 ? color.slice(0, 7) : color
}

/** Palette line: "symbol visual value [visual value ...]". */
function paletteLine(symbol: string, entry: PaletteEntry): string {
  if (typeof entry === 'string') {
    return `${symbol} c ${normalizeColor(entry)}`
  }

  const parts: string[] = []
  for (const visual of VISUALS) {
    const value = entry[visual]
    if (value !== undefined) parts.push(`${visual} ${normalizeColor(value)}`)
  }

  if (parts.length === 0) {
    throw new Error(`xpm2: symbol "${symbol}" has an empty palette entry`)
  }
  return `${symbol} ${parts.join(' ')}`
}

/** Builds XPM2 from a palette and a character grid. */
export function generateXpm2({ palette, grid }: Xpm2Spec): string {
  const symbols = Object.keys(palette)
  const height = grid.length
  const width = height > 0 ? grid[0].length : 0

  if (width === 0 || height === 0) {
    throw new Error('xpm2: empty grid')
  }
  if (symbols.length === 0) {
    throw new Error('xpm2: empty palette')
  }
  if (symbols.length > MAX_COLORS) {
    throw new Error(`xpm2: ${symbols.length} colors, maximum is ${MAX_COLORS}`)
  }

  // Pixel rows are parsed cpp characters at a time.
  const cpp = symbols[0].length
  for (const symbol of symbols) {
    if (symbol.length !== cpp) {
      throw new Error(`xpm2: palette symbols differ in length ("${symbols[0]}" and "${symbol}")`)
    }
  }
  if (cpp < 1 || cpp > 4) {
    throw new Error(`xpm2: symbol length ${cpp}, must be 1..4`)
  }

  const lines: string[] = ['! XPM2', `${width / cpp} ${height} ${symbols.length} ${cpp}`]

  for (const symbol of symbols) {
    lines.push(paletteLine(symbol, palette[symbol]))
  }

  for (let y = 0; y < height; y++) {
    const row = grid[y]
    // Uneven rows mean a skewed image or a parser rejection.
    if (row.length !== width) {
      throw new Error(`xpm2: row ${y} has length ${row.length}, expected ${width}`)
    }

    for (let x = 0; x < width; x += cpp) {
      const symbol = row.slice(x, x + cpp)
      if (palette[symbol] === undefined) {
        throw new Error(`xpm2: symbol "${symbol}" in row ${y} is not in the palette`)
      }
    }

    lines.push(row)
  }

  return lines.join('\n')
}

/**
 * Scales a grid by `factor` on both axes.
 *
 * @param cpp symbol length, if the grid uses more than one character
 */
export function scaleGrid(grid: string[], factor: number, cpp = 1): string[] {
  if (factor <= 1) return grid

  const out: string[] = []
  for (const row of grid) {
    let wide = ''
    for (let x = 0; x < row.length; x += cpp) {
      const symbol = row.slice(x, x + cpp)
      for (let i = 0; i < factor; i++) wide += symbol
    }
    for (let i = 0; i < factor; i++) out.push(wide)
  }
  return out
}
