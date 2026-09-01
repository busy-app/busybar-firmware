// Dev-only resource upload, standing in for what the firmware does at install time. Collected from the whole app folder, keeping their source file names.

import { device } from '@shared/device'

// Non-source files of every app; only the running app's are fetched.
// Apps sit at the project root; .platform and node_modules are not apps.
const RESOURCES = import.meta.glob<string>(
  [
    '/*/**/*',
    '!/*/**/*.{ts,tsx,js,mjs,cjs,jsx}',
    '!/*/appmeta/**',
    '!/.platform/**',
    '!/node_modules/**',
  ],
  { query: '?url', import: 'default' },
)

/** App files: source path → URL loader. */
function resourcesOf(app: string): [string, () => Promise<string>][] {
  const prefix = `/${app}/`
  return Object.entries(RESOURCES).filter(([path]) => path.startsWith(prefix))
}

/** Names of files already present in the app's device assets. */
async function existing(app: string): Promise<Set<string>> {
  try {
    const { list } = await device.StorageListGet({ path: `/ext/user_assets/${app}` })
    return new Set(list.filter((e) => e.type === 'file').map((e) => e.name))
  } catch {
    // No directory yet on first run.
    return new Set()
  }
}

/** Uploads the app's resources, skipping existing ones. Returns the count. */
export async function uploadResources(
  folder: string,
  appId: string,
  log: (msg: string) => void = () => {},
): Promise<number> {
  // Looked up by folder name, uploaded under the device name.
  const resources = resourcesOf(folder)
  if (resources.length === 0) return 0

  const present = await existing(appId)
  let uploaded = 0

  for (const [path, load] of resources) {
    const file = path.slice(path.lastIndexOf('/') + 1)
    if (present.has(file)) continue
    try {
      const data = await (await fetch(await load())).arrayBuffer()
      await device.AssetsUpload({ application_name: appId, file, data })
      uploaded++
    } catch (err) {
      log(`  ⚠ ${file}: ${err instanceof Error ? err.message : String(err)}`)
    }
  }

  return uploaded
}
