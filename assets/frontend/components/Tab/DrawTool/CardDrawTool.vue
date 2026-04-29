<template>
  <div ref="drawToolRootRef">
    <SectionCard
      data-id="draw-tool-section-primary"
      class="overflow-visible"
      :title="statusFileName"
      :subtitle="dts.hasUnsavedChanges ? 'Unsaved changes' : undefined"
      :ui="{
        title: 'text-lg',
        subtitle: 'text-sm',
        titleWrapper: 'h-12'
      }"
    >
      <template #actions>
        <UTooltip
          :delay-duration="0"
          :text="!dts.hasEditorContent ? 'Nothing to save' : undefined"
        >
          <UButtonGroup>
            <UButton
              label="Save"
              color="neutral"
              variant="outline"
              :disabled="!dts.hasEditorContent"
              @click="saveStatus()"
            />
            <UDropdownMenu
              :items="[
                ...(hasSavedStatusFile
                  ? [
                    {
                      label: 'Save as new file',
                      icon: 'i-bi-plus',
                      disabled: !dts.hasEditorContent,
                      onClick: () => saveStatus({ saveAsNew: true })
                    }
                  ]
                  : []),
                {
                  label: 'Download PNG',
                  icon: 'i-bi-download',
                  onClick: downloadImage
                }
              ]"
              :content="{
                align: 'end',
                side: 'bottom',
                sideOffset: 8
              }"
              :ui="{
                content: 'min-w-40 bg-elevated ring-accented/50',
                group: 'border-accented/50',
                item: 'data-[state=open]:before:bg-accented/50 data-highlighted:before:bg-accented/50',
                itemLabelExternalIcon: 'hidden'
              }"
            >
              <UButton
                icon="i-bi-chevron-down"
                color="neutral"
                variant="outline"
                :disabled="!dts.hasEditorContent"
              />
            </UDropdownMenu>
          </UButtonGroup>
        </UTooltip>

        <UTooltip
          :delay-duration="0"
          :text="!dts.hasEditorContent ? 'Nothing to show' : undefined"
        >
          <UButton
            label="Show on BUSY Bar"
            color="neutral"
            variant="solid"
            :icon="showStatusCheckmarkIcon ? 'i-bi-checkmark' : 'i-bi-play-fill'"
            :disabled="!dts.hasEditorContent"
            :loading="isShowingStatusOnDevice"
            :ui="{
              leadingIcon: `${showStatusCheckmarkIcon ? 'text-success' : ''}`
            }"
            @click="showStatusOnBusyBar"
          />
        </UTooltip>
      </template>
      <template #raw-body>
        <div class="flex flex-col gap-4">
          <div
            :ref="dtsRefs.stageContainerRef"
            class="relative w-full min-h-[400px] rounded-[28px]"
          >
            <div class="w-full overflow-hidden rounded-2xl bg-neutral-500 dark:bg-neutral-950">
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

            <div
              v-if="selectedTextMenuStyle"
              class="absolute z-30 flex items-center gap-2 bg-transparent"
              :style="selectedTextMenuStyle"
              data-draw-tool-preserve-selection
              @mousedown.stop
              @click.stop
            >
              <USelect
                :model-value="activeTextFontId"
                :items="TEXT_FONT_OPTIONS"
                :value-key="'id'"
                :ui="{ content: 'draw-tool-preserve-selection' }"
                color="neutral"
                variant="outline"
                size="sm"
                class="w-40"
                data-draw-tool-preserve-selection
                @update:model-value="dts.handleActiveTextFontChange"
                >
                  <template #default>
                    <span
                      :style="{
                        fontFamily: activeTextFont.family,
                        fontSize: `calc(1rem + ${activeTextFont.fontSize - activeTextFont.capHeight}px)`,
                        lineHeight: '1rem'
                      }"
                    >
                      {{ activeTextFont.label }}
                    </span>
                  </template>

                  <template #item-label="{ item }">
                    <span
                      :style="{
                        fontFamily: item.family,
                        fontSize: `calc(1rem + ${item.fontSize - item.capHeight}px)`,
                        lineHeight: '1rem'
                      }"
                    >
                      {{ item.label }}
                    </span>
                  </template>
                </USelect>

              <UPopover>
                <UButton
                  color="neutral"
                  variant="outline"
                  size="sm"
                  label="Color"
                  class="bg-default"
                  data-draw-tool-preserve-selection
                >
                  <template #leading>
                    <span
                      class="size-3 rounded-full ring-1 ring-default"
                      :style="`background-color: ${activeTextColor}`"
                    />
                  </template>
                </UButton>

                <template #content>
                  <div
                    class="p-3"
                    data-draw-tool-preserve-selection
                  >
                    <ColorPicker
                      data-draw-tool-preserve-selection
                      :model-value="activeTextColor"
                      class="p-1"
                      :throttle="50"
                      @update:model-value="dts.handleActiveTextColorChange"
                    />
                  </div>
                </template>
              </UPopover>
            </div>

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
              :icon="selectedTextShape ? 'i-bi-move' : 'i-bi-resize-alt'"
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
              icon="i-bi-rotate"
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

    <div
      ref="toolbarContainerRef"
      :class="toolbarContainerClass"
      :style="toolbarContainerStyle"
    >
      <div class="flex max-w-[calc(100vw-2rem)] items-center gap-4 overflow-x-auto rounded-2xl p-2 ring-1 ring-glass bg-surface-container backdrop-blur-sm">
        <UPopover
          :content="{
            side: 'top',
            sideOffset: 16
          }"
          :ui="{
            content: 'rounded-xl bg-surface-container ring-accented/75'
          }"
        >
          <UTooltip
            :delay-duration="0"
            :content="{
              side: 'top',
              sideOffset: 16
            }"
            text="Background color"
          >
            <UButton
              color="neutral"
              variant="ghost"
              square
              :class="toolbarIconButtonClass"
            >
              <UIcon
                name="i-bi-background-color"
                class="size-6"
              />
            </UButton>
          </UTooltip>

          <template #content>
            <div class="flex flex-col items-end gap-3 p-3">
              <ColorPicker
                :model-value="dts.backgroundColor"
                class="p-1"
                :throttle="50"
                @update:model-value="dts.handleBackgroundColorChange"
              />
              <UButton
                label="Clear background"
                color="neutral"
                variant="outline"
                size="sm"
                :disabled="!hasVisibleBackgroundColor"
                @click="dts.clearBackgroundColor"
              />
            </div>
          </template>
        </UPopover>

        <UPopover
          :content="{
            side: 'top',
            sideOffset: 16
          }"
          :ui="{
            content: 'rounded-xl bg-surface-container ring-accented/75'
          }"
        >
          <UTooltip
            :delay-duration="0"
            :content="{
              side: 'top',
              sideOffset: 16
            }"
            text="Border"
          >
            <UButton
              color="neutral"
              variant="ghost"
              square
              :class="toolbarIconButtonClass"
            >
              <UIcon
                name="i-bi-border-color"
                class="size-6"
              />
            </UButton>
          </UTooltip>

          <template #content>
            <div class="flex gap-6 p-3">
              <ColorPicker
                :model-value="dts.borderColor"
                class="p-1"
                :throttle="50"
                @update:model-value="dts.handleBorderColorChange"
              />

              <div class="flex flex-col gap-4 rounded-xl">
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
                    class="min-w-36"
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

                <div class="flex justify-end">
                  <UButton
                    label="Reset border"
                    color="neutral"
                    variant="outline"
                    size="sm"
                    :disabled="!canResetBorder"
                    @click="dts.clearBorderColor"
                  />
                </div>
              </div>
            </div>
          </template>
        </UPopover>

        <UTooltip
          :delay-duration="0"
          :content="{
            side: 'top',
            sideOffset: 16
          }"
          text="Text"
        >
          <UButton
            color="neutral"
            variant="ghost"
            square
            :class="toolbarIconButtonClass"
            data-draw-tool-preserve-selection
            @click="dts.addText(activeTextValue, activeTextColor, activeTextFontId)"
          >
            <UIcon
              name="i-bi-text"
              class="size-6"
            />
          </UButton>
        </UTooltip>

        <UPopover
          v-model:open="isIconPickerOpen"
          :content="{
            side: 'top',
            sideOffset: 16
          }"
          :ui="{
            content: 'rounded-xl bg-surface-container ring-accented/75'
          }"
        >
          <UTooltip
            v-model:open="isIconTooltipOpen"
            :delay-duration="0"
            :disabled="isIconTooltipSuppressed"
            :content="{
              side: 'top',
              sideOffset: 16
            }"
            text="Icon"
          >
            <UButton
              color="neutral"
              variant="ghost"
              square
              :class="toolbarIconButtonClass"
              data-draw-tool-preserve-selection
              @click="handleIconTriggerClick"
              @pointerleave="handleIconTriggerPointerLeave"
            >
              <UIcon
                name="i-bi-emoji"
                class="size-6"
              />
            </UButton>
          </UTooltip>

          <template #content>
            <div
              class="max-w-[calc(100vw-2rem)] overflow-y-auto px-2 pt-1 pb-2"
              data-draw-tool-preserve-selection
            >
              <UTabs
                v-model="activeIconCategoryIndex"
                :items="iconCategories"
                :content="false"
                color="neutral"
                variant="link"
                class="w-full mb-3"
              />

              <div class="h-36 overflow-y-auto">
                <div
                  v-if="activeIcons.length"
                  class="grid grid-cols-4 gap-2 overflow-y-auto pr-1 sm:grid-cols-10"
                  data-draw-tool-preserve-selection
                >
                  <UButton
                    v-for="icon in activeIcons"
                    :key="icon.id"
                    data-draw-tool-preserve-selection
                    color="neutral"
                    variant="ghost"
                    class="p-1 rounded-xl"
                    @click="insertDrawToolIcon(icon)"
                  >
                    <img
                      :src="icon.src"
                      class="size-12 object-contain"
                      draggable="false"
                    >
                  </UButton>
                </div>
                <div
                  v-else
                  class="flex min-h-32 items-center justify-center rounded-xl border border-dashed border-accented/40 px-4 text-sm text-muted"
                  data-draw-tool-preserve-selection
                >
                  No icons in this category.
                </div>
              </div>
            </div>
          </template>
        </UPopover>

        <UTooltip
          :delay-duration="0"
          :content="{
            side: 'top',
            sideOffset: 16
          }"
          text="Image"
        >
          <UButton
            color="neutral"
            variant="ghost"
            square
            :class="toolbarIconButtonClass"
            @click="dts.showImageUploadModal = true"
          >
            <UIcon
              name="i-bi-image"
              class="size-6"
            />
          </UButton>
        </UTooltip>

        <div class="h-[calc(100%_-_1em)] w-0.5 shrink-0 bg-accented" />

        <UTooltip
          :delay-duration="0"
          :content="{
            side: 'top',
            sideOffset: 16
          }"
          :text="showGrid ? 'Hide grid' : 'Show grid'"
        >
          <UButton
            color="neutral"
            variant="ghost"
            square
            :class="toolbarIconButtonClass"
            @click="showGrid = !showGrid"
          >
            <UIcon
              :name="showGrid ? 'i-bi-grid' : 'i-bi-grid-off'"
              class="size-6"
            />
          </UButton>
        </UTooltip>

        <div class="h-[calc(100%_-_1em)] w-0.5 shrink-0 bg-accented" />

        <UTooltip
          :delay-duration="0"
          :content="{
            side: 'top',
            sideOffset: 16
          }"
          text="Undo"
        >
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
        </UTooltip>

        <UTooltip
          :delay-duration="0"
          :content="{
            side: 'top',
            sideOffset: 16
          }"
          text="Redo"
        >
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
        </UTooltip>

        <UTooltip
          :delay-duration="0"
          :content="{
            side: 'top',
            sideOffset: 16
          }"
          text="Clear editor"
        >
          <UButton
            color="neutral"
            variant="ghost"
            square
            :class="toolbarIconButtonClass"
            :disabled="!dts.hasEditorContent"
            @click="dts.clearStage"
          >
            <UIcon
              name="i-bi-trash"
              class="size-6"
            />
          </UButton>
        </UTooltip>
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
import drawToolIconsData from '@/generated/drawTool/icons.json';
import type { TransformerBox } from '@/util/drawTool';
import type { DisplayDrawParams } from '@busy-app/busy-lib';

