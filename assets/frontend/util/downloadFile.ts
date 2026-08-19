export function downloadFile (blob: Blob, fileName: string): void {
  const objectUrl = URL.createObjectURL(blob);
  const link = document.createElement('a');

  link.href = objectUrl;
  link.download = fileName;
  link.click();

  // Revoking synchronously can abort the download in Safari, as the click only queues it
  setTimeout(() => URL.revokeObjectURL(objectUrl), 0);
}
