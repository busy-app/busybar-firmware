// Builds apps into the BusyBar package format (documentation/JSAppsSpec.dox.md):
//
//   ../assets/applications_js/<app>/
//   ├── appmeta/manifest.json      # required
//   ├── appmeta/settings.json      # optional
//   ├── scripts/main.js            # fixed entry point, plus sibling modules
//   ├── images/ animations/ sounds/ resources/
//
// Resources in <app>/{images,animations,sounds}/ are copied as-is; anything else is sorted by file type.

import { execFileSync } from 'node:child_process'
import {
  cpSync,
  existsSync,
  mkdirSync,
  readdirSync,
  readFileSync,
  rmSync,
  writeFileSync,
} from 'node:fs'
import { resolve } from 'node:path'
import { appsDir, discoverApps, outRoot, platform, root } from './paths.mjs'
import { classify, KNOWN_DIRS } from './resources.mjs'

/** Validates the manifest against the spec; throws on violation. */
function validateManifest(path, app) {
  if (!existsSync(path)) {
    throw new Error(`${app}: appmeta/manifest.json is missing and required`)
  }
  const manifest = JSON.parse(readFileSync(path, 'utf8'))
  if (typeof manifest.id !== 'string' || !/^[a-zA-Z0-9._-]+$/.test(manifest.id)) {
    throw new Error(`${app}: manifest.id must match ^[a-zA-Z0-9._-]+$`)
  }
  if (!/^\d+\.\d+\.\d+$/.test(manifest.version ?? '')) {
    throw new Error(`${app}: manifest.version must be semver, e.g. "1.0.0"`)
  }
  if (typeof manifest.name !== 'string' || manifest.name === '') {
    throw new Error(`${app}: manifest.name is required`)
  }
  return manifest
}

/** App sources: built by Vite, never treated as resources. */
const SOURCE_FILE = /\.(ts|tsx|js|mjs|cjs|jsx|d\.ts|map)$/i
/** Housekeeping entries that must not end up in resources. */
const IGNORED = new Set(['appmeta', 'node_modules', '.DS_Store'])

/**
 * Recursively sorts a directory's files into the package's target folders. Returns a map of `file name` → `path inside the package`.
 */
function layoutResources(srcDir, outDir, sourceDirName, moved = new Map()) {
  for (const entry of readdirSync(srcDir, { withFileTypes: true })) {
    if (IGNORED.has(entry.name)) continue
    const src = resolve(srcDir, entry.name)

    if (entry.isDirectory()) {
      // A known folder name sets the type; any other is transparent.
      const nested = KNOWN_DIRS.includes(entry.name) ? entry.name : sourceDirName
      layoutResources(src, outDir, nested, moved)
      continue
    }

    if (SOURCE_FILE.test(entry.name)) continue

    const { target, warning } = classify(entry.name, sourceDirName)
    if (warning) console.warn(`  ⚠ ${warning}`)
    const destDir = resolve(outDir, target)
    mkdirSync(destDir, { recursive: true })
    cpSync(src, resolve(destDir, entry.name))
    moved.set(entry.name, `${target}/${entry.name}`)
  }
  return moved
}

/** Rewrites "/_assets/<file>" references to the real path in the package. */
function rewriteAssetUrls(scriptsDir, moved) {
  for (const entry of readdirSync(scriptsDir, { withFileTypes: true })) {
    if (!entry.isFile() || !entry.name.endsWith('.js')) continue
    const path = resolve(scriptsDir, entry.name)
    const code = readFileSync(path, 'utf8')
    // Word boundary on the left.
    const patched = code.replace(/(^|[^\w.-])\/_assets\/([\w.-]+)/g, (match, prefix, file) =>
      moved.has(file) ? `${prefix}${moved.get(file)}` : match,
    )
    if (patched !== code) writeFileSync(path, patched)
  }
}

const args = process.argv.slice(2)
// --no-minify: build unminified
const noMinify = args.includes('--no-minify')
// --bundle: inline the whole module tree into a single scripts/main.js
const bundle = args.includes('--bundle')
const only = args.find((a) => !a.startsWith('-')) // optional: build one app
const apps = discoverApps().filter((a) => !only || a === only)

if (apps.length === 0) {
  console.error(only ? `App "${only}" not found` : 'No apps found')
  process.exit(1)
}

for (const app of apps) {
  console.log(`\n▶ Building ${app}`)
  const appDir = resolve(appsDir, app)
  const outDir = resolve(outRoot, app)

  const manifest = validateManifest(resolve(appDir, 'appmeta/manifest.json'), app)

  // vite only empties scripts/.
  rmSync(outDir, { recursive: true, force: true })

  execFileSync('pnpm', ['exec', 'vite', 'build', '--config', resolve(platform, 'vite.config.ts')], {
    stdio: 'inherit',
    cwd: root,
    env: {
      ...process.env,
      APP: app,
      ...(noMinify && { NO_MINIFY: '1' }),
      ...(bundle && { BUNDLE: '1' }),
    },
  })

  if (!existsSync(resolve(outDir, 'scripts/main.js'))) {
    throw new Error(`${app}: build produced no scripts/main.js`)
  }

  // appmeta/: the manifest and, if present, the settings descriptor.
  cpSync(resolve(appDir, 'appmeta'), resolve(outDir, 'appmeta'), { recursive: true })
  if (manifest.settings && !existsSync(resolve(outDir, manifest.settings))) {
    throw new Error(`${app}: manifest.settings points at ${manifest.settings}, which is missing`)
  }

  // images/animations/sounds keep their folder; the rest go by extension.
  layoutResources(appDir, outDir, null)
  // Assets vite emitted land in scripts/_assets/; sort them out and drop it.
  const emitted = resolve(outDir, 'scripts/_assets')
  if (existsSync(emitted)) {
    const moved = layoutResources(emitted, outDir, null)
    rmSync(emitted, { recursive: true, force: true })
    // Code still refers to them by their old path.
    rewriteAssetUrls(resolve(outDir, 'scripts'), moved)
  }

  const scripts = readdirSync(resolve(outDir, 'scripts')).sort()
  console.log(`  ${manifest.id} v${manifest.version} — scripts: ${scripts.join(', ')}`)
}

console.log(`\n✔ Done: built ${apps.length} app(s)`)
