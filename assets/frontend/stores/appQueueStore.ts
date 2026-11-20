import { defineStore } from 'pinia';
import type { AppQueueAddResultItem, AppQueueItem, AppQueueOperation, AppQueueRequestItem, ProcessedUpdate } from '@busy-app/busy-lib';
import type { CatalogApplication, CatalogApplicationsResponse } from '@/util/catalog';
import { catalogAppId, catalogIconUri, catalogInstallOperation, catalogTitle, fetchCatalogApplications, isInstallable } from '@/util/catalog';

type QueueStateUpdate = NonNullable<ProcessedUpdate['appQueueState']>;
type QueueDetailUpdate = NonNullable<QueueStateUpdate['detail']>;
type QueueEventUpdate = NonNullable<ProcessedUpdate['appQueueEvent']>;
type QueueOperationResult = NonNullable<QueueEventUpdate['result']>;
export type AppQueueErrorName = NonNullable<QueueOperationResult['error']>;

/** How long a finished operation is kept for display, in ms. The device drops it from the queue immediately. */
const FINISHED_TTL_MS = 60_000;
/** How many finished operations are kept at most. */
const FINISHED_MAX_COUNT = 32;
/** Delay after an operation is added to the queue before the REST state is re-read. */
const REFRESH_AFTER_ADD_MS = 300;
/** Minimum interval between REST refreshes caused by unknown websocket items, in ms. */
const UNKNOWN_ITEM_REFRESH_INTERVAL_MS = 1000;
/** How long the installed app list is considered fresh after a refresh. */
const INSTALLED_APPS_REFRESH_DELAY_MS = 500;

export interface ActiveQueueItem {
  item_id: number;
  app_id: string;
  operation: AppQueueOperation;
  status: 'pending' | 'in_progress';
  detail?: { step?: string | null; progress?: number | null };
  addedAt: number;
}

export type QueueOutcome =
  | { kind: 'success' }
  | { kind: 'error'; error: AppQueueErrorName }
  | { kind: 'unknown' };

export interface FinishedQueueItem {
  item_id: number;
  app_id: string;
  operation: AppQueueOperation;
  outcome: QueueOutcome;
  finishedAt: number;
}

interface KnownQueueItem {
  app_id: string;
  operation: AppQueueOperation;
}