type DrawToolIcon = {
  id: string;
  fileName: string;
  path: string;
};

type ResolvedDrawToolIcon = DrawToolIcon & {
  src: string;
};

const toast = useToast();

const drawToolRootRef = ref<HTMLDivElement | null>(null);
const resizeObserver = ref<ResizeObserver | null>(null);
const pixelatedDisplayFrame = ref<number | null>(null);
const transformerFrame = ref<number | null>(null);
const stageContainerViewportTop = ref(0);
const preserveSelectionUntilClick = ref(false);
const toolbarContainerRef = ref<HTMLDivElement | null>(null);
const toolbarFixedTop = ref(0);
const toolbarShouldStickToViewport = ref(true);
const showGrid = ref(true);

const dts = useDrawToolStore();
const dtsRefs = storeToRefs(dts);
const { statusFileName, savedStatusFilePath, hasSavedStatusFile } = dtsRefs;

const FLOATING_TEXT_MENU_HEIGHT = 40;
const FLOATING_TEXT_MENU_GAP = 8;
const DRAW_TOOL_DISPLAY_APPLICATION_NAME = 'draw_tool';
const DRAW_TOOL_TEMP_FILE_NAME = 'temp.png';
const DRAW_TOOL_SAVE_DIR = '/ext/user_assets/draw_tool';
const TOOLBAR_MAX_CANVAS_GAP = 48;
const TOOLBAR_VIEWPORT_BOTTOM_OFFSET = 24;

