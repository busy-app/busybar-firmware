<template>
  <SectionCard
    data-id="draw-tool-section-primary"
    class="overflow-visible"
  >
    <template #raw-body>
      <div class="flex flex-col gap-6 my-6">
        <!-- undo/redo -->
        <div class="flex flex-wrap gap-4">
          <UButton
            label="Undo"
            color="neutral"
            variant="outline"
            :disabled="!canUndo"
            @click="undo"
          />
          <UButton
            label="Redo"
            color="neutral"
            variant="outline"
            :disabled="!canRedo"
            @click="redo"
          />
        </div>

        <!-- background/border -->
        <div class="flex flex-wrap gap-4">
          <UPopover>
            <UButton
              color="neutral"
              variant="outline"
              label="Background"
            >
              <template #leading>
                <span
                  class="size-3 rounded-full ring-1 ring-default"
                  :style="backgroundColorChipStyle"
                />
              </template>
            </UButton>

            <template #content>
              <div class="flex flex-col gap-3 p-3">
                <UColorPicker
                  :model-value="backgroundPickerColor"
                  class="p-1"
                  :throttle="50"
                  @update:model-value="handleBackgroundColorChange"
                />
                <UButton
                  label="Clear background"
                  color="neutral"
                  variant="ghost"
                  :disabled="!backgroundColor"
                  @click="clearBackgroundColor"
                />
              </div>
            </template>
          </UPopover>

          <UPopover>
            <UButton
              color="neutral"
              variant="outline"
              label="Border"
            >
              <template #leading>
                <span
                  class="size-3 rounded-full ring-1 ring-default"
                  :style="borderColorChipStyle"
                />
              </template>
            </UButton>

            <template #content>
              <div class="flex w-80 flex-col gap-3 p-3">
                <UColorPicker
                  :model-value="borderPickerColor"
                  class="p-1"
                  :throttle="50"
                  @update:model-value="handleBorderColorChange"
                />

                <div class="flex gap-2">
                  <UButton
                    label="Reset border"
                    color="neutral"
                    variant="ghost"
                    :disabled="!borderColor"
                    @click="clearBorderColor"
                  />
                  <UButton
                    :label="showBorderGapControls ? 'Hide gap controls' : 'Gap controls'"
                    color="neutral"
                    variant="ghost"
                    @click="showBorderGapControls = !showBorderGapControls"
                  />
                </div>

                <div
                  v-if="showBorderGapControls"
                  class="flex flex-col gap-4 rounded-xl border border-default/70 p-3"
                >
                  <div class="flex flex-col gap-2">
                    <div class="flex items-center justify-between text-sm text-muted">
                      <span>Gap size</span>
                      <span>{{ borderGapSize }}px</span>
                    </div>
                    <USlider
                      v-model="borderGapSize"
                      :min="0"
                      :max="borderGapSliderMax"
                      :step="1"
                      @change="handleBorderSettingsChange"
                    />
                  </div>

                  <div class="flex flex-col gap-2">
                    <div class="flex items-center justify-between text-sm text-muted">
                      <span>Dash size</span>
                      <span>{{ borderDashSize }}px</span>
                    </div>
                    <USlider
                      v-model="borderDashSize"
                      :min="1"
                      :max="borderDashSliderMax"
                      :step="1"
                      @change="handleBorderSettingsChange"
                    />
                  </div>

                  <div class="flex flex-col gap-2">
                    <div class="flex items-center justify-between text-sm text-muted">
                      <span>Gap offset</span>
                      <span>{{ borderGapOffset }}px</span>
                    </div>
                    <USlider
                      v-model="borderGapOffset"
                      :min="0"
                      :max="borderGapOffsetSliderMax"
                      :step="1"
                      @change="handleBorderSettingsChange"
                    />
                  </div>
                </div>
              </div>
            </template>
          </UPopover>
        </div>

        <!-- shapes/images -->
        <div class="flex flex-wrap gap-4">
          <UButton
            label="Add rectangle"
            color="neutral"
            variant="outline"
            @click="addRectangle"
          />
          <UButton
            label="Add image"
            color="neutral"
            variant="outline"
            @click="showImageUploadModal = true"
          />
        </div>

        <!-- text -->
        <div class="flex flex-wrap gap-4">
          <UButton
            label="Add text"
            color="neutral"
            variant="outline"
            @click="addText"
          />

          <UPopover>
            <UButton
              color="neutral"
              variant="outline"
              label="Edit text"
            >
              <template #leading>
                <span
                  class="size-3 rounded-full ring-1 ring-default"
                  :style="textColorChipStyle"
                />
              </template>
            </UButton>

            <template #content>
              <div class="flex w-80 flex-col gap-3 p-3">
                <UInput
                  :model-value="activeTextValue"
                  placeholder="Text"
                  @update:model-value="handleActiveTextValueInput"
                  @blur="commitActiveTextChange"
                />

                <USelect
                  :model-value="activeTextFontId"
                  :items="textFontSelectItems"
                  @update:model-value="handleActiveTextFontChange"
                />

                <UColorPicker
                  :model-value="activeTextColor"
                  class="p-1"
                  :throttle="50"
                  @update:model-value="handleActiveTextColorChange"
                />
              </div>
            </template>
          </UPopover>
        </div>

        <!-- export -->
        <div class="flex flex-wrap gap-4">
          <UButton
            label="Download image"
            color="neutral"
            variant="solid"
            @click="downloadImage"
          />
        </div>
      </div>

      <div class="flex flex-col gap-4">
        <div
          ref="stageContainerRef"
          class="relative w-full min-h-[400px] rounded-[28px] border border-default/60 bg-elevated/35"
        >
          <div class="w-full overflow-hidden rounded-2xl bg-[#050505]">
            <VStage
              ref="stageRef"
              :config="stageConfig"
              @mousedown="handleStagePointerDown"
              @tap="handleStagePointerDown"
            >
              <VLayer ref="displayLayerRef">
                <VRect :config="stageBackgroundConfig" />

                <VGroup :config="workspaceBackgroundGroupConfig">
                  <VRect :config="workspaceBackgroundConfig" />
                </VGroup>

                <VGroup :config="workspaceCustomBackgroundGroupConfig">
                  <VRect :config="workspaceColorLayerConfig" />
                </VGroup>

                <VGroup
                  ref="displayGroupRef"
                  :config="displayGroupConfig"
                >
                  <VGroup :config="displayShapesGroupConfig">
                    <template
                      v-for="shape in shapes"
                      :key="`${shape.id}-display`"
                    >
                      <VRect
                        v-if="shape.type === 'rect'"
                        :config="getDisplayRectConfig(shape)"
                      />
                      <VText
                        v-else-if="shape.type === 'text'"
                        :config="getDisplayTextConfig(shape)"
                      />
                      <VImage
                        v-else
                        :config="getDisplayImageConfig(shape)"
                      />
                    </template>
                  </VGroup>
                </VGroup>

                <VGroup :config="workspaceGridGroupConfig">
                  <VLine
                    v-for="line in verticalGridLines"
                    :key="line.key"
                    :config="line"
                  />
                  <VLine
                    v-for="line in horizontalGridLines"
                    :key="line.key"
                    :config="line"
                  />
                  <VRect
                    v-for="pixel in borderPixelConfigs"
                    :key="pixel.key"
                    :config="pixel"
                  />
                </VGroup>
              </VLayer>

              <VLayer>
                <VGroup :config="shapeClipGroupConfig">
                  <template
                    v-for="shape in shapes"
                    :key="shape.id"
                  >
                    <VRect
                      v-if="shape.type === 'rect'"
                      :config="getRectConfig(shape)"
                      @mousedown="handleShapePointerDown"
                      @tap="handleShapePointerDown"
                      @dragmove="handleShapeDragMove"
                      @transform="handleShapeTransform"
                      @dragend="handleShapeDragEnd"
                      @transformend="handleShapeTransformEnd"
                    />
                    <VText
                      v-else-if="shape.type === 'text'"
                      :config="getTextConfig(shape)"
                      @mousedown="handleShapePointerDown"
                      @tap="handleShapePointerDown"
                      @dragmove="handleShapeDragMove"
                      @transform="handleShapeTransform"
                      @dragend="handleShapeDragEnd"
                      @transformend="handleShapeTransformEnd"
                    />
                    <VImage
                      v-else
                      :config="getImageConfig(shape)"
                      @mousedown="handleShapePointerDown"
                      @tap="handleShapePointerDown"
                      @dragmove="handleShapeDragMove"
                      @transform="handleShapeTransform"
                      @dragend="handleShapeDragEnd"
                      @transformend="handleShapeTransformEnd"
                    />
                  </template>
                </VGroup>
              </VLayer>

              <VLayer ref="overlayLayerRef">
                <VTransformer
                  ref="transformerRef"
                  :config="transformerConfig"
                />
                <VGroup
                  v-if="deleteButtonPosition"
                  :config="deleteButtonGroupConfig"
                  @click="deleteSelectedShape"
                  @tap="deleteSelectedShape"
                >
                  <VCircle :config="deleteButtonCircleConfig" />
                  <VText :config="deleteButtonTextConfig" />
                </VGroup>
              </VLayer>
            </VStage>
          </div>
        </div>

        <ModalGeneric
          v-model:open="showImageUploadModal"
          data-id="modal-draw-tool-image-upload"
          title="Add image"
          wide
          show-close-button
          :primary-action-props="{
            label: 'Insert image',
            disabled: !imageUploadFile,
            onClick: insertImage
          }"
          :secondary-action-props="{
            label: 'Cancel',
            variant: 'ghost',
            onClick: resetImageUploadModal
          }"
        >
          <template #body>
            <UFileUpload
              v-model="imageUploadFile"
              data-id="draw-tool-image-upload"
              accept="image/*"
              class="w-full rounded-xl"
              label="Upload image"
              description="Drag and drop to upload"
              :ui="{
                base: 'cursor-pointer',
                icon: 'size-6',
                label: 'text-lg',
                description: 'text-sm'
              }"
            >
              <template #actions>
                <UButton
                  label="Select file"
                  color="neutral"
                  class="mt-2"
                />
              </template>
            </UFileUpload>
          </template>
        </ModalGeneric>
      </div>
    </template>
  </SectionCard>
