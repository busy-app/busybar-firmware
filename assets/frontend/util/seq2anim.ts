export type ComposeResult = Blob;

/** Compose animation from an array of browser `File` objects. */
export async function composeAnimation (
  files: File[],
  fps: number
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
  const bytesPerPixel = 3; // RGB only (alpha dropped)

  const framesData: Uint8Array[] = [];

  const canvas = document.createElement('canvas');
  const ctx = canvas.getContext('2d');
  if (!ctx) {
    throw new Error('Unable to create 2D canvas context');
  }

  async function processFileToFrame (file: File): Promise<Uint8Array> {
    const bitmap = await decodeImageBitmapFromFile(file);
    if (bitmap.width !== width || bitmap.height !== height) {
      throw new Error(`Image ${file.name} dimensions ${bitmap.width}x${bitmap.height} do not match first image ${width}x${height}`);
    }

    if (!ctx) {
      throw new Error('2D canvas context not available');
    }

    canvas.width = width;
    canvas.height = height;
    ctx.drawImage(bitmap, 0, 0, width, height);
    const imgData = ctx.getImageData(0, 0, width, height);
    const rgba = imgData.data; // Uint8ClampedArray, RGBA

    const rgbSwapped = new Uint8Array((width * height * bytesPerPixel) | 0);
    let out = 0;
    for (let i = 0; i < rgba.length; i += 4) {
      const r = rgba[i];
      const g = rgba[i + 1];
      const b = rgba[i + 2];
      // BGR
      rgbSwapped[out++] = b;
      rgbSwapped[out++] = g;
      rgbSwapped[out++] = r;
    }

    return rgbSwapped;
  }

  for (const f of pngFiles) {
    const frame = await processFileToFrame(f);
    framesData.push(frame);
  }

  const frames = framesData.length;

  const headerBuf = new ArrayBuffer(7 * 4);
  const headerView = new DataView(headerBuf);
  headerView.setUint32(0, 0x69, true); // magic
  headerView.setUint32(4, 0x00, true); // version
  headerView.setUint32(8, fps >>> 0, true);
  headerView.setUint32(12, bytesPerPixel >>> 0, true);
  headerView.setUint32(16, width >>> 0, true);
  headerView.setUint32(20, height >>> 0, true);
  headerView.setUint32(24, frames >>> 0, true);

  // total size
  const frameBytes = width * height * bytesPerPixel;
  const totalBytes = headerBuf.byteLength + frames * frameBytes;
  const outBuf = new Uint8Array(totalBytes);

  // copy header
  outBuf.set(new Uint8Array(headerBuf), 0);

  // copy frames
  let offset = headerBuf.byteLength;
  for (const fd of framesData) {
    outBuf.set(fd, offset);
    offset += fd.byteLength;
  }

  const blob = new Blob([outBuf.buffer], { type: 'application/octet-stream' });
  return blob;
}

export async function decodeImageBitmapFromFile (file: File): Promise<ImageBitmap> {
  if (window.createImageBitmap) {
    const blob = file;
    return await window.createImageBitmap(blob);
  }

  // fallback: HTMLImageElement
  const dataUrl = await fileToDataURL(file);
  return await new Promise<ImageBitmap>((resolve, reject) => {
    const img = new Image();
    img.onload = () => {
      // draw onto an offscreen canvas and create ImageBitmap
      const canvas = document.createElement('canvas');
      canvas.width = img.width;
      canvas.height = img.height;
      const ctx = canvas.getContext('2d');
      if (!ctx) {
        return reject(new Error('2D context not available'));
      }
      ctx.drawImage(img, 0, 0);
      if (window.createImageBitmap) {
        window
          .createImageBitmap(canvas)
          .then((b: ImageBitmap) => resolve(b))
          .catch(reject);
      } else {
        resolve((canvas as unknown) as ImageBitmap);
      }
    };
    img.onerror = () => reject(new Error('Failed to load image'));
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
  // fallback
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
