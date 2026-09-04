const MOCK_MIN_DURATION = 2000;
const MOCK_MAX_DURATION = 8000;
const MOCK_BYTES_PER_SECOND = 512 * 1024;
const MOCK_TICK = 100;

export function uploadAppPackage (file: File, onProgress: (percent: number) => void, signal: AbortSignal): Promise<void> {
  return mockUploadAppPackage(file, onProgress, signal);
}

function mockUploadAppPackage (file: File, onProgress: (percent: number) => void, signal: AbortSignal): Promise<void> {
  return new Promise((resolve, reject) => {
    if (signal.aborted) {
      reject(signal.reason);
      return;
    }

    const duration = Math.min(MOCK_MAX_DURATION, Math.max(MOCK_MIN_DURATION, (file.size / MOCK_BYTES_PER_SECOND) * 1000));
    const startedAt = Date.now();

    const timer = setInterval(() => {
      const percent = Math.min(100, Math.round(((Date.now() - startedAt) / duration) * 100));
      onProgress(percent);

      if (percent === 100) {
        stop();
        resolve();
      }
    }, MOCK_TICK);

    function stop () {
      clearInterval(timer);
      signal.removeEventListener('abort', onAbort);
    }

    function onAbort () {
      stop();
      reject(signal.reason);
    }

    signal.addEventListener('abort', onAbort);
  });
}
