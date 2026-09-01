// Sorts app resources into the package's target folders (see documentation/JSAppsSpec.dox.md → Package Format).

import { extname } from 'node:path'

/** Extension → destination folder inside the app package. */
const BY_EXTENSION = new Map([
  // images/
  ['.png', 'images'],
  ['.jpg', 'images'],
  ['.jpeg', 'images'],
  ['.gif', 'images'],
  ['.bmp', 'images'],
  ['.webp', 'images'],
  ['.svg', 'images'],
  ['.image', 'images'],
  ['.ico', 'images'],
  // animations/
  ['.anim', 'animations'],
  ['.animation', 'animations'],
  // sounds/ — .wav in firmware sources, .snd in built device assets
  ['.wav', 'sounds'],
  ['.snd', 'sounds'],
  ['.sng', 'sounds'],
  ['.mp3', 'sounds'],
  ['.ogg', 'sounds'],
  ['.rtttl', 'sounds'],
])

/** Source folders already named after their target. */
export const KNOWN_DIRS = ['images', 'animations', 'sounds']

/**
 * Picks the destination for a file. A known source folder wins over the extension; anything unrecognized goes to resources/. `warning` is set when the extension disagrees with the containing folder.
 */
export function classify(fileName, sourceDir) {
  const byExtension = BY_EXTENSION.get(extname(fileName).toLowerCase())

  if (sourceDir && KNOWN_DIRS.includes(sourceDir)) {
    const warning =
      byExtension === sourceDir
        ? null
        : byExtension
          ? `${fileName} is in ${sourceDir}/, but its extension says ${byExtension}/`
          : `${fileName} is in ${sourceDir}/, but its extension is unknown`
    return { target: sourceDir, warning }
  }

  return { target: byExtension ?? 'resources', warning: null }
}