// const toolbarLabeledButtonClass = 'flex flex-col items-center gap-2 p-2 rounded-lg text-xs';
const toolbarIconButtonClass = 'rounded-lg';

const isSavingStatus = ref(false);
const isShowingStatusOnDevice = ref(false);

const isIconPickerOpen = ref(false);
const isIconTooltipOpen = ref(false);
const isIconTooltipSuppressed = ref(false);
const iconAssets = import.meta.glob('../../../assets/icons/draw_tool/**/*.svg', {
  eager: true,
  import: 'default',
  query: '?url'
}) as Record<string, string>;
const iconsByCategory = drawToolIconsData as Record<string, DrawToolIcon[]>;
const iconCategoriesMap = [
  { id: 'faces', label: 'Smiles & Emotions' },
  { id: 'food', label: 'Food & Drinks' },
  { id: 'nature', label: 'Nature' },
  { id: 'work', label: 'Work & Study' },
  { id: 'sport', label: 'Sports' },
  { id: 'hearts', label: 'Hearts & Sparks' }
];
const iconCategories = Object.keys(iconsByCategory).map(category => {
  const mappedCategory = iconCategoriesMap.find(c => c.id === category);
  if (mappedCategory) {
    return mappedCategory;
  }

  return {
    id: category,
    label: category.charAt(0).toUpperCase() + category.slice(1)
  };
});
const iconUrlByPath = Object.fromEntries(Object.entries(iconAssets).flatMap(([assetPath, assetUrl]) => {
  const normalizedAssetPath = assetPath
    .replace(/\\/g, '/')
    .match(/assets\/icons\/(.+)$/)?.[1];

  return normalizedAssetPath ? [[normalizedAssetPath, assetUrl]] : [];
})) as Record<string, string>;
const activeIconCategoryIndex = ref(0);
const activeIconCategory = computed(() => iconCategories[activeIconCategoryIndex.value].id);
const activeIcons = computed<ResolvedDrawToolIcon[]>(() => {
  return (iconsByCategory[activeIconCategory.value] || []).flatMap(icon => {
    const src = iconUrlByPath[icon.path];

    return src ? [{ ...icon, src }] : [];
  });
});

