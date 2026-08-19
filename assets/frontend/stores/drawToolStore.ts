import type { StorageListElement, DisplayDrawParams } from '@busy-app/busy-lib';
import { defineStore } from 'pinia';
import { DRAW_TOOL_DISPLAY_APPLICATION_NAME, DRAW_TOOL_DISPLAY_PRIORITY, DRAW_TOOL_SAVE_DIR, DRAW_TOOL_TEMP_FILE_NAME } from '@/util/drawTool';

type DrawToolStatusDirectoryFile = {
  name: string;
  size: number;
};

type DrawToolStatusGalleryFile = DrawToolStatusDirectoryFile & {
  path: string;
  previewUrl: string;
};

function getStatusFileSize (file: StorageListElement): number {
  if ('size' in file && typeof file.size === 'number') {
    return file.size;
  }

  return -1;
}

function createStatusFilePath (fileName: string) {
  return `${DRAW_TOOL_SAVE_DIR}/${fileName}`;
}

export const useDrawToolStore = defineStore('drawTool', () => {
  const statusDirectoryFiles = ref<DrawToolStatusDirectoryFile[]>([]);
  const statusGalleryFiles = ref<DrawToolStatusGalleryFile[]>([]);
  const isRefreshingStatusDirectory = ref(false);

  function normalizeDirectoryFiles (files: StorageListElement[]) {
    return files
      .filter((file): file is StorageListElement & { type: 'file' } => file.type === 'file')
      .filter(file => file.name !== DRAW_TOOL_TEMP_FILE_NAME)
      .map(file => ({
        name: file.name,
        size: getStatusFileSize(file)
      }))
      .sort((left, right) => right.name.localeCompare(left.name));
  }

  async function downloadStatusPreview (path: string) {
    const blob = await readStatusFile(path);

    return URL.createObjectURL(blob);
  }

  async function readStatusFile (path: string) {
    const deviceStore = useDeviceStore();
    const file = await deviceStore.busyBar.StorageRead({ path }, { timeout: 0 });

    return file instanceof Blob ? file : new Blob([file], { type: 'image/png' });
  }

  function revokeStatusPreviewUrl (previewUrl: string) {
    URL.revokeObjectURL(previewUrl);
  }

  async function syncStatusDirectory (files: StorageListElement[]) {
    const nextDirectoryFiles = normalizeDirectoryFiles(files);
    const currentGalleryFilesByName = new Map(statusGalleryFiles.value.map(file => [file.name, file]));
    const nextFileNames = new Set(nextDirectoryFiles.map(file => file.name));

    const nextGalleryFiles = await Promise.all(nextDirectoryFiles.map(async file => {
      const existingFile = currentGalleryFilesByName.get(file.name);

      if (existingFile && file.size >= 0 && existingFile.size === file.size) {
        return existingFile;
      }

      const previewUrl = await downloadStatusPreview(createStatusFilePath(file.name));

      if (existingFile) {
        revokeStatusPreviewUrl(existingFile.previewUrl);
      }

      return {
        ...file,
        path: createStatusFilePath(file.name),
        previewUrl
      };
    }));

    statusDirectoryFiles.value = nextDirectoryFiles;
    statusGalleryFiles.value
      .filter(file => !nextFileNames.has(file.name))
      .forEach(file => revokeStatusPreviewUrl(file.previewUrl));
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

  async function clearStatusDisplay () {
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

  async function showSavedStatusOnBusyBar (fileName: string) {
    const deviceStore = useDeviceStore();

    await clearStatusDisplay();

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
      priority: DRAW_TOOL_DISPLAY_PRIORITY
    } as DisplayDrawParams)
      .catch(async error => {
        if (isDisplayPriorityConflict(error)) {
          notifyDisplayPriorityConflict();
        } else {
          await handleHTTPError(error, 'Display draw command failed', true);
        }

        throw error;
      });
  }

  async function downloadStatusFile (fileName: string) {
    const blob = await readStatusFile(createStatusFilePath(fileName));
    const objectUrl = URL.createObjectURL(blob);

    try {
      const link = document.createElement('a');
      link.href = objectUrl;
      link.download = fileName;
      link.click();
    } finally {
      URL.revokeObjectURL(objectUrl);
    }
  }

  async function deleteStatusFiles (fileNames: string[]) {
    const deviceStore = useDeviceStore();

    for (const fileName of [...new Set(fileNames)]) {
      const fullPath = createStatusFilePath(fileName);

      await deviceStore.busyBar.StorageRemove({ path: fullPath }, { timeout: 0 })
        .catch(async error => {
          await handleHTTPError(error, `Couldn't delete ${fullPath}`, false, 0);
        });
    }

    await refreshStatusDirectory({ silent: true });
  }

  return {
    isRefreshingStatusDirectory,
    statusDirectoryFiles,
    statusGalleryFiles,
    syncStatusDirectory,
    refreshStatusDirectory,
    clearStatusDisplay,
    showSavedStatusOnBusyBar,
    downloadStatusFile,
    deleteStatusFiles
  };
});
