
export type ColorMode = 'rgb888' | 'gray4';

export interface AnimationMeta {
  fps: number;
  colorMode: ColorMode;
  sections?: Array<{ name: string; start: number; end: number }>;
}

export type ComposeResult = Blob;

export const ANIM_FILE_SIGNATURE = 'bicycle1';
export const ANIM_FILE_HEADER_LENGTH = 34;
export const ANIM_FILE_SECTION_HEADER_LENGTH = 12;
export const ANIM_FILE_FRAME_HEADER_LENGTH = 5;
export const ANIM_FILE_MAX_FPS = 255;
export const ANIM_FILE_MAX_DIMENSION = 255;
export const ANIM_FILE_MAX_PIXEL_LENGTH = 0xFFFF;
export const ANIM_FILE_EXTENSION = '.anim';

export const ANIM_MASK_SHORT_RUN_BITS = 3;
export const ANIM_MASK_LONG_RUN_BITS = 8;
export const ANIM_MASK_LONG_RUN_MARKER = (1 << ANIM_MASK_SHORT_RUN_BITS) - 1;

export const AnimMaskEncoding = {
  FullyBlack: 0,
  FullyWhite: 1,
  RleFirstBlack: 2,
  RleFirstWhite: 3,
  Bitmap: 4
} as const;

export const AnimPixelEncoding = {
  Raw: 0,
  Rle: 1,
  QoiLike: 2
} as const;

const MASK_LONG_RUN_MAX = (1 << ANIM_MASK_LONG_RUN_BITS) - 1;
const MASK_RLE_MAX_RATIO = 0.75;
const RLE_MAX_BLOCKS_PER_BYTE = 127;
const RLE_BLOCK_THRESHOLD = 3;
const QOI_RUN_LIMIT = 0b11111110;

interface EncodedChunk {
  data: Uint8Array;
  encoding: number;
}

interface EncodedMask extends EncodedChunk {
  bits: number;
}

interface FrameDifference {
  pixels: Uint8Array;
  previous: Uint8Array | null;
  mask: Uint8Array;
}

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

  const canvas = document.createElement('canvas');
  // eslint-disable-next-line @typescript-eslint/no-explicit-any
  const ctx = canvas.getContext('2d', { willReadFrequently: true } as any) as CanvasRenderingContext2D | null;
  if (!ctx) {
    throw new Error('Unable to create 2D canvas context');
  }
  canvas.width = width;
  canvas.height = height;

  const frames: ImageData[] = [];

  for (const file of pngFiles) {
    const bitmap = await decodeImageBitmapFromFile(file);
    if (bitmap.width !== width || bitmap.height !== height) {
      throw new Error(`Image ${file.name} dimensions ${bitmap.width}x${bitmap.height} do not match first image ${width}x${height}`);
    }

    ctx.clearRect(0, 0, width, height);
    ctx.drawImage(bitmap, 0, 0, width, height);
    frames.push(ctx.getImageData(0, 0, width, height));
  }

  return composeAnimationFromFrames(frames, meta);
}

/**
 * Compose animation from already decoded RGBA frames of identical dimensions.
 * Output must stay byte-identical to scripts/seq2anim.py, the firmware reference encoder.
 */