</template>

<script setup lang="ts">
import Konva from 'konva';
import {
  Circle as VCircle,
  Group as VGroup,
  Image as VImage,
  Layer as VLayer,
  Line as VLine,
  Rect as VRect,
  Stage as VStage,
  Text as VText,
  Transformer as VTransformer
} from 'vue-konva';

const WORKSPACE_WIDTH = 72;
const WORKSPACE_HEIGHT = 16;
const STAGE_PADDING_X = 20;
const STAGE_PADDING_Y = 28;
const MIN_STAGE_WIDTH = 320;
const MIN_STAGE_HEIGHT = 400;
const HISTORY_LIMIT = 10;
const ROTATION_SNAP_STEP = 5;
const DEFAULT_WORKSPACE_BACKGROUND = '#101010';
const DEFAULT_BACKGROUND_PICKER_COLOR = '#000000';
const DEFAULT_BORDER_PICKER_COLOR = '#000000';
const DEFAULT_BORDER_DASH_SIZE = 4;
const DEFAULT_BORDER_GAP_SIZE = 0;
const DEFAULT_BORDER_GAP_OFFSET = 0;
const MAX_BORDER_GAP_SIZE = 14;
const MAX_BORDER_DASH_SIZE = 72;
const DEFAULT_TEXT_VALUE = 'Text';
const DEFAULT_TEXT_COLOR = '#ffffff';
const DEFAULT_TEXT_FONT_ID = 'busy_regular_7px';