const toolbarContainerClass = computed(() => {
  return toolbarShouldStickToViewport.value
    ? 'fixed inset-x-0 z-40 flex justify-center px-4'
    : 'mt-6 flex justify-center px-4';
});
const toolbarContainerStyle = computed(() => {
  if (!toolbarShouldStickToViewport.value) {
    return undefined;
  }

  return {
    top: `${toolbarFixedTop.value}px`
  };
});

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
  fill: 'transparent',
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
    stroke: index === 0 || index === WORKSPACE_WIDTH ? DEFAULT_GRID_BORDER_COLOR : DEFAULT_GRID_COLOR,
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
    stroke: index === 0 || index === WORKSPACE_HEIGHT ? DEFAULT_GRID_BORDER_COLOR : DEFAULT_GRID_COLOR,
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
  anchorStroke: '#0a0a0a',
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

watch(() => dts.shapes, async () => {
  await nextTick();
  schedulePixelatedDisplaySync();
}, { deep: true });

watch(() => stageMetrics.value.cellSize, async () => {
  await nextTick();
  schedulePixelatedDisplaySync();
  scheduleTransformerSync();
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
const activeTextFont = computed(() => getFontOption(activeTextFontId.value));
const selectedTextEditorBounds = computed(() => {
  if (!selectedTextShape.value) {
    return null;
  }

  const font = getFontOption(selectedTextShape.value.fontId);
  const displayConfig = getDisplayTextConfig(selectedTextShape.value);
  const textConfig = getTextConfig(selectedTextShape.value);

  return {
    left: stageMetrics.value.workspaceX + (displayConfig.x * stageMetrics.value.cellSize),
    top: stageMetrics.value.workspaceY + ((displayConfig.y + (font.fontSize - font.capHeight) / 2) * stageMetrics.value.cellSize),
    width: textConfig.width * stageMetrics.value.cellSize,
    height: textConfig.height * stageMetrics.value.cellSize,
    fontFamily: font.family,
    fontSize: font.fontSize * stageMetrics.value.cellSize,
    lineHeight: font.capHeight * stageMetrics.value.cellSize
  };
});

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
    cursor: selectedTextShape.value ? dts.activeSelectionHandleDrag?.mode === 'move' ? 'grabbing' : 'grab' : 'nwse-resize',
    touchAction: 'none'
  };
});
const selectedTextTextareaStyle = computed(() => {
  if (!selectedTextShape.value || !selectedTextEditorBounds.value) {
    return '';
  }

  return {
    left: `${selectedTextEditorBounds.value.left}px`,
    top: `${selectedTextEditorBounds.value.top}px`,
    width: `${selectedTextEditorBounds.value.width}px`,
    height: `${selectedTextEditorBounds.value.height}px`,
    color: selectedTextShape.value.fill,
    caretColor: 'white',
    opacity: '0.9',
    fontFamily: selectedTextEditorBounds.value.fontFamily,
    fontSize: `${selectedTextEditorBounds.value.fontSize}px`,
    lineHeight: `${selectedTextEditorBounds.value.lineHeight}px`,
    overflow: 'hidden',
    whiteSpace: 'pre',
    textAlign: 'left'
  } as const;
});
const selectedTextMenuStyle = computed<Record<string, string> | null>(() => {
  if (!selectedTextEditorBounds.value) {
    return null;
  }

  const editorTopInViewport = stageContainerViewportTop.value + selectedTextEditorBounds.value.top;
  const hasSpaceAbove = editorTopInViewport >= FLOATING_TEXT_MENU_HEIGHT + FLOATING_TEXT_MENU_GAP;
  const top = hasSpaceAbove
    ? selectedTextEditorBounds.value.top - FLOATING_TEXT_MENU_HEIGHT - FLOATING_TEXT_MENU_GAP
    : selectedTextEditorBounds.value.top + selectedTextEditorBounds.value.height + FLOATING_TEXT_MENU_GAP;

  return {
    left: `${selectedTextEditorBounds.value.left}px`,
    top: `${top}px`
  };
});

