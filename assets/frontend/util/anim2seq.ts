import {
  ANIM_FILE_FRAME_HEADER_LENGTH,
  ANIM_FILE_HEADER_LENGTH,
  ANIM_FILE_SIGNATURE,
  ANIM_MASK_LONG_RUN_BITS,
  ANIM_MASK_LONG_RUN_MARKER,
  ANIM_MASK_SHORT_RUN_BITS,
  AnimMaskEncoding,
  AnimPixelEncoding
} from './seq2anim';

export interface AnimationFrame {
  imageData: ImageData;
  duration: number;
}

export interface DecodedAnimation {
  width: number;
  height: number;
  fps: number;
  frames: AnimationFrame[];
}

const LEGACY_SIGNATURE = 'bicycle0';
const LEGACY_HEADER_LENGTH = 36;
const LEGACY_FRAME_HEADER_LENGTH = 4;

const ColorFormat = {
  Bgr888: 0,
  Gray4: 1,
  Bgra8888: 2
} as const;

type ColorFormat = typeof ColorFormat[keyof typeof ColorFormat];

export function createAnimationFromFrames (frames: ImageData[], fps: number): DecodedAnimation {
  if (!frames.length) {
    throw new Error('No frames provided');
  }

  return {
    width: frames[0].width,
    height: frames[0].height,
    fps,
    frames: frames.map(imageData => ({ imageData, duration: 1 }))
  };
}

export function getAnimationDisplayFrameCount (animation: DecodedAnimation): number {
  return animation.frames.reduce((total, frame) => total + frame.duration, 0);
}

export function decodeAnimation (buffer: ArrayBuffer): DecodedAnimation {
  const bytes = new Uint8Array(buffer);

  if (bytes.length < 8) {
    throw new Error('File is too short to be an animation');
  }

  const signature = new TextDecoder().decode(bytes.subarray(0, 8));

  if (signature === ANIM_FILE_SIGNATURE) {
    return decodeInterframeAnimation(bytes);
  }

  if (signature === LEGACY_SIGNATURE) {
    return decodeLegacyAnimation(bytes);
  }

  throw new Error('Not an animation file');
}

function readColorFormat (value: number): ColorFormat {
  if (value !== ColorFormat.Bgr888 && value !== ColorFormat.Gray4 && value !== ColorFormat.Bgra8888) {
    throw new Error(`Unsupported color format ${value}`);
  }

  return value;
}

function getBlockSize (colorFormat: ColorFormat) {
  return colorFormat === ColorFormat.Bgr888 ? 3 : colorFormat === ColorFormat.Bgra8888 ? 4 : 1;
}

function getPackedLength (colorFormat: ColorFormat, pixelCount: number) {
  return colorFormat === ColorFormat.Gray4 ? Math.ceil(pixelCount / 2) : pixelCount * getBlockSize(colorFormat);
}

function decodeInterframeAnimation (bytes: Uint8Array): DecodedAnimation {
  if (bytes.length < ANIM_FILE_HEADER_LENGTH) {
    throw new Error('File is too short to be an animation');
  }

  const view = new DataView(bytes.buffer, bytes.byteOffset, bytes.byteLength);
  const width = view.getUint8(9);
  const height = view.getUint8(10);
  const colorFormat = readColorFormat(view.getUint8(11));
  const fps = view.getUint8(16);
  const sectionsChunkLength = view.getUint32(18, true);
  const framesChunkLength = view.getUint32(22, true);
  const frameCount = view.getUint32(30, true);
  const framesStart = ANIM_FILE_HEADER_LENGTH + sectionsChunkLength;
  const framesEnd = framesStart + framesChunkLength;

  if (framesEnd > bytes.length) {
    throw new Error('Animation file is truncated');
  }

  const pixelCount = width * height;
  const canvas = new Uint8ClampedArray(pixelCount * 4);

  for (let offset = 3; offset < canvas.length; offset += 4) {
    canvas[offset] = 255;
  }

  const frames: AnimationFrame[] = [];
  let ptr = framesStart;

  for (let index = 0; index < frameCount && ptr + ANIM_FILE_FRAME_HEADER_LENGTH <= framesEnd; index++) {
    const jointEncoding = view.getUint8(ptr);
    const maskBits = view.getUint16(ptr + 1, true);
    const pixelLength = view.getUint16(ptr + 3, true);
    const maskStart = ptr + ANIM_FILE_FRAME_HEADER_LENGTH;
    const pixelStart = maskStart + Math.ceil(maskBits / 8);

    ptr = pixelStart + pixelLength;

    if (ptr > framesEnd) {
      throw new Error(`Frame ${index} is truncated`);
    }

    const pixels = decodeFramePixels(bytes.subarray(pixelStart, ptr), jointEncoding & 0x0F, colorFormat, pixelCount);
    const availablePixels = pixels.length / 4;
    let sourcePixel = 0;

    iterateMask(bytes.subarray(maskStart, pixelStart), maskBits, jointEncoding >> 4, pixelCount, (start, count) => {
      const placed = Math.min(count, availablePixels - sourcePixel);

      if (placed > 0) {
        canvas.set(pixels.subarray(sourcePixel * 4, (sourcePixel + placed) * 4), start * 4);
        sourcePixel += placed;
      }
    });

    frames.push({
      imageData: new ImageData(new Uint8ClampedArray(canvas), width, height),
      duration: 1
    });
  }

  if (!frames.length) {
    throw new Error('Animation has no frames');
  }

  return { width, height, fps, frames };
}

