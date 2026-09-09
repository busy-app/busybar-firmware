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
const ENTRY_SCRIPT_PATH = 'scripts/main.js';

export async function readAppPackage (file: File) {
  const entries = (await parseTarGzip(await file.arrayBuffer()))
    .map(entry => ({ ...entry, name: entry.name.replace(/^\.\//, '') }));

  const manifestEntry = entries.find(entry => entry.name.endsWith(`/${MANIFEST_PATH}`));
  if (!manifestEntry) {
    throw new Error(`${MANIFEST_PATH} is missing in the app package`);
  }

  const root = manifestEntry.name.slice(0, -(MANIFEST_PATH.length + 1));
  if (root.includes('/')) {
    throw new Error(`${MANIFEST_PATH} must be placed in the app root directory`);
  }

  const manifest = parseManifest(manifestEntry.text);
  if (manifest.id !== root) {
    throw new Error(`The app root directory "${root}" does not match the manifest id "${manifest.id}"`);
  }

  if (!entries.some(entry => entry.name === `${root}/${ENTRY_SCRIPT_PATH}`)) {
    throw new Error(`${ENTRY_SCRIPT_PATH} is missing in the app package`);
  }

  const iconEntry = entries.find(entry => entry.name === `${root}/${ICON_PATH}`);

  return {
    manifest,
    icon: iconEntry?.data ? toDataUrl(iconEntry.data, 'image/png') : undefined
  };
}

function parseManifest (text: string): AppManifest {
  const manifest = JSON.parse(text) as AppManifest;

  const isValid = typeof manifest.format_version === 'number'
    && isFilledString(manifest.id)
    && isFilledString(manifest.name)
    && isFilledString(manifest.version);

  if (!isValid) {
    throw new Error(`${MANIFEST_PATH} is missing required fields`);
  }

  return manifest;
}

function isFilledString (value: unknown) {
  return typeof value === 'string' && value.length > 0;
}

function toDataUrl (data: Uint8Array, type: string) {
  let binary = '';
  for (let i = 0; i < data.length; i++) {
    binary += String.fromCharCode(data[i]!);
  }

  return `data:${type};base64,${btoa(binary)}`;
}