export function composeAnimationFromFrames (
  frames: readonly ImageData[],
  meta: AnimationMeta
): ComposeResult {
  if (!frames || frames.length === 0) {
    throw new Error('No frames provided');
  }

  const width = frames[0].width;
  const height = frames[0].height;

  if (width > ANIM_FILE_MAX_DIMENSION || height > ANIM_FILE_MAX_DIMENSION) {
    throw new Error(`Frame dimensions ${width}x${height} exceed the ${ANIM_FILE_MAX_DIMENSION}px limit`);
  }

  const fps = Math.round(meta.fps);
  if (!Number.isFinite(fps) || fps < 1 || fps > ANIM_FILE_MAX_FPS) {
    throw new Error(`FPS must be between 1 and ${ANIM_FILE_MAX_FPS}`);
  }

  const sections = [
    { name: 'default', start: 0, end: frames.length - 1 },
    ...(meta.sections ?? [])
  ];

  sections.forEach((section, index) => {
    if (section.start < 0 || section.end >= frames.length || section.start > section.end) {
      throw new Error(`Invalid section '${section.name}': range [${section.start}, ${section.end}] is out of bounds [0, ${frames.length - 1}] or invalid`);
    }
    if (index > 0 && section.name === 'default') {
      throw new Error('Section name "default" is reserved');
    }
  });

  const keyframes = new Set(sections.map(section => section.start));
  const encodedFrames: Uint8Array[] = [];
  let framesChunkLength = 0;
  let maxMaskLength = 0;
  let maxPixelLength = 0;
  let previousPixels: Uint8Array | null = null;

  for (let index = 0; index < frames.length; index++) {
    const frame = frames[index];

    if (frame.width !== width || frame.height !== height) {
      throw new Error(`Frame ${index} dimensions ${frame.width}x${frame.height} do not match first frame ${width}x${height}`);
    }

    const pixels = reduceColor(frame.data, meta.colorMode);
    const difference = subtractPixels(pixels, keyframes.has(index) ? null : previousPixels);
    const encodedPixels = encodePixels(difference, meta.colorMode);
    const mask = encodeMask(difference.mask);

    if (encodedPixels.data.length > ANIM_FILE_MAX_PIXEL_LENGTH) {
      throw new Error(`Frame ${index} is too large to encode`);
    }

    const encoded = new Uint8Array(ANIM_FILE_FRAME_HEADER_LENGTH + mask.data.length + encodedPixels.data.length);
    const view = new DataView(encoded.buffer);

    view.setUint8(0, encodedPixels.encoding | (mask.encoding << 4));
    view.setUint16(1, mask.bits, true);
    view.setUint16(3, encodedPixels.data.length, true);
    encoded.set(mask.data, ANIM_FILE_FRAME_HEADER_LENGTH);
    encoded.set(encodedPixels.data, ANIM_FILE_FRAME_HEADER_LENGTH + mask.data.length);

    encodedFrames.push(encoded);
    framesChunkLength += encoded.length;
    maxMaskLength = Math.max(maxMaskLength, mask.bits);
    maxPixelLength = Math.max(maxPixelLength, encodedPixels.data.length);
    previousPixels = pixels;
  }

  const textEncoder = new TextEncoder();
  const sectionNames = sections.map(section => textEncoder.encode(section.name));
  const sectionsChunkLength = sectionNames.reduce((total, name) => total + ANIM_FILE_SECTION_HEADER_LENGTH + name.length + 1, 0);

  const frameOffsets: number[] = [];
  let fileLength = ANIM_FILE_HEADER_LENGTH + sectionsChunkLength;

  for (const encoded of encodedFrames) {
    frameOffsets.push(fileLength);
    fileLength += encoded.length;
  }

  const output = new Uint8Array(fileLength);
  const view = new DataView(output.buffer);

  output.set(textEncoder.encode(ANIM_FILE_SIGNATURE), 0);
  view.setUint8(8, 0);
  view.setUint8(9, width);
  view.setUint8(10, height);
  view.setUint8(11, meta.colorMode === 'rgb888' ? 0 : 1);
  view.setUint16(12, maxMaskLength, true);
  view.setUint16(14, maxPixelLength, true);
  view.setUint8(16, fps);
  view.setUint8(17, 0);
  view.setUint32(18, sectionsChunkLength, true);
  view.setUint32(22, framesChunkLength, true);
  view.setUint32(26, sections.length, true);
  view.setUint32(30, encodedFrames.length, true);

  let ptr = ANIM_FILE_HEADER_LENGTH;

  sections.forEach((section, index) => {
    view.setUint32(ptr, section.start, true);
    view.setUint32(ptr + 4, section.end, true);
    view.setUint32(ptr + 8, frameOffsets[section.start], true);
    output.set(sectionNames[index], ptr + ANIM_FILE_SECTION_HEADER_LENGTH);
    ptr += ANIM_FILE_SECTION_HEADER_LENGTH + sectionNames[index].length;
    output[ptr++] = 0;
  });

  for (const encoded of encodedFrames) {
    output.set(encoded, ptr);
    ptr += encoded.length;
  }

  return new Blob([output], { type: 'application/octet-stream' });
}

function reduceColor (rgba: Uint8ClampedArray, mode: ColorMode): Uint8Array {
  const reduced = new Uint8Array(rgba.length);

  for (let i = 0; i < rgba.length; i += 4) {
    if (mode === 'gray4') {
      const value = rgba[i] & 0xF0;
      reduced[i] = value;
      reduced[i + 1] = value;
      reduced[i + 2] = value;
    } else {
      reduced[i] = rgba[i];
      reduced[i + 1] = rgba[i + 1];
      reduced[i + 2] = rgba[i + 2];
    }
    reduced[i + 3] = 0xFF;
  }

  return reduced;
}

