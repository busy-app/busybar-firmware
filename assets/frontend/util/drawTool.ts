import Konva from 'konva';

export const WORKSPACE_WIDTH = 72;
export const WORKSPACE_HEIGHT = 16;
export const STAGE_PADDING_X = 20;
export const STAGE_PADDING_Y = 28;
export const MIN_STAGE_WIDTH = 320;
export const MIN_STAGE_HEIGHT = 400;
export const HISTORY_LIMIT = 10;
export const ROTATION_SNAP_STEP = 15;
export const DEFAULT_WORKSPACE_BACKGROUND = '#181818';
export const DEFAULT_GRID_COLOR = '#000000';
export const DEFAULT_GRID_BORDER_COLOR = '#262626';
export const DEFAULT_BACKGROUND_COLOR = '#00000000';
export const DEFAULT_BORDER_COLOR = '#00000000';
export const DEFAULT_BORDER_DASH_SIZE = 4;
export const DEFAULT_BORDER_GAP_SIZE = 0;
export const DEFAULT_BORDER_GAP_OFFSET = 0;
export const MAX_BORDER_GAP_SIZE = 14;
export const MAX_BORDER_DASH_SIZE = 72;
export const DEFAULT_TEXT_VALUE = 'text';
export const DEFAULT_TEXT_COLOR = '#ffffff';
export const DEFAULT_TEXT_FONT_ID = 'busy_regular_7px';
export const DEFAULT_STATUS_FILE_NAME = 'New status';
export const DRAW_TOOL_EXPORT_PIXEL_SIZE = 8;
export const DRAW_TOOL_PIXEL_ART_MAX_DIMENSION = 72;
export const DRAW_TOOL_DISPLAY_APPLICATION_NAME = 'draw_tool';
export const DRAW_TOOL_TEMP_FILE_NAME = 'temp.png';
export const DRAW_TOOL_SAVE_DIR = '/ext/user_assets/draw_tool';
export const DRAW_TOOL_DISPLAY_PRIORITY = 40;

let drawToolShapeIdCounter = 0;
const pixelArtImageSourceCache = new WeakMap<HTMLImageElement, HTMLCanvasElement>();

export type KonvaRef<T extends Konva.Node> = {
  getNode: () => T;
};

export type DrawToolStageMetrics = {
  width: number;
  height: number;
  cellSize: number;
  workspaceWidth: number;
  workspaceHeight: number;
  workspaceX: number;
  workspaceY: number;
};

export type TransformerBox = {
  x: number;
  y: number;
  width: number;
  height: number;
  rotation?: number;
};

export type BorderPixel = {
  x: number;
  y: number;
};

export type BorderPixelConfig = {
  key: string;
  x: number;
  y: number;
  width: number;
  height: number;
  fill: string;
  listening: boolean;
  perfectDrawEnabled: boolean;
};

export interface ShapeBase {
  id: string;
  x: number;
  y: number;
  width: number;
  height: number;
  rotation: number;
}

export interface RectShape extends ShapeBase {
  type: 'rect';
  fill: string;
}

export interface ImageShape extends ShapeBase {
  type: 'image';
  fileName: string;
  image: HTMLImageElement;
  pixelArt?: boolean;
}

export interface TextShape extends ShapeBase {
  type: 'text';
  text: string;
  fill: string;
  fontId: string;
}

export type EditorShape = RectShape | ImageShape | TextShape;

export type FontOption = {
  id: string;
  label: string;
  family: string;
  fontSize: number;
  capHeight: number;
};

export const TEXT_FONT_OPTIONS: FontOption[] = [
  { id: 'busy_regular_5px', label: 'Small', family: 'busy_regular_5px', fontSize: 16, capHeight: 5 },
  { id: 'busy_regular_7px', label: 'Basic', family: 'busy_regular_7px', fontSize: 16, capHeight: 7 },
  { id: 'busy_bold_7px', label: 'Basic bold', family: 'busy_bold_7px', fontSize: 16, capHeight: 7 },
  { id: 'LanaPixel_regular_11px', label: 'Basic global', family: 'LanaPixel_regular_11px', fontSize: 11, capHeight: 11 },
  { id: 'busy_regular_9px', label: 'Large', family: 'busy_regular_9px', fontSize: 16, capHeight: 9 },
  { id: 'busy_bold_10px', label: 'Extra large', family: 'busy_bold_10px', fontSize: 16, capHeight: 10 }
];

