declare module 'gifenc' {
  export type GifPalette = number[][];

  export interface GifFrameOptions {
    palette?: GifPalette;
    delay?: number;
    repeat?: number;
    transparent?: boolean;
    transparentIndex?: number;
    dispose?: number;
    first?: boolean;
  }

  export interface GifEncoderInstance {
    writeFrame: (index: Uint8Array, width: number, height: number, options?: GifFrameOptions) => void;
    finish: () => void;
    bytes: () => Uint8Array;
    bytesView: () => Uint8Array;
    reset: () => void;
  }

  export function GIFEncoder (options?: { auto?: boolean; initialCapacity?: number }): GifEncoderInstance;
  export function quantize (
    rgba: Uint8Array | Uint8ClampedArray,
    maxColors: number,
    options?: { format?: string; oneBitAlpha?: boolean | number; clearAlpha?: boolean; clearAlphaThreshold?: number; clearAlphaColor?: number }
  ): GifPalette;
  export function applyPalette (rgba: Uint8Array | Uint8ClampedArray, palette: GifPalette, format?: string): Uint8Array;
}