const BORDER_RING_PIXELS = [
  ...Array.from({ length: WORKSPACE_WIDTH }, (_, x) => ({ x, y: 0 })),
  ...Array.from({ length: WORKSPACE_HEIGHT - 1 }, (_, index) => ({ x: WORKSPACE_WIDTH - 1, y: index + 1 })),
  ...Array.from({ length: WORKSPACE_WIDTH - 1 }, (_, index) => ({ x: WORKSPACE_WIDTH - 2 - index, y: WORKSPACE_HEIGHT - 1 })),
  ...Array.from({ length: WORKSPACE_HEIGHT - 2 }, (_, index) => ({ x: 0, y: WORKSPACE_HEIGHT - 2 - index }))
];

type KonvaRef<T extends Konva.Node> = {
  getNode: () => T;
};

type TransformerBox = {
  x: number;
  y: number;
  width: number;
  height: number;
  rotation?: number;
};

type BorderPixel = {
  x: number;
  y: number;
};

type BorderPixelConfig = {
  key: string;
  x: number;
  y: number;
  width: number;
  height: number;
  fill: string;
  listening: boolean;
  perfectDrawEnabled: boolean;
};

type FontOption = {
  id: string;
  label: string;
  family: string;
  fontSize: number;
  offsetY: number;
  heightMultiplier: number;
};

interface HistorySnapshot {
  shapes: EditorShape[];
  selectedShapeId: string | null;
  backgroundColor?: string;
  borderColor?: string;
  borderGapSize: number;
  borderDashSize: number;
  borderGapOffset: number;
}

interface ShapeBase {
  id: string;
  x: number;
  y: number;
  width: number;
  height: number;
  rotation: number;
}

interface RectShape extends ShapeBase {
  type: 'rect';
  fill: string;
}

interface ImageShape extends ShapeBase {
  type: 'image';
  fileName: string;
  image: HTMLImageElement;
}

interface TextShape extends ShapeBase {
  type: 'text';
  text: string;
  fill: string;
  fontId: string;
}

type EditorShape = RectShape | ImageShape | TextShape;

const TEXT_FONT_OPTIONS: FontOption[] = [
  { id: 'busy_regular_5px', label: 'SMALL', family: 'busy_regular_5px', fontSize: 16, offsetY: 3, heightMultiplier: 0.5 },
  { id: 'busy_bold_7px', label: 'BOLD SMALL', family: 'busy_bold_7px', fontSize: 16, offsetY: 1, heightMultiplier: 0.5 },
  { id: 'busy_regular_7px', label: 'MEDIUM', family: 'busy_regular_7px', fontSize: 16, offsetY: 1, heightMultiplier: 0.5 },
  { id: 'busy_bold_10px', label: 'BOLD MEDIUM', family: 'busy_bold_10px', fontSize: 16, offsetY: -2, heightMultiplier: 0.5 },
  { id: 'busy_regular_9px', label: 'LARGE', family: 'busy_regular_9px', fontSize: 16, offsetY: -1, heightMultiplier: 0.5 },
  { id: 'LanaPixel_regular_11px', label: 'LanaPixel', family: 'LanaPixel_regular_11px', fontSize: 11, offsetY: 1, heightMultiplier: 1 },
  { id: 'busy_regular_14px', label: 'EXTRA LARGE', family: 'busy_regular_7px', fontSize: 32, offsetY: 2, heightMultiplier: 0.5 }
];

const stageContainerRef = ref<HTMLDivElement | null>(null);
const stageWidth = ref(MIN_STAGE_WIDTH);
const resizeObserver = ref<ResizeObserver | null>(null);

const stageRef = ref<KonvaRef<Konva.Stage> | null>(null);
const displayLayerRef = ref<KonvaRef<Konva.Layer> | null>(null);
const displayGroupRef = ref<KonvaRef<Konva.Group> | null>(null);
const overlayLayerRef = ref<KonvaRef<Konva.Layer> | null>(null);
const transformerRef = ref<KonvaRef<Konva.Transformer> | null>(null);

const shapes = ref<EditorShape[]>([]);
const selectedShapeId = ref<string | null>(null);
const deleteButtonPosition = ref<{ x: number; y: number } | null>(null);
const backgroundColor = ref<string | undefined>();
const borderColor = ref<string | undefined>();
const borderGapSize = ref(DEFAULT_BORDER_GAP_SIZE);
const borderDashSize = ref(DEFAULT_BORDER_DASH_SIZE);
const borderGapOffset = ref(DEFAULT_BORDER_GAP_OFFSET);
const showBorderGapControls = ref(false);
const textDraftValue = ref(DEFAULT_TEXT_VALUE);
const textDraftColor = ref(DEFAULT_TEXT_COLOR);
const textDraftFontId = ref(DEFAULT_TEXT_FONT_ID);

const historyEntries = ref<HistorySnapshot[]>([{
  shapes: [],
  selectedShapeId: null,
  backgroundColor: undefined,
  borderColor: undefined,
  borderGapSize: DEFAULT_BORDER_GAP_SIZE,
  borderDashSize: DEFAULT_BORDER_DASH_SIZE,
  borderGapOffset: DEFAULT_BORDER_GAP_OFFSET
}]);
const historyIndex = ref(0);

const showImageUploadModal = ref(false);
const imageUploadFile = ref<File | null>(null);

const stageMetrics = computed(() => {
  const width = Math.max(MIN_STAGE_WIDTH, stageWidth.value || MIN_STAGE_WIDTH);
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
});

const stageConfig = computed(() => ({
  width: stageMetrics.value.width,
  height: stageMetrics.value.height
}));

const stageBackgroundConfig = computed(() => ({
  x: 0,
  y: 0,
  width: stageMetrics.value.width,
  height: stageMetrics.value.height,
  fill: '#050505',
  listening: false
}));

