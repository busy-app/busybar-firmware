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
/**
 * The REST queue is read only to initialize the state and to reconcile it when a websocket message describes something
 * this page did not know about. Its state is otherwise trusted, so no periodic polling is needed.
 * This is the delay used to coalesce a burst of such messages into one read.
 */
const UNKNOWN_ITEM_SYNC_DELAY_MS = 300;
/** Minimum interval between queue reads caused by unknown websocket items, in ms. */
const UNKNOWN_ITEM_SYNC_INTERVAL_MS = 1000;
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

  let unknownItemSyncTimer: ReturnType<typeof setTimeout> | undefined;
  let lastQueueSyncAt = 0;
  let refreshPromise: Promise<void> | undefined;
  let pruneTimer: ReturnType<typeof setTimeout> | undefined;
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

  function schedulePrune () {
    if (pruneTimer) {
      clearTimeout(pruneTimer);
      pruneTimer = undefined;
    }

    if (finishedItems.value.length === 0) {
      return;
    }

    // Wake up when the oldest finished operation expires; no other timer is needed.
    const oldest = finishedItems.value[0]!;
    const delay = Math.max(0, oldest.finishedAt + FINISHED_TTL_MS - Date.now()) + 100;
    pruneTimer = setTimeout(() => {
      pruneTimer = undefined;
      pruneFinished();
    }, delay);
  }

  function pruneFinished () {
    const now = Date.now();
    const kept = finishedItems.value.filter(item => now - item.finishedAt < FINISHED_TTL_MS).slice(-FINISHED_MAX_COUNT);
    if (kept.length !== finishedItems.value.length) {
      finishedItems.value = kept;
    }

    schedulePrune();
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

  /**
   * Reconciles the local queue with the REST state after a websocket message described an item this page did not know
   * about: the queue was changed externally, so the local copy can no longer be trusted. Bursts of messages are
   * coalesced into one read, and reads are limited to one per UNKNOWN_ITEM_SYNC_INTERVAL_MS.
   */
  function syncQueueForUnknownItem () {
    if (unknownItemSyncTimer) {
      // A read is already scheduled, it will observe the latest state.
      return;
    }

    const now = Date.now();
    const delay = Math.max(UNKNOWN_ITEM_SYNC_DELAY_MS, lastQueueSyncAt + UNKNOWN_ITEM_SYNC_INTERVAL_MS - now);
    unknownItemSyncTimer = setTimeout(() => {
      unknownItemSyncTimer = undefined;
      lastQueueSyncAt = Date.now();
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

    return response.result;
  }

  async function cancelItem (itemId: number) {
    try {
      await deviceStore.busyBar.appsQueueCancel({ item_id: itemId });
    } catch (error) {
      // 404 means the operation already finished or was cancelled: the local state is stale, reconcile it.
      if ((error as { status?: number }).status !== 404) {
        throw error;
      }

      syncQueueForUnknownItem();
    }
  }

  async function cancelAll () {
    await deviceStore.busyBar.appsQueueCancel({ all: true });
  }

  /** Applies an operation detail update from the state stream. */
  function applyOperationDetail (detail: QueueDetailUpdate | null | undefined) {
    if (detail?.itemId === null || detail?.itemId === undefined || isFinished(detail.itemId)) {
      return;
    }

    const item = activeItems.value.find(entry => entry.item_id === detail.itemId);
    if (!item) {
      // The queue changed externally: re-read it to learn the app of this operation.
      syncQueueForUnknownItem();
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
        schedulePrune();
      }
      return;
    }

    const item = activeItems.value.find(entry => entry.item_id === result.itemId);
    const known = knownItems.value[result.itemId];
    if (!item && !known) {
      // A result for an operation this page never saw: the queue changed externally.
      syncQueueForUnknownItem();
      return;
    }

    addFinished({
      item_id: result.itemId,
      app_id: item?.app_id ?? known!.app_id,
      operation: item?.operation ?? known!.operation,
      outcome
    });
    scheduleInstalledAppsRefresh();
  }

  /**
   * Reads the queue once when the page has no queue state yet (page load or first open of the tab). Later changes are
   * taken from the state stream and the REST state is only re-read to reconcile unexpected websocket messages.
   */
  async function initializeQueue () {
    if (queueLoaded.value) {
      return;
    }

    await refreshQueue();
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

    initializeQueue,
    refreshQueue,
    addOperations,
    cancelItem,
    cancelAll,
    applyOperationDetail,
    applyOperationResult,
    fetchCatalog,

    catalogAppFor,
    appOperation,
    appTitle,
    hasTitle,
    appIcon
  };
});
