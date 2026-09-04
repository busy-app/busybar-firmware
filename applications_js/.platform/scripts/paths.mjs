// Project layout, in one place. Apps sit at the project root, one folder per app;
// everything else — shared code, the dev hub, these scripts — lives in .platform/.
//
//   applications_js/
//   ├── app.busy.clock/     ← an app: main.ts + appmeta/
//   ├── app.busy.weather/
//   └── .platform/          ← shared/, dev/, scripts/, configs
//
// Imported by vite.config.ts and by the build scripts, so the two never drift apart.

import { existsSync, readdirSync, statSync } from 'node:fs'
import { resolve } from 'node:path'
import { fileURLToPath } from 'node:url'

/** .platform/ — shared code, the dev hub and the build scripts. */
export const platform = fileURLToPath(new URL('..', import.meta.url))
/** applications_js/ — holds the apps themselves. */
export const root = resolve(platform, '..')
/** Apps are the root's own folders. */
export const appsDir = root
/** Code the apps import as `@shared/*`. */
export const sharedDir = resolve(platform, 'shared')
/** Glyph maps generated from the firmware fonts. */
export const mapsDir = resolve(sharedDir, 'fontMaps')
/** The firmware's fonts, read in place. */
export const fontsDir = resolve(root, '../assets/shared/fonts')
/** Built packages land next to the firmware's other assets. */
export const outRoot = resolve(root, '../assets/applications_js')

/** Root entries that are never apps: .platform and other dot-dirs, node_modules. */
export function isAppCandidate(name) {
  return !name.startsWith('.') && name !== 'node_modules'
}

/** App entry point: main.ts or main.js. Returns the path, or null. */
export function findEntry(dir) {
  for (const name of ['main.ts', 'main.js']) {
    const path = resolve(dir, name)
    if (existsSync(path)) return path
  }
  return null
}

/** Every root folder that looks like an app, by name. */
export function discoverApps() {
  return readdirSync(appsDir).filter((name) => {
    if (!isAppCandidate(name)) return false
    const dir = resolve(appsDir, name)
    return statSync(dir).isDirectory() && findEntry(dir) !== null
  })
}
