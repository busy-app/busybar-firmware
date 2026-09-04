import { parseTarGzip } from 'nanotar';

export interface AppManifest {
  format_version: number;
  id: string;
  name: string;
  version: string;
  description?: string;
  author?: string;
  heap_size_kib?: number;
  debug?: boolean;
}

export interface AppPackage {
  manifest: AppManifest;
  icon?: string;
}

const MANIFEST_PATH = 'appmeta/manifest.json';
const ICON_PATH = 'appmeta/icon_front_8x8.png';

export async function readAppPackage (file: File) {
  const entries = await parseTarGzip(await file.arrayBuffer());

  const manifestEntry = entries.find(entry => entry.name.endsWith(MANIFEST_PATH));
  if (!manifestEntry) {
    throw new Error(`${MANIFEST_PATH} is missing in the app package`);
  }

  const manifest = JSON.parse(manifestEntry.text) as AppManifest;
  if (!manifest.id || !manifest.name || !manifest.version) {
    throw new Error(`${MANIFEST_PATH} is missing required fields`);
  }

  const iconEntry = entries.find(entry => entry.name.endsWith(ICON_PATH));

  return {
    manifest,
    icon: iconEntry?.data ? toDataUrl(iconEntry.data, 'image/png') : undefined
  };
}

function toDataUrl (data: Uint8Array, type: string) {
  let binary = '';
  for (let i = 0; i < data.length; i++) {
    binary += String.fromCharCode(data[i]!);
  }

  return `data:${type};base64,${btoa(binary)}`;
}
