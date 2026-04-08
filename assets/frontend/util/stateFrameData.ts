import type { StateFrameMessage } from '@/util/stateStreamMessage';

function decodeRunLength (source: Uint8Array, blockSize: number): Uint8Array {
  const output: number[] = [];

  for (let sourceIndex = 0; sourceIndex < source.length;) {
    const opcode = source[sourceIndex++];
    const blockCount = opcode & 0x7F;

    if (!blockCount) {
      continue;
    }

    if (opcode & 0x80) {
      const byteLength = blockCount * blockSize;
      const chunk = source.subarray(sourceIndex, sourceIndex + byteLength);
      output.push(...chunk);
      sourceIndex += byteLength;
      continue;
    }

    const block = source.subarray(sourceIndex, sourceIndex + blockSize);
    sourceIndex += blockSize;

    for (let count = 0; count < blockCount; count++) {
      output.push(...block);
    }
  }

  return new Uint8Array(output);
}

async function inflateData (source: Uint8Array): Promise<Uint8Array> {
  if (typeof DecompressionStream === 'undefined') {
    throw new Error('Deflate frame encoding is not supported in this browser');
  }

  const buffer = source.slice().buffer as ArrayBuffer;
  const stream = new Blob([buffer]).stream().pipeThrough(new DecompressionStream('deflate'));
  const decompressedBuffer = await new Response(stream).arrayBuffer();

  return new Uint8Array(decompressedBuffer);
}

function unpackL4 (source: Uint8Array): Uint8Array {
  const unpacked = new Uint8Array(source.length * 2);

  for (let index = 0; index < source.length; index++) {
    const value = source[index];
    unpacked[index * 2] = value & 0x0F;
    unpacked[index * 2 + 1] = value >> 4;
  }

  return unpacked;
}

function getPixelCount (frame: StateFrameMessage): number {
  return (frame.width ?? 0) * (frame.height ?? 0);
}

function getExpectedPackedLength (frame: StateFrameMessage): number {
  const pixelCount = getPixelCount(frame);

  switch (frame.pixelFormat) {
    case 'L4':
      return Math.ceil(pixelCount / 2);
    case 'L8':
      return pixelCount;
    case 'RGB888':
    default:
      return pixelCount * 3;
  }
}

function getExpectedDecodedLength (frame: StateFrameMessage): number {
  const pixelCount = getPixelCount(frame);

  switch (frame.pixelFormat) {
    case 'L4':
    case 'L8':
      return pixelCount;
    case 'RGB888':
    default:
      return pixelCount * 3;
  }
}

function getRunLengthBlockSizes (frame: StateFrameMessage): number[] {
  if (frame.screen === 'FRONT') {
    return [3];
  }

  if (frame.pixelFormat === 'L8') {
    return [1, 2];
  }

  return [2, 1];
}

function tryDecodeRunLength (source: Uint8Array, frame: StateFrameMessage): Uint8Array | null {
  const expectedLength = getExpectedPackedLength(frame);

  for (const blockSize of getRunLengthBlockSizes(frame)) {
    const decoded = decodeRunLength(source, blockSize);

    if (decoded.length === expectedLength) {
      return decoded;
    }
  }

  return null;
}

function finalizeFramePayload (frame: StateFrameMessage, data: Uint8Array): Uint8Array {
  const normalized = frame.pixelFormat === 'L4' ? unpackL4(data) : data;
  const expectedLength = getExpectedDecodedLength(frame);

  if (expectedLength > 0 && normalized.length !== expectedLength) {
    throw new Error(`Decoded frame length mismatch for ${frame.screen ?? 'UNKNOWN'} ${frame.pixelFormat ?? 'RGB888'}: expected ${expectedLength}, got ${normalized.length}`);
  }

  return normalized;
}

async function tryDecodeCandidates (
  frame: StateFrameMessage,
  source: Uint8Array,
  transforms: Array<() => Uint8Array | Promise<Uint8Array>>
): Promise<Uint8Array | null> {
  for (const transform of transforms) {
    try {
      const decoded = await transform();
      return finalizeFramePayload(frame, decoded);
    } catch {
      continue;
    }
  }

  return null;
}

export async function decodeFramePayload (frame: StateFrameMessage): Promise<Uint8Array> {
  const source = frame.data ?? new Uint8Array();

  const runLengthFallback = () => tryDecodeRunLength(source, frame) ?? decodeRunLength(source, getRunLengthBlockSizes(frame)[0]);
  const inflateThenRunLength = async () => {
    const inflated = await inflateData(source);
    return tryDecodeRunLength(inflated, frame) ?? decodeRunLength(inflated, getRunLengthBlockSizes(frame)[0]);
  };
  const runLengthThenInflate = async () => {
    const runLengthDecoded = tryDecodeRunLength(source, frame) ?? decodeRunLength(source, getRunLengthBlockSizes(frame)[0]);
    return await inflateData(runLengthDecoded);
  };

  const candidatesByEncoding: Record<NonNullable<StateFrameMessage['encoding']>, Array<() => Uint8Array | Promise<Uint8Array>>> = {
    PLAIN: [
      () => source,
      runLengthFallback,
      () => inflateData(source),
      inflateThenRunLength,
      runLengthThenInflate
    ],
    RUN_LENGTH: [
      runLengthFallback,
      () => source,
      inflateThenRunLength,
      runLengthThenInflate,
      () => inflateData(source)
    ],
    DEFLATE: [
      () => inflateData(source),
      inflateThenRunLength,
      runLengthThenInflate,
      runLengthFallback,
      () => source
    ],
    DEFLATE_RUN_LENGTH: [
      inflateThenRunLength,
      runLengthThenInflate,
      () => inflateData(source),
      runLengthFallback,
      () => source
    ]
  };

  const decoded = await tryDecodeCandidates(frame, source, candidatesByEncoding[frame.encoding ?? 'PLAIN']);

  if (decoded) {
    return decoded;
  }

  throw new Error(`Could not decode ${frame.screen ?? 'UNKNOWN'} frame (${frame.encoding ?? 'PLAIN'}, ${frame.pixelFormat ?? 'RGB888'}, ${source.length} bytes)`);
}