export const BORDER_RING_PIXELS: BorderPixel[] = [
  ...Array.from({ length: WORKSPACE_WIDTH }, (_, x) => ({ x, y: 0 })),
  ...Array.from({ length: WORKSPACE_HEIGHT - 1 }, (_, index) => ({ x: WORKSPACE_WIDTH - 1, y: index + 1 })),
  ...Array.from({ length: WORKSPACE_WIDTH - 1 }, (_, index) => ({ x: WORKSPACE_WIDTH - 2 - index, y: WORKSPACE_HEIGHT - 1 })),
  ...Array.from({ length: WORKSPACE_HEIGHT - 2 }, (_, index) => ({ x: 0, y: WORKSPACE_HEIGHT - 2 - index }))
];

export function getStageMetrics (stageWidth: number): DrawToolStageMetrics {
  const width = Math.max(MIN_STAGE_WIDTH, stageWidth || MIN_STAGE_WIDTH);
  const height = MIN_STAGE_HEIGHT;
  const usableWidth = Math.max(width - STAGE_PADDING_X * 2, WORKSPACE_WIDTH);
  const usableHeight = Math.max(height - STAGE_PADDING_Y * 2, WORKSPACE_HEIGHT);
  const cellSize = Math.max(1, Math.floor(Math.min(usableWidth / WORKSPACE_WIDTH, usableHeight / WORKSPACE_HEIGHT)));
  const workspaceWidth = WORKSPACE_WIDTH * cellSize;
  const workspaceHeight = WORKSPACE_HEIGHT * cellSize;

  return {
    width,
    height,
    cellSize,
    workspaceWidth,
    workspaceHeight,
    workspaceX: (width - workspaceWidth) / 2,
    workspaceY: (height - workspaceHeight) / 2
  };
}

export function getBorderGapOffsetSliderMax (borderDashSize: number, borderGapSize: number): number {
  return Math.max(0, borderDashSize + borderGapSize);
}

export function normalizeBorderSettings (
  borderGapSize: number,
  borderDashSize: number,
  borderGapOffset: number,
  borderGapOffsetMax: number
) {
  return {
    borderGapSize: Math.min(MAX_BORDER_GAP_SIZE, Math.max(0, borderGapSize)),
    borderDashSize: Math.min(MAX_BORDER_DASH_SIZE, Math.max(1, borderDashSize)),
    borderGapOffset: Math.min(borderGapOffsetMax, Math.max(0, borderGapOffset))
  };
}

export function getFontOption (fontId: string, fontOptions: FontOption[] = TEXT_FONT_OPTIONS): FontOption {
  return fontOptions.find(option => option.id === fontId) || fontOptions[2];
}

export function measureTextShapeDimensions (
  text: string,
  fontId: string,
  fontOptions: FontOption[] = TEXT_FONT_OPTIONS
): Pick<TextShape, 'width' | 'height'> {
  const font = getFontOption(fontId, fontOptions);
  const node = new Konva.Text({
    text: text || ' ',
    fontFamily: font.family,
    fontSize: font.fontSize,
    listening: false,
    perfectDrawEnabled: false
  });

  return {
    width: Math.max(1, Math.ceil(node.width())),
    height: Math.max(1, Math.ceil(node.height()))
  };
}

export function createShapeId (): string {
  drawToolShapeIdCounter += 1;

  return `shape-${Date.now().toString(36)}-${drawToolShapeIdCounter.toString(36)}-${Math.random().toString(36).slice(2, 8)}`;
}

export function cloneShape<T extends EditorShape> (shape: T): T {
  return {
    ...shape
  };
}