function subtractPixels (pixels: Uint8Array, previous: Uint8Array | null): FrameDifference {
  const pixelCount = pixels.length / 4;

  if (!previous) {
    return { pixels, previous: null, mask: new Uint8Array(pixelCount).fill(1) };
  }

  const mask = new Uint8Array(pixelCount);
  let changedCount = 0;

  for (let pixel = 0; pixel < pixelCount; pixel++) {
    const offset = pixel * 4;

    if (
      pixels[offset] !== previous[offset]
      || pixels[offset + 1] !== previous[offset + 1]
      || pixels[offset + 2] !== previous[offset + 2]
      || pixels[offset + 3] !== previous[offset + 3]
    ) {
      mask[pixel] = 1;
      changedCount++;
    }
  }

  const selected = new Uint8Array(changedCount * 4);
  const selectedPrevious = new Uint8Array(changedCount * 4);
  let ptr = 0;

  for (let pixel = 0; pixel < pixelCount; pixel++) {
    if (mask[pixel]) {
      const offset = pixel * 4;
      selected.set(pixels.subarray(offset, offset + 4), ptr);
      selectedPrevious.set(previous.subarray(offset, offset + 4), ptr);
      ptr += 4;
    }
  }

  return { pixels: selected, previous: selectedPrevious, mask };
}

function packPixels (pixels: Uint8Array, mode: ColorMode): Uint8Array {
  if (mode === 'rgb888') {
    const packed = new Uint8Array((pixels.length / 4) * 3);
    let ptr = 0;

    for (let i = 0; i < pixels.length; i += 4) {
      packed[ptr++] = pixels[i + 2];
      packed[ptr++] = pixels[i + 1];
      packed[ptr++] = pixels[i];
    }

    return packed;
  }

  const packed = new Uint8Array(Math.ceil(pixels.length / 8));
  let ptr = 0;

  for (let i = 0; i < pixels.length; i += 8) {
    const left = pixels[i] & 0xF0;
    const right = i + 4 < pixels.length ? pixels[i + 4] & 0xF0 : 0;
    packed[ptr++] = left | (right >> 4);
  }

  return packed;
}

function encodePixels (difference: FrameDifference, mode: ColorMode): EncodedChunk {
  const packed = packPixels(difference.pixels, mode);
  const candidates: EncodedChunk[] = [
    { data: packed, encoding: AnimPixelEncoding.Raw },
    { data: compressRle(packed, mode === 'rgb888' ? 3 : 1), encoding: AnimPixelEncoding.Rle }
  ];

  if (mode === 'rgb888' && difference.previous) {
    candidates.push({ data: compressQoi(difference.pixels), encoding: AnimPixelEncoding.QoiLike });
  }

  return candidates.reduce((best, candidate) => candidate.data.length < best.data.length ? candidate : best);
}

function createBitWriter () {
  const bytes: number[] = [];
  let length = 0;

  return {
    push (value: number, count: number) {
      for (let bit = count - 1; bit >= 0; bit--) {
        if ((length & 7) === 0) {
          bytes.push(0);
        }
        if ((value >> bit) & 1) {
          bytes[bytes.length - 1] |= 0x80 >> (length & 7);
        }
        length++;
      }
    },
    bitLength: () => length,
    toBytes: () => Uint8Array.from(bytes)
  };
}

function encodeMask (mask: Uint8Array): EncodedMask {
  const runs: number[] = [];
  let runValue = mask[0];
  let runLength = 0;

  for (const value of mask) {
    if (value === runValue) {
      runLength++;
    } else {
      runs.push(runLength);
      runValue = value;
      runLength = 1;
    }
  }
  runs.push(runLength);

  const firstIsWhite = mask[0] === 1;

  if (runs.length === 1) {
    return {
      data: new Uint8Array(0),
      bits: 0,
      encoding: firstIsWhite ? AnimMaskEncoding.FullyWhite : AnimMaskEncoding.FullyBlack
    };
  }

  const writer = createBitWriter();

  for (const run of runs) {
    let length = run;

    if (length >= ANIM_MASK_LONG_RUN_MARKER) {
      while (length >= MASK_LONG_RUN_MAX) {
        writer.push(ANIM_MASK_LONG_RUN_MARKER, ANIM_MASK_SHORT_RUN_BITS);
        writer.push(MASK_LONG_RUN_MAX, ANIM_MASK_LONG_RUN_BITS);
        length -= MASK_LONG_RUN_MAX;
        writer.push(0, ANIM_MASK_SHORT_RUN_BITS);
      }
      writer.push(ANIM_MASK_LONG_RUN_MARKER, ANIM_MASK_SHORT_RUN_BITS);
      writer.push(length, ANIM_MASK_LONG_RUN_BITS);
    } else {
      writer.push(length, ANIM_MASK_SHORT_RUN_BITS);
    }
  }

  if (writer.bitLength() <= mask.length * MASK_RLE_MAX_RATIO) {
    return {
      data: writer.toBytes(),
      bits: writer.bitLength(),
      encoding: firstIsWhite ? AnimMaskEncoding.RleFirstWhite : AnimMaskEncoding.RleFirstBlack
    };
  }

  const bitmap = new Uint8Array(Math.ceil(mask.length / 8));

  mask.forEach((value, index) => {
    if (value) {
      bitmap[index >> 3] |= 0x80 >> (index & 7);
    }
  });

  return { data: bitmap, bits: mask.length, encoding: AnimMaskEncoding.Bitmap };
}