export const useAppQueueStore = defineStore('appQueue', () => {
  const deviceStore = useDeviceStore();
  const appsStore = useAppsStore();

  /** Queue as reported by the device REST API, with websocket detail updates applied. */
  const activeItems = ref<ActiveQueueItem[]>([]);
  /** Operations finished during this session: the device removes them from the queue and only the websocket reports them. */
  const finishedItems = ref<FinishedQueueItem[]>([]);
  /** Last known app id and operation per queue item id, so a result can be attributed even before the queue is re-read. */
  const knownItems = ref<Record<number, KnownQueueItem>>({});
  const queueLoading = ref(false);
  const queueLoaded = ref(false);
  const queueError = ref<string>();
  const lastFinished = ref<{ entry: FinishedQueueItem; serial: number } | null>(null);
  let finishedSerial = 0;

  const catalogApps = ref<CatalogApplication[]>([]);
  const catalogTotal = ref(0);
  const catalogLoading = ref(false);
  const catalogLoaded = ref(false);
  const catalogError = ref<string>();

  let refreshTimer: ReturnType<typeof setTimeout> | undefined;
  let refreshPromise: Promise<void> | undefined;
  let lastUnknownItemRefresh = 0;
  let syncInterval: ReturnType<typeof setInterval> | undefined;
  let installedAppsTimer: ReturnType<typeof setTimeout> | undefined;

  const activeByApp = computed(() => {
    const map = new Map<string, ActiveQueueItem>();
    for (const item of activeItems.value) {
      if (!map.has(item.app_id)) {
        map.set(item.app_id, item);
      }
    }
    return map;
  });

  const finishedByApp = computed(() => {
    const map = new Map<string, FinishedQueueItem>();
    for (const item of finishedItems.value) {
      const previous = map.get(item.app_id);
      if (!previous || previous.finishedAt < item.finishedAt) {
        map.set(item.app_id, item);
      }
    }
    return map;
  });

  const catalogByApp = computed(() => {
    const map = new Map<string, CatalogApplication>();
    for (const app of catalogApps.value) {
      map.set(catalogAppId(app), app);
    }
    return map;
  });

  const hasActiveOperations = computed(() => activeItems.value.length > 0);

  function pruneFinished () {
    const now = Date.now();
    const kept = finishedItems.value.filter(item => now - item.finishedAt < FINISHED_TTL_MS).slice(-FINISHED_MAX_COUNT);
    if (kept.length !== finishedItems.value.length) {
      finishedItems.value = kept;
    }
  }

  function remember (itemId: number, appId: string, operation: AppQueueOperation) {
    knownItems.value = { ...knownItems.value, [itemId]: { app_id: appId, operation } };
  }

  function isFinished (itemId: number): boolean {
    return finishedItems.value.some(item => item.item_id === itemId);
  }

  function addActive (entry: Omit<ActiveQueueItem, 'addedAt'>) {
    if (isFinished(entry.item_id) || activeItems.value.some(item => item.item_id === entry.item_id)) {
      return;
    }

    activeItems.value = [...activeItems.value, { ...entry, addedAt: Date.now() }];
  }

  function addFinished (entry: Omit<FinishedQueueItem, 'finishedAt'>) {
    if (isFinished(entry.item_id)) {
      return;
    }

    activeItems.value = activeItems.value.filter(item => item.item_id !== entry.item_id);
    const finished: FinishedQueueItem = { ...entry, finishedAt: Date.now() };
    finishedItems.value = [...finishedItems.value, finished];
    finishedSerial += 1;
    lastFinished.value = { entry: finished, serial: finishedSerial };
    pruneFinished();
  }

  function scheduleRefresh (delay = REFRESH_AFTER_ADD_MS) {
    if (refreshTimer) {
      clearTimeout(refreshTimer);
    }

    refreshTimer = setTimeout(() => {
      refreshTimer = undefined;
      void refreshQueue({ silent: true });
    }, delay);
  }

  function scheduleInstalledAppsRefresh () {
    if (installedAppsTimer) {
      clearTimeout(installedAppsTimer);
    }

    installedAppsTimer = setTimeout(() => {
      installedAppsTimer = undefined;
      void appsStore.fetchApps();
    }, INSTALLED_APPS_REFRESH_DELAY_MS);
  }

  function mergeActiveItems (items: AppQueueItem[], requestedAt: number) {
    const next: ActiveQueueItem[] = [];
    const seen = new Set<number>();

    for (const item of items) {
      // A finished operation can still be reported by a REST response that was created before it finished.
      if (isFinished(item.item_id)) {
        continue;
      }

      seen.add(item.item_id);
      remember(item.item_id, item.app_id, item.operation);

      const existing = activeItems.value.find(entry => entry.item_id === item.item_id);
      next.push({
        item_id: item.item_id,
        app_id: item.app_id,
        operation: item.operation,
        status: item.status,
        detail: item.detail ?? existing?.detail,
        addedAt: existing?.addedAt ?? requestedAt
      });
    }

    // Items that are not in the response anymore finished while the state stream was not reporting them.
    for (const entry of activeItems.value) {
      if (seen.has(entry.item_id) || entry.addedAt >= requestedAt) {
        continue;
      }

      addFinished({
        item_id: entry.item_id,
        app_id: entry.app_id,
        operation: entry.operation,
        outcome: { kind: 'unknown' }
      });
      scheduleInstalledAppsRefresh();
    }

    activeItems.value = next;
  }

  async function refreshQueue ({ silent = false }: { silent?: boolean } = {}) {
    if (refreshPromise) {
      return refreshPromise;
    }

    if (!silent) {
      queueLoading.value = true;
    }

    const requestedAt = Date.now();
    refreshPromise = (async () => {
      try {
        const response = await deviceStore.busyBar.appsQueueGet();
        mergeActiveItems(response.result, requestedAt);
        queueError.value = undefined;
        queueLoaded.value = true;
      } catch (error) {
        if (!silent) {
          queueError.value = error instanceof Error ? error.message : 'Failed to load the app queue';
        }
      } finally {
        queueLoading.value = false;
        refreshPromise = undefined;
      }
    })();

    return refreshPromise;
  }

  /** Adds operations to the device queue and returns the per-operation enqueue results. */
  async function addOperations (operations: AppQueueRequestItem[]): Promise<AppQueueAddResultItem[]> {
    const response = await deviceStore.busyBar.appsQueueAdd({ operations });

    for (const item of response.result) {
      if (item.item_id === null || item.item_id === undefined) {
        continue;
      }

      remember(item.item_id, item.app_id, item.operation);
      addActive({
        item_id: item.item_id,
        app_id: item.app_id,
        operation: item.operation,
        status: 'pending'
      });
    }

    scheduleRefresh();
    return response.result;
  }

  async function cancelItem (itemId: number) {
    try {
      await deviceStore.busyBar.appsQueueCancel({ item_id: itemId });
    } catch (error) {
      // 404 means the operation already finished or was cancelled; let the refresh reconcile the list.
      if ((error as { status?: number }).status !== 404) {
        throw error;
      }
    }

    scheduleRefresh();
  }

  async function cancelAll () {
    await deviceStore.busyBar.appsQueueCancel({ all: true });
    scheduleRefresh();
  }

  /** Applies an operation detail update from the state stream. */
  function applyOperationDetail (detail: QueueDetailUpdate | null | undefined) {
    if (detail?.itemId === null || detail?.itemId === undefined || isFinished(detail.itemId)) {
      return;
    }

    const item = activeItems.value.find(entry => entry.item_id === detail.itemId);
    if (!item) {
      // An operation started elsewhere: re-read the queue to learn its app.
      const now = Date.now();
      if (now - lastUnknownItemRefresh > UNKNOWN_ITEM_REFRESH_INTERVAL_MS) {
        lastUnknownItemRefresh = now;
        scheduleRefresh();
      }
      return;
    }

    item.status = 'in_progress';
    item.detail = { step: detail.step ?? item.detail?.step, progress: detail.progress };
  }

  /** Applies an operation result from the state stream: the device has removed the operation from the queue. */
  function applyOperationResult (result: QueueOperationResult | null | undefined) {
    if (result?.itemId === null || result?.itemId === undefined) {
      return;
    }

    const outcome: QueueOutcome = result.error === 'ok' || result.error === null || result.error === undefined
      ? { kind: 'success' }
      : { kind: 'error', error: result.error };

    // A REST refresh can notice that the operation is gone before its result event arrives; upgrade that entry.
    const finished = finishedItems.value.find(item => item.item_id === result.itemId);
    if (finished) {
      if (finished.outcome.kind === 'unknown') {
        finished.outcome = outcome;
        finished.finishedAt = Date.now();
        finishedSerial += 1;
        lastFinished.value = { entry: finished, serial: finishedSerial };
      }
      return;
    }

    const item = activeItems.value.find(entry => entry.item_id === result.itemId);
    const known = knownItems.value[result.itemId];
    if (!item && !known) {
      scheduleRefresh();
      return;
    }

    addFinished({
      item_id: result.itemId,
      app_id: item?.app_id ?? known!.app_id,
      operation: item?.operation ?? known!.operation,
      outcome
    });
    scheduleRefresh();
    scheduleInstalledAppsRefresh();
  }

  /** Starts a periodic REST refresh used as a safety net for missed or coalesced websocket state. */
  function startSync () {
    if (syncInterval) {
      return;
    }

    syncInterval = setInterval(() => {
      void refreshQueue({ silent: true });
      pruneFinished();
    }, 2000);
  }

  function stopSync () {
    if (syncInterval) {
      clearInterval(syncInterval);
      syncInterval = undefined;
    }
  }

  async function fetchCatalog () {
    if (catalogLoading.value) {
      return;
    }

    catalogLoading.value = true;
    try {
      const response: CatalogApplicationsResponse = await fetchCatalogApplications();
      catalogApps.value = response.items.filter(isInstallable);
      catalogTotal.value = response.total;
      catalogError.value = undefined;
      catalogLoaded.value = true;
    } catch (error) {
      catalogError.value = error instanceof Error ? error.message : 'Failed to load the apps catalog';
    } finally {
      catalogLoading.value = false;
    }
  }

  function catalogAppFor (appId: string): CatalogApplication | undefined {
    return catalogByApp.value.get(appId);
  }

  function appOperation (app: CatalogApplication): AppQueueRequestItem['operation'] | null {
    return catalogInstallOperation(app);
  }

  /** Display title for an app in the queue: catalog name, then installed app name, then the app id itself. */
  function appTitle (appId: string): string {
    const fromCatalog = catalogTitle(catalogByApp.value.get(appId));
    if (fromCatalog) {
      return fromCatalog;
    }

    return appsStore.apps.find(app => app.id === appId)?.name ?? appId;
  }

  function hasTitle (appId: string): boolean {
    return appTitle(appId) !== appId;
  }

  /** Icon for an app in the queue: catalog icon, then the icon read from the device, or undefined for the placeholder. */
  function appIcon (appId: string): string | undefined {
    return catalogIconUri(catalogByApp.value.get(appId)) ?? appsStore.icons[appId];
  }

  return {
    activeItems,
    finishedItems,
    queueLoading,
    queueLoaded,
    queueError,
    lastFinished,
    activeByApp,
    finishedByApp,
    hasActiveOperations,

    catalogApps,
    catalogTotal,
    catalogLoading,
    catalogLoaded,
    catalogError,
    catalogByApp,

    refreshQueue,
    addOperations,
    cancelItem,
    cancelAll,
    applyOperationDetail,
    applyOperationResult,
    startSync,
    stopSync,
    fetchCatalog,
    pruneFinished,

    catalogAppFor,
    appOperation,
    appTitle,
    hasTitle,
    appIcon
  };
});
