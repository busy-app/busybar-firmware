// Vite plugin: collects the fonts an app draws with and registers their glyph maps.
//
// Apps write `font: "bold"`; the plugin scans for those literals and serves
// `virtual:font-maps`, which imports only the maps in use. @shared/font pulls
// that module in, so apps register nothing by hand.

import { existsSync, readdirSync, readFileSync } from 'node:fs'
import { relative, resolve } from 'node:path'
import { mapsDir, root } from './paths.mjs'

const VIRTUAL_ID = 'virtual:font-maps'
const RESOLVED_ID = '\0' + VIRTUAL_ID

const SOURCE_FILE = /\.(ts|tsx|js|mjs|jsx)$/

/** Every `font:` property, literal or not. */
const FONT_PROP = /\bfont\s*:\s*([^,;}\n)]+)/g
/** The value of a literal one: "bold", 'bold', optionally `as const`. */
const LITERAL = /^(["'])([a-z_]+)\1(\s+as\s+const)?$/

/** Font names that have a generated map. */
function available() {
  if (!existsSync(mapsDir)) return new Set()
  return new Set(readdirSync(mapsDir).filter((f) => f.endsWith('.json')).map((f) => f.slice(0, -5)))
}

function scan(code, file, found, dynamic) {
  for (const m of code.matchAll(FONT_PROP)) {
    const lineStart = code.lastIndexOf('\n', m.index) + 1
    const before = code.slice(lineStart, m.index)
    // Skip comments and type annotations (`font: DeviceFont`).
    if (/(\/\/|\*)/.test(before)) continue

    const literal = LITERAL.exec(m[1].trim())
    if (literal) {
      found.add(literal[2])
    } else if (!/^[A-Z]/.test(m[1].trim())) {
      dynamic.push({ file, line: code.slice(0, m.index).split('\n').length, expr: m[1].trim() })
    }
  }
}

export function fontMaps() {
  const found = new Set()
  const dynamic = []
  let known = new Set()

  function scanSources() {
    found.clear()
    dynamic.length = 0
    const walk = (dir) => {
      for (const entry of readdirSync(dir, { withFileTypes: true })) {
        if (entry.name === 'node_modules' || entry.name.startsWith('.')) continue
        const path = resolve(dir, entry.name)
        if (entry.isDirectory()) walk(path)
        else if (SOURCE_FILE.test(entry.name)) scan(readFileSync(path, 'utf8'), path, found, dynamic)
      }
    }
    // Apps sit at the root; walk() skips node_modules and dot-dirs (.platform included).
    walk(root)
  }

  return {
    name: 'font-maps',

    buildStart() {
      known = available()
      if (known.size === 0) {
        this.error('font-maps: no maps in .platform/shared/fontMaps — run pnpm fonts:build')
      }
      scanSources()

      if (dynamic.length > 0) {
        const where = dynamic
          .map((d) => `  ${relative(root, d.file)}:${d.line}  font: ${d.expr}`)
          .join('\n')
        this.error(
          `font-maps: font must be a string literal, so the build can tell which maps to bundle.\n${where}\n` +
            `Use a literal, or import the map and call registerFontMaps() yourself.`,
        )
      }

      const unknown = [...found].filter((f) => !known.has(f))
      if (unknown.length > 0) {
        this.error(`font-maps: no map for font ${unknown.map((f) => `"${f}"`).join(', ')}`)
      }
    },

    resolveId(id) {
      return id === VIRTUAL_ID ? RESOLVED_ID : null
    },

    load(id) {
      if (id !== RESOLVED_ID) return null

      const fonts = [...found].sort()
      const imports = fonts
        .map((f, i) => `import f${i} from '${resolve(mapsDir, `${f}.json`)}';`)
        .join('\n')

      return `${imports}\nexport const MAPS = [${fonts.map((_, i) => `f${i}`).join(', ')}];\n`
    },

    handleHotUpdate({ file, server }) {
      if (!SOURCE_FILE.test(file)) return
      const before = [...found].join()
      scanSources()
      if ([...found].join() === before) return

      const virtual = server.moduleGraph.getModuleById(RESOLVED_ID)
      if (!virtual) return
      server.moduleGraph.invalidateModule(virtual)
      server.ws.send({ type: 'full-reload' })
    },
  }
}
