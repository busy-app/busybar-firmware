import { basename, extname, resolve } from 'node:path'
import { defineConfig, type Plugin } from 'vite'
// @ts-expect-error — untyped .mjs, build-only plugin.
import { fontMaps } from './scripts/vite-font-maps.mjs'
// @ts-expect-error — untyped .mjs, shared with the build scripts.
import { appsDir, findEntry, outRoot, root, sharedDir } from './scripts/paths.mjs'

// Set in build mode only; unset in dev, where the hub is served.
const app = process.env.APP
// Inlines the whole module tree into a single scripts/main.js.
const bundle = Boolean(process.env.BUNDLE)

// Rewrites `export{X as default}` into `X();` for JerryScript, which runs main.js as a plain script.
function callDefaultExport(): Plugin {
  return {
    name: 'call-default-export',
    renderChunk(code, chunk) {
      if (!chunk.isEntry) return null
      // Emitted as the chunk's last statement, in either form.
      const re = /export\s*(?:\{\s*([A-Za-z_$][\w$]*)\s+as\s+default\s*\}|default\s+([A-Za-z_$][\w$]*)\s*)\s*;?\s*$/
      const match = code.match(re)
      if (!match) {
        this.error(
          'call-default-export: no default export at the end of main.js. ' +
            'The app must have `export default function run()`.',
        )
      }
      const name = match![1] ?? match![2]
      return { code: code.replace(re, `${name}();\n`), map: null }
    },
  }
}

export default defineConfig(({ command }) => {
  // dev: serves the hub listing all apps.
  if (command === 'serve') {
    return {
      root,
      // The hub page lives in .platform/; the root holds apps only.
      server: { open: '/.platform/index.html' },
      // .anim files are binary device assets, served as URLs.
      assetsInclude: ['**/*.anim'],
      plugins: [fontMaps()],
      // Keeps the ?v= hash stable across src edits.
      optimizeDeps: { include: ['@busy-app/busy-lib'] },
      resolve: { alias: { '@shared': sharedDir } },
    }
  }

  // build: one app (APP=<name>) into <outRoot>/<name>/scripts/main.js.
  if (!app) {
    throw new Error('build: APP is not set. Use pnpm build / pnpm build:app <name>')
  }
  const appDir = resolve(appsDir, app)
  // Apps may be written in either.
  const entry = findEntry(appDir)
  if (!entry) {
    throw new Error(`build: ${appDir} has neither main.ts nor main.js`)
  }

  return {
    // .anim files are binary device assets, served as URLs.
    assetsInclude: ['**/*.anim'],
    plugins: [fontMaps(), callDefaultExport()],
    build: {
      // build.mjs lays out resources and appmeta.
      outDir: resolve(outRoot, app, 'scripts'),
      emptyOutDir: true,
      target: 'es2020',
      minify: process.env.NO_MINIFY ? false : 'oxc',
      rollupOptions: {
        input: entry,
        // The entry export must survive tree-shaking.
        preserveEntrySignatures: 'strict',
        output: {
          entryFileNames: 'main.js',
          // No hashes: the runtime resolves relative paths as-is.
          chunkFileNames: '[name].js',
          // inlineDynamicImports and manualChunks are mutually exclusive.
          ...(bundle
            ? { inlineDynamicImports: true }
            : {
                // Each app module gets its own file next to main.js; external deps are inlined into the chunk using them.
                manualChunks(id: string) {
                  if (!id.startsWith(appDir)) return undefined
                  if (id === entry) return undefined
                  // Sources only; assets take their own path.
                  if (!/\.(ts|js)$/.test(id)) return undefined
                  // Chunk name = module file name: time.ts → time.js
                  return basename(id, extname(id))
                },
              }),
          // Temp folder inside outDir; build.mjs spreads it out and removes it.
          assetFileNames: '_assets/[name][extname]',
        },
      },
    },
    resolve: { alias: { '@shared': sharedDir } },
  }
})