const workspaceDisplayGroupConfig = computed(() => ({
  x: stageMetrics.value.workspaceX,
  y: stageMetrics.value.workspaceY,
  scaleX: stageMetrics.value.cellSize,
  scaleY: stageMetrics.value.cellSize,
  listening: false
}));

const workspaceBackgroundGroupConfig = computed(() => ({
  ...workspaceDisplayGroupConfig.value,
  listening: false
}));

const workspaceCustomBackgroundGroupConfig = computed(() => ({
  ...workspaceDisplayGroupConfig.value,
  listening: false
}));

const workspaceGridGroupConfig = computed(() => ({
  ...workspaceDisplayGroupConfig.value,
  listening: false,
  opacity: 0.5
}));

const workspaceBackgroundConfig = computed(() => ({
  x: 0,
  y: 0,
  width: WORKSPACE_WIDTH,
  height: WORKSPACE_HEIGHT,
  fill: DEFAULT_WORKSPACE_BACKGROUND,
  visible: !backgroundColor.value,
  listening: false
}));

const workspaceColorLayerConfig = computed(() => ({
  x: 0,
  y: 0,
  width: WORKSPACE_WIDTH,
  height: WORKSPACE_HEIGHT,
  fill: backgroundColor.value || 'transparent',
  visible: Boolean(backgroundColor.value),
  listening: false
}));

function isBorderPixelActive (index: number): boolean {
  if (borderGapSize.value <= 0) {
    return true;
  }

  const patternLength = borderDashSize.value + borderGapSize.value;
  const normalizedIndex = ((index - borderGapOffset.value) % patternLength + patternLength) % patternLength;

  return normalizedIndex < borderDashSize.value;
}

