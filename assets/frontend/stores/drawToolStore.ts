import type { AnimationElement, DisplayElement, ImageElement, StorageListElement } from '@busy-app/busy-lib';
import { defineStore } from 'pinia';
import {
  DRAW_TOOL_DISPLAY_APPLICATION_NAME,
  DRAW_TOOL_DISPLAY_PRIORITY,
  DRAW_TOOL_SAVE_DIR,
  DRAW_TOOL_TEMP_ANIMATION_FILE_NAME,
  DRAW_TOOL_TEMP_FILE_NAME,
  getStatusFileKind
} from '@/util/drawTool';
import type { DrawToolStatusKind } from '@/util/drawTool';
import { decodeAnimation } from '@/util/anim2seq';
import type { DecodedAnimation } from '@/util/anim2seq';

type DrawToolStatusDirectoryFile = {
  name: string;
  size: number;
};

type DrawToolStatusPreview = {
  kind: DrawToolStatusKind;
  previewUrl: string | null;
  animation: DecodedAnimation | null;
};

type DrawToolStatusGalleryFile = DrawToolStatusDirectoryFile & DrawToolStatusPreview & {
  path: string;
};

const TEMP_FILE_NAMES = new Set([DRAW_TOOL_TEMP_FILE_NAME, DRAW_TOOL_TEMP_ANIMATION_FILE_NAME]);

function getStatusFileSize (file: StorageListElement): number {
  if ('size' in file && typeof file.size === 'number') {
    return file.size;
  }

  return -1;
}

function createStatusFilePath (fileName: string) {
  return `${DRAW_TOOL_SAVE_DIR}/${fileName}`;
}

function hasLoadedPreview (file: DrawToolStatusPreview) {
  return !!file.previewUrl || !!file.animation;
}

function createDisplayElement (fileName: string): ImageElement | AnimationElement {
  const base: Omit<DisplayElement, 'type'> = {
    id: '0',
    timeout: 0,
    align: 'top_left',
    display: 'front',
    x: 0,
    y: 0
  };

  if (getStatusFileKind(fileName) === 'animation') {
    return {
      ...base,
      type: 'animation',
      path: fileName,
      loop: true,
      await_previous_end: false,
      opacity: 100
    };
  }

  return {
    ...base,
    type: 'image',
    path: fileName,
    opacity: 100
  };
}

