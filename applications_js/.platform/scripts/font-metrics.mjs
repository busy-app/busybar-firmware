// Extracts glyph widths from the device TTF fonts (assets/shared/fonts/ttf/).
// Usage:
//   node scripts/font-metrics.mjs                     # table of all fonts
//   node scripts/font-metrics.mjs busy_regular_5px    # one font, all glyphs
//   node scripts/font-metrics.mjs busy_bold_7px "12:05 PM"   # string width
//   node scripts/font-metrics.mjs --generate          # emit src/shared/fontMetrics.ts
//
// TTF is read directly (head/cmap/hmtx tables), with no dependencies.

import { readdirSync, readFileSync } from 'node:fs'
import { basename, resolve } from 'node:path'
import { fileURLToPath } from 'node:url'
import { fontsDir as binFontsDir } from './paths.mjs'

// Source TTFs are the firmware's own, read in place.
const fontsDir = resolve(binFontsDir, 'ttf')

// ── TTF parsing ────────────────────────────────────────────────────────────

/** Font tables: tag → data slice. */
function readTables(buf) {
  const numTables = buf.readUInt16BE(4)
  const tables = new Map()
  for (let i = 0; i < numTables; i++) {
    const off = 12 + i * 16
    const tag = buf.toString('ascii', off, off + 4)
    tables.set(tag, { offset: buf.readUInt32BE(off + 8), length: buf.readUInt32BE(off + 12) })
  }
  return tables
}

/** cmap: character code → glyph index. Formats 4 and 12 (Unicode) supported. */
function readCmap(buf, table) {
  const base = table.offset
  const numSubtables = buf.readUInt16BE(base + 2)

  // Look for a Unicode subtable: (3,1) BMP, (3,10) or (0,x).
  let best = null
  for (let i = 0; i < numSubtables; i++) {
    const rec = base + 4 + i * 8
    const platform = buf.readUInt16BE(rec)
    const encoding = buf.readUInt16BE(rec + 2)
    const offset = buf.readUInt32BE(rec + 4)
    const score =
      platform === 3 && encoding === 10 ? 3 : platform === 3 && encoding === 1 ? 2 : platform === 0 ? 1 : 0
    if (score > 0 && (!best || score > best.score)) best = { score, offset: base + offset }
  }
  if (!best) throw new Error('cmap: no Unicode subtable found')

  const map = new Map()
  const format = buf.readUInt16BE(best.offset)

  if (format === 4) {
    const segCountX2 = buf.readUInt16BE(best.offset + 6)
    const segCount = segCountX2 / 2
    const endAt = best.offset + 14
    const startAt = endAt + segCountX2 + 2
    const deltaAt = startAt + segCountX2
    const rangeAt = deltaAt + segCountX2

    for (let s = 0; s < segCount; s++) {
      const end = buf.readUInt16BE(endAt + s * 2)
      const start = buf.readUInt16BE(startAt + s * 2)
      const delta = buf.readInt16BE(deltaAt + s * 2)
      const rangeOffset = buf.readUInt16BE(rangeAt + s * 2)
      if (start === 0xffff) continue

      for (let code = start; code <= end && code !== 0x10000; code++) {
        let gid
        if (rangeOffset === 0) {
          gid = (code + delta) & 0xffff
        } else {
          const glyphAt = rangeAt + s * 2 + rangeOffset + (code - start) * 2
          if (glyphAt + 2 > buf.length) continue
          gid = buf.readUInt16BE(glyphAt)
          if (gid !== 0) gid = (gid + delta) & 0xffff
        }
        if (gid !== 0) map.set(code, gid)
      }
    }
  } else if (format === 12) {
    const nGroups = buf.readUInt32BE(best.offset + 12)
    for (let g = 0; g < nGroups; g++) {
      const rec = best.offset + 16 + g * 12
      const start = buf.readUInt32BE(rec)
      const end = buf.readUInt32BE(rec + 4)
      const startGid = buf.readUInt32BE(rec + 8)
      for (let code = start; code <= end; code++) map.set(code, startGid + (code - start))
    }
  } else {
    throw new Error(`cmap: format ${format} is not supported`)
  }

  return map
}

/** hmtx: glyph index → advance width in font units. */
function readAdvances(buf, tables) {
  const hhea = tables.get('hhea')
  const hmtx = tables.get('hmtx')
  const maxp = tables.get('maxp')
  if (!hhea || !hmtx || !maxp) throw new Error('missing hhea/hmtx/maxp tables')

  const numHMetrics = buf.readUInt16BE(hhea.offset + 34)
  const numGlyphs = buf.readUInt16BE(maxp.offset + 4)

  const advances = new Array(numGlyphs)
  let last = 0
  for (let gid = 0; gid < numGlyphs; gid++) {
    if (gid < numHMetrics) last = buf.readUInt16BE(hmtx.offset + gid * 4)
    advances[gid] = last
  }
  return advances
}