export function pixelateImageData (sourceImageData: ImageData, pixelSize: number): ImageData {
  const blockSize = Math.max(1, Math.ceil(pixelSize));
  const { width, height, data } = sourceImageData;
  const outputImageData = new ImageData(width, height);
  const nBinsX = Math.ceil(width / blockSize);
  const nBinsY = Math.ceil(height / blockSize);

  for (let xBin = 0; xBin < nBinsX; xBin += 1) {
    for (let yBin = 0; yBin < nBinsY; yBin += 1) {
      let red = 0;
      let green = 0;
      let blue = 0;
      let alpha = 0;
      let pixelsInBin = 0;
      const xBinStart = xBin * blockSize;
      const xBinEnd = xBinStart + blockSize;
      const yBinStart = yBin * blockSize;
      const yBinEnd = yBinStart + blockSize;

      for (let x = xBinStart; x < xBinEnd; x += 1) {
        if (x >= width) {
          continue;
        }

        for (let y = yBinStart; y < yBinEnd; y += 1) {
          if (y >= height) {
            continue;
          }

          const pixelIndex = ((width * y) + x) * 4;
          red += data[pixelIndex + 0];
          green += data[pixelIndex + 1];
          blue += data[pixelIndex + 2];
          alpha += data[pixelIndex + 3];
          pixelsInBin += 1;
        }
      }

      if (!pixelsInBin) {
        continue;
      }

      red = red / pixelsInBin;
      green = green / pixelsInBin;
      blue = blue / pixelsInBin;
      alpha = alpha / pixelsInBin;

      for (let x = xBinStart; x < xBinEnd; x += 1) {
        if (x >= width) {
          continue;
        }

        for (let y = yBinStart; y < yBinEnd; y += 1) {
          if (y >= height) {
            continue;
          }

          const pixelIndex = ((width * y) + x) * 4;
          outputImageData.data[pixelIndex + 0] = red;
          outputImageData.data[pixelIndex + 1] = green;
          outputImageData.data[pixelIndex + 2] = blue;
          outputImageData.data[pixelIndex + 3] = alpha;
        }
      }
    }
  }

  return outputImageData;
}

export function isPixelArtImageSource (image: Pick<HTMLImageElement, 'width' | 'height'>): boolean {
  return image.width <= DRAW_TOOL_PIXEL_ART_MAX_DIMENSION || image.height <= DRAW_TOOL_PIXEL_ART_MAX_DIMENSION;
}

export function isPixelArtImageShape (shape: Pick<ImageShape, 'image' | 'pixelArt'>): boolean {
  if (shape.pixelArt !== undefined) {
    return shape.pixelArt;
  }

  return isPixelArtImageSource(shape.image);
}

export function getRenderedImageSource (shape: ImageShape): HTMLImageElement | HTMLCanvasElement {
  if (!isPixelArtImageShape(shape)) {
    return shape.image;
  }

  const cachedSource = pixelArtImageSourceCache.get(shape.image);

  if (cachedSource) {
    return cachedSource;
  }

  const renderCanvas = document.createElement('canvas');
  renderCanvas.width = shape.image.width * DRAW_TOOL_EXPORT_PIXEL_SIZE;
  renderCanvas.height = shape.image.height * DRAW_TOOL_EXPORT_PIXEL_SIZE;

  const context = renderCanvas.getContext('2d');

  if (!context) {
    return shape.image;
  }

  context.imageSmoothingEnabled = false;
  context.drawImage(shape.image, 0, 0, renderCanvas.width, renderCanvas.height);
  pixelArtImageSourceCache.set(shape.image, renderCanvas);

  return renderCanvas;
}

export function areShapesEqual (left: EditorShape, right: EditorShape): boolean {
  if (
    left.id !== right.id
    || left.type !== right.type
    || left.x !== right.x
    || left.y !== right.y
    || left.width !== right.width
    || left.height !== right.height
    || left.rotation !== right.rotation
  ) {
    return false;
  }

  if (left.type === 'rect') {
    return left.fill === (right as RectShape).fill;
  }

  if (left.type === 'text') {
    return left.text === (right as TextShape).text
      && left.fill === (right as TextShape).fill
      && left.fontId === (right as TextShape).fontId;
  }

  return left.fileName === (right as ImageShape).fileName
    && left.image === (right as ImageShape).image;
}

export function isBorderPixelActive (
  index: number,
  borderDashSize: number,
  borderGapSize: number,
  borderGapOffset: number
): boolean {
  if (borderGapSize <= 0) {
    return true;
  }

  const patternLength = borderDashSize + borderGapSize;
  const normalizedIndex = ((index - borderGapOffset) % patternLength + patternLength) % patternLength;

  return normalizedIndex < borderDashSize;
}

export function createBorderPixelConfigs (
  fill: string,
  borderRingPixels: BorderPixel[],
  borderDashSize: number,
  borderGapSize: number,
  borderGapOffset: number
): BorderPixelConfig[] {
  if (!fill) {
    return [];
  }

  return borderRingPixels.flatMap((pixel, index) => {
    if (!isBorderPixelActive(index, borderDashSize, borderGapSize, borderGapOffset)) {
      return [];
    }

    return [{
      key: `border-${pixel.x}-${pixel.y}`,
      x: pixel.x,
      y: pixel.y,
      width: 1,
      height: 1,
      fill,
      listening: false,
      perfectDrawEnabled: false
    }];
  });
}

