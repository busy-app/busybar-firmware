// Glyph map generator: assets/shared/fonts/*.font  →  .platform/shared/fontMaps/<font>.json
// Run by hand when fonts change; the result is committed.
// Usage:
//   node scripts/build-font-maps.mjs           # generate
//   node scripts/build-font-maps.mjs --check   # verify it's up to date (CI)

import { mkdirSync, readFileSync, writeFileSync } from 'node:fs'
import { resolve } from 'node:path'
import { readBinFont } from './binfont.mjs'
import { fontsDir, mapsDir as outDir, root } from './paths.mjs'

/**
 * DisplayDraw font names → firmware files. Source: api_display.c (font_names/font_paths) + font_registry/fonts.h. Fonts unreachable through the API are omitted.
 */
const DEVICE_FONTS = {
  tiny: 'busy_tiny',
  small: 'busy_regular_5',
  normal: 'busy_regular_7',
  condensed: 'busy_condensed_7',
  bold: 'busy_bold_7',
  large: 'busy_regular_9',
  extra_large: 'busy_bold_10',
  global: 'lana_pixel_regular_11',
}

// Every glyph a font declares is taken, and the range comes from the font.

/**
 * Metrics of one font, as delta-coded runs. Consecutive codes sharing metrics collapse into one run, and the metrics covering the most codes become `fallback`, whose runs are dropped. Codes with no glyph inherit the preceding code's metrics.
 *
 *   inkTop = ascent - (ofs_y + box_h)
 *   inkH   = box_h
 */
function buildFontMap(file) {
  const font = readBinFont(resolve(fontsDir, `${file}.font`))
  const { ascent, descent } = font.head

  const codes = [...font.codeToGid.keys()].sort((a, b) => a - b)
  const firstCode = codes[0]
  const lastCode = codes[codes.length - 1]

  const metricsAt = (code) => {
    const gid = font.codeToGid.get(code)
    if (gid === undefined) return null
    const g = font.glyph(gid)
    // Empty glyphs (space) have box_h = 1 with no pixels.
    return [g.adv, ascent - (g.ofs_y + g.box_h), g.box_h]
  }

  const runs = []
  let prev = null
  let carried = metricsAt(firstCode)
  for (let code = firstCode; code <= lastCode; code++) {
    const m = metricsAt(code) ?? carried
    carried = m
    if (prev && prev.last === code - 1 && prev.m[0] === m[0] && prev.m[1] === m[1] && prev.m[2] === m[2]) {
      prev.last = code
      continue
    }
    prev = { first: code, last: code, m }
    runs.push(prev)
  }

  // Most codes, not most runs.
  const spanOf = new Map()
  for (const r of runs) {
    const key = r.m.join(',')
    spanOf.set(key, (spanOf.get(key) ?? 0) + (r.last - r.first + 1))
  }
  let fallback = [0, 0, 0]
  let widest = -1
  for (const [key, span] of spanOf) {
    if (span > widest) {
      widest = span
      fallback = key.split(',').map(Number)
    }
  }

  const kept = runs.filter((r) => !(r.m[0] === fallback[0] && r.m[1] === fallback[1] && r.m[2] === fallback[2]))

  // [gapFromPrevEnd, length, advance, inkTop, inkH, …]
  const runsFlat = []
  let prevEnd = firstCode
  for (const r of kept) {
    runsFlat.push(r.first - prevEnd, r.last - r.first, r.m[0], r.m[1], r.m[2])
    prevEnd = r.last
  }

  return {
    file,
    lineHeight: ascent - descent,
    ascent,
    descent,
    firstCode,
    lastCode,
    fallback,
    runs: runsFlat,
    glyphCount: codes.length,
    runCount: kept.length,
  }
}

function build(name) {
  const file = DEVICE_FONTS[name]
  const map = buildFontMap(file)
  const { glyphCount, runCount, ...rest } = map

  return {
    json: { fonts: { [name]: rest } },
    report: { name, file, lineHeight: map.lineHeight, glyphs: glyphCount, runs: runCount },
  }
}

// Indented, except the metric arrays.
function serialize(json) {
  return (
    JSON.stringify(json, null, 2).replace(/\[\n\s+(-?\d+(?:,\n\s+-?\d+)*)\n\s+\]/g, (_, body) =>
      `[${body.replace(/\s+/g, '')}]`,
    ) + '\n'
  )
}

const built = Object.keys(DEVICE_FONTS).map((name) => {
  const { json, report } = build(name)
  return { file: resolve(outDir, `${name}.json`), report, serialized: serialize(json) }
})

if (process.argv.includes('--check')) {
  for (const { file, serialized } of built) {
    const rel = file.slice(root.length)
    let current = null
    try {
      current = readFileSync(file, 'utf8')
    } catch {
      console.error(`fonts: ${rel} is missing — run pnpm fonts:build`)
      process.exit(1)
    }
    if (current !== serialized) {
      console.error(`fonts: ${rel} is out of date — run pnpm fonts:build`)
      process.exit(1)
    }
  }
  console.log('fonts: maps are up to date')
  process.exit(0)
}

mkdirSync(outDir, { recursive: true })
for (const { file, serialized } of built) writeFileSync(file, serialized)

console.log(`${'font'.padEnd(14)}${'file'.padEnd(24)}lineH  glyphs   runs     size`)
console.log('─'.repeat(70))
for (const { report: r, serialized } of built) {
  console.log(
    `${r.name.padEnd(14)}${r.file.padEnd(24)}${String(r.lineHeight).padStart(4)}` +
      `${String(r.glyphs).padStart(8)}${String(r.runs).padStart(7)}` +
      `${(serialized.length / 1024).toFixed(1).padStart(9)} KB`,
  )
}