watch(() => [selectedShapeSyncKey.value, stageMetrics.value.cellSize], async () => {
  await nextTick();
  scheduleTransformerSync();
}, { flush: 'post' });

watch(() => selectedTextShape.value?.id, async selectedTextId => {
  if (!selectedTextId) {
    return;
  }

  await nextTick();
  updateStageContainerViewportTop();
  dts.textEditorRef?.focus();
  const textLength = dts.textEditorRef?.value.length ?? 0;

  dts.textEditorRef?.setSelectionRange(0, textLength);
}, { flush: 'post' });

function updateStageContainerViewportTop () {
  const stageContainerRect = dts.stageContainerRef?.getBoundingClientRect();
  const componentRootRect = drawToolRootRef.value?.getBoundingClientRect();
  const toolbarHeight = toolbarContainerRef.value?.getBoundingClientRect().height ?? 0;
  const desiredToolbarTop = window.innerHeight - toolbarHeight - TOOLBAR_VIEWPORT_BOTTOM_OFFSET;

  stageContainerViewportTop.value = stageContainerRect?.top ?? 0;

  if (!stageContainerRect || !componentRootRect) {
    toolbarFixedTop.value = Math.max(0, desiredToolbarTop);
    toolbarShouldStickToViewport.value = true;
    return;
  }

  toolbarFixedTop.value = Math.max(componentRootRect.top, desiredToolbarTop);
  toolbarShouldStickToViewport.value = toolbarFixedTop.value - stageContainerRect.bottom <= TOOLBAR_MAX_CANVAS_GAP;
}

function shouldPreserveSelectionForEvent (event: Event) {
  const path = typeof event.composedPath === 'function' ? event.composedPath() : [];

  if (dts.stageContainerRef && path.includes(dts.stageContainerRef)) {
    return true;
  }

  return path.some(target => {
    return target instanceof HTMLElement
      && (
        target.hasAttribute('data-draw-tool-preserve-selection')
        || target.classList.contains('draw-tool-preserve-selection')
      );
  });
}

function handleWindowPointerDown (event: PointerEvent) {
  preserveSelectionUntilClick.value = shouldPreserveSelectionForEvent(event);
}

function handleWindowClick (event: MouseEvent) {
  if (!dts.selectedShapeId) {
    preserveSelectionUntilClick.value = false;
    return;
  }

  const shouldPreserveSelection = preserveSelectionUntilClick.value || shouldPreserveSelectionForEvent(event);
  preserveSelectionUntilClick.value = false;

  if (shouldPreserveSelection) {
    return;
  }

  dts.commitActiveTextChange();
  dts.selectedShapeId = null;
}

function isEditableKeyboardTarget (target: EventTarget | null) {
  if (!(target instanceof HTMLElement)) {
    return false;
  }

  return target instanceof HTMLInputElement
    || target instanceof HTMLTextAreaElement
    || target instanceof HTMLSelectElement
    || target.isContentEditable;
}

function handleWindowKeyDown (event: KeyboardEvent) {
  const normalizedKey = event.key.toLowerCase();
  const isPrimaryModifierPressed = (event.metaKey || event.ctrlKey) && !event.altKey;

  if (isPrimaryModifierPressed && normalizedKey === 's' && !event.shiftKey) {
    event.preventDefault();

    if (!isSavingStatus.value) {
      void saveStatus();
    }

    return;
  }

  if (isEditableKeyboardTarget(event.target)) {
    return;
  }

  const isUndoModifierPressed = isPrimaryModifierPressed;

  if (isUndoModifierPressed) {
    if (normalizedKey === 'z') {
      event.preventDefault();

      if (event.shiftKey) {
        dts.redo();
      } else {
        dts.undo();
      }

      return;
    }

    if (normalizedKey === 'y') {
      event.preventDefault();
      dts.redo();
      return;
    }

    return;
  }

  if (!dts.selectedShapeId || event.ctrlKey || event.metaKey || event.altKey) {
    return;
  }

  if (event.key === 'Backspace' || event.key === 'Delete') {
    event.preventDefault();
    dts.deleteSelectedShape();
    return;
  }

  if (event.key === 'Escape') {
    event.preventDefault();
    dts.commitActiveTextChange();
    dts.selectedShapeId = null;
    return;
  }

  if (normalizedKey === 'q') {
    event.preventDefault();
    dts.rotateSelectedShape(-ROTATION_SNAP_STEP);
    return;
  }

  if (normalizedKey === 'e') {
    event.preventDefault();
    dts.rotateSelectedShape(ROTATION_SNAP_STEP);
    return;
  }

  const movementByKey: Record<string, { x: number; y: number }> = {
    ArrowUp: { x: 0, y: -1 },
    ArrowDown: { x: 0, y: 1 },
    ArrowLeft: { x: -1, y: 0 },
    ArrowRight: { x: 1, y: 0 }
  };
  const movement = movementByKey[event.key];

  if (!movement) {
    return;
  }

  event.preventDefault();
  dts.moveSelectedShape(movement.x, movement.y);
}

