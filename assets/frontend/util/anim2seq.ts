import { ANIM_FILE_HEADER_LENGTH, ANIM_FILE_SIGNATURE } from './seq2anim';

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

const ColorFormat = {
  Bgr888: 0,
  Gray4: 1,
  Bgra8888: 2
} as const;

type ColorFormat = typeof ColorFormat[keyof typeof ColorFormat];

const FrameEncoding = {
  Raw: 0,
  Rle: 1
} as const;

type FrameEncoding = typeof FrameEncoding[keyof typeof FrameEncoding];

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
  const view = new DataView(buffer);

  if (bytes.length < ANIM_FILE_HEADER_LENGTH) {
    throw new Error('File is too short to be an animation');
  }

  const signature = new TextDecoder().decode(bytes.subarray(0, 8));

  if (signature !== ANIM_FILE_SIGNATURE) {
    throw new Error('Not an animation file');
  }

  const width = view.getUint8(9);
  const height = view.getUint8(10);
  const colorFormat = view.getUint8(11) as ColorFormat;
  const fps = view.getUint8(12);
  const sectionsChunkLength = view.getUint32(16, true);
  const framesChunkLength = view.getUint32(20, true);
  const fileFrameCount = view.getUint32(28, true);

  if (![ColorFormat.Bgr888, ColorFormat.Gray4, ColorFormat.Bgra8888].includes(colorFormat)) {
    throw new Error(`Unsupported color format ${colorFormat}`);
  }

  const framesStart = ANIM_FILE_HEADER_LENGTH + sectionsChunkLength;
  const framesEnd = framesStart + framesChunkLength;

  if (framesEnd > bytes.length) {
    throw new Error('Animation file is truncated');
  }

  const blockSize = colorFormat === ColorFormat.Bgr888 ? 3 : colorFormat === ColorFormat.Bgra8888 ? 4 : 1;
  const packedLength = colorFormat === ColorFormat.Gray4
    ? Math.ceil((width * height) / 2)
    : width * height * blockSize;

  const frames: AnimationFrame[] = [];
  let ptr = framesStart;

  for (let index = 0; index < fileFrameCount && ptr + 4 <= framesEnd; index++) {
    const encoding = view.getUint8(ptr) as FrameEncoding;
    const duration = view.getUint8(ptr + 1);
    const encodedLength = view.getUint16(ptr + 2, true);
    ptr += 4;

    if (ptr + encodedLength > framesEnd) {
      throw new Error(`Frame ${index} is truncated`);
    }

    const encoded = bytes.subarray(ptr, ptr + encodedLength);
    ptr += encodedLength;

    const packed = encoding === FrameEncoding.Rle
      ? decompress(encoded, blockSize, packedLength)
      : encoded;

    frames.push({
      imageData: unpackFrame(packed, width, height, colorFormat),
      duration: Math.max(1, duration)
    });
  }

  if (!frames.length) {
    throw new Error('Animation has no frames');
  }

  return { width, height, fps, frames };
}

function decompress (source: Uint8Array, blockSize: number, expectedLength: number): Uint8Array {
  const dest = new Uint8Array(expectedLength);
  let srcI = 0;
  let destI = 0;

  while (srcI < source.length && destI < expectedLength) {
    const opcode = source[srcI++];
    const count = opcode & 0x7F;

    if (opcode & 0x80) {
      const length = Math.min(count * blockSize, expectedLength - destI, source.length - srcI);
      dest.set(source.subarray(srcI, srcI + length), destI);
      srcI += length;
      destI += length;
      continue;
    }

    for (let repeat = 0; repeat < count && destI < expectedLength; repeat++) {
      for (let k = 0; k < blockSize && destI < expectedLength; k++) {
        dest[destI++] = source[srcI + k];
      }
    }

    srcI += blockSize;
  }

  return dest;
}

function unpackFrame (packed: Uint8Array, width: number, height: number, colorFormat: ColorFormat): ImageData {
  const imageData = new ImageData(width, height);
  const rgba = imageData.data;
  const totalPixels = width * height;

  if (colorFormat === ColorFormat.Gray4) {
    for (let pixel = 0; pixel < totalPixels; pixel++) {
      const byte = packed[pixel >> 1] ?? 0;
      const nibble = pixel % 2 === 0 ? byte >> 4 : byte & 0x0F;
      const value = nibble * 17;
      const offset = pixel * 4;

      rgba[offset] = value;
      rgba[offset + 1] = value;
      rgba[offset + 2] = value;
      rgba[offset + 3] = 255;
    }

    return imageData;
  }

  const blockSize = colorFormat === ColorFormat.Bgra8888 ? 4 : 3;

  for (let pixel = 0; pixel < totalPixels; pixel++) {
    const src = pixel * blockSize;
    const offset = pixel * 4;

    rgba[offset] = packed[src + 2] ?? 0;
    rgba[offset + 1] = packed[src + 1] ?? 0;
    rgba[offset + 2] = packed[src] ?? 0;
    rgba[offset + 3] = blockSize === 4 ? packed[src + 3] ?? 255 : 255;
  }

  return imageData;
}
