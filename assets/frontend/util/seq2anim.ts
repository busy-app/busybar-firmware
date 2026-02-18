
export type ColorMode = 'rgb888' | 'gray4';

export interface AnimationMeta {
  fps: number;
  colorMode: ColorMode;
  sections?: Array<{ name: string; start: number; end: number }>;
}

export type ComposeResult = Blob;

/** Compose animation from an array of browser `File` objects. */
export async function composeAnimation (
  files: File[],
  meta: AnimationMeta
): Promise<ComposeResult> {
  if (!files || files.length === 0) {
    throw new Error('No files provided');
  }

  // Filter to PNG files (case-insensitive)
  const pngFiles = files.filter(f => /\.png$/i.test(f.name));
  if (pngFiles.length === 0) {
    throw new Error('No PNG images found in provided files.');
  }

  pngFiles.sort(sortByFilename);

  // We'll decode the first image to determine width/height
  const firstBitmap = await decodeImageBitmapFromFile(pngFiles[0]);
  const width = firstBitmap.width;
  const height = firstBitmap.height;

  const framesData: Uint8Array[] = [];

  const canvas = document.createElement('canvas');
  // eslint-disable-next-line @typescript-eslint/no-explicit-any
  const ctx = canvas.getContext('2d', { willReadFrequently: true } as any) as CanvasRenderingContext2D | null;
  if (!ctx) {
    throw new Error('Unable to create 2D canvas context');
  }
  canvas.width = width;
  canvas.height = height;

  async function processFileToFrame (file: File): Promise<Uint8Array> {
    const bitmap = await decodeImageBitmapFromFile(file);
    if (bitmap.width !== width || bitmap.height !== height) {
      throw new Error(`Image ${file.name} dimensions ${bitmap.width}x${bitmap.height} do not match first image ${width}x${height}`);
    }

    if (!ctx) {
      throw new Error('2D canvas context not available');
    }

    ctx.clearRect(0, 0, width, height);
    ctx.drawImage(bitmap, 0, 0, width, height);
    const imgData = ctx.getImageData(0, 0, width, height);
    const rgba = imgData.data; // Uint8ClampedArray, RGBA

    return packFrame(rgba, width, height, meta.colorMode);
  }

  for (const f of pngFiles) {
    const frame = await processFileToFrame(f);
    framesData.push(frame); // This is raw packed data (BGR or Gray4)
  }

  // Encode Frames
  const encodedFrames: FileFrame[] = [];
  let framesChunkLen = 0;
  let maxEncodedLen = 0;
  let lastFrame: Uint8Array | null = null;

  const blockSize = meta.colorMode === 'rgb888' ? 3 : 1;

  for (let i = 0; i < framesData.length; i++) {
    const frame = framesData[i];

    if (lastFrame && areArraysEqual(frame, lastFrame)) {
      encodedFrames[encodedFrames.length - 1].duration++;
      continue;
    }

    lastFrame = frame;
    const encoded = compress(frame, blockSize);

    let fileFrame: FileFrame;
    // Should we use compressed or raw?
    if (encoded.length < frame.length) {
      fileFrame = { encoding: 1, duration: 1, encoded: encoded };
    } else {
      fileFrame = { encoding: 0, duration: 1, encoded: frame };
    }

    encodedFrames.push(fileFrame);
    framesChunkLen += getFileFrameLength(fileFrame);
    maxEncodedLen = Math.max(maxEncodedLen, fileFrame.encoded.length);
  }

  // Encode Sections
  // Default section "default" covering all frames
  const sectionsInput = meta.sections || [];
  const allSections = [
    { name: 'default', start: 0, end: framesData.length - 1 },
    ...sectionsInput
  ];

  const encodedSections: Section[] = [];
  let sectionsChunkLen = 0;

  for (let i = 0; i < allSections.length; i++) {
    const s = allSections[i];
    if (s.start < 0 || s.end >= framesData.length || s.start > s.end) {
      throw new Error(`Invalid section '${s.name}': range [${s.start}, ${s.end}] is out of bounds [0, ${framesData.length - 1}] or invalid`);
    }
    if (i > 0 && s.name === 'default') {
      throw new Error('Section name "default" is reserved');
    }

    const section: Section = {
      start: s.start,
      end: s.end,
      frameOffs: 0, // Filled later
      durationOverride: 0, // Filled later
      name: s.name
    };
    encodedSections.push(section);
    sectionsChunkLen += getSectionLength(section);
  }

  // Fill section precomputed start info
  const displayFrameStart: Array<{ offset: number; duration: number }> = [];
  const HEADER_LENGTH = 36;
  let fileFrameOffs = HEADER_LENGTH + sectionsChunkLen;

  for (const ff of encodedFrames) {
    const ffLen = getFileFrameLength(ff);
    for (let dispOffset = ff.duration; dispOffset > 0; dispOffset--) {
      displayFrameStart.push({ offset: fileFrameOffs, duration: dispOffset });
    }
    fileFrameOffs += ffLen;
  }

  // Update sections
  for (const section of encodedSections) {
    const info = displayFrameStart[section.start];
    section.frameOffs = info.offset;
    section.durationOverride = info.duration;
  }

  // Assemble file
  const totalSize = HEADER_LENGTH + sectionsChunkLen + framesChunkLen;
  const outBuf = new Uint8Array(totalSize);
  const view = new DataView(outBuf.buffer);

  // Header
  let ptr = 0;
  // Signature "bicycle0"
  const sig = new TextEncoder().encode('bicycle0');
  outBuf.set(sig, ptr);
  ptr += 8;

  view.setUint8(ptr++, 0); // flags
  view.setUint8(ptr++, width);
  view.setUint8(ptr++, height);
  view.setUint8(ptr++, meta.colorMode === 'rgb888' ? 0 : 1);

  view.setUint8(ptr++, meta.fps);
  view.setUint16(ptr, maxEncodedLen, true);
  ptr += 2;
  view.setUint8(ptr++, 0); // padding

  view.setUint32(ptr, sectionsChunkLen, true);
  ptr += 4;
  view.setUint32(ptr, framesChunkLen, true);
  ptr += 4;

  view.setUint32(ptr, encodedSections.length, true);
  ptr += 4;
  view.setUint32(ptr, encodedFrames.length, true);
  ptr += 4;
  view.setUint32(ptr, framesData.length, true);
  ptr += 4; // display_frame_count

  // Sections
  for (const s of encodedSections) {
    view.setUint32(ptr, s.start, true);
    ptr += 4;
    view.setUint32(ptr, s.end, true);
    ptr += 4;
    view.setUint32(ptr, s.frameOffs, true);
    ptr += 4;
    view.setUint8(ptr++, s.durationOverride);

    const nameBytes = new TextEncoder().encode(s.name);
    outBuf.set(nameBytes, ptr);
    ptr += nameBytes.length;
    view.setUint8(ptr++, 0); // null terminator
  }

  // Frames
  for (const f of encodedFrames) {
    view.setUint8(ptr++, f.encoding);
    view.setUint8(ptr++, f.duration);
    view.setUint16(ptr, f.encoded.length, true);
    ptr += 2;
    outBuf.set(f.encoded, ptr);
    ptr += f.encoded.length;
  }

  return new Blob([outBuf], { type: 'application/octet-stream' });
}