function schedulePixelatedDisplaySync () {
  if (pixelatedDisplayFrame.value !== null) {
    return;
  }

  pixelatedDisplayFrame.value = requestAnimationFrame(() => {
    pixelatedDisplayFrame.value = null;
    dts.syncPixelatedDisplay();
  });
}

function scheduleTransformerSync () {
  if (transformerFrame.value !== null) {
    return;
  }

  transformerFrame.value = requestAnimationFrame(() => {
    transformerFrame.value = null;
    dts.syncTransformer();
  });
}

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

async function loadImageFromUrl (url: string): Promise<HTMLImageElement> {
  return new Promise((resolve, reject) => {
    const image = new window.Image();

    image.onload = () => {
      resolve(image);
    };
    image.onerror = () => {
      reject(new Error('Could not load icon asset'));
    };

    image.src = url;
  });
}

function handleIconTriggerClick () {
  isIconTooltipOpen.value = false;
  isIconTooltipSuppressed.value = true;
}

function handleIconTriggerPointerLeave () {
  isIconTooltipSuppressed.value = false;
}

async function insertDrawToolIcon (icon: ResolvedDrawToolIcon) {
  try {
    const imageElement = await loadImageFromUrl(icon.src);

    dts.addImageShape(imageElement, icon.fileName);
    isIconTooltipOpen.value = false;
    isIconTooltipSuppressed.value = true;
    isIconPickerOpen.value = false;
  } catch (error) {
    toast.add({
      id: 'draw-tool-icon-error',
      title: 'Failed to insert icon',
      description: error instanceof Error ? error.message : String(error),
      color: 'error',
      duration: 10000
    });
  }
}

