/** Human-readable labels for the app queue protobuf errors (BSB_State.AppQueueError). */
const APP_QUEUE_ERROR_LABELS: Record<string, string> = {
  download: 'Download failed',
  unpack: 'Failed to unpack the package',
  manifest: 'Invalid or missing app manifest',
  api_version: 'Incompatible API version',
  storage: 'Storage error',
  timeout: 'Operation timed out',
  not_found: 'App not found',
  queue_full: 'Queue is full',
  cancelled: 'Cancelled',
  app_running: 'App is running',
  parameter: 'Invalid parameter',
  internal: 'Internal error'
};

/** Readable text for a non-successful queue operation result. */
export function appQueueErrorLabel (error: string | null | undefined): string {
  if (!error) {
    return 'Operation failed';
  }

  return APP_QUEUE_ERROR_LABELS[error] ?? error;
}

/** "unpacking" -> "Unpacking"; empty for a missing step. */
export function appQueueStepLabel (step: string | null | undefined): string {
  if (!step) {
    return '';
  }

  return step.charAt(0).toUpperCase() + step.slice(1);
}

/**
 * Progress of an operation in percent, or undefined when the device does not know the total.
 * Only the downloading step reports progress; other steps run with an indeterminate bar.
 */
export function appQueueProgress (step: string | null | undefined, progress: number | null | undefined): number | undefined {
  if (step !== 'downloading' || progress === null || progress === undefined) {
    return undefined;
  }

  return Math.min(100, Math.max(0, progress));
}

/** "1 item" / "2 items". */
export function formatItemCount (count: number): string {
  return `${count} ${count === 1 ? 'item' : 'items'}`;
}
