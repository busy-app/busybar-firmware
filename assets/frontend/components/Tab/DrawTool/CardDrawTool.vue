<template>
  <div>
    <SectionCard
      data-id="draw-tool-section-primary"
      class="overflow-visible"
    >
      <template #raw-body>
        <div class="flex flex-col gap-6 my-6">
          <div class="flex flex-wrap gap-4">
            <UButton
              label="Add rectangle"
              color="neutral"
              variant="outline"
              @click="dts.addRectangle"
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
                    :style="`background-color: ${activeTextColor}`"
                  />
                </template>
              </UButton>

              <template #content>
                <div class="flex w-80 flex-col gap-3 p-3">
                  <UInput
                    :model-value="activeTextValue"
                    placeholder="Text"
                    @update:model-value="dts.handleActiveTextValueInput"
                    @blur="dts.commitActiveTextChange"
                  />

                  <USelect
                    :model-value="activeTextFontId"
                    :items="TEXT_FONT_OPTIONS"
                    :value-key="'id'"
                    @update:model-value="dts.handleActiveTextFontChange"
                  />

                  <UColorPicker
                    :model-value="activeTextColor"
                    class="p-1"
                    :throttle="50"
                    @update:model-value="dts.handleActiveTextColorChange"
                  />
                </div>
              </template>
            </UPopover>

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
            :ref="dtsRefs.stageContainerRef"
            class="relative w-full min-h-[400px] rounded-[28px] border border-default/60 bg-elevated/35"
          >
            <div class="w-full overflow-hidden rounded-2xl bg-[#050505]">
              <VStage
                :ref="dtsRefs.stageRef"
                :config="stageConfig"
                @mousedown="dts.handleStagePointerDown"
                @tap="dts.handleStagePointerDown"
              >
                <VLayer :ref="dtsRefs.displayLayerRef">
                  <VRect :config="stageBackgroundConfig" />

                <VGroup :config="workspaceBackgroundGroupConfig">
                  <VRect :config="workspaceBackgroundConfig" />
                </VGroup>

                <VGroup :config="workspaceCustomBackgroundGroupConfig">
                  <VRect :config="workspaceColorLayerConfig" />
                </VGroup>

                <VGroup
                  :ref="dtsRefs.displayGroupRef"
                  :config="displayGroupConfig"
                >
                  <VGroup :config="displayShapesGroupConfig">
                    <template
                      v-for="shape in dts.shapes"
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
                  <template v-if="showGrid">
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
                  </template>
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
                      v-for="shape in dts.shapes"
                      :key="shape.id"
                    >
                      <VRect
                        v-if="shape.type === 'rect'"
                        :config="getRectConfig(shape)"
                        @mousedown="dts.handleShapePointerDown"
                        @tap="dts.handleShapePointerDown"
                        @dragmove="dts.handleShapeDragMove"
                        @transform="dts.handleShapeTransform"
                        @dragend="dts.handleShapeDragEnd"
                        @transformend="dts.handleShapeTransformEnd"
                      />
                      <VText
                        v-else-if="shape.type === 'text'"
                        :config="getTextConfig(shape)"
                        @mousedown="dts.handleShapePointerDown"
                        @tap="dts.handleShapePointerDown"
                        @dragmove="dts.handleShapeDragMove"
                        @transform="dts.handleShapeTransform"
                        @dragend="dts.handleShapeDragEnd"
                        @transformend="dts.handleShapeTransformEnd"
                      />
                      <VImage
                        v-else
                        :config="getImageConfig(shape)"
                        @mousedown="dts.handleShapePointerDown"
                        @tap="dts.handleShapePointerDown"
                        @dragmove="dts.handleShapeDragMove"
                        @transform="dts.handleShapeTransform"
                        @dragend="dts.handleShapeDragEnd"
                        @transformend="dts.handleShapeTransformEnd"
                      />
                    </template>
                  </VGroup>
                </VLayer>

                <VLayer :ref="dtsRefs.overlayLayerRef">
                  <VTransformer
                    :ref="dtsRefs.transformerRef"
                    :config="transformerConfig"
                  />
                </VLayer>
              </VStage>
            </div>

            <textarea
              v-if="selectedTextTextareaStyle"
              :ref="dtsRefs.textEditorRef"
              :value="activeTextValue"
              :style="selectedTextTextareaStyle"
              autofocus
              rows="1"
              spellcheck="false"
              wrap="off"
              class="absolute z-20 resize-none overflow-hidden border-0 bg-transparent p-0 outline-none"
              @input="dts.handleTextTextareaInput"
              @keydown.enter.prevent="dts.handleTextTextareaEnter"
              @blur="dts.commitActiveTextChange"
              @mousedown.stop
              @wheel.prevent
              @click.stop
            />

            <UButton
              v-if="deleteButtonStyle"
              color="error"
              variant="solid"
              square
              size="xs"
              icon="i-bi-trash"
              class="absolute z-30 rounded-full"
              :style="deleteButtonStyle"
              @pointerdown.stop.prevent
              @click.stop="dts.deleteSelectedShape"
            />

            <UButton
              v-if="selectionHandleStyle"
              color="neutral"
              variant="solid"
              square
              size="xs"
              :icon="selectedTextShape ? 'i-bi-location' : 'i-bi-plus'"
              class="absolute z-30 rounded-full"
              :style="selectionHandleStyle"
              @pointerdown.stop.prevent="dts.handleSelectionHandlePointerDown"
            />

            <UButton
              v-if="rotationHandleStyle"
              color="neutral"
              variant="solid"
              square
              size="xs"
              icon="i-bi-arrow-clockwise"
              class="absolute z-30 rounded-full"
              :style="rotationHandleStyle"
              @pointerdown.stop.prevent="dts.handleRotationHandlePointerDown"
            />
          </div>

          <ModalGeneric
            v-model:open="dts.showImageUploadModal"
            data-id="modal-draw-tool-image-upload"
            title="Add image"
            wide
            show-close-button
            :primary-action-props="{
              label: 'Insert image',
              disabled: !dts.imageUploadFile,
              onClick: insertImage
            }"
            :secondary-action-props="{
              label: 'Cancel',
              variant: 'ghost',
              onClick: dts.resetImageUploadModal
            }"
          >
            <template #body>
              <UFileUpload
                v-model="dts.imageUploadFile"
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

    <div class="fixed inset-x-0 bottom-6 z-40 flex justify-center px-4">
      <div class="flex max-w-[calc(100vw-2rem)] items-center gap-4 overflow-x-auto rounded-2xl p-2 ring-1 ring-glass bg-surface-container backdrop-blur-sm">
        <UPopover>
          <UButton
            color="neutral"
            variant="ghost"
            :class="toolbarLabeledButtonClass"
          >
            <UIcon
              name="i-bi-background-color"
              class="size-6"
            />
            <span>Fill</span>
          </UButton>

          <template #content>
            <div class="flex flex-col gap-3 p-3">
              <ColorPicker
                :model-value="dts.backgroundColor"
                class="p-1"
                :throttle="50"
                @update:model-value="dts.handleBackgroundColorChange"
              />
              <UButton
                label="Clear background"
                color="neutral"
                variant="ghost"
                :disabled="!hasVisibleBackgroundColor"
                @click="dts.clearBackgroundColor"
              />
            </div>
          </template>
        </UPopover>

        <UPopover>
          <UButton
            color="neutral"
            variant="ghost"
            :class="toolbarLabeledButtonClass"
          >
            <UIcon
              name="i-bi-border-color"
              class="size-6"
            />
            <span>Border</span>
          </UButton>

          <template #content>
            <div class="flex w-80 flex-col gap-3 p-3">
              <ColorPicker
                :model-value="dts.borderColor"
                class="p-1"
                :throttle="50"
                @update:model-value="dts.handleBorderColorChange"
              />

              <div class="flex gap-2">
                <UButton
                  label="Reset border"
                  color="neutral"
                  variant="ghost"
                  :disabled="!canResetBorder"
                  @click="dts.clearBorderColor"
                />
              </div>

              <div class="flex flex-col gap-4 rounded-xl border border-default/70 p-3">
                <div class="flex flex-col gap-2">
                  <div class="flex items-center justify-between text-sm text-muted">
                    <span>Gap size</span>
                    <span>{{ dts.borderGapSize }}px</span>
                  </div>
                  <USlider
                    v-model="dts.borderGapSize"
                    :min="0"
                    :max="MAX_BORDER_GAP_SIZE"
                    :step="1"
                    @change="dts.handleBorderSettingsChange"
                  />
                </div>

                <div class="flex flex-col gap-2">
                  <div class="flex items-center justify-between text-sm text-muted">
                    <span>Dash size</span>
                    <span>{{ dts.borderDashSize }}px</span>
                  </div>
                  <USlider
                    v-model="dts.borderDashSize"
                    :min="1"
                    :max="MAX_BORDER_DASH_SIZE"
                    :step="1"
                    @change="dts.handleBorderSettingsChange"
                  />
                </div>

                <div class="flex flex-col gap-2">
                  <div class="flex items-center justify-between text-sm text-muted">
                    <span>Gap offset</span>
                    <span>{{ dts.borderGapOffset }}px</span>
                  </div>
                  <USlider
                    v-model="dts.borderGapOffset"
                    :min="0"
                    :max="Math.max(0, dts.borderDashSize + dts.borderGapSize)"
                    :step="1"
                    @change="dts.handleBorderSettingsChange"
                  />
                </div>
              </div>
            </div>
          </template>
        </UPopover>

        <UButton
          color="neutral"
          variant="ghost"
          :class="toolbarLabeledButtonClass"
          @click="dts.addText(activeTextValue, activeTextColor, activeTextFontId)"
        >
          <UIcon
            name="i-bi-text"
            class="size-6"
          />
          <span>Text</span>
        </UButton>

        <UButton
          color="neutral"
          variant="ghost"
          :class="toolbarLabeledButtonClass"
        >
          <UIcon
            name="i-bi-emoji"
            class="size-6"
          />
          <span>Icon</span>
        </UButton>

        <UButton
          color="neutral"
          variant="ghost"
          :class="toolbarLabeledButtonClass"
          @click="dts.showImageUploadModal = true"
        >
          <UIcon
            name="i-bi-image"
            class="size-6"
          />
          <span>Image</span>
        </UButton>

        <div class="h-[calc(100%_-_1em)] w-0.5 shrink-0 bg-accented" />

        <UButton
          color="neutral"
          variant="ghost"
          :class="toolbarLabeledButtonClass"
          @click="showGrid = !showGrid"
        >
          <UIcon
            name="i-bi-grid"
            class="size-6"
          />
          <span>Grid</span>
        </UButton>

        <div class="h-[calc(100%_-_1em)] w-0.5 shrink-0 bg-accented" />

        <UButton
          color="neutral"
          variant="ghost"
          square
          :class="toolbarIconButtonClass"
          :disabled="!(dts.historyIndex > 0)"
          @click="dts.undo"
        >
          <UIcon
            name="i-bi-undo"
            class="size-6"
          />
        </UButton>

        <UButton
          color="neutral"
          variant="ghost"
          square
          :class="toolbarIconButtonClass"
          :disabled="!(dts.historyIndex < dts.historyEntries.length - 1)"
          @click="dts.redo"
        >
          <UIcon
            name="i-bi-redo"
            class="size-6"
          />
        </UButton>

        <UButton
          color="neutral"
          variant="ghost"
          square
          :class="toolbarIconButtonClass"
          :disabled="!selectedShape"
          @click="dts.deleteSelectedShape"
        >
          <UIcon
            name="i-bi-trash"
            class="size-6"
          />
        </UButton>
      </div>
    </div>
  </div>
