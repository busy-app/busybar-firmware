import { isBrowser } from './isBrowser';
import type { ClientMessage, WorkerMessage } from './stateStreamWorkerTypes';

type StatusCallback = (isConnected: boolean, isReconnected: boolean) => void;
type DataCallback = (data: ArrayBuffer) => void;
type ErrorCallback = (message: string) => void;
type ConnectionCheckCallback = () => Promise<void>;

export type StateStreamSubscription = {
  onStatus?: StatusCallback;
  onData?: DataCallback;
  onError?: ErrorCallback;
  onCheckConnection?: ConnectionCheckCallback;
};

type InternalSubscription = StateStreamSubscription & {
  isReady: boolean;
  resolveReady?: () => void;
  rejectReady?: (reason?: unknown) => void;
};

class StateStreamWebSocketClient {
  private worker: SharedWorker | Worker | null = null;
  private port: MessagePort | Worker | null = null;
  private subscriptions = new Set<InternalSubscription>();
  private activityCheckPauseCount = 0;
  private lastStatus = {
    isConnected: false,
    isReconnected: false
  };

  constructor () {
    if (isBrowser()) {
      try {
        if (typeof SharedWorker !== 'undefined') {
          const shared = new SharedWorker(new URL('./stateStreamWorker.ts', import.meta.url), { type: 'module' });
          this.worker = shared;
          this.port = shared.port;
          shared.port.start();
        } else {
          const dedicated = new Worker(new URL('./stateStreamWorker.ts', import.meta.url), { type: 'module' });
          this.worker = dedicated;
          this.port = dedicated;
        }
      } catch (error) {
        console.error('Failed to initialize state stream worker', error);
      }
    }

    if (!this.port) {
      return;
    }

    this.port.onmessage = (event: MessageEvent<ClientMessage>) => {
      this.handleMessage(event.data);
    };

    window.addEventListener('beforeunload', () => {
      if (this.subscriptions.size === 0) {
        return;
      }

      this.port?.postMessage({ type: 'UNSUBSCRIBE' } satisfies WorkerMessage);
    });

    window.addEventListener('device-disconnected', () => {
      this.port?.postMessage({ type: 'DEVICE_DISCONNECTED' } satisfies WorkerMessage);
    });

    window.addEventListener('device-reconnected', () => {
      this.port?.postMessage({ type: 'DEVICE_RECONNECTED' } satisfies WorkerMessage);
    });
  }

  private handleMessage (message: ClientMessage) {
    if (message.type === 'STATUS') {
      this.lastStatus = {
        isConnected: message.isConnected,
        isReconnected: message.isReconnected
      };

      this.subscriptions.forEach(subscription => {
        subscription.onStatus?.(message.isConnected, message.isReconnected);
        if (message.isConnected && !subscription.isReady) {
          subscription.isReady = true;
          subscription.resolveReady?.();
          delete subscription.resolveReady;
          delete subscription.rejectReady;
        }
      });
      return;
    }

    if (message.type === 'DATA') {
      this.subscriptions.forEach(subscription => {
        subscription.onData?.(message.data);
      });
      return;
    }

    if (message.type === 'CHECK_CONNECTION') {
      this.handleConnectionCheck();
      return;
    }

    this.subscriptions.forEach(subscription => {
      subscription.onError?.(message.message);

      if (!subscription.isReady) {
        subscription.rejectReady?.(new Error(message.message));
        delete subscription.resolveReady;
        delete subscription.rejectReady;
      }
    });
  }

  private handleConnectionCheck () {
    if (this.activityCheckPauseCount > 0) {
      console.debug('Ignoring queued state stream connection check while paused');
      return;
    }

    let checker: ConnectionCheckCallback | undefined;
    for (const subscription of this.subscriptions) {
      if (subscription.onCheckConnection) {
        checker = subscription.onCheckConnection;
        break;
      }
    }

    if (!checker) {
      return;
    }

    checker().catch(error => {
      console.error('State stream connection check failed', error);
    });
  }

  public async connect (
    wsEndpoint: string,
    apiToken?: string | null,
    cbs: StateStreamSubscription = {}
  ): Promise<StateStreamSubscription> {
    if (!this.port) {
      throw new Error('State stream worker is unavailable.');
    }

    const subscription: InternalSubscription = {
      ...cbs,
      isReady: false
    };

    const shouldSubscribeWorker = this.subscriptions.size === 0;
    this.subscriptions.add(subscription);

    const readyPromise = new Promise<void>((resolve, reject) => {
      subscription.resolveReady = resolve;
      subscription.rejectReady = reject;
    });

    if (this.lastStatus.isConnected) {
      subscription.isReady = true;
      subscription.resolveReady?.();
      delete subscription.resolveReady;
      delete subscription.rejectReady;
    } else if (shouldSubscribeWorker) {
      this.lastStatus = {
        isConnected: false,
        isReconnected: false
      };

      this.port.postMessage({
        type: 'SUBSCRIBE',
        wsEndpoint,
        apiToken
      } satisfies WorkerMessage);
    }

    try {
      await readyPromise;
      return subscription;
    } catch (error) {
      this.disconnect(subscription);
      throw error;
    }
  }

  public disconnect (subscription: StateStreamSubscription) {
    const removed = this.subscriptions.delete(subscription as InternalSubscription);
    if (!removed) {
      return;
    }

    const pendingSubscription = subscription as InternalSubscription;
    if (!pendingSubscription.isReady) {
      pendingSubscription.rejectReady?.(new Error('State stream subscription was cancelled.'));
    }

    if (this.subscriptions.size > 0) {
      return;
    }

    this.lastStatus = {
      isConnected: false,
      isReconnected: false
    };

    this.port?.postMessage({ type: 'UNSUBSCRIBE' } satisfies WorkerMessage);
  }

  public pauseActivityChecks () {
    this.activityCheckPauseCount++;
    console.debug('Pausing state stream activity checks');
    this.port?.postMessage({ type: 'PAUSE_ACTIVITY_CHECKS' } satisfies WorkerMessage);
  }

  public resumeActivityChecks () {
    if (this.activityCheckPauseCount === 0) {
      return;
    }

    this.activityCheckPauseCount--;
    console.debug('Resuming state stream activity checks');
    this.port?.postMessage({ type: 'RESUME_ACTIVITY_CHECKS' } satisfies WorkerMessage);
  }
}

export const stateStreamWebSocketClient = new StateStreamWebSocketClient();