function buildExportLayer (scale = 1): Konva.Layer {
  const layer = new Konva.Layer({
    listening: false,
    scaleX: scale,
    scaleY: scale
  });

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
      layer.add(new Konva.Text(getDisplayTextConfig(shape)));

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

function parseRgbaKey (key: string) {
  const [red, green, blue, alpha] = key.split(',').map(Number);

  return { red, green, blue, alpha };
}

function getDominantCellColor (
  pixels: Uint8ClampedArray,
  sourceWidth: number,
  startX: number,
  startY: number,
  cellSize: number
) {
  let dominantKey = '0,0,0,0';
  let dominantCount = 0;
  const colorCounts = new Map<string, number>();

  for (let y = startY; y < startY + cellSize; y += 1) {
    for (let x = startX; x < startX + cellSize; x += 1) {
      const pixelIndex = ((y * sourceWidth) + x) * 4;
      const colorKey = [
        pixels[pixelIndex],
        pixels[pixelIndex + 1],
        pixels[pixelIndex + 2],
        pixels[pixelIndex + 3]
      ].join(',');
      const nextCount = (colorCounts.get(colorKey) || 0) + 1;

      colorCounts.set(colorKey, nextCount);

      if (nextCount > dominantCount) {
        dominantKey = colorKey;
        dominantCount = nextCount;
      }
    }
  }

  return parseRgbaKey(dominantKey);
}

function buildLogicalExportCanvas (sourceCanvas: HTMLCanvasElement, cellSize: number) {
  const sourceContext = sourceCanvas.getContext('2d', { willReadFrequently: true });
  const logicalCanvas = document.createElement('canvas');
  const logicalContext = logicalCanvas.getContext('2d');

  logicalCanvas.width = WORKSPACE_WIDTH;
  logicalCanvas.height = WORKSPACE_HEIGHT;

  if (!sourceContext || !logicalContext) {
    throw new Error('Could not create export canvas context');
  }

  const sourceImageData = sourceContext.getImageData(0, 0, sourceCanvas.width, sourceCanvas.height);
  const logicalImageData = logicalContext.createImageData(WORKSPACE_WIDTH, WORKSPACE_HEIGHT);

  for (let y = 0; y < WORKSPACE_HEIGHT; y += 1) {
    for (let x = 0; x < WORKSPACE_WIDTH; x += 1) {
      const dominantColor = getDominantCellColor(
        sourceImageData.data,
        sourceImageData.width,
        x * cellSize,
        y * cellSize,
        cellSize
      );
      const logicalPixelIndex = ((y * WORKSPACE_WIDTH) + x) * 4;

      logicalImageData.data[logicalPixelIndex] = dominantColor.red;
      logicalImageData.data[logicalPixelIndex + 1] = dominantColor.green;
      logicalImageData.data[logicalPixelIndex + 2] = dominantColor.blue;
      logicalImageData.data[logicalPixelIndex + 3] = dominantColor.alpha;
    }
  }

  logicalContext.putImageData(logicalImageData, 0, 0);

  return logicalCanvas;
}

function createStatusTimestamp (date = new Date()) {
  const year = date.getFullYear();
  const month = String(date.getMonth() + 1).padStart(2, '0');
  const day = String(date.getDate()).padStart(2, '0');
  const hours = String(date.getHours()).padStart(2, '0');
  const minutes = String(date.getMinutes()).padStart(2, '0');
  const seconds = String(date.getSeconds()).padStart(2, '0');

  return `${year}-${month}-${day}_${hours}-${minutes}-${seconds}`;
}

function createNextStatusFileName () {
  let candidate = `${createStatusTimestamp()}.png`;

  if (candidate === statusFileName.value) {
    candidate = `${createStatusTimestamp(new Date(Date.now() + 1000))}.png`;
  }

  return candidate;
}

function renderExportPngDataUrl () {
  let exportStage: Konva.Stage | null = null;
  let exportContainer: HTMLDivElement | null = null;

  try {
    const exportCellSize = Math.max(1, stageMetrics.value.cellSize);
    const exportSurface = createExportStage(
      WORKSPACE_WIDTH * exportCellSize,
      WORKSPACE_HEIGHT * exportCellSize
    );
    exportStage = exportSurface.stage;
    exportContainer = exportSurface.container;

    const layer = buildExportLayer(exportCellSize);
    exportStage.add(layer);
    layer.draw();

    const sourceCanvas = exportStage.toCanvas({ pixelRatio: 1 });
    const logicalCanvas = buildLogicalExportCanvas(sourceCanvas, exportCellSize);

    return logicalCanvas.toDataURL('image/png');
  } finally {
    exportStage?.destroy();
    exportContainer?.remove();
  }
}

async function createExportPngFile (fileName: string) {
  const dataUrl = renderExportPngDataUrl();
  const response = await fetch(dataUrl);
  const blob = await response.blob();

  return new File([blob], fileName, { type: 'image/png' });
}

function isAssetDeleteFailedError (error: unknown) {
  return String(error).includes('File delete failed');
}

async function clearStatusDisplay (): Promise<void> {
  const deviceStore = useDeviceStore();

  try {
    await deviceStore.busyBar.DisplayClear({
      application_name: DRAW_TOOL_DISPLAY_APPLICATION_NAME
    });
  } catch (error) {
    await handleHTTPError(error, 'Couldn\'t clear existing status display', true);
    throw error;
  }
}

async function deleteStatusAssets (hasRetried = false): Promise<void> {
  const deviceStore = useDeviceStore();

  try {
    await deviceStore.busyBar.AssetsDelete({
      application_name: DRAW_TOOL_DISPLAY_APPLICATION_NAME
    });
  } catch (error) {
    if (String(error).includes('Assets missing')) {
      return;
    }

    if (!hasRetried && isAssetDeleteFailedError(error)) {
      await clearStatusDisplay();
      await deleteStatusAssets(true);
      return;
    }

    await handleHTTPError(error, 'Couldn\'t delete existing status assets', true);
    throw error;
  }
}

async function uploadStatusAsset (image: Blob) {
  const deviceStore = useDeviceStore();

  return deviceStore.busyBar.AssetsUpload({
    application_name: DRAW_TOOL_DISPLAY_APPLICATION_NAME,
    data: image,
    file: DRAW_TOOL_TEMP_FILE_NAME
  })
    .catch(async error => {
      await handleHTTPError(error, 'Couldn\'t upload status image', true);
      throw error;
    });
}

async function drawStatusOnBusyBar (fileName: string) {
  const deviceStore = useDeviceStore();

  return deviceStore.busyBar.DisplayDraw({
    application_name: DRAW_TOOL_DISPLAY_APPLICATION_NAME,
    elements: [
      {
        id: '0',
        timeout: 0,
        align: 'top_left',
        display: 'front',
        x: 0,
        y: 0,
        type: 'image',
        path: fileName
      }
    ],
    priority: 50
  } as DisplayDrawParams)
    .catch(async error => {
      await handleHTTPError(error, 'Display draw command failed', true);
      throw error;
    });
}

async function listSaveDirectory () {
  const deviceStore = useDeviceStore();
  const result = await deviceStore.busyBar.StorageListGet({ path: DRAW_TOOL_SAVE_DIR, timeout: 10000 });

  if (!result.list) {
    throw new Error('Empty response');
  }

  return result.list;
}

async function tryListSaveDirectory () {
  try {
    return await listSaveDirectory();
  } catch {
    return null;
  }
}

async function ensureSaveDirectoryExists () {
  const deviceStore = useDeviceStore();

  try {
    await deviceStore.busyBar.StorageMkdir({ path: DRAW_TOOL_SAVE_DIR, timeout: 10000 });
  } catch {
    // Ignore mkdir failure here and let the follow-up list call decide whether the directory is usable.
  }

  return await listSaveDirectory();
}

async function initializeSaveDirectory () {
  const listedFiles = await tryListSaveDirectory();

  if (listedFiles) {
    console.debug('Draw tool save directory contents', listedFiles);
    return;
  }

  try {
    const ensuredFiles = await ensureSaveDirectoryExists();
    console.debug('Draw tool save directory contents', ensuredFiles);
  } catch (error) {
    await handleHTTPError(error, `Couldn't prepare ${DRAW_TOOL_SAVE_DIR}`, false, 10000);
  }
}

async function writeStatusFile (path: string, file: File) {
  const deviceStore = useDeviceStore();

  await deviceStore.busyBar.StorageWrite({
    path,
    file,
    timeout: 0
  });
}

async function saveStatus (options?: { saveAsNew?: boolean }) {
  if (!dts.hasEditorContent) {
    return;
  }

  const saveAsNew = !!options?.saveAsNew;
  const hadSavedStatusFile = hasSavedStatusFile.value;
  const fileName = saveAsNew || !savedStatusFilePath.value
    ? createNextStatusFileName()
    : statusFileName.value;
  const targetPath = saveAsNew || !savedStatusFilePath.value
    ? `${DRAW_TOOL_SAVE_DIR}/${fileName}`
    : savedStatusFilePath.value;

  if (!targetPath) {
    return;
  }

  isSavingStatus.value = true;

  try {
    const file = await createExportPngFile(fileName);

    try {
      await writeStatusFile(targetPath, file);
    } catch (error) {
      const listedFiles = await tryListSaveDirectory();

      if (listedFiles) {
        await handleHTTPError(error, `Couldn't save ${fileName}`, false, 10000);
        return;
      }

      try {
        await ensureSaveDirectoryExists();
        await writeStatusFile(targetPath, file);
      } catch (retryError) {
        await handleHTTPError(retryError, `Couldn't save ${fileName}`, false, 10000);
        return;
      }
    }

    dts.markStatusSaved(fileName, targetPath);
    toast.add({
      id: 'draw-tool-save-success',
      title: saveAsNew || !hadSavedStatusFile ? 'Saved as new file' : 'Saved to device',
      description: fileName /* `${DRAW_TOOL_SAVE_DIR}/${fileName}` */,
      color: 'success'
    });
  } catch (error) {
    await handleHTTPError(error, `Couldn't save ${fileName}`, false, 10000);
  } finally {
    isSavingStatus.value = false;
  }
}

const showStatusCheckmarkIcon = ref(false);
async function showStatusOnBusyBar () {
  if (!dts.hasEditorContent) {
    return;
  }

  isShowingStatusOnDevice.value = true;

  try {
    if (!hasSavedStatusFile.value || dts.hasUnsavedChanges) {
      const image = await createExportPngFile(DRAW_TOOL_TEMP_FILE_NAME);

      await deleteStatusAssets();
      await uploadStatusAsset(image);
      await clearStatusDisplay();
      await drawStatusOnBusyBar(DRAW_TOOL_TEMP_FILE_NAME);
    } else {
      await clearStatusDisplay();
      await drawStatusOnBusyBar(statusFileName.value);
    }

    showStatusCheckmarkIcon.value = true;
    setTimeout(() => {
      showStatusCheckmarkIcon.value = false;
    }, 5000);
  } catch {
    // Request errors are already handled by the helper chain above.
  } finally {
    isShowingStatusOnDevice.value = false;
  }
}

async function downloadImage () {
  try {
    const file = await createExportPngFile('draw-tool.png');
    const link = document.createElement('a');
    const objectUrl = URL.createObjectURL(file);

    link.href = objectUrl;
    link.download = file.name;
    link.click();
    URL.revokeObjectURL(objectUrl);
  } catch (error) {
    toast.add({
      id: 'draw-tool-download-error',
      title: 'Could not download image',
      description: error instanceof Error ? error.message : String(error),
      color: 'error',
      duration: 10000
    });
  }
}

onMounted(() => {
  dts.measureStage();
  updateStageContainerViewportTop();

  resizeObserver.value = new ResizeObserver(() => {
    dts.measureStage();
    updateStageContainerViewportTop();
  });

  if (dts.stageContainerRef) {
    resizeObserver.value.observe(dts.stageContainerRef);
  }

  window.addEventListener('scroll', updateStageContainerViewportTop, { passive: true });
  window.addEventListener('resize', updateStageContainerViewportTop);
  window.addEventListener('pointerdown', handleWindowPointerDown);
  window.addEventListener('click', handleWindowClick);
  window.addEventListener('keydown', handleWindowKeyDown);

  loadEditorFonts().then(() => {
    nextTick(schedulePixelatedDisplaySync);
  });
  nextTick(schedulePixelatedDisplaySync);
  void initializeSaveDirectory();
});

onBeforeUnmount(() => {
  dts.stopSelectionHandleDrag(false);
  resizeObserver.value?.disconnect();
  window.removeEventListener('scroll', updateStageContainerViewportTop);
  window.removeEventListener('resize', updateStageContainerViewportTop);
  window.removeEventListener('pointerdown', handleWindowPointerDown);
  window.removeEventListener('click', handleWindowClick);
  window.removeEventListener('keydown', handleWindowKeyDown);

  if (pixelatedDisplayFrame.value !== null) {
    cancelAnimationFrame(pixelatedDisplayFrame.value);
  }

  if (transformerFrame.value !== null) {
    cancelAnimationFrame(transformerFrame.value);
  }
});
</script>