</template>

<script setup lang="ts">
import Konva from 'konva';
import { storeToRefs } from 'pinia';
import {
  Group as VGroup,
  Image as VImage,
  Layer as VLayer,
  Line as VLine,
  Rect as VRect,
  Stage as VStage,
  Text as VText,
  Transformer as VTransformer
} from 'vue-konva';
import type { TransformerBox } from '@/util/drawTool';

const toast = useToast();
const resizeObserver = ref<ResizeObserver | null>(null);
const showGrid = ref(true);

const dts = useDrawToolStore();
const dtsRefs = storeToRefs(dts);

const toolbarLabeledButtonClass = 'flex flex-col items-center gap-2 p-2 rounded-xl text-xs';
const toolbarIconButtonClass = 'rounded-xl';

const stageMetrics = computed(() => getStageMetrics(dts.stageWidth));

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
  listening: false
}));

const hasVisibleBackgroundColor = computed(() => !isColorFullyTransparent(dts.backgroundColor));

const workspaceBackgroundConfig = computed(() => ({
  x: 0,
  y: 0,
  width: WORKSPACE_WIDTH,
  height: WORKSPACE_HEIGHT,
  fill: DEFAULT_WORKSPACE_BACKGROUND,
  visible: !hasVisibleBackgroundColor.value,
  listening: false
}));