// Helpers

interface FileFrame {
  encoding: number;
  duration: number;
  encoded: Uint8Array;
}

interface Section {
  start: number;
  end: number;
  frameOffs: number;
  durationOverride: number;
  name: string;
}

function getFileFrameLength (f: FileFrame): number {
  // 1 (encoding) + 1 (duration) + 2 (len) + encoded_len
  return 4 + f.encoded.length;
}

function getSectionLength (s: Section): number {
  // 4 (start) + 4 (end) + 4 (frameOffs) + 1 (durationOverride) + name_len + 1 (null)
  const nameLen = new TextEncoder().encode(s.name).length;
  return 13 + nameLen + 1;
}

function areArraysEqual (a: Uint8Array, b: Uint8Array): boolean {
  if (a.length !== b.length) {
    return false;
  }
  for (let i = 0; i < a.length; i++) {
    if (a[i] !== b[i]) {
      return false;
    }
  }
  return true;
}

function packFrame (rgba: Uint8ClampedArray, width: number, height: number, mode: ColorMode): Uint8Array {
  if (mode === 'rgb888') {
    const out = new Uint8Array(width * height * 3);
    let ptr = 0;
    for (let i = 0; i < rgba.length; i += 4) {
      // RGBA -> BGR
      out[ptr++] = rgba[i + 2]; // B
      out[ptr++] = rgba[i + 1]; // G
      out[ptr++] = rgba[i]; // R
    }
    return out;
  } else {
    // gray4
    // Logic: Take R channel. Pack 2 pixels per byte.
    // P1 high nibble, P2 low nibble.
    // Assuming width * height is even.
    const totalPixels = width * height;
    const out = new Uint8Array(Math.ceil(totalPixels / 2));
    let ptr = 0;
    for (let i = 0; i < rgba.length; i += 8) {
      // Process 2 pixels at a time: i and i+4
      const r1 = rgba[i];
      const r2 = (i + 4 < rgba.length) ? rgba[i + 4] : 0;

      const px1 = r1 & 0xF0;
      const px2 = r2 & 0xF0;
      out[ptr++] = px1 | (px2 >> 4);
    }
    return out;
  }
}