export const useDrawToolStore = defineStore('drawTool', () => {
  const statusDirectoryFiles = ref<DrawToolStatusDirectoryFile[]>([]);
  const statusGalleryFiles = ref<DrawToolStatusGalleryFile[]>([]);
  const isRefreshingStatusDirectory = ref(false);

  function normalizeDirectoryFiles (files: StorageListElement[]) {
    return files
      .filter((file): file is StorageListElement & { type: 'file' } => file.type === 'file')
      .filter(file => !TEMP_FILE_NAMES.has(file.name))
      .map(file => ({
        name: file.name,
        size: getStatusFileSize(file)
      }))
      .sort((left, right) => right.name.localeCompare(left.name));
  }

  async function readStatusFile (path: string) {
    const deviceStore = useDeviceStore();
    const file = await deviceStore.busyBar.StorageRead({ path }, { timeout: 0 });
    const mimeType = getStatusFileKind(path) === 'animation' ? 'application/octet-stream' : 'image/png';

    return file instanceof Blob ? file : new Blob([file], { type: mimeType });
  }

  async function loadStatusPreview (file: DrawToolStatusDirectoryFile): Promise<DrawToolStatusPreview> {
    const kind = getStatusFileKind(file.name);

    try {
      const blob = await readStatusFile(createStatusFilePath(file.name));

      if (kind === 'animation') {
        return {
          kind,
          previewUrl: null,
          animation: decodeAnimation(await blob.arrayBuffer())
        };
      }

      return {
        kind,
        previewUrl: URL.createObjectURL(blob),
        animation: null
      };
    } catch (error) {
      console.warn(`Couldn't load preview for ${file.name}`, error);

      return {
        kind,
        previewUrl: null,
        animation: null
      };
    }
  }

  function revokeStatusPreview (file: Pick<DrawToolStatusGalleryFile, 'previewUrl'>) {
    if (file.previewUrl) {
      URL.revokeObjectURL(file.previewUrl);
    }
  }

  async function syncStatusDirectory (files: StorageListElement[]) {
    const nextDirectoryFiles = normalizeDirectoryFiles(files);
    const currentGalleryFilesByName = new Map(statusGalleryFiles.value.map(file => [file.name, file]));
    const nextFileNames = new Set(nextDirectoryFiles.map(file => file.name));

    const nextGalleryFiles = await Promise.all(nextDirectoryFiles.map(async file => {
      const existingFile = currentGalleryFilesByName.get(file.name);

      if (existingFile && file.size >= 0 && existingFile.size === file.size && hasLoadedPreview(existingFile)) {
        return existingFile;
      }

      const preview = await loadStatusPreview(file);

      if (existingFile) {
        revokeStatusPreview(existingFile);
      }

      return {
        ...file,
        path: createStatusFilePath(file.name),
        ...preview
      };
    }));

    statusDirectoryFiles.value = nextDirectoryFiles;
    statusGalleryFiles.value
      .filter(file => !nextFileNames.has(file.name))
      .forEach(revokeStatusPreview);
    statusGalleryFiles.value = nextGalleryFiles;
  }

  async function listStatusDirectory () {
    const deviceStore = useDeviceStore();
    const result = await deviceStore.busyBar.StorageListGet({ path: DRAW_TOOL_SAVE_DIR });

    if (!result.list) {
      throw new Error('Empty response');
    }

    return result.list;
  }

  async function ensureStatusDirectoryExists () {
    const deviceStore = useDeviceStore();

    try {
      await deviceStore.busyBar.StorageMkdir({ path: DRAW_TOOL_SAVE_DIR });
    } catch {
      // Ignore mkdir failure and rely on the follow-up list call.
    }

    return await listStatusDirectory();
  }

  async function refreshStatusDirectory (options?: { silent?: boolean }) {
    if (isRefreshingStatusDirectory.value) {
      return;
    }

    isRefreshingStatusDirectory.value = true;

    try {
      let directoryFiles: Awaited<ReturnType<typeof listStatusDirectory>>;

      try {
        directoryFiles = await listStatusDirectory();
      } catch {
        directoryFiles = await ensureStatusDirectoryExists();
      }

      await syncStatusDirectory(directoryFiles);
    } catch (error) {
      if (!options?.silent) {
        await handleHTTPError(error, `Couldn't load ${DRAW_TOOL_SAVE_DIR}`, false, 10000);
      }
    } finally {
      isRefreshingStatusDirectory.value = false;
    }
  }

  async function writeStatusFile (fileName: string, file: File) {
    const deviceStore = useDeviceStore();
    const path = createStatusFilePath(fileName);

    try {
      await deviceStore.busyBar.StorageWrite({ path, file }, { timeout: 0 });
    } catch (error) {
      try {
        await listStatusDirectory();
      } catch {
        await ensureStatusDirectoryExists();
        await deviceStore.busyBar.StorageWrite({ path, file }, { timeout: 0 });
        return path;
      }

      throw error;
    }

    return path;
  }

  async function saveStatusFile (fileName: string, file: File) {
    try {
      const path = await writeStatusFile(fileName, file);

      await refreshStatusDirectory({ silent: true });

      return path;
    } catch (error) {
      await handleHTTPError(error, `Couldn't save ${fileName}`, false, 10000);
      throw error;
    }
  }

  async function requestClearStatusDisplay () {
    const deviceStore = useDeviceStore();

    return deviceStore.busyBar.DisplayClear({
      application_name: DRAW_TOOL_DISPLAY_APPLICATION_NAME
    });
  }

  async function clearStatusDisplay () {
    try {
      await requestClearStatusDisplay();
    } catch (error) {
      await handleHTTPError(error, 'Couldn\'t clear existing status display', true);
      throw error;
    }
  }

  async function tryClearStatusDisplay () {
    try {
      await requestClearStatusDisplay();

      return true;
    } catch {
      return false;
    }
  }

  async function showSavedStatusOnBusyBar (fileName: string) {
    await clearStatusDisplay();

    return drawStatusOnBusyBar(fileName);
  }

  async function drawStatusOnBusyBar (fileName: string) {
    const deviceStore = useDeviceStore();

    return deviceStore.busyBar.DisplayDraw({
      application_name: DRAW_TOOL_DISPLAY_APPLICATION_NAME,
      elements: [createDisplayElement(fileName)],
      priority: DRAW_TOOL_DISPLAY_PRIORITY
    })
      .catch(async error => {
        if (isDisplayPriorityConflict(error)) {
          notifyDisplayPriorityConflict();
        } else {
          await handleHTTPError(error, 'Display draw command failed', true);
        }

        throw error;
      });
  }

  async function showTempAnimationOnBusyBar (animation: Blob) {
    const deviceStore = useDeviceStore();

    await clearStatusDisplay();

    try {
      await deviceStore.busyBar.AssetsUpload({
        application_name: DRAW_TOOL_DISPLAY_APPLICATION_NAME,
        data: animation,
        file: DRAW_TOOL_TEMP_ANIMATION_FILE_NAME
      }, { timeout: 0 });
    } catch (error) {
      await handleHTTPError(error, 'Couldn\'t upload animation', true);
      throw error;
    }

    await drawStatusOnBusyBar(DRAW_TOOL_TEMP_ANIMATION_FILE_NAME);
  }

  async function downloadStatusFile (fileName: string) {
    const blob = await readStatusFile(createStatusFilePath(fileName));

    downloadFile(blob, fileName);
  }

  async function tryRemoveStatusFile (path: string): Promise<unknown> {
    const deviceStore = useDeviceStore();

    try {
      await deviceStore.busyBar.StorageRemove({ path }, { timeout: 0 });

      return null;
    } catch (error) {
      return error;
    }
  }

  async function deleteStatusFiles (fileNames: string[]) {
    let hasClearedStatusDisplay = false;

    for (const fileName of [...new Set(fileNames)]) {
      const fullPath = createStatusFilePath(fileName);
      let error = await tryRemoveStatusFile(fullPath);

      if (error && getStatusFileKind(fileName) === 'animation' && !hasClearedStatusDisplay) {
        hasClearedStatusDisplay = true;

        if (await tryClearStatusDisplay()) {
          error = await tryRemoveStatusFile(fullPath);
        }
      }

      if (error) {
        await handleHTTPError(error, `Couldn't delete ${fullPath}`, false, 0);
      }
    }

    await refreshStatusDirectory({ silent: true });
  }

  return {
    isRefreshingStatusDirectory,
    statusDirectoryFiles,
    statusGalleryFiles,
    syncStatusDirectory,
    refreshStatusDirectory,
    saveStatusFile,
    clearStatusDisplay,
    showSavedStatusOnBusyBar,
    showTempAnimationOnBusyBar,
    downloadStatusFile,
    deleteStatusFiles
  };
});