const workspaceColorLayerConfig = computed(() => ({
  x: 0,
  y: 0,
  width: WORKSPACE_WIDTH,
  height: WORKSPACE_HEIGHT,
  fill: dts.backgroundColor,
  visible: hasVisibleBackgroundColor.value,
  listening: false
}));

const hasVisibleBorderColor = computed(() => !isColorFullyTransparent(dts.borderColor));

const borderPixelConfigs = computed(() => {
  if (!hasVisibleBorderColor.value) {
    return [];
  }

  return createBorderPixelConfigs(
    dts.borderColor,
    BORDER_RING_PIXELS,
    dts.borderDashSize,
    dts.borderGapSize,
    dts.borderGapOffset
  );
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
    stroke: index === 0 || index === WORKSPACE_WIDTH ? '#4b5563' : DEFAULT_GRID_COLOR,
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
    stroke: index === 0 || index === WORKSPACE_HEIGHT ? '#4b5563' : DEFAULT_GRID_COLOR,
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
    x: snapStageCoordinate(newAbsPos.x, stageMetrics.value),
    y: snapStageCoordinate(newAbsPos.y, stageMetrics.value)
  })
}));

watch(() => dts.showImageUploadModal, isOpen => {
  if (!isOpen) {
    dts.imageUploadFile = null;
  }
});