function compressQoi (pixels: Uint8Array): Uint8Array {
  const hashLut = new Uint32Array(64);
  const output: number[] = [];
  let lastR = 0;
  let lastG = 0;
  let lastB = 0;
  let lastA = 0xFF;
  let lastOpcodeIsRun = false;

  for (let i = 0; i < pixels.length; i += 4) {
    const r = pixels[i];
    const g = pixels[i + 1];
    const b = pixels[i + 2];
    const a = pixels[i + 3];
    const key = ((r << 24) | (g << 16) | (b << 8) | a) >>> 0;
    const hash = (r * 3 + g * 5 + b * 7 + a * 11) % 64;

    if (r === lastR && g === lastG && b === lastB && a === lastA) {
      const lastIndex = output.length - 1;

      if (lastOpcodeIsRun && output[lastIndex] + 1 < QOI_RUN_LIMIT) {
        output[lastIndex] += 1;
      } else {
        output.push(0b11000000);
      }

      hashLut[hash] = key;
      lastOpcodeIsRun = true;
      continue;
    }

    lastOpcodeIsRun = false;

    if (hashLut[hash] === key) {
      output.push(hash);
    } else {
      const dr = r - lastR;
      const dg = g - lastG;
      const db = b - lastB;
      const drDg = dr - dg;
      const dbDg = db - dg;

      if (a === lastA && dr >= -2 && dr <= 1 && dg >= -2 && dg <= 1 && db >= -2 && db <= 1) {
        output.push(0b01000000 | ((dr + 2) << 4) | ((dg + 2) << 2) | (db + 2));
      } else if (a === lastA && dg >= -32 && dg <= 31 && drDg >= -8 && drDg <= 7 && dbDg >= -8 && dbDg <= 7) {
        output.push(0b10000000 | (dg + 32), ((drDg + 8) << 4) | (dbDg + 8));
      } else if (a === lastA) {
        output.push(0xFE, r, g, b);
      } else {
        output.push(0xFF, r, g, b, a);
      }

      hashLut[hash] = key;
    }

    lastR = r;
    lastG = g;
    lastB = b;
    lastA = a;
  }

  return Uint8Array.from(output);
}

function compressRle (source: Uint8Array, blockSize: number): Uint8Array {
  const sourceLength = source.length;
  const output: number[] = [];
  let sourceIndex = 0;

  const blocksEqual = (left: number, right: number) => {
    if (left + blockSize > sourceLength || right + blockSize > sourceLength) {
      return false;
    }
    for (let k = 0; k < blockSize; k++) {
      if (source[left + k] !== source[right + k]) {
        return false;
      }
    }
    return true;
  };

  while (sourceIndex < sourceLength) {
    let repeatCount = 0;

    for (let i = sourceIndex; i < sourceLength; i += blockSize) {
      if (!blocksEqual(i, sourceIndex)) {
        break;
      }
      repeatCount++;
    }

    repeatCount = Math.min(repeatCount, RLE_MAX_BLOCKS_PER_BYTE);

    if (repeatCount === 0) {
      break;
    }

    if (repeatCount < RLE_BLOCK_THRESHOLD) {
      let pendingRepeats = 0;
      let verbatimCount = 0;

      for (let i = sourceIndex; i < sourceLength; i += blockSize) {
        if (blocksEqual(i, i + blockSize)) {
          pendingRepeats++;
          if (pendingRepeats > RLE_BLOCK_THRESHOLD) {
            break;
          }
        } else {
          verbatimCount += 1 + pendingRepeats;
          pendingRepeats = 0;
        }
      }

      verbatimCount = Math.min(verbatimCount + pendingRepeats, RLE_MAX_BLOCKS_PER_BYTE);
      output.push(0x80 | verbatimCount);

      for (let k = 0; k < verbatimCount * blockSize; k++) {
        output.push(source[sourceIndex + k]);
      }

      sourceIndex += verbatimCount * blockSize;
    } else {
      output.push(repeatCount);

      for (let k = 0; k < blockSize; k++) {
        output.push(source[sourceIndex + k]);
      }

      sourceIndex += repeatCount * blockSize;
    }
  }

  return Uint8Array.from(output);
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