function createBitReader (data: Uint8Array, bitLength: number) {
  let position = 0;

  return {
    done: () => position >= bitLength,
    read (width: number) {
      let value = 0;

      for (let i = 0; i < width && position < bitLength; i++) {
        value = (value << 1) | ((data[position >> 3] >> (7 - (position & 7))) & 1);
        position++;
      }

      return value;
    }
  };
}

function iterateMask (
  data: Uint8Array,
  bitLength: number,
  encoding: number,
  pixelCount: number,
  place: (start: number, count: number) => void
) {
  const placeClipped = (start: number, count: number) => {
    const clipped = Math.min(count, pixelCount - start);

    if (clipped > 0) {
      place(start, clipped);
    }
  };

  if (encoding === AnimMaskEncoding.FullyBlack) {
    return;
  }

  if (encoding === AnimMaskEncoding.FullyWhite) {
    place(0, pixelCount);
    return;
  }

  const reader = createBitReader(data, bitLength);
  let index = 0;

  if (encoding === AnimMaskEncoding.Bitmap) {
    while (!reader.done()) {
      if (reader.read(1)) {
        placeClipped(index, 1);
      }
      index++;
    }
    return;
  }

  if (encoding !== AnimMaskEncoding.RleFirstBlack && encoding !== AnimMaskEncoding.RleFirstWhite) {
    throw new Error(`Unsupported mask encoding ${encoding}`);
  }

  let isWhite = encoding === AnimMaskEncoding.RleFirstWhite;

  while (!reader.done()) {
    let runLength = reader.read(ANIM_MASK_SHORT_RUN_BITS);

    if (runLength === ANIM_MASK_LONG_RUN_MARKER) {
      runLength = reader.read(ANIM_MASK_LONG_RUN_BITS);
    }

    if (isWhite) {
      placeClipped(index, runLength);
    }

    index += runLength;
    isWhite = !isWhite;
  }
}

function decodeFramePixels (data: Uint8Array, encoding: number, colorFormat: ColorFormat, pixelCount: number): Uint8Array {
  if (encoding === AnimPixelEncoding.QoiLike) {
    return decompressQoi(data, pixelCount);
  }

  if (encoding === AnimPixelEncoding.Rle) {
    return unpackPixels(decompressRle(data, getBlockSize(colorFormat), getPackedLength(colorFormat, pixelCount)), colorFormat);
  }

  if (encoding === AnimPixelEncoding.Raw) {
    return unpackPixels(data, colorFormat);
  }

  throw new Error(`Unsupported pixel encoding ${encoding}`);
}

function decompressQoi (data: Uint8Array, pixelCount: number): Uint8Array {
  const output = new Uint8Array(pixelCount * 4);
  const hashLut = new Uint32Array(64);
  let r = 0;
  let g = 0;
  let b = 0;
  let a = 255;
  let written = 0;

  const emit = () => {
    hashLut[(r * 3 + g * 5 + b * 7 + a * 11) % 64] = ((r << 24) | (g << 16) | (b << 8) | a) >>> 0;

    if (written < pixelCount) {
      const offset = written * 4;
      output[offset] = r;
      output[offset + 1] = g;
      output[offset + 2] = b;
      output[offset + 3] = a;
    }

    written++;
  };

  for (let i = 0; i < data.length; i++) {
    const tag = data[i];
    const payload = tag & 0x3F;

    if (tag === 0xFE) {
      if (i + 3 >= data.length) {
        break;
      }
      r = data[i + 1];
      g = data[i + 2];
      b = data[i + 3];
      i += 3;
      emit();
    } else if (tag === 0xFF) {
      if (i + 4 >= data.length) {
        break;
      }
      r = data[i + 1];
      g = data[i + 2];
      b = data[i + 3];
      a = data[i + 4];
      i += 4;
      emit();
    } else if ((tag & 0xC0) === 0x00) {
      const key = hashLut[payload];
      r = key >>> 24;
      g = (key >> 16) & 0xFF;
      b = (key >> 8) & 0xFF;
      a = key & 0xFF;
      emit();
    } else if ((tag & 0xC0) === 0x40) {
      r = (r + ((payload >> 4) & 0x03) - 2) & 0xFF;
      g = (g + ((payload >> 2) & 0x03) - 2) & 0xFF;
      b = (b + (payload & 0x03) - 2) & 0xFF;
      emit();
    } else if ((tag & 0xC0) === 0x80) {
      if (i + 1 >= data.length) {
        break;
      }
      const dg = payload - 32;
      const next = data[++i];
      r = (r + dg + ((next >> 4) & 0x0F) - 8) & 0xFF;
      g = (g + dg) & 0xFF;
      b = (b + dg + (next & 0x0F) - 8) & 0xFF;
      emit();
    } else {
      for (let run = 0; run <= payload; run++) {
        emit();
      }
    }
  }

  return output.subarray(0, Math.min(written, pixelCount) * 4);
}