watch(stageMetrics, metrics => {
  dts.setStageMetrics(metrics);
}, { immediate: true });

watch(dts.shapes, async () => {
  await nextTick();
  dts.syncPixelatedDisplay();
}, { deep: true });

watch(() => stageMetrics.value.cellSize, async () => {
  await nextTick();
  dts.syncPixelatedDisplay();
  dts.syncTransformer();
});

const selectedShape = computed(() => dts.shapes.find(shape => shape.id === dts.selectedShapeId) || null);
const selectedTextShape = computed(() => selectedShape.value?.type === 'text' ? selectedShape.value : null);
const selectedShapeSyncKey = computed(() => {
  const shape = selectedShape.value;

  if (!shape) {
    return 'none';
  }

  return shape.type === 'text'
    ? [shape.id, shape.type, shape.x, shape.y, shape.width, shape.height, shape.rotation, shape.fontId, shape.text].join(':')
    : [shape.id, shape.type, shape.x, shape.y, shape.width, shape.height, shape.rotation].join(':');
});

const canResetBorder = computed(() => {
  return dts.borderColor !== DEFAULT_BORDER_COLOR
    || (
      dts.borderGapSize !== DEFAULT_BORDER_GAP_SIZE
      || dts.borderDashSize !== DEFAULT_BORDER_DASH_SIZE
      || dts.borderGapOffset !== DEFAULT_BORDER_GAP_OFFSET
    );
});

const activeTextValue = computed(() => selectedTextShape.value?.text ?? dts.textDraftValue);
const activeTextColor = computed(() => selectedTextShape.value?.fill ?? dts.textDraftColor);
const activeTextFontId = computed(() => selectedTextShape.value?.fontId ?? dts.textDraftFontId);

