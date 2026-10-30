export interface DroppedFile {
  file: File;
  relativePath: string;
}

const IGNORED_FILE_NAMES = ['.DS_Store'];

function readFileEntry (entry: FileSystemFileEntry): Promise<File> {
  return new Promise((resolve, reject) => entry.file(resolve, reject));
}

function readDirectoryBatch (reader: FileSystemDirectoryReader): Promise<FileSystemEntry[]> {
  return new Promise((resolve, reject) => reader.readEntries(resolve, reject));
}

async function readDirectoryEntries (entry: FileSystemDirectoryEntry): Promise<FileSystemEntry[]> {
  const reader = entry.createReader();
  const entries: FileSystemEntry[] = [];

  let batch = await readDirectoryBatch(reader);
  while (batch.length > 0) {
    entries.push(...batch);
    batch = await readDirectoryBatch(reader);
  }

  return entries;
}

async function collectEntryFiles (entry: FileSystemEntry, parentPath: string): Promise<DroppedFile[]> {
  const relativePath = parentPath ? `${parentPath}/${entry.name}` : entry.name;

  if (entry.isFile) {
    if (IGNORED_FILE_NAMES.includes(entry.name)) {
      return [];
    }

    return [{ file: await readFileEntry(entry as FileSystemFileEntry), relativePath }];
  }

  const children = await readDirectoryEntries(entry as FileSystemDirectoryEntry);
  const nested = await Promise.all(children.map(child => collectEntryFiles(child, relativePath)));
  return nested.flat();
}

export async function getDroppedFiles (dataTransfer: DataTransfer): Promise<DroppedFile[]> {
  const files = Array.from(dataTransfer.files);
  const entries = Array.from(dataTransfer.items)
    .filter(item => item.kind === 'file')
    .map(item => item.webkitGetAsEntry?.() ?? null);

  if (entries.length === 0 || entries.some(entry => entry === null)) {
    return files.map(file => ({ file, relativePath: file.name }));
  }

  const nested = await Promise.all(entries.map(entry => collectEntryFiles(entry!, '')));
  return nested.flat();
}