export function snapLogical (value: number): number {
  return Math.round(value);
}

export function snapStageCoordinate (value: number, stageMetrics: DrawToolStageMetrics): number {
  return stageMetrics.workspaceX + (Math.round((value - stageMetrics.workspaceX) / stageMetrics.cellSize) * stageMetrics.cellSize);
}

export function workspaceViewportBounds (stageMetrics: DrawToolStageMetrics) {
  return {
    left: stageMetrics.workspaceX,
    top: stageMetrics.workspaceY,
    right: stageMetrics.workspaceX + stageMetrics.workspaceWidth,
    bottom: stageMetrics.workspaceY + stageMetrics.workspaceHeight
  };
}

export function stageDeltaToLogical (delta: number, cellSize: number): number {
  if (delta === 0) {
    return 0;
  }

  const logicalDelta = delta / cellSize;
  return delta > 0 ? Math.ceil(logicalDelta) : Math.floor(logicalDelta);
}

export function getRotatedLogicalDelta (deltaX: number, deltaY: number, rotation: number, cellSize: number) {
  const radians = rotation * (Math.PI / 180);
  const localDeltaX = (deltaX * Math.cos(radians)) + (deltaY * Math.sin(radians));
  const localDeltaY = (-deltaX * Math.sin(radians)) + (deltaY * Math.cos(radians));

  return {
    x: stageDeltaToLogical(localDeltaX, cellSize),
    y: stageDeltaToLogical(localDeltaY, cellSize)
  };
}

export function getStageDeltaFromLocalDelta (deltaX: number, deltaY: number, rotation: number) {
  const radians = rotation * (Math.PI / 180);

  return {
    x: (deltaX * Math.cos(radians)) - (deltaY * Math.sin(radians)),
    y: (deltaX * Math.sin(radians)) + (deltaY * Math.cos(radians))
  };
}

export function getLogicalCenter (shape: ShapeBase) {
  const centerOffset = getStageDeltaFromLocalDelta(shape.width / 2, shape.height / 2, shape.rotation);

  return {
    x: shape.x + centerOffset.x,
    y: shape.y + centerOffset.y
  };
}

export function getShapePositionForRotation (shape: ShapeBase, rotation: number) {
  const center = getLogicalCenter(shape);
  const rotatedOffset = getStageDeltaFromLocalDelta(shape.width / 2, shape.height / 2, rotation);

  return {
    x: center.x - rotatedOffset.x,
    y: center.y - rotatedOffset.y
  };
}

export function getRotationHandleAngle (clientX: number, clientY: number, centerX: number, centerY: number) {
  return Math.atan2(clientY - centerY, clientX - centerX);
}

export function snapRotationValue (rotation: number, rotationSnapStep = ROTATION_SNAP_STEP): number {
  return Math.round(rotation / rotationSnapStep) * rotationSnapStep;
}

export function getHandleResizeDimensions (
  shape: ShapeBase,
  deltaX: number,
  deltaY: number,
  preserveAspectRatio: boolean,
  resizeFromCenter: boolean
) {
  let nextWidth = Math.max(1, shape.width + deltaX);
  let nextHeight = Math.max(1, shape.height + deltaY);

  if (preserveAspectRatio) {
    const widthScale = nextWidth / shape.width;
    const heightScale = nextHeight / shape.height;
    const nextScale = Math.max(
      1 / shape.width,
      1 / shape.height,
      Math.abs(widthScale - 1) >= Math.abs(heightScale - 1) ? widthScale : heightScale
    );

    nextWidth = Math.max(1, Math.round(shape.width * nextScale));
    nextHeight = Math.max(1, Math.round(shape.height * nextScale));
  }

  if (!resizeFromCenter) {
    return {
      x: shape.x,
      y: shape.y,
      width: nextWidth,
      height: nextHeight
    };
  }

  const stageOffset = getStageDeltaFromLocalDelta(
    -(nextWidth - shape.width) / 2,
    -(nextHeight - shape.height) / 2,
    shape.rotation
  );

  return {
    x: shape.x + stageOffset.x,
    y: shape.y + stageOffset.y,
    width: nextWidth,
    height: nextHeight
  };
}

export function getDragBoundPosition (position: Konva.Vector2d): Konva.Vector2d {
  return {
    x: snapLogical(position.x),
    y: snapLogical(position.y)
  };
}

