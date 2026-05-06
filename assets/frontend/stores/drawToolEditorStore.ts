import Konva from 'konva';
import { defineStore } from 'pinia';

type OverlayControlPosition = {
  x: number;
  y: number;
};

type SelectionHandleDragState = {
  pointerId: number;
  shapeId: string;
  mode: 'move' | 'resize' | 'rotate';
  startClientX: number;
  startClientY: number;
  startShape: EditorShape;
  centerX?: number;
  centerY?: number;
  startPointerAngle?: number;
};

interface EditorSnapshot {
  shapes: EditorShape[];
  backgroundColor: string;
  borderColor: string;
  borderGapSize: number;
  borderDashSize: number;
  borderGapOffset: number;
}

interface HistorySnapshot extends EditorSnapshot {
  selectedShapeId: string | null;
}

let hasRegisteredDrawToolBeforeUnloadListener = false;

export const useDrawToolEditorStore = defineStore('drawToolEditor', () => {
  let pendingLeaveEditorAction: (() => void | Promise<void>) | null = null;
  let saveBeforeLeaveEditorHandler: (() => Promise<boolean>) | null = null;

  const stageContainerRef = ref<HTMLDivElement | null>(null);
  const textEditorRef = ref<HTMLTextAreaElement | null>(null);
  const stageWidth = ref(MIN_STAGE_WIDTH);
  const stageMetrics = ref<DrawToolStageMetrics>(getStageMetrics(MIN_STAGE_WIDTH));

  const stageRef = ref<KonvaRef<Konva.Stage> | null>(null);
  const displayLayerRef = ref<KonvaRef<Konva.Layer> | null>(null);
  const displayGroupRef = ref<KonvaRef<Konva.Group> | null>(null);
  const overlayLayerRef = ref<KonvaRef<Konva.Layer> | null>(null);
  const transformerRef = ref<KonvaRef<Konva.Transformer> | null>(null);

  const shapes = ref<EditorShape[]>([]);
  const selectedShapeId = ref<string | null>(null);
  const deleteButtonPosition = ref<OverlayControlPosition | null>(null);
  const rotationHandlePosition = ref<OverlayControlPosition | null>(null);
  const selectionHandlePosition = ref<OverlayControlPosition | null>(null);
  const activeSelectionHandleDrag = ref<SelectionHandleDragState | null>(null);
  const backgroundColor = ref(DEFAULT_BACKGROUND_COLOR);
  const borderColor = ref(DEFAULT_BORDER_COLOR);
  const borderGapSize = ref(DEFAULT_BORDER_GAP_SIZE);
  const borderDashSize = ref(DEFAULT_BORDER_DASH_SIZE);
  const borderGapOffset = ref(DEFAULT_BORDER_GAP_OFFSET);
  const textDraftValue = ref(DEFAULT_TEXT_VALUE);
  const textDraftColor = ref(DEFAULT_TEXT_COLOR);
  const textDraftFontId = ref(DEFAULT_TEXT_FONT_ID);
  const showImageUploadModal = ref(false);
  const imageUploadFile = ref<File | null>(null);
  const showLeaveEditorModal = ref(false);
  const isLeavingEditor = ref(false);
  const statusFileName = ref(DEFAULT_STATUS_FILE_NAME);
  const savedStatusFilePath = ref<string | null>(null);
  const lastSavedSnapshot = ref<EditorSnapshot | null>(null);

  function createDefaultEditorSnapshot (): EditorSnapshot {
    return {
      shapes: [],
      backgroundColor: DEFAULT_BACKGROUND_COLOR,
      borderColor: DEFAULT_BORDER_COLOR,
      borderGapSize: DEFAULT_BORDER_GAP_SIZE,
      borderDashSize: DEFAULT_BORDER_DASH_SIZE,
      borderGapOffset: DEFAULT_BORDER_GAP_OFFSET
    };
  }

  const defaultEditorSnapshot = createDefaultEditorSnapshot();

  const historyEntries = ref<HistorySnapshot[]>([{
    ...createDefaultEditorSnapshot(),
    selectedShapeId: null
  }]);
  const historyIndex = ref(0);
  const hasSavedStatusFile = computed(() => !!savedStatusFilePath.value);
  const hasEditorContent = computed(() => {
    return !areEditorSnapshotsEqual(defaultEditorSnapshot, createEditorSnapshot());
  });
  const hasUnsavedChanges = computed(() => {
    const currentSnapshot = createEditorSnapshot();

    if (lastSavedSnapshot.value) {
      return !areEditorSnapshotsEqual(lastSavedSnapshot.value, currentSnapshot);
    }

    return !areEditorSnapshotsEqual(defaultEditorSnapshot, currentSnapshot);
  });

  function setStageMetrics (metrics: DrawToolStageMetrics) {
    stageMetrics.value = metrics;
  }

  function measureStage () {
    stageWidth.value = stageContainerRef.value?.clientWidth || MIN_STAGE_WIDTH;
  }

  function getSelectedShapeState (): EditorShape | null {
    return shapes.value.find(shape => shape.id === selectedShapeId.value) || null;
  }

  function getSelectedTextShapeState (): TextShape | null {
    const selectedShape = getSelectedShapeState();
    return selectedShape?.type === 'text' ? selectedShape : null;
  }

  function getSelectedNode (): Konva.Rect | Konva.Image | Konva.Text | null {
    const stage = stageRef.value?.getNode();

    if (!stage || !selectedShapeId.value) {
      return null;
    }

    return stage.findOne(`#${selectedShapeId.value}`) as Konva.Rect | Konva.Image | Konva.Text | null;
  }

  function normalizeBorderSettingsState () {
    const nextBorderSettings = normalizeBorderSettings(
      borderGapSize.value,
      borderDashSize.value,
      borderGapOffset.value,
      getBorderGapOffsetSliderMax(borderDashSize.value, borderGapSize.value)
    );

    borderGapSize.value = nextBorderSettings.borderGapSize;
    borderDashSize.value = nextBorderSettings.borderDashSize;
    borderGapOffset.value = nextBorderSettings.borderGapOffset;
  }

  function createEditorSnapshot (): EditorSnapshot {
    return {
      shapes: shapes.value.map(shape => cloneShape(shape)),
      backgroundColor: backgroundColor.value,
      borderColor: borderColor.value,
      borderGapSize: borderGapSize.value,
      borderDashSize: borderDashSize.value,
      borderGapOffset: borderGapOffset.value
    };
  }

  function createHistorySnapshot (): HistorySnapshot {
    return {
      ...createEditorSnapshot(),
      selectedShapeId: selectedShapeId.value
    };
  }

  function areEditorSnapshotsEqual (left: EditorSnapshot | undefined, right: EditorSnapshot): boolean {
    if (
      !left
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

  function areSnapshotsEqual (left: HistorySnapshot | undefined, right: HistorySnapshot): boolean {
    if (
      !left
      || left.selectedShapeId !== right.selectedShapeId
    ) {
      return false;
    }

    return areEditorSnapshotsEqual(left, right);
  }

  function markStatusSaved (fileName: string, path: string) {
    statusFileName.value = fileName;
    savedStatusFilePath.value = path;
    lastSavedSnapshot.value = createEditorSnapshot();
  }

  const handleBeforeUnload = (event: BeforeUnloadEvent) => {
    if (!hasUnsavedChanges.value) {
      return;
    }

    event.preventDefault();
    event.returnValue = '';
  };

  if (import.meta.client && !hasRegisteredDrawToolBeforeUnloadListener) {
    window.addEventListener('beforeunload', handleBeforeUnload);
    hasRegisteredDrawToolBeforeUnloadListener = true;
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
    shapes.value = snapshot.shapes.map(shape => cloneShape(shape));
    selectedShapeId.value = snapshot.selectedShapeId;
    backgroundColor.value = snapshot.backgroundColor;
    borderColor.value = snapshot.borderColor;
    borderGapSize.value = snapshot.borderGapSize;
    borderDashSize.value = snapshot.borderDashSize;
    borderGapOffset.value = snapshot.borderGapOffset;
    normalizeBorderSettingsState();
  }

  function clearBackgroundColor () {
    if (isColorFullyTransparent(backgroundColor.value)) {
      return;
    }

    backgroundColor.value = DEFAULT_BACKGROUND_COLOR;
    pushHistorySnapshot();
  }

  function handleBackgroundColorChange (value?: string) {
    backgroundColor.value = value || DEFAULT_BACKGROUND_COLOR;
    pushHistorySnapshot();
  }

  function clearBorderColor () {
    const isDefaultBorderState = borderColor.value === DEFAULT_BORDER_COLOR
      && borderGapSize.value === DEFAULT_BORDER_GAP_SIZE
      && borderDashSize.value === DEFAULT_BORDER_DASH_SIZE
      && borderGapOffset.value === DEFAULT_BORDER_GAP_OFFSET;

    if (isDefaultBorderState) {
      return;
    }

    borderColor.value = DEFAULT_BORDER_COLOR;
    borderGapSize.value = DEFAULT_BORDER_GAP_SIZE;
    borderDashSize.value = DEFAULT_BORDER_DASH_SIZE;
    borderGapOffset.value = DEFAULT_BORDER_GAP_OFFSET;
    pushHistorySnapshot();
  }

  function handleBorderColorChange (value?: string) {
    borderColor.value = value || DEFAULT_BORDER_COLOR;
    pushHistorySnapshot();
  }

  function handleBorderSettingsChange () {
    normalizeBorderSettingsState();
    pushHistorySnapshot();
  }

  function updateShape (shapeId: string, updater: (shape: EditorShape) => EditorShape) {
    const shapeIndex = shapes.value.findIndex(shape => shape.id === shapeId);

    if (shapeIndex === -1) {
      return;
    }

    shapes.value.splice(shapeIndex, 1, updater(shapes.value[shapeIndex]));
  }

  function fitTextValueToWorkspaceWidth (value: string, fontId: string, availableWidth: number) {
    if (!value || availableWidth <= 0) {
      return '';
    }

    const characters = Array.from(value);
    let low = 0;
    let high = characters.length;

    while (low < high) {
      const middle = Math.ceil((low + high) / 2);
      const leadingText = characters.slice(0, Math.max(0, middle - 1)).join('');
      const { width } = measureTextShapeDimensions(leadingText, fontId, TEXT_FONT_OPTIONS);

      if (width < availableWidth) {
        low = middle;
      } else {
        high = middle - 1;
      }
    }

    return characters.slice(0, low).join('');
  }

  function clampTextValueToSelectedTextWorkspace (value: string, selectedTextShape: TextShape) {
    const availableWidth = WORKSPACE_WIDTH - selectedTextShape.x;
    return fitTextValueToWorkspaceWidth(value, selectedTextShape.fontId, availableWidth);
  }

  function handleActiveTextValueInput (value: string | number) {
    const nextValue = String(value ?? '');
    const selectedTextShape = getSelectedTextShapeState();

    if (!selectedTextShape) {
      textDraftValue.value = nextValue;
      return;
    }

    const dimensions = measureTextShapeDimensions(nextValue, selectedTextShape.fontId, TEXT_FONT_OPTIONS);

    updateShape(selectedTextShape.id, shape => ({
      ...(shape as TextShape),
      text: nextValue,
      ...dimensions
    }));
  }

  function handleTextTextareaInput (event: Event) {
    const textarea = event.target as HTMLTextAreaElement;
    const selectedTextShape = getSelectedTextShapeState();
    let nextValue = textarea.value.replace(/[\r\n]+/g, '');

    if (selectedTextShape) {
      nextValue = clampTextValueToSelectedTextWorkspace(nextValue, selectedTextShape);
    }

    if (textarea.value !== nextValue) {
      const selectionStart = textarea.selectionStart ?? nextValue.length;
      const selectionEnd = textarea.selectionEnd ?? nextValue.length;

      textarea.value = nextValue;
      textarea.setSelectionRange(
        Math.min(selectionStart, nextValue.length),
        Math.min(selectionEnd, nextValue.length)
      );
    }

    handleActiveTextValueInput(nextValue);
  }

  function handleTextTextareaEnter () {
    commitActiveTextChange();
    selectedShapeId.value = null;
  }

  function handleActiveTextColorChange (value?: string) {
    const nextColor = value || DEFAULT_TEXT_COLOR;
    textDraftColor.value = nextColor;
    const selectedTextShape = getSelectedTextShapeState();

    if (!selectedTextShape) {
      return;
    }

    updateShape(selectedTextShape.id, shape => ({
      ...(shape as TextShape),
      fill: nextColor
    }));
    pushHistorySnapshot();
  }

  function handleActiveTextFontChange (value: string | number) {
    const nextFontId = String(value);
    textDraftFontId.value = nextFontId;
    const selectedTextShape = getSelectedTextShapeState();

    if (!selectedTextShape) {
      return;
    }

    const dimensions = measureTextShapeDimensions(selectedTextShape.text, nextFontId, TEXT_FONT_OPTIONS);

    updateShape(selectedTextShape.id, shape => ({
      ...(shape as TextShape),
      fontId: nextFontId,
      ...dimensions
    }));
    pushHistorySnapshot();
  }

  function commitActiveTextChange () {
    if (!getSelectedTextShapeState()) {
      return;
    }

    pushHistorySnapshot();
  }

  function commitSelectedTextBeforeSelectionChange () {
    if (!getSelectedTextShapeState()) {
      return;
    }

    pushHistorySnapshot();
  }

  function undo () {
    if (historyIndex.value <= 0) {
      return;
    }

    historyIndex.value -= 1;
    restoreSnapshot(historyEntries.value[historyIndex.value]);
  }

  function redo () {
    if (historyIndex.value >= historyEntries.value.length - 1) {
      return;
    }

    historyIndex.value += 1;
    restoreSnapshot(historyEntries.value[historyIndex.value]);
  }

  function updateOverlayControlPositions () {
    const selectedNode = getSelectedNode();

    if (!selectedNode) {
      deleteButtonPosition.value = null;
      rotationHandlePosition.value = null;
      selectionHandlePosition.value = null;
      return;
    }

    const transform = selectedNode.getAbsoluteTransform();
    const topLeftCorner = transform.point({ x: 0, y: 0 });
    const topCenterCorner = transform.point({ x: selectedNode.width() / 2, y: 0 });
    const rotationOffset = getStageDeltaFromLocalDelta(0, -stageMetrics.value.cellSize * 6, selectedNode.rotation());
    const bottomRightCorner = transform.point({ x: selectedNode.width(), y: selectedNode.height() });

    deleteButtonPosition.value = {
      x: topLeftCorner.x,
      y: topLeftCorner.y
    };

    rotationHandlePosition.value = {
      x: topCenterCorner.x + rotationOffset.x,
      y: topCenterCorner.y + rotationOffset.y
    };

    selectionHandlePosition.value = {
      x: bottomRightCorner.x,
      y: bottomRightCorner.y
    };
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
      rotationHandlePosition.value = null;
      selectionHandlePosition.value = null;
      overlayLayerRef.value?.getNode().batchDraw();
      return;
    }

    const node = stage.findOne(`#${selectedShapeId.value}`);
    transformer.nodes(node ? [node] : []);
    transformer.forceUpdate();

    updateOverlayControlPositions();
    overlayLayerRef.value?.getNode().batchDraw();
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
    const viewport = workspaceViewportBounds(stageMetrics.value);
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
      x: snapLogical(node.x() + stageDeltaToLogical(deltaX, stageMetrics.value.cellSize)),
      y: snapLogical(node.y() + stageDeltaToLogical(deltaY, stageMetrics.value.cellSize))
    });
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

  function stopSelectionHandleDrag (shouldCommit: boolean) {
    window.removeEventListener('pointermove', handleSelectionHandlePointerMove);
    window.removeEventListener('pointerup', handleSelectionHandlePointerUp);
    window.removeEventListener('pointercancel', handleSelectionHandlePointerUp);

    if (!activeSelectionHandleDrag.value) {
      return;
    }

    activeSelectionHandleDrag.value = null;

    if (shouldCommit) {
      pushHistorySnapshot();
    }
  }

  function handleSelectionHandlePointerDown (event: PointerEvent) {
    const currentShape = getSelectedShapeState();
    const selectedNode = getSelectedNode();

    if (!currentShape || !selectedNode) {
      return;
    }

    activeSelectionHandleDrag.value = {
      pointerId: event.pointerId,
      shapeId: currentShape.id,
      mode: currentShape.type === 'text' ? 'move' : 'resize',
      startClientX: event.clientX,
      startClientY: event.clientY,
      startShape: cloneShape(currentShape)
    };

    window.addEventListener('pointermove', handleSelectionHandlePointerMove);
    window.addEventListener('pointerup', handleSelectionHandlePointerUp);
    window.addEventListener('pointercancel', handleSelectionHandlePointerUp);
  }

  function handleRotationHandlePointerDown (event: PointerEvent) {
    const currentShape = getSelectedShapeState();
    const selectedNode = getSelectedNode();
    const stageContainerRect = stageContainerRef.value?.getBoundingClientRect();

    if (!currentShape || !selectedNode || currentShape.type === 'text' || !stageContainerRect) {
      return;
    }

    const center = selectedNode.getAbsoluteTransform().point({
      x: selectedNode.width() / 2,
      y: selectedNode.height() / 2
    });
    const centerClientX = stageContainerRect.left + center.x;
    const centerClientY = stageContainerRect.top + center.y;

    activeSelectionHandleDrag.value = {
      pointerId: event.pointerId,
      shapeId: currentShape.id,
      mode: 'rotate',
      startClientX: event.clientX,
      startClientY: event.clientY,
      startShape: cloneShape(currentShape),
      centerX: centerClientX,
      centerY: centerClientY,
      startPointerAngle: getRotationHandleAngle(event.clientX, event.clientY, centerClientX, centerClientY)
    };

    window.addEventListener('pointermove', handleSelectionHandlePointerMove);
    window.addEventListener('pointerup', handleSelectionHandlePointerUp);
    window.addEventListener('pointercancel', handleSelectionHandlePointerUp);
  }

  function handleSelectionHandlePointerMove (event: PointerEvent) {
    const dragState = activeSelectionHandleDrag.value;
    const selectedNode = getSelectedNode();

    if (!dragState || event.pointerId !== dragState.pointerId || !selectedNode || selectedNode.id() !== dragState.shapeId) {
      return;
    }

    const pointerDeltaX = event.clientX - dragState.startClientX;
    const pointerDeltaY = event.clientY - dragState.startClientY;

    if (dragState.mode === 'move') {
      selectedNode.position({
        x: dragState.startShape.x + stageDeltaToLogical(pointerDeltaX, stageMetrics.value.cellSize),
        y: dragState.startShape.y + stageDeltaToLogical(pointerDeltaY, stageMetrics.value.cellSize)
      });
      syncNodePosition(selectedNode);
    } else if (dragState.mode === 'rotate') {
      if (dragState.centerX === undefined || dragState.centerY === undefined || dragState.startPointerAngle === undefined) {
        return;
      }

      const currentPointerAngle = getRotationHandleAngle(event.clientX, event.clientY, dragState.centerX, dragState.centerY);
      const rotationDelta = (currentPointerAngle - dragState.startPointerAngle) * (180 / Math.PI);
      const nextRotation = snapRotationValue(dragState.startShape.rotation + rotationDelta, ROTATION_SNAP_STEP);
      const nextPosition = getShapePositionForRotation(dragState.startShape, nextRotation);

      selectedNode.position(nextPosition);
      selectedNode.rotation(nextRotation);
      syncRotatingNode(selectedNode);
    } else {
      const resizedDelta = getRotatedLogicalDelta(
        pointerDeltaX,
        pointerDeltaY,
        dragState.startShape.rotation,
        stageMetrics.value.cellSize
      );
      const nextSize = getHandleResizeDimensions(
        dragState.startShape,
        resizedDelta.x,
        resizedDelta.y,
        !(event.shiftKey || event.altKey),
        event.altKey
      );

      selectedNode.position({
        x: nextSize.x,
        y: nextSize.y
      });
      selectedNode.width(nextSize.width);
      selectedNode.height(nextSize.height);
      selectedNode.scaleX(1);
      selectedNode.scaleY(1);
      normalizeTransformedNode(selectedNode);
    }

    transformerRef.value?.getNode().forceUpdate();
    updateOverlayControlPositions();
    overlayLayerRef.value?.getNode().batchDraw();
  }

  function handleSelectionHandlePointerUp (event: PointerEvent) {
    const dragState = activeSelectionHandleDrag.value;

    if (!dragState || event.pointerId !== dragState.pointerId) {
      return;
    }

    stopSelectionHandleDrag(true);
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

  function addText (text: string, fill: string, fontId: string) {
    const dimensions = measureTextShapeDimensions(text, fontId, TEXT_FONT_OPTIONS);
    const textShape: TextShape = {
      id: createShapeId(),
      type: 'text',
      x: 0,
      y: 0,
      rotation: 0,
      text,
      fill,
      fontId,
      ...dimensions
    };

    textDraftValue.value = DEFAULT_TEXT_VALUE;
    textDraftColor.value = fill;
    textDraftFontId.value = fontId;
    shapes.value.push(textShape);
    selectedShapeId.value = textShape.id;
    pushHistorySnapshot();
  }

  function addImageShape (image: HTMLImageElement, fileName: string) {
    const height = WORKSPACE_HEIGHT;
    const width = Math.max(1, Math.round((image.width / image.height) * height));

    const imageShape: ImageShape = {
      id: createShapeId(),
      type: 'image',
      fileName,
      x: 0,
      y: 0,
      width,
      height,
      rotation: 0,
      image
    };

    shapes.value.push(imageShape);
    selectedShapeId.value = imageShape.id;
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

  function clearStage () {
    if (!hasEditorContent.value) {
      return;
    }

    shapes.value = [];
    selectedShapeId.value = null;
    backgroundColor.value = DEFAULT_BACKGROUND_COLOR;
    borderColor.value = DEFAULT_BORDER_COLOR;
    borderGapSize.value = DEFAULT_BORDER_GAP_SIZE;
    borderDashSize.value = DEFAULT_BORDER_DASH_SIZE;
    borderGapOffset.value = DEFAULT_BORDER_GAP_OFFSET;
    pushHistorySnapshot();
  }

  function resetEditor () {
    stopSelectionHandleDrag(false);
    shapes.value = [];
    selectedShapeId.value = null;
    deleteButtonPosition.value = null;
    rotationHandlePosition.value = null;
    selectionHandlePosition.value = null;
    activeSelectionHandleDrag.value = null;
    backgroundColor.value = DEFAULT_BACKGROUND_COLOR;
    borderColor.value = DEFAULT_BORDER_COLOR;
    borderGapSize.value = DEFAULT_BORDER_GAP_SIZE;
    borderDashSize.value = DEFAULT_BORDER_DASH_SIZE;
    borderGapOffset.value = DEFAULT_BORDER_GAP_OFFSET;
    textDraftValue.value = DEFAULT_TEXT_VALUE;
    showImageUploadModal.value = false;
    imageUploadFile.value = null;
    statusFileName.value = DEFAULT_STATUS_FILE_NAME;
    savedStatusFilePath.value = null;
    lastSavedSnapshot.value = null;
    historyEntries.value = [{
      ...createDefaultEditorSnapshot(),
      selectedShapeId: null
    }];
    historyIndex.value = 0;
    normalizeBorderSettingsState();
    nextTick(syncTransformer);
    nextTick(syncPixelatedDisplay);
  }

  function moveSelectedShape (deltaX: number, deltaY: number) {
    if (!selectedShapeId.value || (deltaX === 0 && deltaY === 0)) {
      return;
    }

    const selectedNode = getSelectedNode();

    if (!selectedNode) {
      return;
    }

    selectedNode.position({
      x: selectedNode.x() + deltaX,
      y: selectedNode.y() + deltaY
    });
    syncNodePosition(selectedNode);
    updateOverlayControlPositions();
    overlayLayerRef.value?.getNode().batchDraw();
    pushHistorySnapshot();
  }

  function rotateSelectedShape (rotationDelta: number) {
    const currentShape = getSelectedShapeState();
    const selectedNode = getSelectedNode();

    if (!currentShape || !selectedNode || currentShape.type === 'text' || rotationDelta === 0) {
      return;
    }

    const nextRotation = snapRotationValue(currentShape.rotation + rotationDelta, ROTATION_SNAP_STEP);
    const nextPosition = getShapePositionForRotation(currentShape, nextRotation);

    selectedNode.position(nextPosition);
    selectedNode.rotation(nextRotation);
    syncRotatingNode(selectedNode);
    updateOverlayControlPositions();
    overlayLayerRef.value?.getNode().batchDraw();
    pushHistorySnapshot();
  }

  function reorderSelectedShapeLayer (direction: 'up' | 'down', moveToEdge = false) {
    if (!selectedShapeId.value || shapes.value.length < 2) {
      return false;
    }

    const currentIndex = shapes.value.findIndex(shape => shape.id === selectedShapeId.value);

    if (currentIndex < 0) {
      return false;
    }

    const targetIndex = direction === 'up'
      ? moveToEdge ? shapes.value.length - 1 : currentIndex + 1
      : moveToEdge ? 0 : currentIndex - 1;

    if (targetIndex === currentIndex || targetIndex < 0 || targetIndex >= shapes.value.length) {
      return false;
    }

    const nextShapes = [...shapes.value];
    const [selectedShape] = nextShapes.splice(currentIndex, 1);
    nextShapes.splice(targetIndex, 0, selectedShape);
    shapes.value = nextShapes;
    nextTick(syncTransformer);
    nextTick(syncPixelatedDisplay);
    pushHistorySnapshot();

    return true;
  }

  function handleStagePointerDown (event: Konva.KonvaEventObject<MouseEvent | TouchEvent>) {
    if (event.target !== event.target.getStage()) {
      return;
    }

    commitSelectedTextBeforeSelectionChange();
    selectedShapeId.value = null;
  }

  function handleShapePointerDown (event: Konva.KonvaEventObject<MouseEvent | TouchEvent>) {
    if (event.target.id() !== selectedShapeId.value) {
      commitSelectedTextBeforeSelectionChange();
    }

    selectedShapeId.value = event.target.id();
  }

  function handleShapeDragMove (event: Konva.KonvaEventObject<DragEvent>) {
    syncNodePosition(event.target as Konva.Rect | Konva.Image | Konva.Text);
    updateOverlayControlPositions();
    overlayLayerRef.value?.getNode().batchDraw();
  }

  function handleShapeDragEnd (event: Konva.KonvaEventObject<DragEvent>) {
    syncNodePosition(event.target as Konva.Rect | Konva.Image | Konva.Text);
    pushHistorySnapshot();
  }

  function handleShapeTransform () {
    const transformer = transformerRef.value?.getNode();
    const node = transformer?.nodes()[0] as Konva.Rect | Konva.Image | Konva.Text | undefined;

    if (!node || !transformer) {
      return;
    }

    if (transformer?.getActiveAnchor() === 'rotater') {
      syncRotatingNode(node);
    } else {
      normalizeTransformedNode(node);
      transformer.forceUpdate();
    }

    updateOverlayControlPositions();
    nextTick(syncPixelatedDisplay);
    overlayLayerRef.value?.getNode().batchDraw();
  }

  function handleShapeTransformEnd (event: Konva.KonvaEventObject<Event>) {
    const node = event.target as Konva.Rect | Konva.Image | Konva.Text;

    normalizeTransformedNode(node);
    pushHistorySnapshot();
    nextTick(syncTransformer);
  }

  function resetImageUploadModal () {
    imageUploadFile.value = null;
    showImageUploadModal.value = false;
  }

  function registerSaveBeforeLeaveEditorHandler (handler: (() => Promise<boolean>) | null) {
    saveBeforeLeaveEditorHandler = handler;
  }

  async function requestLeaveEditor (afterLeave: () => void | Promise<void>) {
    if (!hasUnsavedChanges.value) {
      await afterLeave();
      return;
    }

    pendingLeaveEditorAction = afterLeave;
    showLeaveEditorModal.value = true;
  }

  function cancelLeaveEditorRequest () {
    if (isLeavingEditor.value) {
      return;
    }

    showLeaveEditorModal.value = false;
    pendingLeaveEditorAction = null;
  }

  async function discardAndLeaveEditor () {
    if (isLeavingEditor.value) {
      return;
    }

    const pendingAction = pendingLeaveEditorAction;
    showLeaveEditorModal.value = false;
    pendingLeaveEditorAction = null;
    await pendingAction?.();
  }

  async function saveAndLeaveEditor () {
    if (isLeavingEditor.value || !saveBeforeLeaveEditorHandler) {
      return;
    }

    isLeavingEditor.value = true;

    try {
      const didSave = await saveBeforeLeaveEditorHandler();

      if (!didSave) {
        return;
      }

      const pendingAction = pendingLeaveEditorAction;
      showLeaveEditorModal.value = false;
      pendingLeaveEditorAction = null;
      await pendingAction?.();
    } finally {
      isLeavingEditor.value = false;
    }
  }

  return {
    stageContainerRef,
    textEditorRef,
    stageWidth,
    stageRef,
    displayLayerRef,
    displayGroupRef,
    overlayLayerRef,
    transformerRef,
    shapes,
    selectedShapeId,
    deleteButtonPosition,
    rotationHandlePosition,
    selectionHandlePosition,
    activeSelectionHandleDrag,
    backgroundColor,
    borderColor,
    borderGapSize,
    borderDashSize,
    borderGapOffset,
    textDraftValue,
    textDraftColor,
    textDraftFontId,
    statusFileName,
    savedStatusFilePath,
    hasSavedStatusFile,
    hasEditorContent,
    hasUnsavedChanges,
    historyEntries,
    historyIndex,
    showImageUploadModal,
    imageUploadFile,
    showLeaveEditorModal,
    isLeavingEditor,
    setStageMetrics,
    measureStage,
    syncTransformer,
    syncPixelatedDisplay,
    clearBackgroundColor,
    handleBackgroundColorChange,
    clearBorderColor,
    handleBorderColorChange,
    handleBorderSettingsChange,
    handleActiveTextValueInput,
    handleTextTextareaInput,
    handleTextTextareaEnter,
    handleActiveTextColorChange,
    handleActiveTextFontChange,
    commitActiveTextChange,
    undo,
    redo,
    stopSelectionHandleDrag,
    handleSelectionHandlePointerDown,
    handleRotationHandlePointerDown,
    addRectangle,
    addText,
    addImageShape,
    markStatusSaved,
    clearStage,
    resetEditor,
    deleteSelectedShape,
    reorderSelectedShapeLayer,
    moveSelectedShape,
    rotateSelectedShape,
    handleStagePointerDown,
    handleShapePointerDown,
    handleShapeDragMove,
    handleShapeDragEnd,
    handleShapeTransform,
    handleShapeTransformEnd,
    resetImageUploadModal,
    registerSaveBeforeLeaveEditorHandler,
    requestLeaveEditor,
    cancelLeaveEditorRequest,
    discardAndLeaveEditor,
    saveAndLeaveEditor
  };
});