function decompressRle (source: Uint8Array, blockSize: number, maxLength: number): Uint8Array {
  const output = new Uint8Array(maxLength);
  let sourceIndex = 0;
  let outputIndex = 0;

  while (sourceIndex < source.length && outputIndex < maxLength) {
    const opcode = source[sourceIndex++];
    const count = opcode & 0x7F;

    if (opcode & 0x80) {
      const length = Math.min(count * blockSize, maxLength - outputIndex, source.length - sourceIndex);
      output.set(source.subarray(sourceIndex, sourceIndex + length), outputIndex);
      sourceIndex += length;
      outputIndex += length;
      continue;
    }

    for (let repeat = 0; repeat < count && outputIndex < maxLength; repeat++) {
      for (let k = 0; k < blockSize && outputIndex < maxLength; k++) {
        output[outputIndex++] = source[sourceIndex + k];
      }
    }

    sourceIndex += blockSize;
  }

  return output.subarray(0, outputIndex);
}

function unpackPixels (packed: Uint8Array, colorFormat: ColorFormat): Uint8Array {
  if (colorFormat === ColorFormat.Gray4) {
    const output = new Uint8Array(packed.length * 8);

    for (let i = 0; i < packed.length; i++) {
      const left = (packed[i] >> 4) * 17;
      const right = (packed[i] & 0x0F) * 17;
      const offset = i * 8;
      output[offset] = left;
      output[offset + 1] = left;
      output[offset + 2] = left;
      output[offset + 3] = 255;
      output[offset + 4] = right;
      output[offset + 5] = right;
      output[offset + 6] = right;
      output[offset + 7] = 255;
    }

    return output;
  }

  const blockSize = getBlockSize(colorFormat);
  const pixelCount = Math.floor(packed.length / blockSize);
  const output = new Uint8Array(pixelCount * 4);

  for (let pixel = 0; pixel < pixelCount; pixel++) {
    const source = pixel * blockSize;
    const offset = pixel * 4;
    output[offset] = packed[source + 2];
    output[offset + 1] = packed[source + 1];
    output[offset + 2] = packed[source];
    output[offset + 3] = blockSize === 4 ? packed[source + 3] : 255;
  }

  return output;
}

function decodeLegacyAnimation (bytes: Uint8Array): DecodedAnimation {
  if (bytes.length < LEGACY_HEADER_LENGTH) {
    throw new Error('File is too short to be an animation');
  }

  const view = new DataView(bytes.buffer, bytes.byteOffset, bytes.byteLength);
  const width = view.getUint8(9);
  const height = view.getUint8(10);
  const colorFormat = readColorFormat(view.getUint8(11));
  const fps = view.getUint8(12);
  const sectionsChunkLength = view.getUint32(16, true);
  const framesChunkLength = view.getUint32(20, true);
  const fileFrameCount = view.getUint32(28, true);
  const framesStart = LEGACY_HEADER_LENGTH + sectionsChunkLength;
  const framesEnd = framesStart + framesChunkLength;

  if (framesEnd > bytes.length) {
    throw new Error('Animation file is truncated');
  }

  const pixelCount = width * height;
  const frames: AnimationFrame[] = [];
  let ptr = framesStart;

  for (let index = 0; index < fileFrameCount && ptr + LEGACY_FRAME_HEADER_LENGTH <= framesEnd; index++) {
    const encoding = view.getUint8(ptr);
    const duration = view.getUint8(ptr + 1);
    const encodedLength = view.getUint16(ptr + 2, true);
    ptr += LEGACY_FRAME_HEADER_LENGTH;

    if (ptr + encodedLength > framesEnd) {
      throw new Error(`Frame ${index} is truncated`);
    }

    const encoded = bytes.subarray(ptr, ptr + encodedLength);
    ptr += encodedLength;

    const packed = encoding === AnimPixelEncoding.Rle
      ? decompressRle(encoded, getBlockSize(colorFormat), getPackedLength(colorFormat, pixelCount))
      : encoded;
    const rgba = new Uint8ClampedArray(pixelCount * 4);

    rgba.set(unpackPixels(packed, colorFormat).subarray(0, pixelCount * 4));

    frames.push({
      imageData: new ImageData(rgba, width, height),
      duration: Math.max(1, duration)
    });
  }

  if (!frames.length) {
    throw new Error('Animation has no frames');
  }

  return { width, height, fps, frames };
}