export function getDisplayRectConfig (shape: RectShape) {
  return {
    x: shape.x,
    y: shape.y,
    width: shape.width,
    height: shape.height,
    rotation: shape.rotation,
    fill: shape.fill,
    listening: false,
    perfectDrawEnabled: false
  };
}

export function getDisplayImageConfig (shape: ImageShape) {
  const isPixelArt = isPixelArtImageShape(shape);

  return {
    x: shape.x,
    y: shape.y,
    width: shape.width,
    height: shape.height,
    rotation: shape.rotation,
    image: getRenderedImageSource(shape),
    listening: false,
    imageSmoothingEnabled: !isPixelArt,
    perfectDrawEnabled: false
  };
}

export function getExportImageConfig (shape: ImageShape) {
  return {
    id: shape.id,
    x: shape.x,
    y: shape.y,
    width: shape.width,
    height: shape.height,
    rotation: shape.rotation,
    image: getRenderedImageSource(shape),
    opacity: 1,
    draggable: true,
    dragBoundFunc: getDragBoundPosition,
    imageSmoothingEnabled: !isPixelArtImageShape(shape),
    perfectDrawEnabled: false
  };
}

export function getDisplayTextConfig (shape: TextShape, fontOptions: FontOption[] = TEXT_FONT_OPTIONS) {
  const font = getFontOption(shape.fontId, fontOptions);
  const displayOffsetY = (font.fontSize - font.capHeight) / 2;

  return {
    x: shape.x,
    y: shape.y - displayOffsetY,
    width: shape.width,
    height: font.fontSize,
    rotation: shape.rotation,
    text: shape.text,
    fill: shape.fill,
    fontFamily: font.family,
    fontSize: font.fontSize,
    letterSpacing: 0,
    listening: false,
    perfectDrawEnabled: false
  };
}

export function getRectConfig (shape: RectShape) {
  return {
    id: shape.id,
    x: shape.x,
    y: shape.y,
    width: shape.width,
    height: shape.height,
    rotation: shape.rotation,
    fill: shape.fill,
    opacity: 0,
    draggable: true,
    dragBoundFunc: getDragBoundPosition,
    perfectDrawEnabled: false
  };
}

export function getImageConfig (shape: ImageShape) {
  return {
    id: shape.id,
    x: shape.x,
    y: shape.y,
    width: shape.width,
    height: shape.height,
    rotation: shape.rotation,
    image: getRenderedImageSource(shape),
    opacity: 0,
    draggable: true,
    dragBoundFunc: getDragBoundPosition,
    imageSmoothingEnabled: !isPixelArtImageShape(shape),
    perfectDrawEnabled: false
  };
}

export function getTextConfig (shape: TextShape, fontOptions: FontOption[] = TEXT_FONT_OPTIONS) {
  const font = getFontOption(shape.fontId, fontOptions);

  return {
    id: shape.id,
    x: shape.x,
    y: shape.y,
    width: shape.width,
    height: font.capHeight,
    rotation: shape.rotation,
    text: shape.text,
    opacity: 0,
    fontFamily: font.family,
    fontSize: font.fontSize,
    letterSpacing: 0,
    draggable: true,
    dragBoundFunc: getDragBoundPosition,
    perfectDrawEnabled: false
  };
}

export async function loadImageFile (file: File): Promise<HTMLImageElement> {
  const url = URL.createObjectURL(file);

  return new Promise((resolve, reject) => {
    const image = new window.Image();

    image.onload = () => {
      URL.revokeObjectURL(url);
      resolve(image);
    };
    image.onerror = () => {
      URL.revokeObjectURL(url);
      reject(new Error('Could not decode image file'));
    };

    image.src = url;
  });
}

export function createExportStage (
  width = WORKSPACE_WIDTH,
  height = WORKSPACE_HEIGHT
): { stage: Konva.Stage; container: HTMLDivElement } {
  const container = document.createElement('div');

  container.style.position = 'fixed';
  container.style.left = '-99999px';
  container.style.top = '0';
  container.style.width = `${width}px`;
  container.style.height = `${height}px`;
  document.body.append(container);

  const stage = new Konva.Stage({
    container,
    width,
    height,
    listening: false
  });

  return { stage, container };
}

export async function loadEditorFonts (fontOptions: FontOption[] = TEXT_FONT_OPTIONS) {
  if (!('fonts' in document)) {
    return;
  }

  await Promise.all(fontOptions.map(font => document.fonts.load(`${font.fontSize}px "${font.family}"`)));
}
