// LVGL binfont (.font) parser. Source of truth: lib/lvgl/src/font/lv_binfont_loader.c
import { readFileSync } from 'node:fs'

class Bits {
  constructor(buf, off) { this.b = buf; this.byte = off; this.bit = 0 }
  read(n) { // MSB-first, like read_bits() in LVGL
    let v = 0
    for (let i = 0; i < n; i++) {
      const cur = this.b[this.byte]
      const bit = (cur >> (7 - this.bit)) & 1
      v = (v << 1) | bit
      this.bit++
      if (this.bit === 8) { this.bit = 0; this.byte++ }
    }
    return v
  }
  readSigned(n) {
    const v = this.read(n)
    const sign = 1 << (n - 1)
    return v & sign ? -(v & (sign - 1)) : v
  }
}

export function readBinFont(path) {
  const b = readFileSync(path)
  const tables = {}
  let o = 0
  while (o + 8 <= b.length) {
    const len = b.readUInt32LE(o)
    const tag = b.toString('ascii', o + 4, o + 8)
    if (len < 8 || o + len > b.length) break
    tables[tag] = { off: o, len }
    o += len
  }

  const h = tables.head.off + 8
  const head = {
    font_size: b.readUInt16LE(h + 6),
    ascent: b.readUInt16LE(h + 8),
    descent: b.readInt16LE(h + 10),
    min_y: b.readInt16LE(h + 18),
    max_y: b.readInt16LE(h + 20),
    default_advance_width: b.readUInt16LE(h + 22),
    index_to_loc_format: b.readUInt8(h + 26),
    glyph_id_format: b.readUInt8(h + 27),
    advance_width_format: b.readUInt8(h + 28),
    bpp: b.readUInt8(h + 29),
    xy_bits: b.readUInt8(h + 30),
    wh_bits: b.readUInt8(h + 31),
    advance_width_bits: b.readUInt8(h + 32),
  }

  // ── cmap ──
  // Format order from lv_font_fmt_txt.h (note: SPARSE_FULL=1, FORMAT0_TINY=2):
  const FORMAT0_FULL = 0, SPARSE_FULL = 1, FORMAT0_TINY = 2, SPARSE_TINY = 3
  const cm = tables.cmap.off + 8
  const cmapCount = b.readUInt32LE(cm)
  const codeToGid = new Map()
  // Subtable descriptors follow the count contiguously, 16 bytes each.
  const recBase = cm + 4
  for (let i = 0; i < cmapCount; i++) {
    const r = recBase + i * 16
    const dataOffset = b.readUInt32LE(r)
    const rangeStart = b.readUInt32LE(r + 4)
    const rangeLength = b.readUInt16LE(r + 8)
    const glyphIdStart = b.readUInt16LE(r + 10)
    const entries = b.readUInt16LE(r + 12)
    const formatType = b.readUInt8(r + 14)
    // data_offset is relative to the start of the cmap table (cmaps_start).
    const dataAt = tables.cmap.off + dataOffset

    if (formatType === FORMAT0_FULL) {
      // contiguous code range + list of u8 glyph id deltas
      for (let j = 0; j < rangeLength; j++) {
        const d = b.readUInt8(dataAt + j)
        codeToGid.set(rangeStart + j, glyphIdStart + d)
      }
    } else if (formatType === FORMAT0_TINY) {
      // contiguous range, gids run sequentially
      for (let j = 0; j < rangeLength; j++) codeToGid.set(rangeStart + j, glyphIdStart + j)
    } else if (formatType === SPARSE_TINY) {
      // list of u16 codes (offsets from range_start), gids sequential
      for (let j = 0; j < entries; j++) {
        const code = b.readUInt16LE(dataAt + j * 2)
        codeToGid.set(rangeStart + code, glyphIdStart + j)
      }
    } else if (formatType === SPARSE_FULL) {
      // list of u16 codes + parallel list of u16 gid deltas
      for (let j = 0; j < entries; j++) {
        const code = b.readUInt16LE(dataAt + j * 2)
        const d = b.readUInt16LE(dataAt + entries * 2 + j * 2)
        codeToGid.set(rangeStart + code, glyphIdStart + d)
      }
    } else {
      throw new Error(`cmap: unknown format_type ${formatType}`)
    }
  }

  // ── loca ──
  const lo = tables.loca.off + 8
  const locaCount = b.readUInt32LE(lo)
  const offsets = []
  for (let i = 0; i < locaCount; i++) {
    offsets.push(head.index_to_loc_format === 0
      ? b.readUInt16LE(lo + 4 + i * 2)
      : b.readUInt32LE(lo + 4 + i * 4))
  }

  // ── glyf: read the descriptor at each offset ──
  // Offsets are from the table start, including its 8-byte header.
  const glyfBase = tables.glyf.off
  function glyph(gid) {
    if (gid >= offsets.length) return null
    const it = new Bits(b, glyfBase + offsets[gid])
    let adv = head.advance_width_bits === 0
      ? head.default_advance_width
      : it.read(head.advance_width_bits)
    // LVGL normalizes everything to 1/16 px
    if (head.advance_width_format === 0) adv *= 16
    const ofs_x = it.readSigned(head.xy_bits)
    const ofs_y = it.readSigned(head.xy_bits)
    const box_w = it.read(head.wh_bits)
    const box_h = it.read(head.wh_bits)
    return { adv16: adv, adv: adv / 16, ofs_x, ofs_y, box_w, box_h }
  }

  return { head, codeToGid, glyph, tables: Object.keys(tables) }
}
