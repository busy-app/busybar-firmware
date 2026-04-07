export type QrCodeMatrix = boolean[][];

type DrawQrCodeOptions = {
  size?: number;
  marginModules?: number;
  darkColor?: string;
  lightColor?: string;
};

const DEFAULT_QR_CODE_SIZE = 184;
const DEFAULT_QR_MARGIN_MODULES = 4;

export function drawQrCodeOnCanvas (
  canvas: HTMLCanvasElement,
  matrix: QrCodeMatrix | null,
  options: DrawQrCodeOptions = {}
): void {
  const {
    size = DEFAULT_QR_CODE_SIZE,
    marginModules = DEFAULT_QR_MARGIN_MODULES,
    darkColor = '#000000',
    lightColor = '#FFFFFF'
  } = options;

  const context = canvas.getContext('2d');
  if (!context) {
    return;
  }

  const devicePixelRatio = window.devicePixelRatio || 1;

  canvas.width = Math.round(size * devicePixelRatio);
  canvas.height = Math.round(size * devicePixelRatio);
  canvas.style.width = `${size}px`;
  canvas.style.height = `${size}px`;

  context.setTransform(devicePixelRatio, 0, 0, devicePixelRatio, 0, 0);

  context.clearRect(0, 0, size, size);
  context.fillStyle = lightColor;
  context.fillRect(0, 0, size, size);

  if (!matrix?.length || !matrix[0]?.length) {
    return;
  }

  const moduleCount = matrix.length;
  const totalModules = moduleCount + marginModules * 2;
  const moduleSize = size / totalModules;
  const offset = 0;

  context.imageSmoothingEnabled = false;
  context.fillStyle = darkColor;

  for (let rowIndex = 0; rowIndex < matrix.length; rowIndex++) {
    const row = matrix[rowIndex];

    for (let columnIndex = 0; columnIndex < row.length; columnIndex++) {
      if (!row[columnIndex]) {
        continue;
      }

      context.fillRect(
        offset + (columnIndex + marginModules) * moduleSize,
        offset + (rowIndex + marginModules) * moduleSize,
        moduleSize,
        moduleSize
      );
    }
  }
}