/**
 * Character widths in pixels. hmtx advances are already pixels and need no scaling.
 */
export function readFont(path) {
  const buf = readFileSync(path)
  const tables = readTables(buf)

  const head = tables.get('head')
  if (!head) throw new Error('missing head table')
  const unitsPerEm = buf.readUInt16BE(head.offset + 18)

  const hhea = tables.get('hhea')
  const ascent = hhea ? buf.readInt16BE(hhea.offset + 4) : 0
  const descent = hhea ? buf.readInt16BE(hhea.offset + 6) : 0

  const cmap = readCmap(buf, tables.get('cmap'))
  const advances = readAdvances(buf, tables)

  const widths = new Map()
  for (const [code, gid] of cmap) {
    const adv = advances[gid]
    if (adv === undefined) continue
    widths.set(String.fromCodePoint(code), adv)
  }

  return { name: basename(path, '.ttf'), unitsPerEm, ascent, descent, widths }
}

/** String width. Advances include letter spacing. */
export function measure(font, text) {
  let width = 0
  for (const ch of text) width += font.widths.get(ch) ?? 0
  return width
}

// ── Output ─────────────────────────────────────────────────────────────────

function fontFiles() {
  return readdirSync(fontsDir)
    .filter((f) => f.toLowerCase().endsWith('.ttf'))
    .sort()
}

/** Groups characters by width: {3: "abc…", 5: "MW"}. */
function groupByWidth(font, chars) {
  const groups = new Map()
  for (const ch of chars) {
    const w = font.widths.get(ch)
    if (w === undefined) continue
    if (!groups.has(w)) groups.set(w, [])
    groups.get(w).push(ch)
  }
  return [...groups.entries()].sort((a, b) => a[0] - b[0])
}

const PRINTABLE = Array.from({ length: 95 }, (_, i) => String.fromCharCode(32 + i))

// CLI below; the module is also imported for readFont().
const isDirectRun = process.argv[1] && resolve(process.argv[1]) === fileURLToPath(import.meta.url)

const [fontArg, textArg] = isDirectRun ? process.argv.slice(2) : []

if (!isDirectRun) {
  // imported as a module
} else if (fontArg && textArg !== undefined) {
  // Mode 3: width of a specific string.
  const file = fontFiles().find((f) => f.startsWith(fontArg))
  if (!file) {
    console.error(`Font "${fontArg}" not found. Available: ${fontFiles().join(', ')}`)
    process.exit(1)
  }
  const font = readFont(resolve(fontsDir, file))
  console.log(`${font.name}: "${textArg}" = ${measure(font, textArg)}px`)
  const perChar = [...textArg].map((ch) => `${ch}=${font.widths.get(ch) ?? '?'}`).join(' ')
  console.log(`  per character: ${perChar}`)
} else if (fontArg) {
  // Mode 2: all glyphs of one font, grouped by width.
  const file = fontFiles().find((f) => f.startsWith(fontArg))
  if (!file) {
    console.error(`Font "${fontArg}" not found. Available: ${fontFiles().join(', ')}`)
    process.exit(1)
  }
  const font = readFont(resolve(fontsDir, file))
  console.log(
    `${font.name}: ${font.widths.size} glyphs, height ${font.ascent - font.descent}px ` +
      `(ascent ${font.ascent}, descent ${font.descent})\n`,
  )
  console.log('Widths include letter spacing and add up directly.\n')
  for (const [width, chars] of groupByWidth(font, PRINTABLE)) {
    console.log(`  ${String(width).padStart(2)}px: ${chars.join('')}`)
  }
} else {
  // Mode 1: summary across all fonts, for layout work.
  const samples = ['0', '1', ':', ',', '.', ' ', 'A', 'M', 'W', 'P', 'S', 'e', 'p', 'T', 'u']
  console.log('Widths of key characters, px\n')
  console.log(`${'font'.padEnd(22)}${samples.map((s) => (s === ' ' ? '␣' : s).padStart(3)).join('')}`)
  console.log('─'.repeat(22 + samples.length * 3))

  for (const file of fontFiles()) {
    const font = readFont(resolve(fontsDir, file))
    const row = samples.map((ch) => String(font.widths.get(ch) ?? '·').padStart(3)).join('')
    console.log(`${font.name.padEnd(22)}${row}`)
  }

  console.log('\nDetails:  node scripts/font-metrics.mjs <font>')
  console.log('String:   node scripts/font-metrics.mjs <font> "12:05 PM"')
}