function compress (source: Uint8Array, blkSize: number): Uint8Array {
  return compressBody(source, blkSize);
}

function compressBody (source: Uint8Array, blkSize: number): Uint8Array {
  const MAX_BLOCKS_PER_BYTE = 127;
  const RLE_BLOCK_THRESHOLD = 3;

  let srcI = 0;
  const srcLen = source.length;
  // Using simple array for dest then convert to Uint8Array
  const dest: number[] = [];

  // Helper to compare blocks
  const blockEq = (idx1: number, idx2: number) => {
    if (idx1 + blkSize > srcLen || idx2 + blkSize > srcLen) {
      return false;
    }
    for (let k = 0; k < blkSize; k++) {
      if (source[idx1 + k] !== source[idx2 + k]) {
        return false;
      }
    }
    return true;
  };

  while (srcI < srcLen) {
    let repeatCount = 0;
    for (let i = srcI; i < srcLen; i += blkSize) {
      if (blockEq(i, srcI)) {
        repeatCount++;
      } else {
        break;
      }
    }
    repeatCount = Math.min(repeatCount, MAX_BLOCKS_PER_BYTE);

    if (repeatCount === 0) {
      break;
    }

    if (repeatCount < RLE_BLOCK_THRESHOLD) {
      repeatCount = 0;
      let verbatimCount = 0;
      for (let i = srcI; i < srcLen; i += blkSize) {
        if (blockEq(i, i + blkSize)) {
          repeatCount++;
          if (repeatCount > RLE_BLOCK_THRESHOLD) {
            break;
          }
        } else {
          verbatimCount += 1 + repeatCount;
          repeatCount = 0;
        }
      }
      verbatimCount += repeatCount;
      verbatimCount = Math.min(verbatimCount, MAX_BLOCKS_PER_BYTE);

      const opcode = 0x80 | verbatimCount;
      dest.push(opcode);
      // const end = srcI + (verbatimCount * blkSize);
      // for (let k = srcI; k < end; k++) dest.push(source[k]);
      // Explicit calc to avoid loop var mixup logic issue
      for (let k = 0; k < verbatimCount * blkSize; k++) {
        dest.push(source[srcI + k]);
      }
      srcI += verbatimCount * blkSize;
    } else {
      const opcode = repeatCount;
      dest.push(opcode);
      // push one block
      for (let k = 0; k < blkSize; k++) {
        dest.push(source[srcI + k]);
      }
      srcI += repeatCount * blkSize;
    }
  }

  return new Uint8Array(dest);
}

export async function decodeImageBitmapFromFile (file: File): Promise<ImageBitmap> {
  if (typeof window !== 'undefined' && window.createImageBitmap) {
    return await window.createImageBitmap(file);
  }
  const dataUrl = await fileToDataURL(file);
  return await new Promise<ImageBitmap>((resolve, reject) => {

    const img = new Image();
    img.onload = () => {
      const canvas = document.createElement('canvas');
      canvas.width = img.width;
      canvas.height = img.height;
      const ctx = canvas.getContext('2d');
      if (!ctx) {
        return reject(new Error('Ctx null'));
      }
      ctx.drawImage(img, 0, 0);
      resolve(canvas as unknown as ImageBitmap);
    };
    img.onerror = reject;
    img.src = dataUrl;
  });
}

export function fileToDataURL (file: File): Promise<string> {
  return new Promise((resolve, reject) => {
    const reader = new FileReader();
    reader.onload = () => resolve(String(reader.result));
    reader.onerror = reject;
    reader.readAsDataURL(file);
  });
}

export function sortByFilename (a: File, b: File): number {
  const ka = extractDigitsAsNumber(a.name);
  const kb = extractDigitsAsNumber(b.name);
  if (ka !== undefined && kb !== undefined) {
    return ka - kb;
  }
  if (ka !== undefined && kb === undefined) {
    return -1;
  }
  if (ka === undefined && kb !== undefined) {
    return 1;
  }
  return a.name.localeCompare(b.name, undefined, { numeric: true, sensitivity: 'base' });
}

export function extractDigitsAsNumber (name: string): number | undefined {
  const m = name.match(/\d/g);
  if (!m || m.length === 0) {
    return undefined;
  }
  const joined = m.join('');
  const n = parseInt(joined, 10);
  return Number.isNaN(n) ? undefined : n;
}