const deleteButtonStyle = computed(() => {
  if (!dts.deleteButtonPosition) {
    return '';
  }

  return {
    left: `${dts.deleteButtonPosition.x}px`,
    top: `${dts.deleteButtonPosition.y}px`,
    transform: 'translate(-50%, -50%)'
  };
});
const rotationHandleStyle = computed(() => {
  if (!dts.rotationHandlePosition || selectedTextShape.value) {
    return '';
  }

  return {
    left: `${dts.rotationHandlePosition.x}px`,
    top: `${dts.rotationHandlePosition.y}px`,
    transform: 'translate(-50%, -50%)',
    cursor: dts.activeSelectionHandleDrag?.mode === 'rotate' ? 'grabbing' : 'grab',
    touchAction: 'none'
  };
});
const selectionHandleStyle = computed(() => {
  if (!dts.selectionHandlePosition) {
    return '';
  }

  return {
    left: `${dts.selectionHandlePosition.x}px`,
    top: `${dts.selectionHandlePosition.y}px`,
    transform: 'translate(-50%, -50%)',
    cursor: selectedTextShape.value ? 'move' : 'nwse-resize',
    touchAction: 'none'
  };
});
const selectedTextTextareaStyle = computed(() => {
  if (!selectedTextShape.value) {
    return '';
  }

  const font = getFontOption(selectedTextShape.value.fontId);
  const displayConfig = getDisplayTextConfig(selectedTextShape.value);
  const textConfig = getTextConfig(selectedTextShape.value);
  const fontSize = font.fontSize * stageMetrics.value.cellSize;
  const lineHeight = font.capHeight * stageMetrics.value.cellSize;
  const textareaWidth = textConfig.width * stageMetrics.value.cellSize;
  const textareaHeight = textConfig.height * stageMetrics.value.cellSize;

  return {
    left: `${stageMetrics.value.workspaceX + (displayConfig.x * stageMetrics.value.cellSize)}px`,
    top: `${stageMetrics.value.workspaceY + ((displayConfig.y + (font.fontSize - font.capHeight) / 2) * stageMetrics.value.cellSize)}px`,
    width: `${textareaWidth}px`,
    height: `${textareaHeight}px`,
    color: selectedTextShape.value.fill,
    caretColor: 'white',
    opacity: '0.9',
    fontFamily: font.family,
    fontSize: `${fontSize}px`,
    lineHeight: `${lineHeight}px`,
    overflow: 'hidden',
    whiteSpace: 'pre',
    textAlign: 'left'
  } as const;
});

watch(() => [selectedShapeSyncKey.value, stageMetrics.value.cellSize], async () => {
  await nextTick();
  dts.syncTransformer();
}, { flush: 'post' });

watch(() => selectedTextShape.value?.id, async selectedTextId => {
  if (!selectedTextId) {
    return;
  }

  await nextTick();
  dts.textEditorRef?.focus();
  const textLength = dts.textEditorRef?.value.length ?? 0;

  dts.textEditorRef?.setSelectionRange(0, textLength);
}, { flush: 'post' });

async function insertImage () {
  if (!dts.imageUploadFile) {
    return;
  }

  try {
    const imageElement = await loadImageFile(dts.imageUploadFile);

    dts.addImageShape(imageElement, dts.imageUploadFile.name);
    dts.resetImageUploadModal();
  } catch (error) {
    toast.add({
      id: 'draw-tool-image-error',
      title: 'Failed to load image',
      description: error instanceof Error ? error.message : String(error),
      color: 'error',
      duration: 10000
    });
  }
}

function buildExportLayer (): Konva.Layer {
  const layer = new Konva.Layer({ listening: false });

  if (hasVisibleBackgroundColor.value) {
    layer.add(new Konva.Rect({
      x: 0,
      y: 0,
      width: WORKSPACE_WIDTH,
      height: WORKSPACE_HEIGHT,
      fill: dts.backgroundColor,
      listening: false,
      perfectDrawEnabled: false
    }));
  }

  dts.shapes.forEach(shape => {
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

  if (hasVisibleBorderColor.value) {
    createBorderPixelConfigs(
      dts.borderColor,
      BORDER_RING_PIXELS,
      dts.borderDashSize,
      dts.borderGapSize,
      dts.borderGapOffset
    ).forEach(({ key: _key, ...config }) => {
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
  dts.measureStage();

  resizeObserver.value = new ResizeObserver(() => {
    dts.measureStage();
  });

  if (dts.stageContainerRef) {
    resizeObserver.value.observe(dts.stageContainerRef);
  }

  loadEditorFonts().then(() => {
    nextTick(() => dts.syncPixelatedDisplay());
  });
  nextTick(() => dts.syncPixelatedDisplay());
});

onBeforeUnmount(() => {
  dts.stopSelectionHandleDrag(false);
  resizeObserver.value?.disconnect();
});
</script>