function createBorderPixelConfigs (fill: string): BorderPixelConfig[] {
  if (!fill) {
    return [];
  }

  return BORDER_RING_PIXELS.flatMap((pixel: BorderPixel, index) => {
    if (!isBorderPixelActive(index)) {
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

const borderPixelConfigs = computed(() => {
  if (!borderColor.value) {
    return [];
  }

  return createBorderPixelConfigs(borderColor.value);
});

const shapeClipGroupConfig = computed(() => ({
  x: stageMetrics.value.workspaceX,
  y: stageMetrics.value.workspaceY,
  scaleX: stageMetrics.value.cellSize,
  scaleY: stageMetrics.value.cellSize,
  clipX: 0,
  clipY: 0,
  clipWidth: WORKSPACE_WIDTH,
  clipHeight: WORKSPACE_HEIGHT
}));

const displayGroupConfig = computed(() => ({
  x: stageMetrics.value.workspaceX,
  y: stageMetrics.value.workspaceY,
  clipX: 0,
  clipY: 0,
  clipWidth: stageMetrics.value.workspaceWidth,
  clipHeight: stageMetrics.value.workspaceHeight,
  listening: false,
  perfectDrawEnabled: false
}));

const displayShapesGroupConfig = computed(() => ({
  scaleX: stageMetrics.value.cellSize,
  scaleY: stageMetrics.value.cellSize,
  listening: false,
  perfectDrawEnabled: false
}));

const verticalGridLines = computed(() => {
  return Array.from({ length: WORKSPACE_WIDTH + 1 }, (_, index) => ({
    key: `vertical-${index}`,
    points: [index, 0, index, WORKSPACE_HEIGHT],
    stroke: index === 0 || index === WORKSPACE_WIDTH ? '#4b5563' : '#1f1f1f',
    strokeWidth: 1,
    strokeScaleEnabled: false,
    listening: false,
    perfectDrawEnabled: false
  }));
});

const horizontalGridLines = computed(() => {
  return Array.from({ length: WORKSPACE_HEIGHT + 1 }, (_, index) => ({
    key: `horizontal-${index}`,
    points: [0, index, WORKSPACE_WIDTH, index],
    stroke: index === 0 || index === WORKSPACE_HEIGHT ? '#4b5563' : '#1f1f1f',
    strokeWidth: 1,
    strokeScaleEnabled: false,
    listening: false,
    perfectDrawEnabled: false
  }));
});

const transformerConfig = computed(() => ({
  rotateEnabled: !selectedTextShape.value,
  rotationSnaps: Array.from({ length: 360 / ROTATION_SNAP_STEP }, (_, index) => index * ROTATION_SNAP_STEP),
  rotationSnapTolerance: ROTATION_SNAP_STEP / 2,
  flipEnabled: false,
  keepRatio: false,
  resizeEnabled: !selectedTextShape.value,
  ignoreStroke: true,
  padding: 0,
  borderStroke: '#e5e7eb',
  borderStrokeWidth: 1,
  borderDash: [4, 4],
  anchorFill: '#f5f5f5',
  anchorStroke: '#050505',
  anchorStrokeWidth: 1,
  anchorSize: 10,
  enabledAnchors: selectedTextShape.value ? [] : ['bottom-right'],
  boundBoxFunc: (oldBox: TransformerBox, newBox: TransformerBox) => {
    const minSize = stageMetrics.value.cellSize;
    let result = newBox;
    if (Math.abs(newBox.width) < minSize || Math.abs(newBox.height) < minSize) {
      result = oldBox;
    }
    return result;
  },
  anchorDragBoundFunc: (_oldAbsPos: Konva.Vector2d, newAbsPos: Konva.Vector2d) => ({
    x: snapStageCoordinate(newAbsPos.x),
    y: snapStageCoordinate(newAbsPos.y)
  })
}));

watch(
  () => [selectedShapeId.value, stageMetrics.value.cellSize, shapes.value.length],
  async () => {
    await nextTick();
    syncTransformer();
  },
  { flush: 'post' }
);

watch(showImageUploadModal, isOpen => {
  if (!isOpen) {
    imageUploadFile.value = null;
  }
});

watch(
  shapes,
  async () => {
    await nextTick();
    syncPixelatedDisplay();
  },
  { deep: true }
);

watch(
  () => stageMetrics.value.cellSize,
  async () => {
    await nextTick();
    syncPixelatedDisplay();
    syncTransformer();
  }
);

const canUndo = computed(() => historyIndex.value > 0);
const canRedo = computed(() => historyIndex.value < historyEntries.value.length - 1);
const selectedShape = computed(() => shapes.value.find(shape => shape.id === selectedShapeId.value) || null);
const selectedTextShape = computed(() => selectedShape.value?.type === 'text' ? selectedShape.value : null);
const backgroundColorChipStyle = computed(() => ({
  backgroundColor: backgroundColor.value || DEFAULT_WORKSPACE_BACKGROUND
}));
const backgroundPickerColor = computed(() => backgroundColor.value || DEFAULT_BACKGROUND_PICKER_COLOR);
const borderColorChipStyle = computed(() => ({
  backgroundColor: borderColor.value || 'transparent'
}));
const borderPickerColor = computed(() => borderColor.value || DEFAULT_BORDER_PICKER_COLOR);
const borderGapSliderMax = computed(() => MAX_BORDER_GAP_SIZE);
const borderDashSliderMax = computed(() => MAX_BORDER_DASH_SIZE);
const borderGapOffsetSliderMax = computed(() => Math.max(0, borderDashSize.value + borderGapSize.value));
const activeTextValue = computed(() => selectedTextShape.value?.text ?? textDraftValue.value);
const activeTextColor = computed(() => selectedTextShape.value?.fill ?? textDraftColor.value);
const activeTextFontId = computed(() => selectedTextShape.value?.fontId ?? textDraftFontId.value);
const textColorChipStyle = computed(() => ({
  backgroundColor: activeTextColor.value
}));
const textFontSelectItems = computed(() => TEXT_FONT_OPTIONS.map(font => ({
  label: font.label,
  value: font.id
})));

function normalizeBorderSettings () {
  borderGapSize.value = Math.min(MAX_BORDER_GAP_SIZE, Math.max(0, borderGapSize.value));
  borderDashSize.value = Math.min(MAX_BORDER_DASH_SIZE, Math.max(1, borderDashSize.value));
  borderGapOffset.value = Math.min(borderGapOffsetSliderMax.value, Math.max(0, borderGapOffset.value));
}

function getFontOption (fontId: string): FontOption {
  return TEXT_FONT_OPTIONS.find(option => option.id === fontId) || TEXT_FONT_OPTIONS[2];
}

function measureTextShapeDimensions (text: string, fontId: string): Pick<TextShape, 'width' | 'height'> {
  const font = getFontOption(fontId);
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

function createShapeId (): string {
  return crypto.randomUUID();
}

function cloneShape (shape: EditorShape): EditorShape {
  return {
    ...shape
  };
}

function createHistorySnapshot (): HistorySnapshot {
  return {
    shapes: shapes.value.map(cloneShape),
    selectedShapeId: selectedShapeId.value,
    backgroundColor: backgroundColor.value,
    borderColor: borderColor.value,
    borderGapSize: borderGapSize.value,
    borderDashSize: borderDashSize.value,
    borderGapOffset: borderGapOffset.value
  };
}

function areShapesEqual (left: EditorShape, right: EditorShape): boolean {
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

function areSnapshotsEqual (left: HistorySnapshot | undefined, right: HistorySnapshot): boolean {
  if (
    !left
    || left.selectedShapeId !== right.selectedShapeId
    || left.backgroundColor !== right.backgroundColor
    || left.borderColor !== right.borderColor
    || left.borderGapSize !== right.borderGapSize
    || left.borderDashSize !== right.borderDashSize
    || left.borderGapOffset !== right.borderGapOffset
    || left.shapes.length !== right.shapes.length
  ) {
    return false;
  }

  return left.shapes.every((shape, index) => areShapesEqual(shape, right.shapes[index]));
}

function pushHistorySnapshot () {
  const nextSnapshot = createHistorySnapshot();
  const currentSnapshot = historyEntries.value[historyIndex.value];

  if (areSnapshotsEqual(currentSnapshot, nextSnapshot)) {
    return;
  }

  const nextEntries = historyEntries.value.slice(0, historyIndex.value + 1);
  nextEntries.push(nextSnapshot);

  if (nextEntries.length > HISTORY_LIMIT + 1) {
    nextEntries.shift();
  }

  historyEntries.value = nextEntries;
  historyIndex.value = nextEntries.length - 1;
}

function restoreSnapshot (snapshot: HistorySnapshot) {
  shapes.value = snapshot.shapes.map(cloneShape);
  selectedShapeId.value = snapshot.selectedShapeId;
  backgroundColor.value = snapshot.backgroundColor;
  borderColor.value = snapshot.borderColor;
  borderGapSize.value = snapshot.borderGapSize;
  borderDashSize.value = snapshot.borderDashSize;
  borderGapOffset.value = snapshot.borderGapOffset;
  normalizeBorderSettings();
  nextTick(syncTransformer);
}

function clearBackgroundColor () {
  if (!backgroundColor.value) {
    return;
  }

  backgroundColor.value = undefined;
  pushHistorySnapshot();
}

function handleBackgroundColorChange (value?: string) {
  backgroundColor.value = value;
  pushHistorySnapshot();
}

function clearBorderColor () {
  if (!borderColor.value) {
    return;
  }

  borderColor.value = undefined;
  pushHistorySnapshot();
}

function handleBorderColorChange (value?: string) {
  borderColor.value = value;
  pushHistorySnapshot();
}

function handleBorderSettingsChange () {
  normalizeBorderSettings();
  pushHistorySnapshot();
}

function handleActiveTextValueInput (value: string | number) {
  const nextValue = String(value ?? '');

  if (!selectedTextShape.value) {
    textDraftValue.value = nextValue;
    return;
  }

  const dimensions = measureTextShapeDimensions(nextValue, selectedTextShape.value.fontId);

  updateShape(selectedTextShape.value.id, shape => ({
    ...(shape as TextShape),
    text: nextValue,
    ...dimensions
  }));
}

function handleActiveTextColorChange (value?: string) {
  const nextColor = value || DEFAULT_TEXT_COLOR;

  if (!selectedTextShape.value) {
    textDraftColor.value = nextColor;
    return;
  }

  updateShape(selectedTextShape.value.id, shape => ({
    ...(shape as TextShape),
    fill: nextColor
  }));
  pushHistorySnapshot();
}

function handleActiveTextFontChange (value: string | number) {
  const nextFontId = String(value);

  if (!selectedTextShape.value) {
    textDraftFontId.value = nextFontId;
    return;
  }

  const dimensions = measureTextShapeDimensions(selectedTextShape.value.text, nextFontId);

  updateShape(selectedTextShape.value.id, shape => ({
    ...(shape as TextShape),
    fontId: nextFontId,
    ...dimensions
  }));
  pushHistorySnapshot();
}

function commitActiveTextChange () {
  if (!selectedTextShape.value) {
    return;
  }

  pushHistorySnapshot();
}

function undo () {
  if (!canUndo.value) {
    return;
  }

  historyIndex.value -= 1;
  restoreSnapshot(historyEntries.value[historyIndex.value]);
}

function redo () {
  if (!canRedo.value) {
    return;
  }

  historyIndex.value += 1;
  restoreSnapshot(historyEntries.value[historyIndex.value]);
}

function snapLogical (value: number): number {
  return Math.round(value);
}

function snapStageCoordinate (value: number): number {
  const { workspaceX, cellSize } = stageMetrics.value;
  return workspaceX + (Math.round((value - workspaceX) / cellSize) * cellSize);
}

function workspaceViewportBounds () {
  return {
    left: stageMetrics.value.workspaceX,
    top: stageMetrics.value.workspaceY,
    right: stageMetrics.value.workspaceX + stageMetrics.value.workspaceWidth,
    bottom: stageMetrics.value.workspaceY + stageMetrics.value.workspaceHeight
  };
}

function stageDeltaToLogical (delta: number): number {
  if (delta === 0) {
    return 0;
  }

  const logicalDelta = delta / stageMetrics.value.cellSize;
  return delta > 0 ? Math.ceil(logicalDelta) : Math.floor(logicalDelta);
}

function measureStage () {
  stageWidth.value = stageContainerRef.value?.clientWidth || MIN_STAGE_WIDTH;
}

function syncTransformer () {
  const transformer = transformerRef.value?.getNode();
  const stage = stageRef.value?.getNode();

  if (!transformer || !stage) {
    return;
  }

  if (!selectedShapeId.value) {
    transformer.nodes([]);
    deleteButtonPosition.value = null;
    overlayLayerRef.value?.getNode().batchDraw();
    return;
  }

  const node = stage.findOne(`#${selectedShapeId.value}`);
  transformer.nodes(node ? [node] : []);
  transformer.forceUpdate();

  updateDeleteButtonPosition();
  overlayLayerRef.value?.getNode().batchDraw();
}

function updateDeleteButtonPosition () {
  const stage = stageRef.value?.getNode();
  const selectedNode = selectedShapeId.value ? stage?.findOne(`#${selectedShapeId.value}`) : null;

  if (!stage || !selectedNode) {
    deleteButtonPosition.value = null;
    return;
  }

  const topLeftCorner = selectedNode.getAbsoluteTransform().point({ x: 0, y: 0 });

  deleteButtonPosition.value = {
    x: topLeftCorner.x,
    y: topLeftCorner.y
  };
}

function syncPixelatedDisplay () {
  const displayGroup = displayGroupRef.value?.getNode();

  if (!displayGroup) {
    return;
  }

  displayGroup.clearCache();
  displayGroup.cache({
    x: 0,
    y: 0,
    width: stageMetrics.value.workspaceWidth,
    height: stageMetrics.value.workspaceHeight,
    pixelRatio: 1,
    hitCanvasPixelRatio: 1,
    imageSmoothingEnabled: false
  });
  displayGroup.filters([Konva.Filters.Pixelate]);
  displayGroup.pixelSize(stageMetrics.value.cellSize);
  displayLayerRef.value?.getNode().batchDraw();
}

function updateShape (shapeId: string, updater: (shape: EditorShape) => EditorShape) {
  const shapeIndex = shapes.value.findIndex(shape => shape.id === shapeId);

  if (shapeIndex === -1) {
    return;
  }

  shapes.value.splice(shapeIndex, 1, updater(shapes.value[shapeIndex]));
}

function keepNodeVisibleInViewport (node: Konva.Rect | Konva.Image | Konva.Text) {
  const stage = stageRef.value?.getNode();

  if (!stage) {
    return;
  }

  const rect = node.getClientRect({
    relativeTo: stage,
    skipStroke: true,
    skipShadow: true
  });
  const viewport = workspaceViewportBounds();
  const minVisible = stageMetrics.value.cellSize;

  let deltaX = 0;
  let deltaY = 0;

  if (rect.x > viewport.right - minVisible) {
    deltaX = viewport.right - minVisible - rect.x;
  } else if (rect.x + rect.width < viewport.left + minVisible) {
    deltaX = viewport.left + minVisible - (rect.x + rect.width);
  }

  if (rect.y > viewport.bottom - minVisible) {
    deltaY = viewport.bottom - minVisible - rect.y;
  } else if (rect.y + rect.height < viewport.top + minVisible) {
    deltaY = viewport.top + minVisible - (rect.y + rect.height);
  }

  if (deltaX === 0 && deltaY === 0) {
    return;
  }

  node.position({
    x: snapLogical(node.x() + stageDeltaToLogical(deltaX)),
    y: snapLogical(node.y() + stageDeltaToLogical(deltaY))
  });
}

function getDragBoundPosition (position: Konva.Vector2d): Konva.Vector2d {
  return {
    x: snapLogical(position.x),
    y: snapLogical(position.y)
  };
}

function getDisplayRectConfig (shape: RectShape) {
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

function getDisplayImageConfig (shape: ImageShape) {
  return {
    x: shape.x,
    y: shape.y,
    width: shape.width,
    height: shape.height,
    rotation: shape.rotation,
    image: shape.image,
    listening: false,
    imageSmoothingEnabled: false,
    perfectDrawEnabled: false
  };
}

function getDisplayTextConfig (shape: TextShape) {
  const font = getFontOption(shape.fontId);

  return {
    x: shape.x,
    y: shape.y - font.offsetY,
    width: shape.width,
    height: font.fontSize,
    rotation: shape.rotation,
    text: shape.text,
    fill: shape.fill,
    fontFamily: font.family,
    fontSize: font.fontSize,
    letterSpacing: 0,
    listening: false,
    perfectDrawEnabled: false,
    verticalAlign: 'middle'
  };
}

function getRectConfig (shape: RectShape) {
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

function getImageConfig (shape: ImageShape) {
  return {
    id: shape.id,
    x: shape.x,
    y: shape.y,
    width: shape.width,
    height: shape.height,
    rotation: shape.rotation,
    image: shape.image,
    opacity: 0,
    draggable: true,
    dragBoundFunc: getDragBoundPosition,
    imageSmoothingEnabled: false,
    perfectDrawEnabled: false
  };
}

function getTextConfig (shape: TextShape) {
  const font = getFontOption(shape.fontId);

  return {
    id: shape.id,
    x: shape.x,
    y: shape.y,
    width: shape.width,
    height: font.fontSize * font.heightMultiplier - font.offsetY,
    rotation: shape.rotation,
    text: shape.text,
    opacity: 0,
    fontFamily: font.family,
    fontSize: font.fontSize,
    letterSpacing: 0,
    verticalAlign: 'middle',
    draggable: true,
    dragBoundFunc: getDragBoundPosition,
    perfectDrawEnabled: false
  };
}

function addRectangle () {
  const rectangle: RectShape = {
    id: createShapeId(),
    type: 'rect',
    x: 0,
    y: 0,
    width: 12,
    height: 6,
    rotation: 0,
    fill: '#7dd3fc'
  };

  shapes.value.push(rectangle);
  selectedShapeId.value = rectangle.id;
  pushHistorySnapshot();
}

function addText () {
  const dimensions = measureTextShapeDimensions(activeTextValue.value, activeTextFontId.value);
  const textShape: TextShape = {
    id: createShapeId(),
    type: 'text',
    x: 0,
    y: 0,
    rotation: 0,
    text: activeTextValue.value,
    fill: activeTextColor.value,
    fontId: activeTextFontId.value,
    ...dimensions
  };

  shapes.value.push(textShape);
  selectedShapeId.value = textShape.id;
  pushHistorySnapshot();
}

function deleteSelectedShape () {
  if (!selectedShapeId.value) {
    return;
  }

  shapes.value = shapes.value.filter(shape => shape.id !== selectedShapeId.value);
  selectedShapeId.value = null;
  pushHistorySnapshot();
}

function handleStagePointerDown (event: Konva.KonvaEventObject<MouseEvent | TouchEvent>) {
  if (event.target !== event.target.getStage()) {
    return;
  }

  selectedShapeId.value = null;
}

function handleShapePointerDown (event: Konva.KonvaEventObject<MouseEvent | TouchEvent>) {
  selectedShapeId.value = event.target.id();
}

function syncNodePosition (node: Konva.Rect | Konva.Image | Konva.Text) {
  const x = snapLogical(node.x());
  const y = snapLogical(node.y());

  node.position({ x, y });
  keepNodeVisibleInViewport(node);

  updateShape(node.id(), shape => ({
    ...shape,
    x: node.x(),
    y: node.y()
  }));
}

function normalizeTransformedNode (node: Konva.Rect | Konva.Image | Konva.Text) {
  const width = Math.max(1, snapLogical(node.width() * node.scaleX()));
  const height = Math.max(1, snapLogical(node.height() * node.scaleY()));
  const rotation = node.rotation();

  node.position({ x: snapLogical(node.x()), y: snapLogical(node.y()) });
  node.width(width);
  node.height(height);
  node.rotation(rotation);
  node.scaleX(1);
  node.scaleY(1);
  keepNodeVisibleInViewport(node);

  updateShape(node.id(), shape => ({
    ...shape,
    x: node.x(),
    y: node.y(),
    width,
    height,
    rotation
  }));
}

function syncRotatingNode (node: Konva.Rect | Konva.Image | Konva.Text) {
  updateShape(node.id(), shape => ({
    ...shape,
    x: node.x(),
    y: node.y(),
    rotation: node.rotation()
  }));
}

function handleShapeDragMove (event: Konva.KonvaEventObject<DragEvent>) {
  syncNodePosition(event.target as Konva.Rect | Konva.Image | Konva.Text);
  updateDeleteButtonPosition();
  overlayLayerRef.value?.getNode().batchDraw();
}

function handleShapeDragEnd (event: Konva.KonvaEventObject<DragEvent>) {
  syncNodePosition(event.target as Konva.Rect | Konva.Image | Konva.Text);
  pushHistorySnapshot();
}

function handleShapeTransform () {
  const transformer = transformerRef.value?.getNode();
  const node = transformer?.nodes()[0] as Konva.Rect | Konva.Image | Konva.Text | undefined;

  if (!node) {
    return;
  }

  if (transformer?.getActiveAnchor() === 'rotater') {
    syncRotatingNode(node);
  } else {
    normalizeTransformedNode(node);
    transformer?.forceUpdate();
  }

  updateDeleteButtonPosition();
  nextTick(syncPixelatedDisplay);
  overlayLayerRef.value?.getNode().batchDraw();
}

function handleShapeTransformEnd (event: Konva.KonvaEventObject<Event>) {
  const node = event.target as Konva.Rect | Konva.Image | Konva.Text;

  normalizeTransformedNode(node);

  pushHistorySnapshot();
  nextTick(syncTransformer);
}

const deleteButtonGroupConfig = computed(() => {
  if (!deleteButtonPosition.value) {
    return {
      visible: false
    };
  }

  return {
    x: deleteButtonPosition.value.x,
    y: deleteButtonPosition.value.y,
    listening: true
  };
});

const deleteButtonCircleConfig = computed(() => ({
  x: 0,
  y: 0,
  radius: 12,
  fill: '#ef4444',
  stroke: '#ffffff',
  strokeWidth: 1.5,
  shadowColor: '#000000',
  shadowBlur: 4,
  shadowOpacity: 0.35,
  listening: true
}));

const deleteButtonTextConfig = computed(() => ({
  x: -4,
  y: -8,
  text: 'x',
  fill: '#ffffff',
  fontSize: 16,
  fontStyle: 'bold',
  listening: false
}));

function resetImageUploadModal () {
  imageUploadFile.value = null;
  showImageUploadModal.value = false;
}

async function loadImage (file: File): Promise<HTMLImageElement> {
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

async function insertImage () {
  if (!imageUploadFile.value) {
    return;
  }

  try {
    const imageElement = await loadImage(imageUploadFile.value);
    const height = WORKSPACE_HEIGHT;
    const width = Math.max(1, Math.round((imageElement.width / imageElement.height) * height));

    const imageShape: ImageShape = {
      id: createShapeId(),
      type: 'image',
      fileName: imageUploadFile.value.name,
      x: 0,
      y: 0,
      width,
      height,
      rotation: 0,
      image: imageElement
    };

    shapes.value.push(imageShape);
    selectedShapeId.value = imageShape.id;
    pushHistorySnapshot();
    resetImageUploadModal();
  } catch (error) {
    toast.add({
      id: 'draw-tool-image-error',
      title: 'Could not load image',
      description: error instanceof Error ? error.message : String(error),
      color: 'error',
      duration: 10000
    });
  }
}

async function loadEditorFonts () {
  if (!('fonts' in document)) {
    return;
  }

  await Promise.all(TEXT_FONT_OPTIONS.map(font => document.fonts.load(`${font.fontSize}px "${font.family}"`)));
}

function createExportStage (): { stage: Konva.Stage; container: HTMLDivElement } {
  const container = document.createElement('div');

  container.style.position = 'fixed';
  container.style.left = '-99999px';
  container.style.top = '0';
  container.style.width = `${WORKSPACE_WIDTH}px`;
  container.style.height = `${WORKSPACE_HEIGHT}px`;
  document.body.append(container);

  const stage = new Konva.Stage({
    container,
    width: WORKSPACE_WIDTH,
    height: WORKSPACE_HEIGHT,
    listening: false
  });

  return { stage, container };
}

function buildExportLayer (): Konva.Layer {
  const layer = new Konva.Layer({ listening: false });

  if (backgroundColor.value) {
    layer.add(new Konva.Rect({
      x: 0,
      y: 0,
      width: WORKSPACE_WIDTH,
      height: WORKSPACE_HEIGHT,
      fill: backgroundColor.value,
      listening: false,
      perfectDrawEnabled: false
    }));
  }

  shapes.value.forEach(shape => {
    if (shape.type === 'rect') {
      layer.add(new Konva.Rect({
        x: shape.x,
        y: shape.y,
        width: shape.width,
        height: shape.height,
        rotation: shape.rotation,
        fill: shape.fill,
        listening: false,
        perfectDrawEnabled: false
      }));

      return;
    }

    if (shape.type === 'text') {
      const font = getFontOption(shape.fontId);

      layer.add(new Konva.Text({
        x: shape.x,
        y: shape.y,
        width: shape.width,
        height: shape.height,
        rotation: shape.rotation,
        text: shape.text,
        fill: shape.fill,
        fontFamily: font.family,
        fontSize: font.fontSize,
        listening: false,
        perfectDrawEnabled: false
      }));

      return;
    }

    layer.add(new Konva.Image({
      x: shape.x,
      y: shape.y,
      width: shape.width,
      height: shape.height,
      rotation: shape.rotation,
      image: shape.image,
      listening: false,
      imageSmoothingEnabled: false,
      perfectDrawEnabled: false
    }));
  });

  if (borderColor.value) {
    createBorderPixelConfigs(borderColor.value).forEach(({ key: _key, ...config }) => {
      layer.add(new Konva.Rect(config));
    });
  }

  return layer;
}

function downloadImage () {
  let exportStage: Konva.Stage | null = null;
  let exportContainer: HTMLDivElement | null = null;

  try {
    const exportSurface = createExportStage();
    exportStage = exportSurface.stage;
    exportContainer = exportSurface.container;

    const layer = buildExportLayer();
    exportStage.add(layer);
    layer.draw();

    const dataUrl = exportStage.toDataURL({
      mimeType: 'image/png',
      pixelRatio: 1
    });
    const link = document.createElement('a');

    link.href = dataUrl;
    link.download = 'draw-tool.png';
    link.click();
  } catch (error) {
    toast.add({
      id: 'draw-tool-download-error',
      title: 'Could not download image',
      description: error instanceof Error ? error.message : String(error),
      color: 'error',
      duration: 10000
    });
  } finally {
    exportStage?.destroy();
    exportContainer?.remove();
  }
}

onMounted(() => {
  measureStage();

  resizeObserver.value = new ResizeObserver(() => {
    measureStage();
  });

  if (stageContainerRef.value) {
    resizeObserver.value.observe(stageContainerRef.value);
  }

  loadEditorFonts().then(() => {
    nextTick(syncPixelatedDisplay);
  });
  nextTick(syncPixelatedDisplay);
});

onBeforeUnmount(() => {
  resizeObserver.value?.disconnect();
});
</script>
